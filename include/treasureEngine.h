#ifndef __TREASUREENGINE_H__
#define __TREASUREENGINE_H__

/* ---------------------------------------------------------------------- *
 * Constant definitions
 * ---------------------------------------------------------------------- */

#define NONE      (  0 )
#define CP        (  1 )
#define SP        (  2 )
#define GP        (  3 )
#define PP        (  4 )
#define GEMS      (  5 )
#define ART       (  6 )
#define MINOR     (  7 )
#define MEDIUM    (  8 )
#define MAJOR     (  9 )
#define MUNDANE   ( 10 )
#define ARMOR     ( 11 )
#define WEAPONS   ( 12 )
#define POTIONS   ( 13 )
#define RINGS     ( 14 )
#define RODS      ( 15 )
#define SCROLLS   ( 16 )
#define STAFFS    ( 17 )
#define WANDS     ( 18 )
#define WONDROUS  ( 19 )
#define ENHANCED  ( 20 )
#define SPECIAL   ( 21 )
#define SPECIFIC  ( 22 )
#define COMMONMW  ( 23 )
#define UNCOMMNW  ( 24 )
#define COMMONRW  ( 25 )
#define MELEE     ( 26 )
#define RANGED    ( 27 )
#define BANE      ( 28 )
#define ARCANE    ( 29 )
#define DIVINE    ( 30 )

/* maximum number of "levels" for random treasure generation */

#define LEVEL_COUNT  ( 21 )

/* ---------------------------------------------------------------------- *
 * Type definitions
 * ---------------------------------------------------------------------- */

typedef struct __treasureitem__     TREASUREITEM;
typedef struct __treasureoptions__  TREASUREOPTS;

struct __treasureitem__ {
  char          desc[1024];
  int           value;       /* this is the value of the item in cp */
  TREASUREITEM* next;
};

struct __treasureoptions__ {
  TREASUREITEM* treasureList;
  TREASUREITEM* treasureTail;
  int           forceIntelligent;
};

/* ---------------------------------------------------------------------- *
 * Function definitions
 * ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- *
 * Adds the given description and value to the treasure list for the given
 * options.
 * ---------------------------------------------------------------------- */
void  addNewTreasure( TREASUREOPTS *opts, char* desc, int value );

/* ---------------------------------------------------------------------- *
 * Cleans up (deallocates) the treasure list for the given options.
 * ---------------------------------------------------------------------- */
void  cleanupTreasure( TREASUREOPTS *opts );

/* ---------------------------------------------------------------------- *
 * Generates a random armor from the DMG lists -- this will be either
 * armor, a shield, a "specific" armor, or a "specific" shield.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateArmor( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random potion from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generatePotion( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a complete random treasure from the DMG lists.
 * "level" is an integer from 0 to 20 (inclusive), and represents the
 * 0-based encounter level that the treasure is generated for.  Level 20
 * is a special case and contains random magical and mundane treasures from
 * the MUNDANE, MINOR, MEDIUM, and MAJOR lists.
 * ---------------------------------------------------------------------- */
void  generateRandomTreasure( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a complete random treasure from the DMG lists.
 * "level" is an integer from 0 to 20 (inclusive), and represents the
 * 0-based encounter level that the treasure is generated for.  Level 20
 * is a special case and contains random magical and mundane treasures from
 * the MUNDANE, MINOR, MEDIUM, and MAJOR lists.
 * ---------------------------------------------------------------------- */
void  generateRandomTreasureEx( TREASUREOPTS *opts, int level, float* mods );

/* ---------------------------------------------------------------------- *
 * Generates a random ring from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateRing( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random rod from the DMG lists.
 * "level" is one of MEDIUM or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateRod( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random scroll from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateScroll( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a single specific set of armor from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateSpecificArmor( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a single specific shield from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateSpecificShield( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a single specific weapon from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateSpecificWeapon( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a single staff from the DMG lists.
 * "level" is one of MEDIUM or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateStaff( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random treasure for a given encounter level and column from
 * the table.  Column '0' is coins, column '1' is gems and art objects,
 * and column '2' is mundane and magical items.
 * "level" is from 0 to 20, inclusive, representing the 0-based encounter
 * level that the treasure is being rolled for.
 * "rollHigher" is a boolean flag indicating whether or not to roll on a
 * higher treasure table on a d% roll of 96-100. (See DMG treasure lists).
 * ---------------------------------------------------------------------- */
int   generateTreasure( TREASUREOPTS *opts, int level, int column, 
                        int rollHigher, float mod );

/* ---------------------------------------------------------------------- *
 * Generates a random wand from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateWand( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random weapon from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateWeapon( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random wondrous item from the DMG lists.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * ---------------------------------------------------------------------- */
void  generateWondrousItem( TREASUREOPTS *opts, int level );

/* ---------------------------------------------------------------------- *
 * Generates a random spell from the DMG scroll lists.
 * "type" is one of ARCANE or DIVINE.
 * "level" is one of MINOR, MEDIUM, or MAJOR.
 * "value" is a pointer to an integer that will contain the gold piece
 * value for the returned spell.
 * ---------------------------------------------------------------------- */
char* randomSpell( int type, int level, int *value );

#ifdef __cplusplus
}
#endif

#endif /* __TREASUREENGINE_H__ */
