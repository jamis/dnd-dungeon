/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeon
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 * ---------------------------------------------------------------------- */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "gameutil.h"
#include "jbdungeon.h"


JBDungeonOptions::JBDungeonOptions() {
  size.x = 10;
  size.y = 10;
  size.z = 1;

  start.x = 0;
  start.y = 0;
  start.z = 0;

  end.x = -1;
  end.y = -1;
  end.z = -1;

  seed = time(NULL);
  randomness = 100;
  sparseness = 0;
  clearDeadends = 0;
  
  minRoomCount = 0;
  maxRoomCount = 0;
  minRoomX = 0;
  maxRoomX = 0;
  minRoomY = 0;
  maxRoomY = 0;

  secretDoors = 5;
  concealedDoors = 5;

  mask = 0;
}


JBDungeonOptions::~JBDungeonOptions() {
  if( mask != 0 ) {
    delete mask;
  }
}


JBDungeonRoom::JBDungeonRoom() {
  next = 0;
  walls = 0;
  data = 0;
}

JBDungeonRoom::JBDungeonRoom( JBDungeonRoom* nextRoom ) {
  next = nextRoom;
  walls = 0;
  data = 0;
}

JBDungeonRoom::~JBDungeonRoom() {
  if( next != 0 ) {
    delete next;
  }
  if( walls != 0 ) {
    delete[] walls;
  }
  if( data != 0 ) {
    delete data;
  }
}


const int JBDungeonWall::c_NONE = 0;
const int JBDungeonWall::c_WALL = 1;
const int JBDungeonWall::c_DOOR = 2;
const int JBDungeonWall::c_SECRETDOOR = 3;
const int JBDungeonWall::c_CONCEALEDDOOR = 4;


JBDungeonWall::JBDungeonWall( const JBMazePt& p1, const JBMazePt& p2, int wallType ) {
  pt1 = p1;
  pt2 = p2;
  type = wallType;
  next = 0;
  data = 0;
}


JBDungeonWall::~JBDungeonWall() {
  delete data;
}


const int JBDungeon::c_WALL    = 0x0001;
const int JBDungeon::c_PASSAGE = 0x0002;
const int JBDungeon::c_ROOM    = 0x0004;


JBDungeon::JBDungeon( JBDungeonOptions& options ) {
  m_dungeon = 0;
  m_rooms   = 0;
  m_walls   = 0;
  m_dataPath = 0;

  m_x = m_y = m_z = 0;

  if( options.mask != 0 ) {
    m_mask = new JBMazeMask( *options.mask );
  } else {
    m_mask = new JBMazeMask( options.size.x, options.size.y );
  }

  setDataPath( "" );

  m_generate( options );
  m_computeRooms( options );
  m_computeWalls( options );
}


JBDungeon::~JBDungeon() {
  int x;
  int y;

  for( x = 0; x < m_x; x++ ) {
    for( y = 0; y < m_y; y++ ) {
      free( m_dungeon[x][y] );
    }
    free( m_dungeon[x] );
  }
  free( m_dungeon );

  free( m_solution );

  if( m_rooms != 0 ) {
    delete m_rooms;
  }
  if( m_walls != 0 ) {
    delete m_walls;
  }

  delete m_mask;
  delete m_dataPath;
}


