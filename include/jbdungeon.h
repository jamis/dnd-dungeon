/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeon
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * JBDungeon represents a "complete" dungeon -- map, monsters, treasures,
 * rooms, walls, and everything.  To accomplish this, this file also
 * includes the auxiliary objects used by JBDungeon:
 *
 *   - JBDungeonOptions
 *       represents the set of options that will determine the configuration
 *       of the maze.
 *   - JBDungeonRoom
 *       represents a single room (and it's contents) in the dungeon.
 *   - JBDungeonWall
 *       represents a single wall (and it's features) in the dungeon.
 *   - JBDungeonDatum
 *       an abstract object that is used by both JBDungeonRoom and
 *       JBDungeonWall to generalize the contents and features of these
 *       objects.  See JBDungeonData for more information.
 *
 * ---------------------------------------------------------------------- */

#ifndef __JBDUNGEON_H__
#define __JBDUNGEON_H__


#include "jbmaze.h"
#include "jbmazemask.h"

#define REROLL_ONCE           ( 0x10000000 )
#define REROLL_ANY            ( 0x20000000 )  


/* forward declarations */

class JBDungeonOptions;
class JBDungeonRoom;
class JBDungeonWall;
class JBDungeon;
class JBDungeonDatum;


/* --------------------------------------------------------------------- *
 * JBDungeonDatum
 *
 * An abstract class used to encapsulate data associated with the various
 * attributes of a dungeon room or wall.
 *
 * See JBDungeonData for more information on how this object is used.
 * --------------------------------------------------------------------- */
class JBDungeonDatum {
  public:

    JBDungeonDatum() {}
    virtual ~JBDungeonDatum() {}

    virtual void getDatumDescription( char* desc ) = 0;
};


/* --------------------------------------------------------------------- *
 * JBDungeonOptions
 *
 * Contains the various parameters, settings, and attributes of the
 * dungeon to be created.  This object is used solely as a parameter to
 * the JBDungeon constructor.
 * --------------------------------------------------------------------- */
class JBDungeonOptions {
  public:
    JBDungeonOptions();
    ~JBDungeonOptions();

    JBMazePt size;           /* dimensions of the dungeon, in the absense of a mask */
    JBMazePt start;          /* the starting point of the dungeon (for mazes) */
    JBMazePt end;            /* the ending point of the dungeon (for mazes) */

    JBMazeMask* mask;        /* the mask to use on the maze and dungeon (or NULL if no mask is desired) */

    int seed;                /* the random seed to use to generate the dungeon */

    int randomness;          /* (0-100) how random the dungeon's passages are */
    int sparseness;          /* (0+) how sparse the dungeon is */
    int clearDeadends;       /* (0-100) what pct of deadends to remove */

    int minRoomCount;        /* minimum number of rooms to place */
    int maxRoomCount;        /* maximum number of rooms to place */

    int minRoomX;            /* minimum x-dimension of a room */
    int maxRoomX;            /* maximum x-dimension of a room */
    int minRoomY;            /* minimum y-dimension of a room */
    int maxRoomY;            /* maximum y-dimension of a room */

    int secretDoors;         /* percentage of doors to make "secret" doors */
    int concealedDoors;      /* percentage of doors to make "concealed" doors */
};


/* --------------------------------------------------------------------- *
 * JBDungeonRoom
 *
 * One element in a linked list of all the rooms in a dungeon.
 * --------------------------------------------------------------------- */
class JBDungeonRoom {
  public:

    JBDungeonRoom();
    JBDungeonRoom( JBDungeonRoom* nextRoom );
    ~JBDungeonRoom();

    JBMazePt topLeft;           /* top-left coordinate of the room */
    JBMazePt size;              /* x/y dimensions of the room */

    JBDungeonRoom* next;        /* the next room in the sequence */

    JBDungeonWall** walls;      /* an array of the walls referenced by the room */
    int             wallCount;  /* the number of walls contained in the above array */

    JBDungeonDatum* data;       /* description of the room */
};


/* --------------------------------------------------------------------- *
 * JBDungeonWall
 *
 * One element in a linked list of all the explicit walls in a dungeon.
 * --------------------------------------------------------------------- */
class JBDungeonWall {
  public:

    static const int c_NONE;           /* no wall */
    static const int c_WALL;           /* wall */
    static const int c_DOOR;           /* wall is actually a door */
    static const int c_SECRETDOOR;     /* wall is actually a secret door */
    static const int c_CONCEALEDDOOR;  /* wall is actually a concealed door */

  public:

    JBDungeonWall( const JBMazePt& p1, const JBMazePt& p2, int wallType = c_NONE );
    ~JBDungeonWall();

    JBMazePt  pt1;         /* coordinate of point on one side of the wall */
    JBMazePt  pt2;         /* coordinate of point on other side of the wall */
    int       type;        /* one of the c__XXX constants (above) */

    JBDungeonDatum* data;  /* an object describing the wall's attributes */

    JBDungeonWall* next;   /* the next wall in the sequence */
};


/* --------------------------------------------------------------------- *
 * JBDungeon
 *
 * The dungeon object.
 * --------------------------------------------------------------------- */
class JBDungeon {
  public:

    static const int c_WALL;     /* point is in a wall */
    static const int c_PASSAGE;  /* point is in a passage */
    static const int c_ROOM;     /* point is in a room */

  public:

    /* ----------------------------------------------------------------- *
     * JBDungeon( JBDungeonOptions& options )
     *
     * Construct a new dungeon with the given options.
     * ----------------------------------------------------------------- */
    JBDungeon( JBDungeonOptions& options );

    /* ----------------------------------------------------------------- *
     * ~JBDungeon()
     *
     * Destroy and deallocate the dungeon object and all it's referenced
     * resources.
     * ----------------------------------------------------------------- */
    ~JBDungeon();

    /* ----------------------------------------------------------------- *
     * int getX()
     *
     * Retrieve the x-dimension of the dungeon
     * ----------------------------------------------------------------- */
    int getX() { return m_x; }

    /* ----------------------------------------------------------------- *
     * int getY()
     *
     * Retrieve the y-dimension of the dungeon
     * ----------------------------------------------------------------- */
    int getY() { return m_y; }

    /* ----------------------------------------------------------------- *
     * int getZ()
     *
     * Retrieve the z-dimension of the dungeon
     * ----------------------------------------------------------------- */
    int getZ() { return m_z; }

    /* ----------------------------------------------------------------- *
     * int getDungeonAt( int x, int y, int z )
     *
     * Retrieves the rough character of the dungeon at the given point
     * (one of the JBDungeon::c_XXXX constants, above).
     * ----------------------------------------------------------------- */
    int getDungeonAt( int x, int y, int z );

    /* ----------------------------------------------------------------- *
     * int getSolutionLength()
     *
     * Retrieves the number of steps in the solution of the maze.
     * ----------------------------------------------------------------- */
    int getSolutionLength() { return m_solutionLength; }

    /* ----------------------------------------------------------------- *
     * const JBMazePt& getSolutionStep( int i )
     *
     * Retrieves the solution point at the given index.
     * ----------------------------------------------------------------- */
    const JBMazePt& getSolutionStep( int i ) { return m_solution[ i ]; }

    /* ----------------------------------------------------------------- *
     * int getWallBetween( const JBMazePt& p1, const JBMazePt& p2 )
     *
     * Returns the wall type that exists between the two given points.
     * The points given must be vertically or horizontally adjacent.
     * ----------------------------------------------------------------- */
    int getWallBetween( const JBMazePt& p1, const JBMazePt& p2 );

    /* ----------------------------------------------------------------- *
     * int getRoomCount()
     *
     * Returns the number of rooms in the dungeon.
     * ----------------------------------------------------------------- */
    int getRoomCount();

    /* ----------------------------------------------------------------- *
     * JBDungeonRoom* getRoom( int idx )
     *
     * Returns the room at the given index.
     * ----------------------------------------------------------------- */
    JBDungeonRoom* getRoom( int idx );

    /* ----------------------------------------------------------------- *
     * void setDataPath( const char* path )
     *
     * Sets the data path that the dungeon will use.
     * ----------------------------------------------------------------- */
    void setDataPath( const char* path );

    /* ----------------------------------------------------------------- *
     * char* getDataPath()
     *
     * Retrieves the data path that the dungeon will use.
     * ----------------------------------------------------------------- */
    char* getDataPath() { return m_dataPath; }

  private:

    /* ----------------------------------------------------------------- *
     * void m_generate( JBDungeonOptions& options )
     *
     * Generates the dungeon with the given options.
     * ----------------------------------------------------------------- */
    void m_generate( JBDungeonOptions& options );

    /* ----------------------------------------------------------------- *
     * void m_computeRooms( JBDungeonOptions& options )
     *
     * Computes the rooms in the dungeon using the given options.
     * ----------------------------------------------------------------- */
    void m_computeRooms( JBDungeonOptions& options );

    /* ----------------------------------------------------------------- *
     * void m_computeWalls( JBDungeonOptions& options )
     *
     * Computes the walls in the dungeon using the given options.
     * ----------------------------------------------------------------- */
    void m_computeWalls( JBDungeonOptions& options );

    /* ----------------------------------------------------------------- *
     * int m_findOptimalRoomPlacement( int& rx, int& ry, int z, int& cx, int& cy )
     *
     * Finds the optimal placement for a room of cx/cy dimenions, at depth
     * z in the dungeon.  The resulting x/y coordinates are returned in cx
     * and cy.  NOTE:  the rx and ry parameters may change as well, if there
     * is not room for a room of the requested size.
     * ----------------------------------------------------------------- */
    int  m_findOptimalRoomPlacement( int& rx, int& ry, int z, int& cx, int& cy );

    /* ----------------------------------------------------------------- *
     * void m_addRoom( int cx, int cy, int z, int rx, int ry )
     *
     * Adds a room of the given dimensions at the given point in the
     * dungeon.
     * ----------------------------------------------------------------- */
    void m_addRoom( int cx, int cy, int z, int rx, int ry );

    /* ----------------------------------------------------------------- *
     * void m_addWall( const JBMazePt& p1, const JBMazePt& p2, int type )
     *
     * Adds a wall of the given type between the two indicated horizontally
     * or vertically adjacent points.
     * ----------------------------------------------------------------- */
    void m_addWall( const JBMazePt& p1, const JBMazePt& p2, int type );

  private:

    int***    m_dungeon;         /* the three dimensional array of dungeon points */

    JBMazePt* m_solution;        /* the list of points in the solution of the maze */
    int       m_solutionLength;  /* the number of steps in the solution */

    int       m_x;               /* the x-dimension of the dungeon */
    int       m_y;               /* the y-dimension of the dungeon */
    int       m_z;               /* the z-dimension of the dungeon */

    JBDungeonRoom* m_rooms;      /* the list of rooms in the dungeon */
    JBDungeonWall* m_walls;      /* the list of walls in the dungeon */

    JBMazeMask*    m_mask;       /* the mask to use for creating the dungeon */

    char*    m_dataPath;         /* the path that the generator looks in to find data */
};

#endif /* __JBDUNGEON_H__ */
