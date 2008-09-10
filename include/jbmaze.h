/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBMaze
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * JBMaze implements a rather simple algorithm for generating random
 * mazes.  The algorithm is as follows:
 *
 *   (1) initialize each element of an n-dimensional grid to 0.
 *   (2) pick a point at random from the grid. ensure that the point lies
 *       within the mask.
 *   (3) pick a random direction and move that way, as long as the new
 *       point is unvisited and within the mask.
 *   (4) perform (3) until there is no valid direction to move in.  Then
 *       pick a new point that HAS been visited before, and perform (3)
 *       and (4) until all the points have been visited.
 *
 * This code currently only supports up to 3-dimensional mazes, but could
 * be extended fairly easily to support mazes of arbitrary dimension
 * (which would be almost the same as having n mazes with teleports
 * between them).  Could be a fun project . . .
 * ---------------------------------------------------------------------- */

#ifndef __JBMAZE_H__
#define __JBMAZE_H__

#include "jbmazemask.h"

/* ---------------------------------------------------------------------- *
 * JBMazePt
 *
 * An auxiliary class that represents a single three-dimensional point.
 * ---------------------------------------------------------------------- */
class JBMazePt {
  public:

    JBMazePt() { }
    JBMazePt( int sx, int sy, int sz ) { x = sx; y = sy; z = sz; }

    bool operator ==( const JBMazePt& p ) {
      return ( ( x == p.x ) && ( y == p.y ) && ( z == p.z ) );
    }

    int x;
    int y;
    int z;
};


class JBMaze {
  public:

    static const int c_NORTH;
    static const int c_SOUTH;
    static const int c_WEST;
    static const int c_EAST;
    static const int c_UP;
    static const int c_DOWN;

  public:

    /* ------------------------------------------------------------------ *
     * JBMaze( ... )
     * Prepare a new maze with the given parameters.
     * ------------------------------------------------------------------ */
    JBMaze( int x, int y, int z, 
            long seed = 0,
            int randomness = 100,
            int sx = 0, int sy = 0, int sz = 0,
            int ex = -1, int ey = -1, int ez = -1 );

    /* ------------------------------------------------------------------ *
     * ~JBMaze()
     * Destroy the maze and release it's resources.
     * ------------------------------------------------------------------ */
    ~JBMaze();

    /* ------------------------------------------------------------------ *
     * Get the x-, y-, and z-dimensions of the maze.
     * ------------------------------------------------------------------ */
    int  getX() { return m_x; }
    int  getY() { return m_y; }
    int  getZ() { return m_z; }

    /* ------------------------------------------------------------------ *
     * Get the start- and end-points of the maze.
     * ------------------------------------------------------------------ */
    const JBMazePt& getStart() { return m_start; }
    const JBMazePt& getEnd() { return m_end; }

    /* ------------------------------------------------------------------ *
     * Get the random seed that generated the maze.
     * ------------------------------------------------------------------ */
    long getSeed() { return m_seed; }

    /* ------------------------------------------------------------------ *
     * Get the randomness of the maze (a value from 0-100 representing
     * how often a passage will bend).
     * ------------------------------------------------------------------ */
    int  getRandomness() { return m_randomness; }

    /* ------------------------------------------------------------------ *
     * Returns the exits at the given point in the maze.  This will be
     * a bitwise combination of the JBMaze::c_XXXX constants (above).
     * ------------------------------------------------------------------ */
    int  getExitsAt( int x, int y, int z );

    /* ------------------------------------------------------------------ *
     * Solve the maze, and return the solution as an array of points.  This
     * will fail and return NO solution if the maze has previously had any
     * number of it's deadends removed (see clearDeadends(), below).
     * ------------------------------------------------------------------ */
    void solve( JBMazePt** path, int* pathLen );

    /* ------------------------------------------------------------------ *
     * Sparsify the maze by the given amount.  The amount represents the
     * number of times to sparsify the maze.  A smaller maze will sparsify
     * faster than a larger maze, so the smaller the maze, the smaller the
     * number should be to accomplish the same relative amount of
     * "sparsification".  Also, a maze will sparsify much better if
     * clearDeadends() has not yet been called.
     * ------------------------------------------------------------------ */
    void sparsify( int amount );

    /* ------------------------------------------------------------------ *
     * Clears the given percentage of deadends from the maze by causing
     * the deadends to extend until they hit another passage.  As this
     * causes cycles in the maze, it can result in multiple possible solutions
     * to the maze.  Thus, solve() must be called BEFORE clearDeadends()
     * is called.
     * ------------------------------------------------------------------ */
    void clearDeadends( int percentage );

    /* ------------------------------------------------------------------ *
     * Generates the maze.  This must be called BEFORE solve(), sparsify(),
     * or clearDeadends().
     * ------------------------------------------------------------------ */
    void generate();

    /* ------------------------------------------------------------------ *
     * Sets the mask to be used when generating the maze.  As such, it
     * must be called BEFORE generate to have any effect.
     * ------------------------------------------------------------------ */
    void setMask( JBMazeMask* mask );

    /* ------------------------------------------------------------------ *
     * Retrieves the mask that the maze is using to generate itself.
     * ------------------------------------------------------------------ */
    JBMazeMask* getMask() { return m_mask; }

  private:
  
    /* ------------------------------------------------------------------ *
     * Used internally to verify that a given point has been visited during
     * "sparsification".
     * ------------------------------------------------------------------ */
    static const int c_MARK;

    struct JBMAZE_SOLUTION {
      int x;
      int y;
      int z;
      int directions;
      JBMAZE_SOLUTION* previous;
    };

    /* ------------------------------------------------------------------ *
     * Used internally by solve() to keep track of the current solution
     * state.
     * ------------------------------------------------------------------ */
    JBMAZE_SOLUTION* m_push( int x, int y, int z, JBMAZE_SOLUTION* stack );
    JBMAZE_SOLUTION* m_pop( JBMAZE_SOLUTION* stack );

    /* ------------------------------------------------------------------ *
     * Used internally by sparsify() to clear all marks of visitation.
     * ------------------------------------------------------------------ */
    void m_clearMarks();

    /* ------------------------------------------------------------------ *
     * Used internally for allocation and reallocation of the maze.
     * ------------------------------------------------------------------ */
    void m_allocateMaze();
    void m_deallocateMaze();

  private:

    int    m_x;               /* x-dimension */
    int    m_y;               /* y-dimension */
    int    m_z;               /* z-dimension */

    JBMazePt m_start;         /* starting point */
    JBMazePt m_end;           /* ending point */

    int*** m_maze;            /* the maze itself (three-dimensional) */

    int    m_randomness;      /* (0-100) how often the passages bend */
    long   m_seed;            /* the random seed value */

    int    m_deadendsClosed;  /* (bool) has clearDeadends() been called? */

    JBMazeMask* m_mask;       /* the mask to use for generating the maze */
};

#endif /* __JBMAZE_H__ */
