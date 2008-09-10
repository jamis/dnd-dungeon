#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>

#include "jbmaze.h"
#include "gd.h"


typedef struct {
  int  width;
  int  height;
  int  depth;
  int  sparseness;
  int  deadends;
  int  randomness;
  int  startx;
  int  starty;
  int  startz;
  int  endx;
  int  endy;
  int  endz;
  long seed;
  int  ofs;
  int  wallWid;
  int  pathWid;
  long bgColor;
  long wallClr;
  long pathClr;
  long boundClr;
  long startClr;
  long endClr;
  int  showSolution;
  int  showMarkers;
  char maskFile[256];
} PARMOPTS;


int verticalBoundary( JBMazeMask* mask, int x, int y ) {
  int one;
  int two;

  if( ( y <= 0 ) || ( y >= mask->getHeight() ) ) {
    return 1;
  }

  if( x >= mask->getWidth() ) {
    return 0;
  }

  one = mask->getMaskAt( x, y );
  two = mask->getMaskAt( x, y-1 );

  return ( one != two );
}

int horizontalBoundary( JBMazeMask* mask, int x, int y ) {
  int one;
  int two;

  if( ( x <= 0 ) || ( x >= mask->getWidth() ) ) {
    return 1;
  }

  if( y >= mask->getHeight() ) {
    return 0;
  }

  one = mask->getMaskAt( x, y );
  two = mask->getMaskAt( x-1, y );

  return ( one != two );
}


#define RED(x) ( x >> 16 )
#define GREEN(x) ( ( x & 0x00FF00 ) >> 8 )
#define BLUE(x) ( x & 0x0000FF )

