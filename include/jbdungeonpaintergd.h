/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonPainterGD
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * JBDungeonPainterGD implements the interface specified by JBDungeonPainter
 * and uses the GD graphics library (from http://www.boutell.com/gd/) to
 * implement the primitives.
 * ---------------------------------------------------------------------- */

#ifndef __JBDUNGEONPAINTERGD_H__
#define __JBDUNGEONPAINTERGD_H__

#include "jbdungeon.h"
#include "jbdungeonpainter.h"
#include "gd.h"

class JBDungeonPainterGD : public JBDungeonPainter {
  public:

    JBDungeonPainterGD( JBDungeon* dungeon, int gridSize, int border );

    virtual ~JBDungeonPainterGD();

    /* ------------------------------------------------------------------ *
     * gdImagePtr getImage()
     *
     * Returns the image data created by the painter routines.
     * ------------------------------------------------------------------ */
    gdImagePtr getImage() { return m_image; }

  protected:

    virtual void m_rectangle( int x1, int y1, int x2, int y2, long color, bool filled = false );
    virtual void m_line( int x1, int y1, int x2, int y2, long color );
    virtual void m_string( int x, int y, char* text, long color, void* font );
    virtual void m_char( int x, int y, char c, long color, void* font );
    virtual void m_charUp( int x, int y, char c, long color, void* font );

    virtual void* m_selectFontToFit( char* text, int width );
    virtual long  m_allocateColor( int red, int green, int blue );

    virtual int   getFontWidth( void* font );
    virtual int   getFontHeight( void* font );

  protected:

    gdImagePtr m_image;
};

#endif /* __JBDUNGEONPAINTERGD_H__ */