void JBDungeon::m_generate( JBDungeonOptions& options ) {
  JBMaze* maze;
  int     x;
  int     y;
  int     z;
  int     dir;

  /* create the maze */
  maze = new JBMaze( options.size.x, options.size.y, options.size.z,
                     options.seed, options.randomness,
                     options.start.x, options.start.y, options.start.z,
                     options.end.x, options.end.y, options.end.z );

  /* set the mask to use for the maze (and dungeon) */
  maze->setMask( new JBMazeMask( *m_mask ) );

  /* generate, solve, sparsify, and clear the deadends */
  maze->generate();
  maze->solve( &m_solution, &m_solutionLength );
  maze->sparsify( options.sparseness );
  maze->clearDeadends( options.clearDeadends );

  /* the dimension of the dungeon is twice (plus 1) the dimension of the
   * maze on which it was based.  This is to allow the walls of the dungeon
   * to be considered full-blocks.  Here, we are converting the solution
   * of the maze to the new dimensions. */

  for( x = 0; x < m_solutionLength; x++ ) {
    m_solution[ x ].x = m_solution[ x ].x * 2 + 1;
    m_solution[ x ].y = m_solution[ x ].y * 2 + 1;
  }

  /* set the dimensions of the dungeon to be the dimensions of the mask,
   * times 2 plus 1 (to account for walls) */

  m_x = m_mask->getWidth() * 2 + 1;
  m_y = m_mask->getHeight() * 2 + 1;
  m_z = options.size.z;

  /* allocate and initialize the dungeon */

  m_dungeon = (int***)malloc( m_x * sizeof( int** ) );

  for( x = 0; x < m_mask->getWidth(); x++ ) {
    m_dungeon[ x*2 ] = (int**)malloc( m_y * sizeof( int* ) );
    m_dungeon[ x*2+1 ] = (int**)malloc( m_y * sizeof( int* ) );
    for( y = 0; y < m_mask->getHeight(); y++ ) {
      m_dungeon[ x*2 ][ y*2 ] = (int*)malloc( m_z * sizeof( int ) );
      m_dungeon[ x*2 ][ y*2+1 ] = (int*)malloc( m_z * sizeof( int ) );
      m_dungeon[ x*2+1 ][ y*2 ] = (int*)malloc( m_z * sizeof( int ) );
      m_dungeon[ x*2+1 ][ y*2+1 ] = (int*)malloc( m_z * sizeof( int ) );
      for( z = 0; z < m_z; z++ ) {
        m_dungeon[ x*2 ][ y*2 ][ z ] = c_WALL;
        m_dungeon[ x*2 ][ y*2+1 ][ z ] = c_WALL;
        m_dungeon[ x*2+1 ][ y*2 ][ z ] = c_WALL;
        m_dungeon[ x*2+1 ][ y*2+1 ][ z ] = c_WALL;
        dir = maze->getExitsAt( x, y, z );
        if( dir != 0 ) {
          m_dungeon[ x*2+1 ][ y*2+1 ][ z ] = c_PASSAGE;
        }
        if( ( dir & JBMaze::c_NORTH ) != 0 ) {
          m_dungeon[ x*2+1 ][ y*2 ][ z ] = c_PASSAGE;
        }
        if( ( dir & JBMaze::c_WEST ) != 0 ) {
          m_dungeon[ x*2 ][ y*2+1 ][ z ] = c_PASSAGE;
        }
      }
    }
    m_dungeon[ x*2 ][ m_y-1 ] = (int*)malloc( m_z * sizeof( int ) );
    m_dungeon[ x*2+1 ][ m_y-1 ] = (int*)malloc( m_z * sizeof( int ) );
    for( z = 0; z < m_z; z++ ) {
      m_dungeon[ x*2 ][ m_y-1 ][ z ] = c_WALL;
      m_dungeon[ x*2+1 ][ m_y-1 ][ z ] = c_WALL;
    }
  }

  m_dungeon[m_x-1] = (int**)malloc( m_y * sizeof( int* ) );
  for( y = 0; y < m_y; y++ ) {
    m_dungeon[ m_x-1 ][ y ] = (int*)malloc( m_z * sizeof( int ) );
    for( z = 0; z < m_z; z++ ) {
      m_dungeon[ m_x-1 ][ y ][ z ] = c_WALL;
    }
  }

  delete maze;
}


JBDungeonRoom* JBDungeon::getRoom( int idx ) {
  JBDungeonRoom* room;
  int            i;

  for( i = 0, room = m_rooms; room != 0; i++, room = room->next ) {
    if( i == idx ) {
      return room;
    }
  }

  return room;
}


int JBDungeon::getDungeonAt( int x, int y, int z ) {
  if( ( x < 0 ) || ( y < 0 ) || ( z < 0 ) ) {
    return 0;
  }
  if( ( x >= m_x ) || ( y >= m_y ) || ( z >= m_z ) ) {
    return 0;
  }

  return m_dungeon[ x ][ y ][ z ];
}


