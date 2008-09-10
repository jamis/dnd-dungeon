/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonPainter
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * JBDungeonPainter is an abstract class to standardizes how dungeons are
 * drawn.  To implement a dungeon painter, you need merely implement the
 * following primitive draw methods of JBDungeonPainter:
 *
 *   - m_rectangle
 *   - m_line
 *   - m_string
 *   - m_char
 *   - m_charUp
 *   - m_selectFontToFit
 *   - m_allocateColor
 *   - m_getFontWidth
 *   - m_getFontHeight
 *
 * See JBDungeonPainterGD for an example of an implementation of
 * JBDungeonPainter.
 *
 * ---------------------------------------------------------------------- */

#ifndef __JBDUNGEONPAINTER_H__
#define __JBDUNGEONPAINTER_H__

#include "jbdungeon.h"

class JBDungeonPainter {
  public:

    JBDungeonPainter( JBDungeon* dungeon, int gridSize, int border ) {
      m_dungeon  = dungeon;
      m_gridSize = gridSize;
      m_border = border;
    }

    virtual ~JBDungeonPainter() {
    }

    /* ------------------------------------------------------------------ *
     * void paint()
     *
     * Paints the dungeon.  How this is done depends on how the primitives
     * have been implemented.
     * ------------------------------------------------------------------ */
    void paint();

    /* ------------------------------------------------------------------ *
     * int getCanvasWidth()
     *
     * Returns the width (in pixels) of the canvas.
     * ------------------------------------------------------------------ */
    int  getCanvasWidth();

    /* ------------------------------------------------------------------ *
     * int getCanvasHeight()
     *
     * Returns the height (in pixels) of the canvas.
     * ------------------------------------------------------------------ */
    int  getCanvasHeight();

    /* ------------------------------------------------------------------ *
     * JBDungeon* getDungeon()
     *
     * Returns the dungeon this painter is painting.
     * ------------------------------------------------------------------ */
    JBDungeon* getDungeon() { return m_dungeon; }

  protected:

    /* ------------------------------------------------------------------ *
     * void m_centerNumberAt( int x, int y, int num, int maxWidth, long color )
     *
     * Centers the given number around the given coordinates, and tries to
     * ensure that the resulting text is no longer than maxWidth.  It is
     * drawn in the specified color.
     * ------------------------------------------------------------------ */
            void m_centerNumberAt( int x, int y, int num, int maxWidth, long color );

    /* ------------------------------------------------------------------ *
     * void m_drawDoor( int x, int y, int doorType, int horiz, long color )
     *
     * Draws a door of the specified type, orientation and color at the
     * indicated position.
     * ------------------------------------------------------------------ */
            void m_drawDoor( int x, int y, int doorType, int horiz, long color );

    /* ------------------------------------------------------------------ *
     * virtual void m_rectangle( int x1, int y1, int x2, int y2, long color, bool filled = false )
     *
     * Should draw a rectangle (filled if the 'filled' parameter is true, of
     * the given color, where one corner is at x1,y1 and the opposite corner
     * is at x2,y2.
     * ------------------------------------------------------------------ */
    virtual void m_rectangle( int x1, int y1, int x2, int y2, long color, bool filled = false ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual void m_line( int x1, int y1, int x2, int y2, long color )
     *
     * Should draw a line of the specified color between the points x1,y1
     * and x2,y2.
     * ------------------------------------------------------------------ */
    virtual void m_line( int x1, int y1, int x2, int y2, long color ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual void m_string( int x, int y, char* text, long color, void* font )
     *
     * Draws the given string in the given color with the given font at the
     * given point (the lower-left coordinate of the text should begin at
     * the given point).
     * ------------------------------------------------------------------ */
    virtual void m_string( int x, int y, char* text, long color, void* font ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual void m_char( int x, int y, char c, long color, void* font )
     *
     * As m_string, but should only draw the specified character.
     * ------------------------------------------------------------------ */
    virtual void m_char( int x, int y, char c, long color, void* font ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual void m_charUp( int x, int y, char c, long color, void* font )
     *
     * As m_char, but should draw the specified character turned 90 degrees.
     * ------------------------------------------------------------------ */
    virtual void m_charUp( int x, int y, char c, long color, void* font ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual void* m_selectFontToFit( char* text, int width )
     *
     * Return a font for which the indicated text is less than or equal to
     * the requested width.  If this is not possible, return the smallest
     * font it can.
     * ------------------------------------------------------------------ */
    virtual void* m_selectFontToFit( char* text, int width ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual long m_allocateColor( int red, int green, int blue )
     *
     * Should ensures that the given color exists in the palette and return
     * a number that will be displayed as a color with the requested amount
     * of red, green, and blue.
     * ------------------------------------------------------------------ */
    virtual long m_allocateColor( int red, int green, int blue ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual int getFontWidth( void* font )
     *
     * Returns the average width of a single character of the given font.
     * ------------------------------------------------------------------ */
    virtual int getFontWidth( void* font ) = 0;

    /* ------------------------------------------------------------------ *
     * virtual int getFontHeight( void* font )
     *
     * Returns the maximum height of the given font.
     * ------------------------------------------------------------------ */
    virtual int getFontHeight( void* font ) = 0;

  protected:

    int        m_gridSize;   /* the size of a single square of the grid in pixels */
    int        m_border;     /* how many pixels are reserved for the border of the dungeon */

    JBDungeon* m_dungeon;    /* the dungeon object to draw */
};

#endif /* __JBDUNGEONPAINTER_H__ */
