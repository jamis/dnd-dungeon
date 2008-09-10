/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonPainterGD
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * These routines require the existance of the GD library (available from
 * http://www.boutell.com/gd/), as well as any other libraries the GD
 * library requires (most notably libpng, libjpeg, and libz).
 * ---------------------------------------------------------------------- */

#include <string.h>

#include "jbdungeon.h"
#include "jbdungeonpaintergd.h"

#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"


JBDungeonPainterGD::JBDungeonPainterGD( JBDungeon* dungeon, int gridSize, int border ) 
  : JBDungeonPainter( dungeon, gridSize, border ) 
{
  m_image = gdImageCreate( getCanvasWidth(), getCanvasHeight() );
}


JBDungeonPainterGD::~JBDungeonPainterGD() {
  gdImageDestroy( m_image );
}


void JBDungeonPainterGD::m_rectangle( int x1, int y1, int x2, int y2, long color, bool filled ) {
  if( filled ) {
    gdImageFilledRectangle( m_image, x1, y1, x2, y2, color );
  } else {
    gdImageRectangle( m_image, x1, y1, x2, y2, color );
  }
}


void JBDungeonPainterGD::m_line( int x1, int y1, int x2, int y2, long color ) {
  gdImageLine( m_image, x1, y1, x2, y2, color );
}


void JBDungeonPainterGD::m_string( int x, int y, char* text, long color, void* font ) {
  gdImageString( m_image, (gdFontPtr)font, x, y, (unsigned char*)text, color );
}


void JBDungeonPainterGD::m_char( int x, int y, char c, long color, void* font ) {
  gdImageChar( m_image, (gdFontPtr)font, x, y, c, color );
}


void JBDungeonPainterGD::m_charUp( int x, int y, char c, long color, void* font ) {
  gdImageCharUp( m_image, (gdFontPtr)font, x, y, c, color );
}


void* JBDungeonPainterGD::m_selectFontToFit( char* text, int width ) {
  gdFontPtr fonts[] = { gdFontGiant, gdFontLarge, gdFontMediumBold, gdFontSmall, gdFontTiny, 0 };
  int  i;
  int  length;

  length = strlen( text );

  for( i = 0; fonts[ i ] != 0; i++ ) {
    if( length * fonts[ i ]->w <= width ) {
      return (void*)( fonts[ i ] );
    }
  }

  return (void*)( gdFontTiny );
}


long JBDungeonPainterGD::m_allocateColor( int red, int green, int blue ) {
  return gdImageColorAllocate( m_image, red, green, blue );
}


int JBDungeonPainterGD::getFontWidth( void* font ) {
  gdFontPtr f = (gdFontPtr)font;
  return f->w;
}


int JBDungeonPainterGD::getFontHeight( void* font ) {
  gdFontPtr f = (gdFontPtr)font;
  return f->h;
}