void JBDungeon::m_computeRooms( JBDungeonOptions& options ) {
  int roomCount;
  int rx;
  int ry;
  int i;
  int j;
  int k;
  int z;
  int cx;
  int cy;

  for( z = 0; z < m_z; z++ ) {
    if( options.maxRoomCount == options.minRoomCount ) {
      roomCount = options.minRoomCount;
    } else {
      roomCount = rand() % ( options.maxRoomCount - options.minRoomCount + 1 ) + options.minRoomCount;
    }

    for( i = 0; i < roomCount; i++ ) {
      if( options.maxRoomX == options.minRoomX ) {
        rx = options.maxRoomX;
      } else {
        rx = rand() % ( options.maxRoomX - options.minRoomX + 1 ) + options.minRoomX;
      }

      if( options.maxRoomY == options.minRoomY ) {
        ry = options.minRoomY;
      } else {
        ry = rand() % ( options.maxRoomY - options.minRoomY + 1 ) + options.minRoomY;
      }

      /* disallow extremely narrow rooms by requiring that a room never be
       * thinner than half it's longest dimension. */

      if( rx > ( ry << 1 ) ) {
        ry = ( rx >> 1 ) + 1;
      }
      if( ry > ( rx << 1 ) ) {
        rx = ( ry >> 1 ) + 1;
      }

      if( m_findOptimalRoomPlacement( rx, ry, z, cx, cy ) != 0 ) {
        break;
      }

      m_addRoom( cx, cy, z, rx, ry );

      for( j = 0; j < rx; j++ ) {
        for( k = 0; k < ry; k++ ) {
          if( m_mask->getMaskAt( (cx+j)>>1, (cy+k)>>1 ) ) {
            m_dungeon[ cx+j ][ cy+k ][ z ] = c_ROOM;
          }
        }
      }
    }
  }
}


int determineRandomDoorType( JBDungeonOptions& options ) {
  int d;

  d = rollDice( 1, 100 );
  if( d <= options.secretDoors ) {
    return JBDungeonWall::c_SECRETDOOR;
  }

  d -= options.secretDoors;
  if( d <= options.concealedDoors ) {
    return JBDungeonWall::c_CONCEALEDDOOR;
  }

  return JBDungeonWall::c_DOOR;
}


