/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBMaze
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 * ---------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "jbmaze.h"

const int JBMaze::c_NORTH = 0x0001;
const int JBMaze::c_SOUTH = 0x0002;
const int JBMaze::c_WEST  = 0x0004;
const int JBMaze::c_EAST  = 0x0008;
const int JBMaze::c_UP    = 0x0010;
const int JBMaze::c_DOWN  = 0x0020;

const int JBMaze::c_MARK  = 0x8000;


JBMaze::JBMaze( int x, int y, int z, long seed, int randomness,
                int sx, int sy, int sz,
                int ex, int ey, int ez ) 
{
  int i;
  int j;

  m_deadendsClosed = 1;

  m_maze = 0;
  m_x = m_y = m_z = 0;
  m_seed = 0;
  m_randomness = 0;

  if( ( x < 1 ) || ( y < 1 ) || ( z < 1 ) ) {
    return;
  }

  m_x = x;
  m_y = y;
  m_z = z;

  m_mask = new JBMazeMask( m_x, m_y );

  m_start.x = sx;
  m_start.y = sy;
  m_start.z = sz;

  m_end.x = ( ex < 0 ? m_x - 1 : ex );
  m_end.y = ( ey < 0 ? m_y - 1 : ey );
  m_end.z = ( ez < 0 ? m_z - 1 : ez );

  if( seed > 0 ) {
    m_seed = seed;
    srand( m_seed );
  }

  m_randomness = randomness;

  m_maze = (int***)malloc( m_x * sizeof( int** ) );
  for( i = 0; i < x; i++ ) {
    m_maze[ i ] = (int**)malloc( m_y * sizeof( int* ) );
    for( j = 0; j < y; j++ ) {
      m_maze[ i ][ j ] = (int*)malloc( m_z * sizeof( int ) );
      memset( m_maze[ i ][ j ], 0, m_z * sizeof( int ) );
    }
  }
}


JBMaze::~JBMaze() {
  m_deallocateMaze();

  m_maze = 0;
  m_x = m_y = m_z = 0;
  m_seed = 0;

  delete m_mask;
}


int JBMaze::getExitsAt( int x, int y, int z ) {
  if( m_maze == 0 ) {
    return 0;
  }
  if( ( x < 0 ) || ( y < 0 ) || ( z < 0 ) ) {
    return 0;
  }
  if( ( x >= m_x ) || ( y >= m_y ) || ( z >= m_z ) ) {
    return 0;
  }
  return m_maze[ x ][ y ][ z ];
}


