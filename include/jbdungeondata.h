/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonDescription
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 *
 * The JBDungeonDescription object is auxiliary to the JBDungeon object.  It
 * encapsulates the logic that populates the dungeon and it's rooms.
 *
 * This file also defines the JBDungeonWallDatum and JBDungeonRoomDatum
 * classes, which derive from JBDungeonDatum.  They describe, respectively,
 * the walls and rooms of the dungeon.
 * ---------------------------------------------------------------------- */

#ifndef __JBDUNGEONDATA_H__
#define __JBDUNGEONDATA_H__

#include <string.h>
#include "jbdungeon.h"
#include "treasureEngine.h"

/* door types */

#define dtWOODEN              ( 0x00000001 )
#define dtSTONE               ( 0x00000002 )
#define dtIRON                ( 0x00000004 )
#define dtSIDESLIDE           ( 0x00000008 )
#define dtDOWNSLIDE           ( 0x00000010 )
#define dtUPSLIDE             ( 0x00000020 )
#define dtMAGIC               ( 0x00000040 )
#define dtSIMPLE              ( 0x00000080 )
#define dtGOOD                ( 0x00000100 )
#define dtSTRONG              ( 0x00000200 )

#define dtFREE                ( 0x00000400 )
#define dtSTUCK               ( 0x00000800 )
#define dtLOCKED              ( 0x00001000 )
#define dtTRAPPED             ( 0x00002000 )

/* secret door types */

#define sdtROTATINGWALL       ( 0x00004000 )
#define sdtPASSWALL           ( 0x00008000 )

#define sdtPUSHBRICK          ( 0x00010000 )
#define sdtMAGICWORD          ( 0x00020000 )
#define sdtGESTURE            ( 0x00040000 )
#define sdtPRESSUREPLATE      ( 0x00080000 )

/* concealed door types */

#define cdtILLUSORYWALL       ( 0x00100000 )
#define cdtBEHINDTAPESTRY     ( 0x00200000 )
#define cdtBEHINDRUBBISH      ( 0x00400000 )
#define cdtFALSEWALL          ( 0x00800000 )

#define DUNGEON_DATA_DELIM    "|"

class JBDungeonWallDatum : public JBDungeonDatum {
  public:

    JBDungeonWallDatum( long wtype, char* wtrap, char* wdescriptor ) {
      type = wtype;

      trap = 0;      
      if( wtrap != 0 ) {
        trap = new char[ strlen( wtrap ) + 1 ];
        strcpy( trap, wtrap );
      }

      descriptor = 0;
      if( wdescriptor != 0 ) {
        descriptor = new char[ strlen( wdescriptor ) + 1 ];
        strcpy( descriptor, wdescriptor );
      }
    }

    ~JBDungeonWallDatum() { delete[] descriptor; delete[] trap; }

    virtual void getDatumDescription( char* desc );

    long  type;          /* wall type (see dt---, sdt---, and cdt--- constants, above */
    char* trap;          /* textual description of trap, or NULL if there is not trap */
    char* descriptor;    /* descriptor (ie, "secret", "concealed", etc) */
};


class JBDungeonRoomDatum : public JBDungeonDatum {
  public:

    JBDungeonRoomDatum() {
      monsters = features = 0;
      monsterCount = featureCount = 0;
      trap = 0;
      dataPath = "";
      memset( &treasure, 0, sizeof( treasure ) );
      memset( &monsterTreasure, 0, sizeof( monsterTreasure ) );
    }

    ~JBDungeonRoomDatum();

    virtual void getDatumDescription( char* desc );

    void addMonster( char* desc );
    void addFeature( char* desc );

    char** monsters;               /* array of monsters in room */
    int    monsterCount;           /* number of monster entries in room */

    TREASUREOPTS monsterTreasure;  /* the treasure that the monster has (if any) */

    char** features;               /* array of features in room */
    int    featureCount;           /* number of feature entries in room */

    TREASUREOPTS treasure;         /* hidden treasure in room (if any) */
    char*  trap;                   /* description of trap in room (if any) */

    int    dungeonLevel;           /* the dungeon level of the room */

    char*  dataPath;               /* the file path that the generator looks in for data */
};


class JBDungeonDescription {
  public:

    /* ------------------------------------------------------------------ *
     * JBDungeonDescription( JBDungeon* dungeon, int level )
     *
     * Describe the indicated dungeon as if it were at the indicated
     * level.
     * ------------------------------------------------------------------ */
    JBDungeonDescription( JBDungeon* dungeon, int level );

    /* ------------------------------------------------------------------ *
     * ~JBDungeonDescription()
     *
     * Deallocates and destroys the JBDungeonDescription object.
     * ------------------------------------------------------------------ */
    ~JBDungeonDescription();

  private:

    /* ------------------------------------------------------------------ *
     * void m_describeRooms( JBDungeon* dungeon, int level )
     *
     * Describes all the rooms in the dungeon.
     * ------------------------------------------------------------------ */
    void m_describeRooms( JBDungeon* dungeon, int level );

    /* ------------------------------------------------------------------ *
     * void m_describeRoom( JBDungeon* dungeon, JBDungeonRoom* room, int level )
     *
     * Describes the given room as if it were at the given dungeon level.
     * This includes describing the walls in the room, if they have not
     * already been described.
     * ------------------------------------------------------------------ */
    void m_describeRoom( JBDungeon* dungeon, JBDungeonRoom* room, int level );

};

#endif /* __JBDUNGEONDATA_H__ */