void JBDungeon::m_computeWalls( JBDungeonOptions& options ) {
  JBDungeonRoom* room;
  int            walls;
  WEIGHTEDLIST*  one;
  WEIGHTEDLIST*  two;
  int            oneTotal;
  int            twoTotal;
  int            lastOne;
  int            lastTwo;
  int            i;
  int            j;
  JBDungeonWall* wall;

  /* -------------------------------------------------------------------- *
   * What's happening here is the following:  we need to add walls to
   * separate the rooms from the passageways.  Some of these walls are
   * going to be doors.  However, there are stretches of wall that
   * abut a passageway, and we only want ONE door for this section of wall,
   * rather than one door for each square of that section.  So we use
   * the trusty weighted list approach, adding each adjacent section of
   * wall to the list and choosing one of the sections at random.
   *
   * And if that made no sense to you at all -- reread it.  It's a
   * simple procedure that's difficult to describe simply.
   * -------------------------------------------------------------------- */
  
  for( room = m_rooms; room != 0; room = room->next ) {
    walls = 0;

    oneTotal = twoTotal = 0;
    one = two = 0;
    lastOne = lastTwo = -1;

    /* check for walls and doors on the north and south */

    for( j = 0; j < room->size.x; j++ ) {
      if( ( one != 0 ) && ( lastOne != j-1 ) ) {
        wall = (JBDungeonWall*)getWeightedItem( &one, rollDice( 1, oneTotal ), &oneTotal );
        wall->type = determineRandomDoorType( options );

        destroyWeightedList( &one );
        oneTotal = 0;
      }

      if( m_dungeon[ room->topLeft.x+j ][ room->topLeft.y-1 ][ room->topLeft.z ] != c_WALL ) {
        JBMazePt p1( room->topLeft.x+j, room->topLeft.y-1, room->topLeft.z );
        JBMazePt p2( room->topLeft.x+j, room->topLeft.y, room->topLeft.z );

        m_addWall( p1, p2, JBDungeonWall::c_WALL );
        walls++;

        lastOne = j;
        oneTotal += addToWeightedList( &one, (long)m_walls, 1 );
      }

      if( ( two != 0 ) && ( lastTwo != j-1 ) ) {
        wall = (JBDungeonWall*)getWeightedItem( &two, rollDice( 1, twoTotal ), &twoTotal );
        wall->type = determineRandomDoorType( options );

        destroyWeightedList( &two );
        twoTotal = 0;
      }

      if( m_dungeon[ room->topLeft.x+j ][ room->topLeft.y+room->size.y ][ room->topLeft.z ] != c_WALL ) {
        JBMazePt p1( room->topLeft.x+j, room->topLeft.y+room->size.y-1, room->topLeft.z );
        JBMazePt p2( room->topLeft.x+j, room->topLeft.y+room->size.y, room->topLeft.z );

        m_addWall( p1, p2, JBDungeonWall::c_WALL );
        walls++;

        lastTwo = j;
        twoTotal += addToWeightedList( &two, (long)m_walls, 1 );
      }
    }

    if( one != 0 ) {
      wall = (JBDungeonWall*)getWeightedItem( &one, rollDice( 1, oneTotal ), &oneTotal );
        wall->type = determineRandomDoorType( options );
    }
    if( two != 0 ) {
      wall = (JBDungeonWall*)getWeightedItem( &two, rollDice( 1, twoTotal ), &twoTotal );
        wall->type = determineRandomDoorType( options );
    }

    destroyWeightedList( &one );
    destroyWeightedList( &two );

    /* check for walls and doors on the east and west */

    oneTotal = twoTotal = 0;
    one = two = 0;
    lastOne = lastTwo = -1;

    for( j = 0; j < room->size.y; j++ ) {
      if( ( one != 0 ) && ( lastOne != j-1 ) ) {
        wall = (JBDungeonWall*)getWeightedItem( &one, rollDice( 1, oneTotal ), &oneTotal );
        wall->type = determineRandomDoorType( options );

        destroyWeightedList( &one );
        oneTotal = 0;
      }

      if( m_dungeon[ room->topLeft.x-1 ][ room->topLeft.y+j ][ room->topLeft.z ] != c_WALL ) {
        JBMazePt p1( room->topLeft.x-1, room->topLeft.y+j, room->topLeft.z );
        JBMazePt p2( room->topLeft.x, room->topLeft.y+j, room->topLeft.z );

        m_addWall( p1, p2, JBDungeonWall::c_WALL );
        walls++;

        lastOne = j;
        oneTotal += addToWeightedList( &one, (long)m_walls, 1 );
      }

      if( ( two != 0 ) && ( lastTwo != j-1 ) ) {
        wall = (JBDungeonWall*)getWeightedItem( &two, rollDice( 1, twoTotal ), &twoTotal );
        wall->type = determineRandomDoorType( options );

        destroyWeightedList( &two );
        twoTotal = 0;
      }

      if( m_dungeon[ room->topLeft.x+room->size.x ][ room->topLeft.y+j ][ room->topLeft.z ] != c_WALL ) {
        JBMazePt p1( room->topLeft.x+room->size.x-1, room->topLeft.y+j, room->topLeft.z );
        JBMazePt p2( room->topLeft.x+room->size.x, room->topLeft.y+j, room->topLeft.z );

        m_addWall( p1, p2, JBDungeonWall::c_WALL );
        walls++;

        lastTwo = j;
        twoTotal += addToWeightedList( &two, (long)m_walls, 1 );
      }
    }

    if( one != 0 ) {
      wall = (JBDungeonWall*)getWeightedItem( &one, rollDice( 1, oneTotal ), &oneTotal );
        wall->type = determineRandomDoorType( options );
    }
    if( two != 0 ) {
      wall = (JBDungeonWall*)getWeightedItem( &two, rollDice( 1, twoTotal ), &twoTotal );
        wall->type = determineRandomDoorType( options );
    }

    destroyWeightedList( &one );
    destroyWeightedList( &two );

    room->wallCount = walls;
    room->walls = new JBDungeonWall*[ walls ];
    for( wall = m_walls, i = 0; i < walls; i++, wall = wall->next ) {
      room->walls[ i ] = wall;
    }
  }
}