void JBMaze::solve( JBMazePt** path, 
                    int* pathLen ) 
{
  JBMaze::JBMAZE_SOLUTION* stack;
  JBMaze::JBMAZE_SOLUTION* iter;

  int dirs;
  int cx;
  int cy;
  int cz;

  *path = 0;
  *pathLen = 0;

  if( ( m_maze == 0 ) || ( !m_deadendsClosed ) ) {
    return;
  }

  /* put our starting position on the stack */

  cx = m_start.x;
  cy = m_start.y;
  cz = m_start.z;

  stack = m_push( cx, cy, cz, 0 );

  /* loop while we have not reached the end point */

  while( !( ( cx == m_end.x ) && ( cy == m_end.y ) && ( cz == m_end.z ) ) ) {
    if( stack == 0 ) {
      /* if we've run out of stack, then there is no solution to the maze.
       * we have to just exit. */
      return;
    }

    /* test all possible directions from the given point, recursively.
     * First, try north, then south, then east, etc, until all options
     * have been tried, and if none of them worked, pop the stack and try
     * with the previous position. */

    dirs = m_maze[ cx ][ cy ][ cz ] & ~( stack->directions );
    if( ( dirs & c_NORTH ) != 0 ) {
      stack->directions |= c_NORTH;
      if( cy > 0 ) {
        cy--;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_SOUTH;
      }
    } else if( ( dirs & c_SOUTH ) != 0 ) {
      stack->directions |= c_SOUTH;
      if( cy+1 < m_y ) {
        cy++;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_NORTH;
      }
    } else if( ( dirs & c_EAST ) != 0 ) {
      stack->directions |= c_EAST;
      if( cx+1 < m_x ) {
        cx++;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_WEST;
      }
    } else if( ( dirs & c_WEST ) != 0 ) {
      stack->directions |= c_WEST;
      if( cx > 0 ) {
        cx--;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_EAST;
      }
    } else if( ( dirs & c_UP ) != 0 ) {
      stack->directions |= c_UP;
      if( cz > 0 ) {
        cz--;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_DOWN;
      }
    } else if( ( dirs & c_DOWN ) != 0 ) {
      stack->directions |= c_DOWN;
      if( cz+1 < m_z ) {
        cz++;
        stack = m_push( cx, cy, cz, stack );
        stack->directions |= c_UP;
      }
    } else {
      stack = m_pop( stack );
      if( stack == 0 ) {
        /* if the stack is empty, there is no solution! */
        break;
      }
      cx = stack->x;
      cy = stack->y;
      cz = stack->z;
    }
  }

  /* determine the number of items in the solution */
  for( iter = stack, *pathLen = 0; iter != 0; (*pathLen)++, iter = iter->previous );

  /* allocate the solution array */
  *path = (JBMazePt*)malloc( *pathLen * sizeof( JBMazePt ) );

  /* work backwards from the ends, populating the solution array */
  for( cx = *pathLen-1; cx >= 0; cx-- ) {
    (*path)[cx].x = stack->x;
    (*path)[cx].y = stack->y;
    (*path)[cx].z = stack->z;
    stack = m_pop( stack );
  }
}


void JBMaze::sparsify( int amount ) {
  int i;
  int x;
  int y;
  int z;
  int dir;

  if( m_maze == 0 ) {
    return;
  }

  for( i = 0; i < amount; i++ ) {
    for( x = 0; x < m_x; x++ ) {
      for( y = 0; y < m_y; y++ ) {
        for( z = 0; z < m_z; z++ ) {

          /* don't sparsify from the beginning and end points -- this guarantees
           * that the solution to the maze (from solve()) will still be valid
           * after calling sparsify() */

          if( ( x == m_start.x ) && ( y == m_start.y ) && ( z == m_end.z ) ) {
            continue;
          }
          if( ( x == m_end.x ) && ( y == m_end.y ) && ( z == m_end.z ) ) {
            continue;
          }

          /* if the indicated position (x,y,z) is a deadend (ie, there is
           * only one direction out of it), then we "erase" the passage
           * here and mark it as visited. */

          dir = m_maze[ x ][ y ][ z ];
          switch( dir ) {
            case c_NORTH:
            case c_SOUTH:
            case c_WEST:
            case c_EAST:
            case c_UP:
            case c_DOWN:
              break;
            default:
              continue;
          }

          m_maze[ x ][ y ][ z ] = 0;
          if( ( dir & c_NORTH ) != 0 ) {
            m_maze[ x ][ y - 1 ][ z ] &= ~c_SOUTH;
            m_maze[ x ][ y - 1 ][ z ] |= c_MARK;
          } else if( ( dir & c_SOUTH ) != 0 ) {
            m_maze[ x ][ y + 1 ][ z ] &= ~c_NORTH;
            m_maze[ x ][ y + 1 ][ z ] |= c_MARK;
          } else if( ( dir & c_WEST ) != 0 ) {
            m_maze[ x - 1 ][ y ][ z ] &= ~c_EAST;
            m_maze[ x - 1 ][ y ][ z ] |= c_MARK;
          } else if( ( dir & c_EAST ) != 0 ) {
            m_maze[ x + 1 ][ y ][ z ] &= ~c_WEST;
            m_maze[ x + 1 ][ y ][ z ] |= c_MARK;
          } else if( ( dir & c_UP ) != 0 ) {
            m_maze[ x ][ y ][ z - 1 ] &= ~c_DOWN;
            m_maze[ x ][ y ][ z - 1 ] |= c_MARK;
          } else if( ( dir & c_DOWN ) != 0 ) {
            m_maze[ x ][ y ][ z + 1 ] &= ~c_UP;
            m_maze[ x ][ y ][ z + 1 ] |= c_MARK;
          }
        }
      }
    }

    /* clear the marks so we're ready to go for another pass! */
    m_clearMarks();
  }
}


