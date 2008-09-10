/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonPainter
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 * ---------------------------------------------------------------------- */

#include <string.h>
#include <stdio.h>

#include "jbdungeon.h"
#include "jbdungeonpainter.h"

void JBDungeonPainter::paint() {
  long wallClr;
  long bgColor;
  long pathClr;
  long roomClr;
  long gridClr;

  int i;
  int j;
  int ofs;
  int dir;
  int xSize;
  int ySize;
  int wall;
  
  JBDungeonRoom* room;

  ofs = m_border;

  /* compute the size of the canvas we have to work with */

  xSize = getCanvasWidth();
  ySize = getCanvasHeight();

  /* allocate the colors we need to use */
  /* FIXME: this should be customizable! */

  bgColor = m_allocateColor( 255, 255, 255 );
  wallClr = m_allocateColor( 128, 128, 128 );
  pathClr = m_allocateColor( 255, 0, 0 );
  roomClr = m_allocateColor( 255, 255, 255 );
  gridClr = m_allocateColor( 0, 0, 0 );

  /* set the background of the maze */

  m_rectangle( 0, 0, xSize - 1, ySize - 1, bgColor, true );

  /* draw the basic walls and floors */

  ofs++;
  for( j = 0; j < m_dungeon->getY(); j++ ) {
    for( i = 0; i < m_dungeon->getX(); i++ ) {
      dir = m_dungeon->getDungeonAt( i, j, 0 );
      if( dir == JBDungeon::c_WALL ) {
        m_rectangle( ofs + i * m_gridSize,         ofs + j * m_gridSize,
                     ofs + (i+1) * m_gridSize - 1, ofs + (j+1) * m_gridSize - 1,
                     wallClr, true );
      } else if( dir == JBDungeon::c_ROOM ) {
        m_rectangle( ofs + i * m_gridSize,         ofs + j * m_gridSize,
                     ofs + (i+1) * m_gridSize - 1, ofs + (j+1) * m_gridSize - 1,
                     roomClr, true );
      }
    }
  }

  /* draw specific walls and doors, by checking each point and the points
   * to the left and below it to see if there is a wall between them. */
  
  for( i = 0; i < m_dungeon->getX() - 1; i++ ) {
    for( j = 0; j < m_dungeon->getY() - 1; j++ ) {
      JBMazePt p1( i, j, 0 );
      JBMazePt p2( i, j+1, 0 );
      JBMazePt p3( i+1, j, 0 );

      wall = m_dungeon->getWallBetween( p1, p2 ); /* south wall */
      if( wall != JBDungeonWall::c_NONE ) {
        m_rectangle( ofs + i * m_gridSize,   ofs + (j+1) * m_gridSize - 1,
                     ofs + (i+1)*m_gridSize, ofs + (j+1) * m_gridSize + 1,
                     gridClr, true );
        m_drawDoor( ofs + i * m_gridSize, ofs + j*m_gridSize, wall, 1, gridClr );
      }
      wall = m_dungeon->getWallBetween( p1, p3 ); /* east wall */
      if( wall != JBDungeonWall::c_NONE ) {
        m_rectangle( ofs + (i+1) * m_gridSize - 1, ofs + j * m_gridSize,
                    ofs + (i+1) * m_gridSize + 1,  ofs + (j+1) * m_gridSize,
                    gridClr, true );        
        m_drawDoor( ofs + i * m_gridSize, ofs + j*m_gridSize, wall, 0, gridClr );
      }
    }
  }

  /* overlay a grid on top */

  for( i = 0; i < m_dungeon->getX() + 1; i++ ) {
    j = ofs + i * m_gridSize;
    m_line( j, ofs, j, ySize-ofs, gridClr );
  }
  for( i = 0; i < m_dungeon->getY() + 1; i++ ) {
    j = ofs + i * m_gridSize;
    m_line( ofs, j, xSize-ofs, j, gridClr );
  }

  /* lastly, number the rooms */

  for( i = 0; ( room = m_dungeon->getRoom( i ) ) != 0; i++ ) {
    int x;
    int y;

    x = (int)( ofs + m_gridSize * ( room->topLeft.x + room->size.x / 2.0 ) );
    y = (int)( ofs + m_gridSize * ( room->topLeft.y + room->size.y / 2.0 ) );

    m_centerNumberAt( x, y, i+1, (int)( m_gridSize * 0.8 ), gridClr );
  }
}


int JBDungeonPainter::getCanvasWidth() {
  return m_border*2 + m_gridSize * m_dungeon->getX() + 1;
}


int JBDungeonPainter::getCanvasHeight() {
  return m_border*2 + m_gridSize * m_dungeon->getY() + 1;
}


void JBDungeonPainter::m_centerNumberAt( int x, int y, int num, int maxWidth, long color ) {
  char  buffer[10];
  int   length;
  int   cx;
  int   cy;
  void* font;

  sprintf( buffer, "%d", num );
  length = strlen( buffer );

  font = m_selectFontToFit( buffer, maxWidth );
  if( font == 0 ) {
    return;
  }

  length = getFontWidth( font ) * length;
  cx = x - length / 2;
  cy = y - getFontHeight( font ) / 2;

  m_string( cx, cy, buffer, color, font );
}


void JBDungeonPainter::m_drawDoor( int x, int y, int doorType, int horiz, long color ) {
  char  c[2];
  void* font;

  if( horiz ) {
    if( doorType == JBDungeonWall::c_DOOR ) {
      m_rectangle( x + m_gridSize/4,              y + m_gridSize - m_gridSize/5,
                   x + m_gridSize - m_gridSize/4, y + m_gridSize + m_gridSize/5,
                   color );
    } else {
      if( doorType == JBDungeonWall::c_SECRETDOOR ) {
        c[0] = 'S';
      } else if( doorType == JBDungeonWall::c_CONCEALEDDOOR ) {
        c[0] = 'C';
      } else {
        return;
      }
      c[1] = 0;
      font = m_selectFontToFit( c, m_gridSize );
      if( font == 0 ) {
        return;
      }

      x = x + m_gridSize/2 - getFontHeight( font ) / 2;
      y = y + m_gridSize + getFontWidth( font) / 2;

      m_charUp( x, y, c[0], color, font );
    }
  } else {
    if( doorType == JBDungeonWall::c_DOOR ) {
      m_rectangle( x + m_gridSize - m_gridSize/5, y + m_gridSize/4,
                   x + m_gridSize + m_gridSize/5, y + m_gridSize - m_gridSize/4,
                   color );
    } else {
      if( doorType == JBDungeonWall::c_SECRETDOOR ) {
        c[0] = 'S';
      } else if( doorType == JBDungeonWall::c_CONCEALEDDOOR ) {
        c[0] = 'C';
      } else {
        return;
      }
      c[1] = 0;
      font = m_selectFontToFit( c, m_gridSize );
      if( font == 0 ) {
        return;
      }

      x = x + m_gridSize - getFontWidth( font ) / 2;
      y = y + m_gridSize/2 - getFontHeight( font ) / 2;

      m_char( x, y, c[0], color, font );
    }
  }
}