int drawAsStructure( gdImagePtr* image, JBMaze* maze, JBMazePt* path, int len, PARMOPTS* opts ) {
  int wallClr;
  int bgColor;
  int boundClr;
  int i;
  int j;
  int ofs;
  int inc;
  int dir;
  int wallWid;
  int xSize;
  int ySize;
  int gridSize;

  JBMazeMask* mask;

  ofs = opts->ofs;
  inc = opts->pathWid;
  wallWid = opts->wallWid;

  gridSize = inc + wallWid + 1;
  mask = maze->getMask();

  xSize = ofs*2 + gridSize * maze->getX() + wallWid+2;
  ySize = ofs*2 + gridSize * maze->getY() + wallWid+2;

  *image = gdImageCreate( xSize, ySize );

  bgColor = gdImageColorAllocate( *image, RED(opts->bgColor), GREEN(opts->bgColor), BLUE(opts->bgColor) );
  wallClr = gdImageColorAllocate( *image, RED(opts->wallClr), GREEN(opts->wallClr), BLUE(opts->wallClr) );
  boundClr = gdImageColorAllocate( *image, RED(opts->boundClr), GREEN(opts->boundClr), BLUE(opts->boundClr) );

  gdImageFilledRectangle( *image, 0, 0, xSize - 1, ySize - 1, bgColor );

  ofs++;
  for( j = 0; j < maze->getY(); j++ ) {
    for( i = 0; i < maze->getX(); i++ ) {
      dir = maze->getExitsAt( i, j, 0 );
      if( dir == 0 ) {
        gdImageFilledRectangle( *image,
              ofs + i * gridSize, ofs + j * gridSize,
              ofs + (i+1) * gridSize, ofs + (j+1) * gridSize,
              wallClr );
      } else {
        if( ( dir & JBMaze::c_NORTH ) == 0  ) {
          gdImageFilledRectangle( *image,
              ofs + i * gridSize, ofs + j * gridSize,
              ofs + (i+1) * gridSize + wallWid, ofs + j * gridSize + wallWid,
              wallClr );
        }
        if( ( dir & JBMaze::c_WEST ) == 0 ) {
          gdImageFilledRectangle( *image,
              ofs + i * gridSize, ofs + j * gridSize,
              ofs + i * gridSize + wallWid, ofs + (j+1) * gridSize + wallWid,
              wallClr );
        }
      }
    }
  }

  /* right wall */
  gdImageFilledRectangle( *image,
                          ofs + maze->getX() * gridSize,
                          ofs,
                          ofs + maze->getX() * gridSize + wallWid,
                          ofs + maze->getY() * gridSize + wallWid,
                          wallClr );

  /* bottom wall */
  gdImageFilledRectangle( *image,
                          ofs,
                          ofs + maze->getY() * gridSize,
                          ofs + maze->getX() * gridSize + wallWid,
                          ofs + maze->getY() * gridSize + wallWid,
                          wallClr );


  /* trace the mask */

  for( j = 0; j < maze->getY(); j++ ) {
    for( i = 0; i < maze->getX(); i++ ) {
      if( verticalBoundary( mask, i, j ) ) {
        gdImageLine( *image, 
                     ofs + i * gridSize, 
                     ofs + j * gridSize,
                     ofs + (i+1) * gridSize,
                     ofs + j * gridSize,
                     boundClr );                  
      }
      if( horizontalBoundary( mask, i, j ) ) {
        gdImageLine( *image, 
                     ofs + i * gridSize, 
                     ofs + j * gridSize,
                     ofs + i * gridSize,
                     ofs + (j+1) * gridSize,
                     boundClr );                  
      }
    }
  }

  gdImageLine( *image, ofs, 
                       ofs + maze->getY() * gridSize + 1,
                       ofs + maze->getX() * gridSize + 1, 
                       ofs + maze->getY() * gridSize + 1,
                       boundClr );

  gdImageLine( *image, ofs + maze->getX() * gridSize + 1,
                       ofs,
                       ofs + maze->getX() * gridSize + 1, 
                       ofs + maze->getY() * gridSize + 1,
                       boundClr );

  /* display the beginning and ending markers */

  if( opts->showMarkers ) {
    int startClr;
    int endClr;

    startClr = gdImageColorAllocate( *image, RED(opts->startClr), GREEN(opts->startClr), BLUE(opts->startClr) );
    endClr   = gdImageColorAllocate( *image, RED(opts->endClr), GREEN(opts->endClr), BLUE(opts->endClr) );

    gdImageFilledRectangle( *image,
                            ofs + maze->getStart().x * gridSize + wallWid + 1,
                            ofs + maze->getStart().y * gridSize + wallWid + 1,
                            ofs + (maze->getStart().x+1) * gridSize - wallWid,
                            ofs + (maze->getStart().y+1) * gridSize - wallWid,
                            startClr );
    gdImageFilledRectangle( *image,
                            ofs + maze->getEnd().x * gridSize + wallWid + 1,
                            ofs + maze->getEnd().y * gridSize + wallWid + 1,
                            ofs + (maze->getEnd().x+1) * gridSize - wallWid,
                            ofs + (maze->getEnd().y+1) * gridSize - wallWid,
                            endClr );
  }

  /* display the solution */

  if( opts->showSolution ) {
    int pathClr;

    pathClr = gdImageColorAllocate( *image, RED(opts->pathClr), GREEN(opts->pathClr), BLUE(opts->pathClr) );

    for( j = 0; j < len-1; j++ ) {
      if( path[j].z == 0 ) {
        int x1 = ofs + path[j].x * gridSize + wallWid + inc/2+1;
        int y1 = ofs + path[j].y * gridSize + wallWid + inc/2+1;
        int x2 = ofs + path[j+1].x * gridSize + wallWid + inc/2+1;
        int y2 = ofs + path[j+1].y * gridSize + wallWid + inc/2+1;

        gdImageLine( *image,
                     x1, y1, x2, y2,
                     pathClr );
      }
    }
  }

  return 0;
}


char* trimleft( char* s ) {
  char* i;

  i = s + strlen( s ) - 1;
  while( i >= i && isspace( *i ) ) {
    *i = 0;
    i--;
  }

  return s;
}


char* getParm( char* line, char** parm ) {
  char* p;
  char* s;

  p = line;
  while( isspace( *p ) ) p++;
  if( ( *p == 0 ) || ( *p == '#' ) ) {
    return 0;
  }

  s = strchr( p, '=' );
  if( s == 0 ) {
    return 0;
  }

  *s = 0;
  s++;

  trimleft( s );
  *parm = s;

  return p;
}


long makeColor( char* line ) {
  char* s;
  long  clr;

  s = line;
  clr = atoi( s ) << 16;

  while(*s && *s != ',') s++;
  if(*s == 0) return clr;
  s++;

  clr |= atoi( s ) << 8;

  while(*s && *s != ',') s++;
  if(*s == 0) return clr;
  s++;

  clr |= atoi( s );
  return clr;
}


