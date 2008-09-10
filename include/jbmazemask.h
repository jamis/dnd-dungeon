/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBMazeMask
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * JBMazeMask represents a two-dimensional mask that may be applied to a
 * maze object (see JBMaze).  The mask then determines what areas of the
 * map are valid for generating the maze.
 * ---------------------------------------------------------------------- */

#ifndef __JBMAZEMASK_H__
#define __JBMAZEMASK_H__

class JBMazeMask {
  public:

    /* ------------------------------------------------------------------ *
     * JBMazeMask( int width, int height )
     *
     * Creates a new JBMazeMask object with the given width and height.
     * The mask is initialized to be empty.
     * ------------------------------------------------------------------ */
    JBMazeMask( int width, int height );

    /* ------------------------------------------------------------------ *
     * JBMazeMask( char* filename )
     *
     * Creates a new JBMazeMask object from the data in the indicated
     * file.  The first line of the file must be the width and the height
     * of the mask (comma delimited).  Subsequent lines represent the rows
     * in the mask, where each character in the line must be a '0' or a '1'
     * and represents whether the mask at that point is 'on' (1) or 'off 
     * (0).
     * ------------------------------------------------------------------ */
    JBMazeMask( char* filename );

    /* ------------------------------------------------------------------ *
     * JBMazeMask( JBMazeMask& master )
     *
     * Copy constructor -- creates a new JBMazeMask that is an exact
     * duplicate of the indicated mask object.
     * ------------------------------------------------------------------ */
    JBMazeMask( JBMazeMask& master );

    /* ------------------------------------------------------------------ *
     * ~JBMazeMask()
     *
     * Destroys the mask object and deallocates any resources it used.
     * ------------------------------------------------------------------ */
    virtual ~JBMazeMask();

    /* ------------------------------------------------------------------ *
     * int getWidth()
     *
     * Returns the width of the mask.
     * ------------------------------------------------------------------ */
    int getWidth() { return m_width; }

    /* ------------------------------------------------------------------ *
     * int getHeight()
     *
     * Returns the height of the mask.
     * ------------------------------------------------------------------ */
    int getHeight() { return m_height; }

    /* ------------------------------------------------------------------ *
     * int getMaskAt( int x, int y )
     *
     * Returns the value of the mask at the indicated point.  A 1 means
     * that the mask is "valid" at that point (ie, the maze may be drawn
     * there).  A 0 means that the maze must consider this point a wall.
     * ------------------------------------------------------------------ */
    int getMaskAt( int x, int y ) { return m_mask[ x ][ y ]; }

  private:

    int    m_width;
    int    m_height;

    char** m_mask;
};

#endif /* __JBMAZEMASK_H__ */