int JBDungeon::m_findOptimalRoomPlacement( int& rx, int& ry, int z, int& cx, int& cy ) {
  int x;
  int y;
  int i;
  int j;
  int tally;
  int spaceX;
  int spaceY;
  int d;
  int minimumTally;
  int xForMin;
  int yForMin;
  int isDiagonal;
  WEIGHTEDLIST* wlist;
  int total;
  int overlapsRoom;
  int lowestOverlapsRoom;

  if( rx > m_x - 2 ) {
    rx = m_x - 2;
  }
  if( ry > m_y - 2 ) {
    ry = m_y - 2;
  }

  spaceX = ( m_x - rx );
  spaceY = ( m_y - ry );

  minimumTally = 100000;
  xForMin = -1;
  yForMin = -1;

  wlist = 0;
  total = 0;
  overlapsRoom = 0;

  lowestOverlapsRoom = 0;
  for( x = 1; x < spaceX; x++ ) {
    for( y = 1; y < spaceY; y++ ) {

      if( !m_mask->getMaskAt( x>>1, y>>1 ) ) {
        continue;
      }

      tally = 0;
      for( i = -1; i < rx+1; i++ ) {
        for( j = -1; j < ry+1; j++ ) {
          d = m_dungeon[ x+i ][ y+j ][ z ];
          if( ( ( i == -1 ) && ( j == -1 ) ) ||
              ( ( i == -1 ) && ( j == ry ) ) ||
              ( ( i == rx ) && ( j == -1 ) ) ||
              ( ( i == rx ) && ( j == ry ) ) )
          {
            isDiagonal = 1;
          } else {
            if( ( d & c_PASSAGE ) != 0 ) {
              if( ( j == -1 ) || ( i == -1 ) || ( j == ry ) || ( i == rx ) ) {
                tally++;
              } else {
                tally += 3;
              }
            }
            if( ( d & c_ROOM ) != 0 ) {
              /* we REALLY don't want rooms to overlap unless they have to */
              tally += 100;
              overlapsRoom = 1;
            }
            if( ( i >= 0 ) && ( j >= 0 ) && ( i < rx ) && ( j < ry ) ) {
              if( !m_mask->getMaskAt( (x+i)/2, (y+j)/2 ) ) {
                tally += 10;
              }
            }
          }
        }
      }

      if( ( tally > 0 ) && ( tally <= minimumTally ) ) {
        if( tally != minimumTally ) {
          destroyWeightedList( &wlist );
          wlist = 0;
          total = 0;
          lowestOverlapsRoom = overlapsRoom;
        }

        minimumTally = tally;
        xForMin = x;
        yForMin = y;
        total += addToWeightedList( &wlist, ( ( xForMin << 16 ) + yForMin ), 1 );
      }

      overlapsRoom = 0;
    }
  }

  if( lowestOverlapsRoom ) {
    if( ( rx == 1 ) && ( ry == 1 ) ) {
      return 1;
    }
    if( rx > ry ) {
      rx--;
    } else {
      ry--;
    }
    return m_findOptimalRoomPlacement( rx, ry, z, cx, cy );
  }

  if( wlist == 0 ) {
    cx = 1 + rand() % ( spaceX - 1 );
    cy = 1 + rand() % ( spaceY - 1 );
  } else {
    total = getWeightedItem( &wlist, rollDice( 1, total ), &total );
    cx = (unsigned int)( total >> 16 );
    cy = (unsigned int)( total & 0xFFFF );
  }

  return 0;
}


void JBDungeon::m_addRoom( int cx, int cy, int z, int rx, int ry ) {
  JBDungeonRoom* room;

  room = new JBDungeonRoom( m_rooms );

  room->size.x = rx;
  room->size.y = ry;
  room->size.z = 0;
  room->topLeft.x = cx;
  room->topLeft.y = cy;
  room->topLeft.z = z;

  m_rooms = room;
}


void JBDungeon::m_addWall( const JBMazePt& p1, const JBMazePt& p2, int type ) {
  JBDungeonWall* wall;

  wall = new JBDungeonWall( p1, p2, type );
  wall->next = m_walls;
  m_walls = wall;
}


int JBDungeon::getWallBetween( const JBMazePt& p1, const JBMazePt& p2 ) {
  JBDungeonWall* wall;

  for( wall = m_walls; wall != 0; wall = wall->next ) {
    if( ( wall->pt1 == p1 ) && ( wall->pt2 == p2 ) ) {
      return wall->type;
    }
  }

  if( ( ( m_dungeon[ p1.x ][ p1.y ][ p1.z ] == c_WALL ) ||
        ( m_dungeon[ p2.x ][ p2.y ][ p2.z ] == c_WALL ) ) &&
      ( ( m_dungeon[ p1.x ][ p1.y ][ p1.z ] != c_WALL ) ||
        ( m_dungeon[ p2.x ][ p2.y ][ p2.z ] != c_WALL ) ) )
  {
    return JBDungeonWall::c_WALL;
  }

  return JBDungeonWall::c_NONE;
}


int JBDungeon::getRoomCount() {
  int i;
  JBDungeonRoom* room;

  for( i = 0, room = m_rooms; room != 0; i++, room = room->next );

  return i;
}


void JBDungeon::setDataPath( const char* path ) {
  if( m_dataPath != 0 ) {
    delete[] m_dataPath;
  }

  m_dataPath = new char[ strlen( path ) + 1 ];
  strcpy( m_dataPath, path );
}