void JBMaze::clearDeadends( int percentage ) {
  int x;
  int y;
  int z;
  int tx;
  int ty;
  int tz;
  int cx;
  int cy;
  int cz;
  int dir;
  int rdir = 0;
  int dirsTested;

  if( m_maze == 0 ) {
    return;
  }

  m_deadendsClosed = 0;

  for( x = 0; x < m_x; x++ ) {
    for( y = 0; y < m_y; y++ ) {
      for( z = 0; z < m_z; z++ ) {
        dir = m_maze[ x ][ y ][ z ];
        switch( dir ) {
          case c_NORTH:
          case c_SOUTH:
          case c_WEST:
          case c_EAST:
          case c_UP:
          case c_DOWN:
            break;
          default:
            continue;
        }

        /* do we close this deadend, or not? */

        if( rand() % 100 + 1 > percentage ) {
          continue;
        }

        /* if so, start at the dead end and randomly meander our way to
         * another point, to eliminate the dead end. */

        cx = x;
        cy = y;
        cz = z;

        do {
          dir = 0;
          dirsTested = 0;
          
          do {
            tx = cx;
            ty = cy;
            tz = cz;
            switch( rand() % 6 ) {
              case 0: if( cy > 0 ) { dir = c_NORTH; rdir = c_SOUTH; ty--; } 
                      else { dirsTested |= c_NORTH; } 
                      break;
              case 1: if( cy+1 < m_y ) { dir = c_SOUTH; rdir = c_NORTH; ty++; } 
                      else { dirsTested |= c_SOUTH; } 
                      break;
              case 2: if( cx > 0 ) { dir = c_WEST; rdir = c_EAST; tx--; } 
                      else { dirsTested |= c_WEST; } 
                      break;
              case 3: if( cx+1 < m_x ) { dir = c_EAST; rdir = c_WEST; tx++; } 
                      else { dirsTested |= c_EAST; } 
                      break;
              case 4: if( cz > 0 ) { dir = c_UP; rdir = c_DOWN; tz--; } 
                      else { dirsTested |= c_UP; } 
                      break;
              case 5: if( cz+1 < m_z ) { dir = c_DOWN; rdir = c_UP; tz++; } 
                      else { dirsTested |= c_DOWN; } 
                      break;
            }
            if( m_maze[ cx ][ cy ][ cz ] == dir ) {
              dirsTested |= dir;
              dir = 0;
            }
            if( !m_mask->getMaskAt( tx, ty ) ) {
              dirsTested |= dir;
              dir = 0;
            }
            if( dirsTested == ( c_NORTH | c_SOUTH | c_WEST | c_EAST | c_UP | c_DOWN ) ) {
              break;
            }
          } while( dir == 0 );

          if( dirsTested == ( c_NORTH | c_SOUTH | c_WEST | c_EAST | c_UP | c_DOWN ) ) {
            break;
          }

          m_maze[ cx ][ cy ][ cz ] |= dir;
          m_maze[ tx ][ ty ][ tz ] |= rdir;

          cx = tx;
          cy = ty;
          cz = tz;
        } while( m_maze[ tx ][ ty ][ tz ] == rdir );
      }
    }
  }
}