int readParameters( char* file, PARMOPTS* opts ) {
  FILE* f;
  char  line[100];
  char* parm;
  char* value;

  f = fopen( file, "rt" );
  if( !f ) {
    return 0;
  }

  opts->seed = time(NULL);

  while( fgets( line, sizeof( line ), f ) != NULL ) {
    parm = getParm( line, &value );

    if( parm == 0 ) {
      continue;
    }

    if( strcmp( parm, "x" ) == 0 ) {
      opts->width = atoi( value );
    } else if( strcmp( parm, "y" ) == 0 ) {
      opts->height = atoi( value );
    } else if( strcmp( parm, "z" ) == 0 ) {
      opts->depth = atoi( value );
    } else if( strcmp( parm, "sparse" ) == 0 ) {
      opts->sparseness = atoi( value );
    } else if( strcmp( parm, "deadends" ) == 0 ) {
      opts->deadends = atoi( value );
    } else if( strcmp( parm, "randomness" ) == 0 ) {
      opts->randomness = atoi( value );
    } else if( strcmp( parm, "startx" ) == 0 ) {
      opts->startx = atoi( value );
    } else if( strcmp( parm, "starty" ) == 0 ) {
      opts->starty = atoi( value );
    } else if( strcmp( parm, "startz" ) == 0 ) {
      opts->startz = atoi( value );
    } else if( strcmp( parm, "endx" ) == 0 ) {
      opts->endx = atoi( value );
    } else if( strcmp( parm, "endy" ) == 0 ) {
      opts->endy = atoi( value );
    } else if( strcmp( parm, "endz" ) == 0 ) {
      opts->endz = atoi( value );
    } else if( strcmp( parm, "mask" ) == 0 ) {
      strcpy( opts->maskFile, value );
    } else if( strcmp( parm, "seed" ) == 0 ) {
      opts->seed = atol( value );
    } else if( strcmp( parm, "border" ) == 0 ) {
      opts->ofs = atoi( value );
    } else if( strcmp( parm, "wall" ) == 0 ) {
      opts->wallWid = atoi( value );
    } else if( strcmp( parm, "path" ) == 0 ) {
      opts->pathWid = atoi( value );
    } else if( strcmp( parm, "bgclr" ) == 0 ) {
      opts->bgColor = makeColor( value );
    } else if( strcmp( parm, "wallclr" ) == 0 ) {
      opts->wallClr = makeColor( value );
    } else if( strcmp( parm, "pathclr" ) == 0 ) {
      opts->pathClr = makeColor( value );
    } else if( strcmp( parm, "boundclr" ) == 0 ) {
      opts->boundClr = makeColor( value );
    } else if( strcmp( parm, "startclr" ) == 0 ) {
      opts->startClr = makeColor( value );
    } else if( strcmp( parm, "endclr" ) == 0 ) {
      opts->endClr = makeColor( value );
    } else if( strcmp( parm, "solution" ) == 0 ) {
      opts->showSolution = ( atoi( value ) != 0 );
    } else if( strcmp( parm, "markers" ) == 0 ) {
      opts->showMarkers = ( atoi( value ) != 0 );
    } else if( strcmp( parm, "include" ) == 0 ) {
      readParameters( value, opts );
    }
  }

  return 1;
}

void printHelp(void) {
  fprintf(stderr,
    "usage: maze <opts>\n"
    "\n"
    "options:\n"
    "  -H       : this help\n"
    "  -w n     : set maze width to n\n"
    "  -h n     : set maze height to n\n"
    "  -d n     : set maze depth to n\n"
    "  -s n     : set maze sparseness to n\n"
    "  -e n     : set maze deadend percentage to n\n"
    "  -r n     : set maze randomness percentage to n\n"
    "  -x n     : set maze starting x coordinate to n\n"
    "  -y n     : set maze starting y coordinate to n\n"
    "  -z n     : set maze starting z coordinate to n\n"
    "  -X n     : set maze ending x coordinate to n\n"
    "  -Y n     : set maze ending y coordinate to n\n"
    "  -Z n     : set maze ending z coordinate to n\n"
    "  -m file  : use file to define the maze mask\n"
    "  -S n     : use n as the random seed for the maze\n"
    "  -b n     : set the outer margin to n pixels\n"
    "  -W n     : set the wall width to n pixels\n"
    "  -p n     : set the path width to n pixels\n"
    "  -B r,g,b : set background color to r,g,b\n"
    "  -A r,g,b : set wall color to r,g,b\n"
    "  -P r,g,b : set path color to r,g,b\n"
    "  -O r,g,b : set boundary color to r,g,b\n"
    "  -T r,g,b : set start position color to r,g,b\n"
    "  -E r,g,b : set end position color to r,g,b\n"
    "  -L n     : set n to non-zero to show maze solution\n"
    "  -M n     : set n to non-zero to show start/end positions\n"
    "  -f file  : read configuration options from file\n"
  );

  exit(-1);
}

int parseArgs(int argc, char* argv[], PARMOPTS* opts) {
  int i;

  opts->width = 10;
  opts->height = 10;
  opts->depth = 1;
  opts->randomness = 50;
  opts->wallWid = 1;
  opts->pathWid = 9;
  opts->bgColor = makeColor("255,255,255");
  opts->wallClr = makeColor("0,0,0");
  opts->pathClr = makeColor("255,0,0");
  opts->startClr = makeColor("0,128,0");
  opts->endClr = makeColor("255,0,0");
  opts->endx = opts->endy = opts->endz = -1;

  for(i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-H") == 0) printHelp();

    if(argv[i][0] != '-') {
      fprintf(stderr, "bad argument: %s\n\n", argv[i]);
      printHelp();
    }

    if(i == argc) {
      fprintf(stderr, "argument is missing a value: %s\n\n", argv[i]);
      printHelp();
    }

    switch(argv[i][1]) {
      case 'w': opts->width = atoi(argv[++i]); break;
      case 'h': opts->height = atoi(argv[++i]); break;
      case 'd': opts->depth = atoi(argv[++i]); break;
      case 's': opts->sparseness = atoi(argv[++i]); break;
      case 'e': opts->deadends = atoi(argv[++i]); break;
      case 'r': opts->randomness = atoi(argv[++i]); break;
      case 'x': opts->startx = atoi(argv[++i]); break;
      case 'y': opts->starty = atoi(argv[++i]); break;
      case 'z': opts->startz = atoi(argv[++i]); break;
      case 'X': opts->endx = atoi(argv[++i]); break;
      case 'Y': opts->endy = atoi(argv[++i]); break;
      case 'Z': opts->endz = atoi(argv[++i]); break;
      case 'm': strcpy( opts->maskFile, argv[++i] ); break;
      case 'S': opts->seed = atol(argv[++i]); break;
      case 'b': opts->ofs = atoi(argv[++i]); break;
      case 'W': opts->wallWid = atoi(argv[++i]); break;
      case 'p': opts->pathWid = atoi(argv[++i]); break;
      case 'B': opts->bgColor = makeColor(argv[++i]); break;
      case 'A': opts->wallClr = makeColor(argv[++i]); break;
      case 'P': opts->pathClr = makeColor(argv[++i]); break;
      case 'O': opts->boundClr = makeColor(argv[++i]); break;
      case 'T': opts->startClr = makeColor(argv[++i]); break;
      case 'E': opts->endClr = makeColor(argv[++i]); break;
      case 'L': opts->showSolution = atoi(argv[++i]); break;
      case 'M': opts->showMarkers = atoi(argv[++i]); break;
      case 'f': readParameters( argv[++i], opts ); break;
      default:
        fprintf(stderr, "unsupported argument: %s\n\n", argv[i]);
        printHelp();
    }
  }

  if(opts->endx < 0) opts->endx = opts->width-1;
  if(opts->endy < 0) opts->endy = opts->height-1;
  if(opts->endz < 0) opts->endz = opts->depth-1;

  return 1;
}

int main( int argc, char* argv[] ) {
  JBMaze* maze;
  gdImagePtr image;
  JBMazePt* path;
  int len;
  PARMOPTS opts;

  memset( &opts, 0, sizeof( opts ) );
  parseArgs( argc, argv, &opts);

  fprintf( stderr, "current seed: %ld\n", opts.seed );

  /* construct the maze */   
  
  maze = new JBMaze( opts.width, opts.height, opts.depth, opts.seed, opts.randomness,
                     opts.startx, opts.starty, opts.startz, opts.endx, opts.endy, opts.endz );

  /* load the mask */

  if( opts.maskFile[0] != 0 ) {
    maze->setMask( new JBMazeMask( opts.maskFile ) );
  }

  /* generate it */

  maze->generate();

  /* solve it */
  
  maze->solve( &path, &len );

  /* sparsify it */

  maze->sparsify( opts.sparseness );

  /* clear up the deadends by reconnecting them into an existing passage */

  maze->clearDeadends( opts.deadends );

  /* draw it */

  drawAsStructure( &image, maze, path, len, &opts );

  free( path );

  gdImagePng( image, stdout );

  gdImageDestroy( image );
  delete maze;

  return 0;
}