void JBMaze::generate() {
  unsigned long remaining;
  int x;
  int y;
  int z;
  int tx;
  int ty;
  int tz;
  int directions;
  int direction;
  int allDirections;
  int doRandomSelection;
  int lastDirection = 0;
  int straightStretch;

  if( m_maze == 0 ) {
    return;
  }

  allDirections = c_NORTH | c_SOUTH | c_WEST | c_EAST | c_UP | c_DOWN;
  straightStretch = 0;

  /* compute how many valid points there are in the maze */

  remaining = 0;
  for( x = 0; x < m_x; x++ ) {
    for( y = 0; y < m_y; y++ ) {
      remaining += m_mask->getMaskAt( x, y );
    }
  }
  remaining *= m_z;
  remaining--;

  /* find the point at which we want to start -- make sure the point we
   * pick is within the mask. */

  directions = 0;
  do {
    x = rand() % m_x;
    y = rand() % m_y;
    z = rand() % m_z;
  } while( !m_mask->getMaskAt( x, y ) );

  /* now, for each point remaining in the maze, we loop! */

  while( remaining > 0 ) {
    if( directions == allDirections ) {
      
      /* if we're stuck (boxed in or otherwise), choose another point, this
       * time choosing one that has already been visited. */

      do {
        x = rand() % m_x;
        y = rand() % m_y;
        z = rand() % m_z;
      } while( m_maze[ x ][ y ][ z ] == 0 );
      directions = m_maze[ x ][ y ][ z ];
    }

    /* eliminate obviously impossible directions */

    if( x < 1 ) directions |= c_WEST;
    if( x+1 >= m_x ) directions |= c_EAST;
    if( y < 1 ) directions |= c_NORTH;
    if( y+1 >= m_y ) directions |= c_SOUTH;
    if( z < 1 ) directions |= c_UP;
    if( z+1 >= m_z ) directions |= c_DOWN;

    doRandomSelection = 0;

    if( rand() % 100 < m_randomness ) {
      /* choose a direction at random */
      doRandomSelection = 1;
    } else {
      /* otherwise, move, based on the direction last chosen.  Note that we're
       * only allowing a straight stretch that is less than half as long as the
       * relevant dimension of the maze. */

      switch( lastDirection ) {
        case c_NORTH:
          if( ( straightStretch < ( m_y >> 1 ) ) && ( y > 0 ) && ( m_maze[ x ][ y-1 ][ z ] == 0 ) && ( m_mask->getMaskAt( x, y-1 ) ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        case c_SOUTH:
          if( ( straightStretch < ( m_y >> 1 ) ) && ( y+1 < m_y ) && ( m_maze[ x ][ y+1 ][ z ] == 0 ) && ( m_mask->getMaskAt( x, y+1 ) ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        case c_WEST:
          if( ( straightStretch < ( m_x >> 1 ) ) && ( x > 0 ) && ( m_maze[ x-1 ][ y ][ z ] == 0 ) && ( m_mask->getMaskAt( x-1, y ) ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        case c_EAST:
          if( ( straightStretch < ( m_x >> 1 ) ) && ( x+1 < m_x ) && ( m_maze[ x+1 ][ y ][ z ] == 0 ) && ( m_mask->getMaskAt( x+1, y ) ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        case c_UP:
          if( ( straightStretch < ( m_z >> 1 ) ) && ( z > 0 ) && ( m_maze[ x ][ y ][ z-1 ] == 0 ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        case c_DOWN:
          if( ( straightStretch < ( m_z >> 1 ) ) && ( z+1 < m_z ) && ( m_maze[ x ][ y ][ z+1 ] == 0 ) ) {
            direction = lastDirection;
          } else {
            doRandomSelection = 1;
          }
          break;
        default:
          doRandomSelection = 1;
      }
    }

    if( doRandomSelection ) {
      /* reset the straight stretch count */

      straightStretch = 0;
      direction = 0;

      /* pick a random direction */

      while( ( direction == 0 ) || ( ( directions & direction ) != 0 ) ) {
        tx = x;
        ty = y;
        tz = z;
        switch( rand() % 6 ) {
          case 0: if( y > 0 ) { direction = c_NORTH; ty--; } else { directions |= c_NORTH; } break;
          case 1: if( y+1 < m_y ) { direction = c_SOUTH; ty++; } else { directions |= c_SOUTH; }  break;
          case 2: if( x > 0 ) { direction = c_WEST; tx--; } else { directions |= c_WEST; }  break;
          case 3: if( x+1 < m_x ) { direction = c_EAST; tx++; } else { directions |= c_EAST; }  break;
          case 4: if( z > 0 ) { direction = c_UP; tz--; } else { directions |= c_UP; }  break;
          case 5: if( z+1 < m_z ) { direction = c_DOWN; tz++; } else { directions |= c_DOWN; }  break;
        }
        if( ( !m_mask->getMaskAt( tx, ty ) ) || ( m_maze[ tx ][ ty ][ tz ] != 0 ) ) {
          directions |= direction;
          if( directions == allDirections ) {
            break;
          }
          direction = 0;
        }
      }
    } else {
      straightStretch++;
    }

    if( directions == allDirections ) {
      /* if we've tested all directions, then we are stuck.  Continue to the
       * top of the loop, where we will select a new point to search from. */
      continue;
    }

    /* set the given direction in the maze, both at the point of origin and
     * the point of destination. */

    lastDirection = direction;
    m_maze[ x ][ y ][ z ] |= direction;
    switch( direction ) {
      case c_NORTH: y--; direction = c_SOUTH; break;
      case c_SOUTH: y++; direction = c_NORTH; break;
      case c_WEST: x--; direction = c_EAST; break;
      case c_EAST: x++; direction = c_WEST; break;
      case c_UP: z--; direction = c_DOWN; break;
      case c_DOWN: z++; direction = c_UP; break;
    }
    m_maze[ x ][ y ][ z ] |= direction;
    directions = m_maze[ x ][ y ][ z ];

    /* decrement the number of points remaining */

    remaining--;
  }
}


JBMaze::JBMAZE_SOLUTION* JBMaze::m_push( int x, int y, int z, JBMAZE_SOLUTION* stack ) {
  JBMaze::JBMAZE_SOLUTION* newItem;

  newItem = new JBMaze::JBMAZE_SOLUTION;
  newItem->x = x;
  newItem->y = y;
  newItem->z = z;
  newItem->directions = 0;
  newItem->previous = stack;

  return newItem;
}


JBMaze::JBMAZE_SOLUTION* JBMaze::m_pop( JBMAZE_SOLUTION* stack ) {
  JBMaze::JBMAZE_SOLUTION* next;

  next = stack->previous;
  delete stack;

  return next;
}


void JBMaze::m_clearMarks() {
  int i;
  int j;
  int k;

  if( m_maze == 0 ) {
    return;
  }

  for( i = 0; i < m_x; i++ ) {
    for( j = 0; j < m_y; j++ ) {
      for( k = 0; k < m_z; k++ ) {
        m_maze[ i ][ j ][ k ] &= ~c_MARK;
      }
    }
  }
}


void JBMaze::setMask( JBMazeMask* mask ) {
  delete m_mask;
  m_mask = mask;

  m_deallocateMaze();
  m_x = m_mask->getWidth();
  m_y = m_mask->getHeight();
  m_allocateMaze();
}


void JBMaze::m_deallocateMaze() {
  int i;
  int j;

  if( m_maze != 0 ) {
    for( i = 0; i < m_x; i++ ) {
      for( j = 0; j < m_y; j++ ) {
        delete[] m_maze[ i ][ j ];
      }
      delete[] m_maze[ i ];
    }
    delete[] m_maze;
  }

  m_maze = 0;
}


void JBMaze::m_allocateMaze() {
  int i;
  int j;

  if( m_maze != 0 ) {
    m_deallocateMaze();
  }

  m_maze = new int**[ m_x ];
  for( i = 0; i < m_x; i++ ) {
    m_maze[ i ] = new int*[ m_y ];
    for( j = 0; j < m_y; j++ ) {
      m_maze[ i ][ j ] = new int[ m_z ];
      memset( m_maze[ i ][ j ], 0, m_z * sizeof( int ) );
    }
  }
}
