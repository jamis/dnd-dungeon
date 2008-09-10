/* ---------------------------------------------------------------------- *
 * D&D Treasure Generator Engine
 * 
 * by Jamis Buck (jgb3@email.byu.edu)
 * online version at http://rover.byu.edu/cgi-bin/treasure.cgi
 *
 * This file is in the public domain, but it should be noted that the
 * contents of the tables is from the D&D Dungeon Master's Guide, and
 * there may be copyright issues involved there.
 *
 * data taken straight from the D&D Dungeon Master's Guide, with a few
 * modifications (in particular, entries that read "DM's choice" were
 * replaced with my own preferences).
 *
 * The generator engine places all items generated in a linked list of
 * TREASUREITEM types (see the two global variables "treasureList" and
 * "treasureTail", as well as the "addNewTreasure" routine).  The treasures
 * are then displayed by looping over the list.  To make the engine
 * really spiffy, all global variables should be eliminated, and the
 * values should be passed from routine to routine . . . I didn't have the
 * luxury of time, so this will have to do for now.
 *
 * TODO: spell-containing magic items have a chance to already contain a
 *   spell (like a ring of spell-storing or counterspells).
 * ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "gameutil.h"
#include "treasureEngine.h"

/* weapon types, used to determine what types of abilities a given weapon
 * can have. */

#define SLASHING  ( 0x01 )
#define BLUDGEON  ( 0x02 )
#define PIERCING  ( 0x04 )
#define ANY       ( SLASHING | BLUDGEON | PIERCING )

#define opLAWFUL    ( 0x0001 )
#define opCHAOTIC   ( 0x0002 )
#define opGOOD      ( 0x0004 )
#define opEVIL      ( 0x0008 )


/* here is the real meat of the program, and the part that took the longest
 * to write -- all the relevant tables from the DMG!!!
 */

typedef struct {
  int pcap;       /* upper percentage cap for treasure */
  int ttype;      /* type of treasure */
  int tdc;        /* die count */
  int tdt;        /* die type */
  int mul;        /* sum multiplier */
} treasureEntry;


treasureEntry treasureTable[ LEVEL_COUNT * 15 ] = {
/*  1 */ 
  /* coins */ {  14,    NONE,  0,  0,     0 },
              {  29,      CP,  1,  6,  1000 },
              {  52,      SP,  1,  8,   100 },
              {  95,      GP,  2,  8,    10 },
              { 100,      PP,  1,  4,    10 },
  /* gems  */ {  90,    NONE,  0,  0,     0 },
              {  95,    GEMS,  1,  1,     1 },
              { 100,     ART,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
  /* magic */ {  71,    NONE,  0,  0,     0 },
              {  95, MUNDANE,  1,  1,     1 },
              { 100,   MINOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  2 */ 
              {  13,    NONE,  0,  0,     0 },
              {  23,      CP,  1, 10,  1000 },
              {  43,      SP,  2, 10,   100 },
              {  95,      GP,  4, 10,    10 },
              { 100,      PP,  2,  8,    10 },
              {  81,    NONE,  0,  0,     0 },
              {  95,    GEMS,  1,  3,     1 },
              { 100,     ART,  1,  3,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  49,    NONE,  0,  0,     0 },
              {  85, MUNDANE,  1,  1,     1 },
              { 100,   MINOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  3 */
              {  11,    NONE,  0,  0,     0 },
              {  21,      CP,  2, 10,  1000 },
              {  41,      SP,  4,  8,   100 },
              {  95,      GP,  1,  4,   100 },
              { 100,      PP,  1, 10,    10 },
              {  70,    NONE,  0,  0,     0 },
              {  95,    GEMS,  1,  4,     1 },
              { 100,     ART,  1,  3,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  49,    NONE,  0,  0,     0 },
              {  79, MUNDANE,  1,  3,     1 },
              { 100,   MINOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  4 */
              {  11,    NONE,  0,  0,     0 },
              {  21,      CP,  3, 10,  1000 },
              {  41,      SP,  4, 12,  1000 },
              {  95,      GP,  1,  6,   100 },
              { 100,      PP,  1,  8,    10 },
              {  70,    NONE,  0,  0,     0 },
              {  95,    GEMS,  1,  4,     1 },
              { 100,     ART,  1,  3,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  42,    NONE,  0,  0,     0 },
              {  62, MUNDANE,  1,  4,     1 },
              { 100,   MINOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  5 */
              {  10,    NONE,  0,  0,     0 },
              {  19,      CP,  1,  4, 10000 },
              {  38,      SP,  1,  6,  1000 },
              {  95,      GP,  1,  8,   100 },
              { 100,      PP,  1, 10,    10 },
              {  60,    NONE,  0,  0,     0 },
              {  95,    GEMS,  1,  4,     1 },
              { 100,     ART,  1,  4,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  57,    NONE,  0,  0,     0 },
              {  67, MUNDANE,  1,  4,     1 },
              { 100,   MINOR,  1,  3,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  6 */ 
              {  10,    NONE,  0,  0,     0 },
              {  18,      CP,  1,  6, 10000 },
              {  37,      SP,  1,  8,  1000 },
              {  95,      GP,  1, 10,   100 },
              { 100,      PP,  1, 12,    10 },
              {  56,    NONE,  0,  0,     0 },
              {  92,    GEMS,  1,  4,     1 },
              { 100,     ART,  1,  4,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  54,    NONE,  0,  0,     0 },
              {  59, MUNDANE,  1,  4,     1 },
              {  99,   MINOR,  1,  3,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/*  7 */
              {  11,    NONE,  0,  0,     0 },
              {  18,      CP,  1, 10, 10000 },
              {  35,      SP,  1, 12,  1000 },
              {  93,      GP,  2,  6,   100 },
              { 100,      PP,  3,  4,    10 },
              {  48,    NONE,  0,  0,     0 },
              {  88,    GEMS,  1,  4,     1 },
              { 100,     ART,  1,  4,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  51,    NONE,  0,  0,     0 },
              {  97,   MINOR,  1,  3,     1 },
              { 100,  MEDIUM,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  8 */
              {  10,    NONE,  0,  0,     0 },
              {  15,      CP,  1, 12, 10000 },
              {  29,      SP,  2,  6,  1000 },
              {  87,      GP,  2,  8,   100 },
              { 100,      PP,  3,  6,    10 },
              {  45,    NONE,  0,  0,     0 },
              {  85,    GEMS,  1,  6,     1 },
              { 100,     ART,  1,  4,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  48,    NONE,  0,  0,     0 },
              {  96,   MINOR,  1,  4,     1 },
              { 100,  MEDIUM,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/*  9 */
              {  10,    NONE,  0,  0,     0 },
              {  15,      CP,  2,  6, 10000 },
              {  29,      SP,  2,  8,  1000 },
              {  85,      GP,  5,  4,   100 },
              { 100,      PP,  2, 12,    10 },
              {  40,    NONE,  0,  0,     0 },
              {  80,    GEMS,  1,  8,     1 },
              { 100,     ART,  1,  4,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  43,    NONE,  0,  0,     0 },
              {  91,   MINOR,  1,  4,     1 },
              { 100,  MEDIUM,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/* 10 */
              {  10,    NONE,  0,  0,     0 },
              {  24,      SP,  2, 10,  1000 },
              {  79,      GP,  6,  4,   100 },
              { 100,      PP,  5,  6,    10 },
              {   0,    NONE,  0,  0,     0 },
              {  35,    NONE,  0,  0,     0 },
              {  79,    GEMS,  1,  8,     1 },
              { 100,     ART,  1,  6,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  40,    NONE,  0,  0,     0 },
              {  88,   MINOR,  1,  4,     1 },
              {  99,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 11 */
              {   8,    NONE,  0,  0,     0 },
              {  14,      SP,  3, 10,  1000 },
              {  75,      GP,  4,  8,  1000 },
              { 100,      PP,  4, 10,    10 },
              {   0,    NONE,  0,  0,     0 },
              {  24,    NONE,  0,  0,     0 },
              {  74,    GEMS,  1, 10,     1 },
              { 100,     ART,  1,  6,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  31,    NONE,  0,  0,     0 },
              {  84,   MINOR,  1,  4,     1 },
              {  98,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 12 */
              {   8,    NONE,  0,  0,     0 },
              {  14,      SP,  3, 12,  1000 },
              {  75,      GP,  1,  4,  1000 },
              { 100,      PP,  1,  4,   100 },
              {   0,    NONE,  0,  0,     0 },
              {  17,    NONE,  0,  0,     0 },
              {  70,    GEMS,  1, 10,     1 },
              { 100,     ART,  1,  8,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  27,    NONE,  0,  0,     0 },
              {  82,   MINOR,  1,  6,     1 },
              {  97,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 13 */
              {   8,    NONE,  0,  0,     0 },
              {  75,      GP,  1,  4,  1000 },
              { 100,      PP,  1, 10,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  11,    NONE,  0,  0,     0 },
              {  66,    GEMS,  1, 12,     1 },
              { 100,     ART,  1, 10,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  19,    NONE,  0,  0,     0 },
              {  73,   MINOR,  1,  6,     1 },
              {  95,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 14 */
              {   8,    NONE,  0,  0,     0 },
              {  75,      GP,  1,  6,  1000 },
              { 100,      PP,  1, 12,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  11,    NONE,  0,  0,     0 },
              {  66,    GEMS,  2,  8,     1 },
              { 100,     ART,  2,  6,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  19,    NONE,  0,  0,     0 },
              {  58,   MINOR,  1,  6,     1 },
              {  92,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 15 */
              {   3,    NONE,  0,  0,     0 },
              {  74,      GP,  1,  8,  1000 },
              { 100,      PP,  3,  4,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   9,    NONE,  0,  0,     0 },
              {  65,    GEMS,  2, 10,     1 },
              { 100,     ART,  2,  8,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  11,    NONE,  0,  0,     0 },
              {  46,   MINOR,  1, 10,     1 },
              {  90,  MEDIUM,  1,  1,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 16 */
              {   3,    NONE,  0,  0,     0 },
              {  74,      GP,  1, 12,  1000 },
              { 100,      PP,  3,  4,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   7,    NONE,  0,  0,     0 },
              {  64,    GEMS,  4,  6,     1 },
              { 100,     ART,  2, 10,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  40,    NONE,  0,  0,     0 },
              {  46,   MINOR,  1, 10,     1 },
              {  90,  MEDIUM,  1,  3,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
/* 17 */
              {   3,    NONE,  0,  0,     0 },
              {  68,      GP,  3,  4,  1000 },
              { 100,      PP,  2, 10,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   4,    NONE,  0,  0,     0 },
              {  63,    GEMS,  4,  8,     1 },
              { 100,     ART,  3,  8,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  33,    NONE,  0,  0,     0 },
              {  83,  MEDIUM,  1,  3,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/* 18 */
              {   2,    NONE,  0,  0,     0 },
              {  65,      GP,  3,  6,  1000 },
              { 100,      PP,  5,  4,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   4,    NONE,  0,  0,     0 },
              {  54,    GEMS,  3, 12,     1 },
              { 100,     ART,  3, 10,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  24,    NONE,  0,  0,     0 },
              {  80,  MEDIUM,  1,  4,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/* 19 */
              {   2,    NONE,  0,  0,     0 },
              {  65,      GP,  3,  8,  1000 },
              { 100,      PP,  3, 10,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   3,    NONE,  0,  0,     0 },
              {  50,    GEMS,  6,  6,     1 },
              { 100,     ART,  6,  6,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   4,    NONE,  0,  0,     0 },
              {  70,  MEDIUM,  1,  4,     1 },
              { 100,   MAJOR,  1,  1,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/* 20 */
              {   2,    NONE,  0,  0,     0 },
              {  65,      GP,  4,  8,  1000 },
              { 100,      PP,  4, 10,   100 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {   2,    NONE,  0,  0,     0 },
              {  38,    GEMS,  4, 10,     1 },
              { 100,     ART,  7,  6,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
              {  25,    NONE,  0,  0,     0 },
              {  65,  MEDIUM,  1,  4,     1 },
              { 100,   MAJOR,  1,  3,     1 },
              {   0,    NONE,  0,  0,     0 },
              {   0,    NONE,  0,  0,     0 },
/* mundane/magic items only */
              {  25, MUNDANE,  1, 10,     1 },
              {  50,   MINOR,  1, 10,     1 },
              {  75,  MEDIUM,  1, 10,     1 },
              { 100,   MAJOR,  1, 10,     1 },
              {   0,    NONE,  0,  0,     0 },
              {  25, MUNDANE,  1, 10,     1 },
              {  50,   MINOR,  1, 10,     1 },
              {  75,  MEDIUM,  1, 10,     1 },
              { 100,   MAJOR,  1, 10,     1 },
              {   0,    NONE,  0,  0,     0 },
              {  25, MUNDANE,  1, 10,     1 },
              {  50,   MINOR,  1, 10,     1 },
              {  75,  MEDIUM,  1, 10,     1 },
              { 100,   MAJOR,  1, 10,     1 },
              {   0,    NONE,  0,  0,     0 }
};


char* gemNames1[] = { /* 4-16 gp */
  (char*) 10,
  "banded eye agate",
  "moss agate",
  "azurite",
  "blue quartz",
  "hematite",
  "lapis lazuli",
  "malachite",
  "obsidian",
  "rhodochrosite",
  "tiger eye turquoise"
};


char* gemNames2[] = { /* 20-40 gp */
  (char*) 16,
  "bloodstone",
  "carnelian",
  "chalcedony",
  "chysoprase",
  "citrine",
  "jasper iolite",
  "moonstone",
  "onyx",
  "peridot",
  "rock crystal",
  "sard",
  "sardonyx",
  "rose quartz",
  "smoky quartz",
  "star rose quartz",
  "zircon"
};


char* gemNames3[] = { /* 40-160 gp */
  (char*) 16,
  "amber",
  "amethyst",
  "chrysoberyl",
  "coral",
  "red garnet",
  "brown-green garnet",
  "jade",
  "jet",
  "white pearl",
  "golden pearl",
  "pink pearl",
  "silver pearl",
  "red spinel",
  "red-brown spinel",
  "deep green spinel",
  "tourmaline"
};


char* gemNames4[] = { /* 200-800 gp */
  (char*) 6,
  "alexandrite",
  "aquamarine",
  "violet garnet",
  "black pearl",
  "deep blue spinel",
  "golden yellow topaz"
};


char* gemNames5[] = { /* 400-1600 gp */
  (char*) 10,
  "emerald",
  "white opal",
  "black opal",
  "fire opal",
  "blue sapphire",
  "fiery yellow corundum",
  "rich purple corundum",
  "blue star sapphire",
  "black star sapphire",
  "star ruby"
};


char* gemNames6[] = { /* 2000-8000 gp */
  (char*) 7,
  "clearest bright green emerald",
  "blue-white diamond",
  "canary diamond",
  "pink diamond",
  "brown diamond",
  "blue diamond",
  "jacinth"
};


struct {
  int    dcap;
  int    dcount;
  int    dtype;
  int    dmul;
  char** names;
} gemTable[] = {
  {  25,  4,  4,    1, gemNames1 },
  {  50,  2,  4,   10, gemNames2 },
  {  70,  4,  4,   10, gemNames3 },
  {  90,  2,  4,  100, gemNames4 },
  {  99,  4,  4,  100, gemNames5 },
  { 100,  2,  4, 1000, gemNames6 },
  {   0,  0,  0,    0,  0 }
};


char* artNames1[] = { /* 1-100 gp */
  (char*) 17,
  "silver ewer",
  "carved bone statuette",
  "carved ivory statuette",
  "finely wrought small gold bracelet",
  "a scroll tube, carved in ivory with gold-plated metal end caps|a large bowl made of chased and pierced gold, worked in the design of leaping dragons and spear-armed warriors",
  "a 6-sided die, 1\" cube, of beaten gold stamped with holes for the pips",
  "a death mask of a noble, bearded male visage, made of beaten gold",
  "a monocle made from a polished glass lens in a gold frame, with hooked and pierced side-handle, but without ribbon or cord",
  "a quill pen case made of gold, held shut by a clasp, carved into the case is a scene of a scribe sitting on a stool amid stacks of parchment and writing in a tome",
  "a chain, 6' long, made of ornamental, gold-plated, triple-interlaced links which are both heavy and strong",
  "a jewel coffer of chased silver, depicting wooded scenes with birds in branches on back and sides, the top is graced by an engraving of a maiden combing her hair while looking into a pool at her reflection",
  "an ornamental skullcap of beaten gold cut in the shape of floral vines meeting, curling away and meeting again",
  "a statuette in solid gold of a flowing-haired maiden riding a rearing unicorn",
  "a statuette of carved ivory of an armored warrior leaning on a great broadsword",
  "a salt cellar made of ornately carved gold with a cork stopper in the bottom, the cellar is shaped like a slumbering gold dragon curled around a pile of gold",
  "a sword hilt made of intricately carved gold with an enameled painting of a hawk in flight in the center of the grip, the sword's pommel is fashioned into a hawk's head, the hilt is ornamental in nature, for it is too soft (solid gold, not plating on a stronger metal) for battle use",
  "a drinking jack of polished black-and-white horn with silver cap and base"
};


char* artNames2[] = { /* 30-180 gp */
  (char*) 8,
  "cloth of gold vestments",
  "black velvet mask with numerous citrines",
  "silver chalice with lapis lazuli gems",
  "a single electrum bracer worked in mock scales with four circular bosses about it, the center of each boss being a claw holding a small bloodstone",
  "a coffer, 6' x 1' x 2', with gold hinges and catch, made of carved ivory worked into a beveled top, with a battle scene covering the sides and top",
  "gold-plated corkscrew with a bloodstone set into each tip of the handle|a golden ball, dimpled from use but still brightly polished, 3-inch-diameter sphere of solid gold",
  "a tulip-shaped flagon with a heavy, bulbous base, carved of clear rock crystal polished glass-smooth, holding about one pint",
  "a gaming piece in the shape of a halfling, carved of ivory with two amber beads"
};


char* artNames3[] = { /* 100-600 gp */
  (char*) 7,
  "large well-done wool tapestry",
  "brass mug with jade inlays",
  "a book with steel-edged, beaten-gold covers, embossed and painted in fine, intricate repeating pattern borders, having as a central scene a warrior with a long sword battling a dragon, which he is grasping by the throat",
  "a golden flute, of delicate workmanship and mirror-smooth finish",
  "a peg-leg made of gilded wood and set with three large, cabochon-cut ovals of amber",
  "a platter of chased and pierced gold, delicate and easily damaged, but in good condition, oval-shaped, 2 feet long by 1 foot wide at widest point",
  "a ring of red gold, beaten into a long knuckle-coil to resemble a miniature snake coiling about the wearer's finger, with two tiny rubies set into its head as eyes"
};


char* artNames4[] = { /* 100-1000 gp */
  (char*) 5,
  "silver comb with moonstones",
  "silver-plated steel longsword with jet jewel in hilt",
  "a cork bottle stopper, fastened by an ornate wire twisting to a large, brilliant-cut topaz",
  "a sarcophagus/casket of bronze sheathed with electrum, worked in an effigy-shape of sleeping form, the face of which is fashioned of gold inlay and its eye sockets once held gems (which are now missing)",
  "a long sword of steel plated with silver, with a simple cross-hilted blade and a cabochon-cut piece of jet set into the center of the tang where the quillons meet it"
};


char* artNames5[] = { /* 200-1200 gp */
  (char*) 6,
  "carved harp of exotic wood with ivory inlay and zircon gems",
  "solid gold idol (10 lb.)",
  "a golden comb, its handle carved into a dragon's head with a ruby set as an eye",
  "a silver cloak pin, fashioned in the shape of a griffon's head (side view, facing right) with a ruby as the eye",
  "a fire-blackened oak staff shod with meteoritic steel at its base, the head of the staff is carved in the shape of a fanged serpent with two rubies as eyes",
  "a ring of carved and beaten gold in curlicue designs, showing a mock beast claw holding a large spherical aquamarine"
};


char* artNames6[] = { /* 300-1800 gp */
  (char*) 6,
  "gold dragon comb with red garnet eye",
  "gold and topaz bottle stopper cork",
  "ceremonial electrum dagger with a star ruby in the pommel",
  "an eye patch, sans chain or thong ties, shaped as a rhomboid of beaten gold set with a mock eye made of a sapphire, which is in turn surrounded by two crescents of polished moonstone, the eye patch is pierced in all four corners for ties",
  "a clappered bell made of carved, polished rose crystal, the bell and clapper are joined by fine gold wire.",
  "a half-mask of black velvet backed by leather, its lower edge trimmed with tiny teardrop citrines. There are 16 small citrines and six 6 slightly larger gems"
};


char* artNames7[] = { /* 400-2400 gp */
  (char*) 5,
  "eyepatch with mock eye of sapphire and moonstone",
  "fire opal pendant on a fine gold chain",
  "old masterpiece painting",
  "a pendant consisting of a fire opal with a gilded, fine, twisted-link neck chain",
  "a mantle with a black silk lining and a black velvet outer face that is adorned with beaded stars and geometric shapes, with one moonstone set into the center of each of the 36 stars; the mantle was created for a tall human and needs a pin to be worn correctly"
};


char* artNames8[] = { /* 500-3000 gp */
  (char*) 5,
  "embroidered silk and velvet mantle with numerous moonstones",
  "sapphire pendant on gold chain",
  "scroll tube made of carved ebony with silver plated end caps, each cap is inset with a large faceted emerald",
  "a crown of yellow gold with six slim spires, with a large zircon set at the base of five of the spires, and a gigantic (2 inches high) amethyst set at the base of the tallest (front) spire",
  "copper chamber pot, chased and embossed in a relief design of rampant, stylized dragon, with two emeralds as eyes"
};


char* artNames9[] = { /* 1000-4000 gp */
  (char*) 5,
  "emroidered and bejeweled glove",
  "jeweled anklet",
  "gold music box",
  "a bracelet made of 46 tiny white pearls strung together on gilded wire, fastened with a clever hook and loop clasp",
  "A leather glove for the right hand of a large human with embroidery along the back, making a curling tendril design utilizing beads and a few gemstones as flower buds, as follows: 8 white pearls, 1 peridot, 9 rock crystal \"tears\" (teardrop-cut, glassy polished), 1 opal"
};


char* artNames10[] = { /* 1000-6000 gp */
  (char*) 4,
  "goldet circlet with four aquamarines",
  "a string of small pink pearls (necklace)",
  "an anklet made of 12 tiny plates of gold linked with gilded wire and fastened by a hook and eye, from each wire loop save the fastening depends a wire-mounted gem, 11 in all, as follows: 4 white pearls, 6 violet garnets, 1 deep blue spinel",
  "a crown made of a thick, soft band of beaten gold, set with 4 large (2-inch-diameter, half-relief cabochon-cut) aquamarines"
};


char* artNames11[] = { /* 2000-8000 gp */
  (char*) 4,
  "jeweled gold crown",
  "jeweled electrum ring",
  "a cup of the thinnest beaten gold set with a lip-ring of 12 tiny emeralds, the whole item chased and embossed in rings of an abstract pattern (interlocked rings, vertical and horizontal bars interwoven with them)",
  "a single bracelet made of heavy gold and set with six small blue-white diamonds, the bracelet's edges cut in curlicues"
};


char* artNames12[] = { /* 2000-12000 gp */
  (char*) 4,
  "gold and ruby ring",
  "gold cup set with emeralds",
  "a garter consisting of nine gold coins linked with gold wire, from which hangs an electrum mesh fringe extending down in six triangles, each triangle ends in a claw-mounted, smoothly polished jacinth, the whole garter is backed with a (rotting) black leather band",
  "a 5-inch-diameter sphere of solid gold cut with a relief design of four sylphs amid clouds, holding up a mirror (a polished area on the sphere), the eyes of the sylphs are tiny cabochon-cut rubies"
};


struct {
  int    dcap;
  int    dcount;
  int    dtype;
  int    dmul;
  char** names;
} artTable[] = {
  {  10,  1, 10,   10, artNames1 },
  {  25,  3,  6,   10, artNames2 },
  {  40,  1,  6,  100, artNames3 },
  {  50,  1, 10,  100, artNames4 },
  {  60,  2,  6,  100, artNames5 },
  {  70,  3,  6,  100, artNames6 },
  {  80,  4,  6,  100, artNames7 },
  {  85,  5,  6,  100, artNames8 },
  {  90,  1,  4, 1000, artNames9 },
  {  95,  1,  6, 1000, artNames10 },
  {  99,  2,  4, 1000, artNames11 },
  { 100,  2,  6, 1000, artNames12 },
  {   0,  0,  0,    0, 0 }
};


struct {
  int   dcap;
  int   dcount;
  int   dtype;
  int   dmul;
  int   other;
  int   value;
  char* name;
} mundaneTable[] = {
  {   5,  1,  4,  1,     NONE,   20, "Alchemist's fire" },
  {  10,  2,  4,  1,     NONE,   10, "Acid" },
  {  12,  1,  4,  1,     NONE,   20, "Smokesticks" },
  {  18,  1,  4,  1,     NONE,   25, "Holy water" },
  {  20,  1,  4,  1,     NONE,   30, "Thunderstones" },
  {  22,  1,  1,  1,     NONE,  100, "Chain shirt" },
  {  27,  1,  4,  1,     NONE,   50, "Antitoxin" },
  {  29,  1,  4,  1,     NONE,   50, "Tanglefoot bag" },
  {  34,  1,  1,  1,     NONE,  175, "Masterwork studded leather" },
  {  39,  1,  1,  1, COMMONRW, -300, "" },
  {  43,  1,  1,  1,     NONE,  200, "Breastplate" },
  {  48,  1,  1,  1,     NONE,  250, "Banded mail" },
  {  66,  1,  1,  1, COMMONMW,    0, "Masterwork " },
  {  68,  1,  1,  1, UNCOMMNW,    0, "Masterwork " },
  {  73,  1,  1,  1, COMMONRW,    0, "Masterwork " },
  {  83,  1,  1,  1, COMMONRW, -300, "" },
  {  93,  1,  1,  1,     NONE,  600, "Half-plate" },
  { 100,  1,  1,  1,     NONE, 1500, "Full-plate" },
  {   0,  0,  0,  0, 0 }
};


struct {
  int minorDtop;
  int mediumDtop;
  int majorDtop;
  int type;
} magicTable[] = {
  {   4,  10,  10, ARMOR },
  {   9,  20,  20, WEAPONS },
  {  44,  30,  25, POTIONS },
  {  46,  40,  35, RINGS },
  {   0,  50,  45, RODS },
  {  81,  65,  55, SCROLLS },
  {   0,  68,  75, STAFFS },
  {  91,  83,  80, WANDS },
  { 100, 100, 100, WONDROUS },
  {   0,   0,   0, 0 }
};


struct {
  int minorDtop;
  int mediumDtop;
  int majorDtop;
  int type;
  int enhancement;
  int armor;
} armorTable[] = {
  {  60,   5,   0,  ENHANCED,  1, 0 },
  {  80,  10,   0,  ENHANCED,  1, 1 },
  {  85,  20,   0,  ENHANCED,  2, 0 },
  {  87,  30,   0,  ENHANCED,  2, 1 },
  {   0,  40,   8,  ENHANCED,  3, 0 },
  {   0,  50,  16,  ENHANCED,  3, 1 },
  {   0,  55,  27,  ENHANCED,  4, 0 },
  {   0,  57,  38,  ENHANCED,  4, 1 },
  {   0,   0,  49,  ENHANCED,  5, 0 },
  {   0,   0,  57,  ENHANCED,  5, 1 },
  {   0,  60,  60,  SPECIFIC,  0, 1 },
  {   0,  63,  63,  SPECIFIC,  0, 0 },
  { 100, 100, 100,   SPECIAL,  0, 0 },
  {   0,   0,   0,         0,  0, 0 }
};


struct {
  int   dtop;
  int   value;
  char* name;
} armorTypes[] = {
  {   1,  155, "padded armor" },
  {   2,  160, "leather armor" },
  {  12,  165, "hide armor" },
  {  27,  175, "studded leather armor" },
  {  42,  250, "chain shirt" },
  {  43,  200, "scale mail" },
  {  44,  300, "chain mail" },
  {  57,  350, "breastplate" },
  {  58,  350, "splint mail" },
  {  59,  400, "banded mail" },
  {  60,  750, "half-plate" },
  { 100, 1650, "full-plate" },
  {   0, 0 }
};


struct {
  int   dtop;
  int   value;
  char* name;
} shieldTypes[] = {
  {  10, 165, "buckler" },
  {  15, 153, "small wooden shield" },
  {  20, 159, "small steel shield" },
  {  30, 157, "large wooden shield" },
  {  95, 170, "large steel shield" },
  { 100, 180, "tower shield" },
  {   0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   plus;
  char* ability;
} armorSpecials[] = {
  {   0,   2,   2, 1, "light fortification" },
  {  30,   7,   8, 1, "glamers" },
  {  52,  19,   9, 1, "slickness" },
  {  74,  30,  11, 1, "shadow" },
  {  96,  49,  14, 1, "silent moves" },
  {   0,  50,  16, 2, "spell resistance (13)" },
  {   0,  60,  21, 3, "ghost touch" },
  {   0,   0,  23, 3, "invulnerability" },
  {  98,  65,  27, 3, "moderate fortification" },
  {   0,  66,  29, 3, "spell resistance (15)" },
  {   0,  71,  31, 3, "acid resistance" },
  {   0,  76,  41, 3, "cold resistance" },
  {   0,  81,  51, 3, "fire resistance" },
  {   0,  86,  61, 3, "lightning resistance" },
  {   0,  91,  64, 3, "sonic resistance" },
  {   0,  94,  67, 4, "spell resistance (17)" },
  {   0,  95,  69, 5, "etherealness" },
  {   0,  98,  72, 5, "heavy fortification" },
  {   0,   0,  74, 5, "spell resistance (19)" },
  { 100, 100, 100, 0, "-" },
  {   0,   0,   0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   plus;
  char* ability;
} shieldSpecials[] = {
  {  30,   0,   0, 1, "bashing" },
  {  50,   0,   0, 1, "blinding" },
  {  60,   0,   0, 1, "light fortification" },
  {  99,  10,   0, 2, "arrow deflection" },
  {   0,  16,  15, 2, "animation" },
  {   0,  20,  20, 2, "spell resistance (13)" },
  {   0,  25,  25, 3, "ghost touch" },
  {   0,  30,  35, 3, "moderate fortification" },
  {   0,  40,  38, 3, "acid resistance" },
  {   0,  50,  41, 3, "cold resistance" },
  {   0,  60,  44, 3, "fire resistance" },
  {   0,  70,  47, 3, "lightning resistance" },
  {   0,  80,  50, 3, "sonic resistance" },
  {   0,   0,  55, 3, "spell resistance (15)" },
  {   0,   0,  60, 4, "spell resistance (17)" },
  {   0,   0,  65, 5, "heavy fortification" },
  {   0,  90,  70, 5, "reflecting" },
  {   0,   0,  80, 5, "spell resistance (19)" },
  { 100, 100, 100, 0, "-" },
  {   0,   0,   0, 0 }
};


struct {
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char *armor;
} specificArmors[] = {
  {  10,   0,  1100, "mithril shirt" },
  {  25,   0,  4150, "elven chain armor" },
  {  35,   0,  5165, "rhino hide armor" },
  {  45,   0,  5350, "adamantine breastplate" },
  {  70,   0, 10500, "dwarven plate" },
  {  80,  10, 16650, "plate armor of the deep" },
  {  90,  40, 18900, "banded mail of luck" },
  { 100,  60, 21600, "breastplate of command" },
  {   0,  80, 25300, "celestial armor" },
  {   0, 100, 41650, "demon armor" },
  {   0,   0,     0, 0 }
};


struct {
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char *shield;
} specificShields[] = {
  {  10,   0,   257, "darkwood shield" },
  {  18,   0,  1020, "mithril large shield" },
  {  25,   0,  2170, "adamantine shield" },
  {  45,  20,  2670, "spined shield" },
  {  65,  40,  3153, "caster's shield" },
  {  90,  60,  9170, "lion's shield" },
  { 100,  80, 15159, "winged shield" },
  {   0, 100, 50170, "absorbing shield" },
  {   0,   0,     0, 0 }
};


struct {
  int minorDtop;
  int mediumDtop;
  int majorDtop;
  int type;
  int enhancement;
} weaponTable[] = {
  {  70,  10,   0, ENHANCED, 1 },
  {  85,  20,   0, ENHANCED, 2 },
  {   0,  58,  20, ENHANCED, 3 },
  {   0,  62,  38, ENHANCED, 4 },
  {   0,   0,  49, ENHANCED, 5 },
  {   0,  68,  63, SPECIFIC, 0 },
  { 100, 100, 100,  SPECIAL, 0 },
  {   0,   0,   0,        0, 0 }
};


struct {
  int dtop;
  int type;
} weaponTypes[] = {
  {  70, COMMONMW },
  {  80, UNCOMMNW },
  { 100, COMMONRW },
  {   0, 0 }
};


struct {
  int   dtop;
  int   type;
  int   value;
  char* name;
} commonMeleeWeapons[] = {
  {   4, PIERCING, 302, "dagger" },
  {  14, SLASHING, 320, "greataxe" },
  {  24, SLASHING, 350, "greatsword" },
  {  28, SLASHING, 302, "kama" },
  {  41, SLASHING, 315, "longsword" },
  {  45, BLUDGEON, 305, "light mace" },
  {  50, BLUDGEON, 312, "heavy mace" },
  {  54, BLUDGEON, 302, "nunchaku" },
  {  57, BLUDGEON, 600, "quarterstaff" },
  {  61, PIERCING, 320, "rapier" },
  {  66, SLASHING, 315, "scimitar" },
  {  70, PIERCING, 302, "shortspear" },
  {  74, PIERCING, 303, "siangham" },
  {  84, SLASHING, 335, "bastard sword" },
  {  89, PIERCING, 310, "short sword" },
  { 100, SLASHING, 330, "dwarven waraxe" },
  {   0,        0,   0, 0 }
};


struct {
  int   dtop;
  int   value;
  int   type;
  int   dtype;
  char* name;
} uncommonWeapons[] = {
  {   3, 660,  MELEE, SLASHING, "orc double axe" },
  {   7, 310,  MELEE, SLASHING, "battleaxe" },
  {  10, 325,  MELEE, PIERCING, "spiked chain" },
  {  12, 300,  MELEE, BLUDGEON, "club" },
  {  16, 400, RANGED, PIERCING, "hand crossbow" },
  {  19, 550, RANGED, PIERCING, "repeating crossbow" },
  {  21, 302,  MELEE, PIERCING, "punching dagger" },
  {  23, 375,  MELEE, SLASHING, "falchion" },
  {  26, 690,  MELEE, BLUDGEON, "dire flail" },
  {  31, 315,  MELEE, BLUDGEON, "heavy flail" },
  {  35, 308,  MELEE, BLUDGEON, "light flail" },
  {  37, 302,  MELEE, BLUDGEON, "gauntlet" },
  {  39, 305,  MELEE, PIERCING, "spiked gauntlet" },
  {  41, 308,  MELEE, SLASHING, "glaive" },
  {  43, 305,  MELEE, BLUDGEON, "greatclub" },
  {  45, 309,  MELEE, SLASHING, "guisarme" },
  {  48, 310,  MELEE, PIERCING | SLASHING, "halberd" },
  {  51, 301,  MELEE, PIERCING, "halfspear" },
  {  54, 620,  MELEE, PIERCING | BLUDGEON, "gnome hooked hammer" },
  {  56, 301,  MELEE, BLUDGEON, "light hammer" },
  {  58, 306,  MELEE, SLASHING, "handaxe" },
  {  61, 308,  MELEE, SLASHING, "kukri" },
  {  63, 310,  MELEE, PIERCING, "heavy lance" },
  {  65, 306,  MELEE, PIERCING, "light lance" },
  {  67, 305,  MELEE, PIERCING, "longspear" },
  {  70, 308,  MELEE, PIERCING | BLUDGEON, "morningstar" },
  {  72, 320, RANGED, BLUDGEON, "net" },
  {  74, 308,  MELEE, PIERCING, "heavy pick" },
  {  76, 304,  MELEE, PIERCING, "light pick" },
  {  78, 310,  MELEE, PIERCING, "ranseur" },
  {  80, 301,  MELEE, BLUDGEON, "sap" },
  {  82, 318,  MELEE, SLASHING | PIERCING, "scythe" },
  {  84, 301, RANGED, PIERCING, "shuriken" },
  {  86, 306,  MELEE, SLASHING, "sickle" },
  {  89, 700,  MELEE, SLASHING, "two-bladed sword" },
  {  91, 315,  MELEE, PIERCING, "trident" },
  {  94, 650,  MELEE, SLASHING | PIERCING, "dwarven urgosh" },
  {  97, 312,  MELEE, BLUDGEON, "warhammer" },
  { 100, 301,  MELEE, SLASHING, "whip" },
  {   0,   0,      0,        0, 0 }
};


struct {
  int   dtop;
  int   value;
  int   type;
  char* name;
} commonRangedWeapons[] = {
  {  10,   0, BLUDGEON, "ammunition" },
  {  50, 350, PIERCING, "arrows (50)" },
  {  80, 350, PIERCING, "crossbow bolts (50)" },
  { 100, 350, BLUDGEON, "sling bullets (50)" },
  { 115, 308, SLASHING, "throwing axe" },
  { 125, 350, PIERCING, "heavy crossbow" },
  { 135, 335, PIERCING, "light crossbow" },
  { 139, 300, PIERCING, "dart" },
  { 141, 301, PIERCING, "javelin" },
  { 146, 330, PIERCING, "shortbow" },
  { 151, 375, PIERCING, "composite shortbow" },
  { 156, 450, PIERCING, "mighty composite shortbow (+1 Str bonus)" },
  { 161, 525, PIERCING, "mighty composite shortbow (+2 Str bonus)" },
  { 165, 300, PIERCING, "sling" },
  { 175, 375, PIERCING, "longbow" },
  { 180, 400, PIERCING, "composite longbow" },
  { 185, 500, PIERCING, "mighty composite longbow (+1 Str bonus)" },
  { 190, 600, PIERCING, "mighty composite longbow (+2 Str bonus)" },
  { 195, 700, PIERCING, "mighty composite longbow (+3 Str bonus)" },
  { 200, 800, PIERCING, "mighty composite longbow (+4 Str bonus)" },
  {   0,   0,        0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   other;
  int   plus;
  int   restrict;
  int   standing;
  char* name;
} meleeSpecials[] = {
  {  15,  10,   0, NONE, 1, ANY, NONE, "defending" },
  {  25,  15,   3, NONE, 1, ANY, NONE, "flaming" },
  {  35,  20,   6, NONE, 1, ANY, NONE, "frost" },
  {  45,  25,   9, NONE, 1, ANY, NONE, "shocking" },
  {  55,  30,  12, NONE, 1, ANY, NONE, "ghost touch" },
  {  70,  40,   0, NONE, 1, SLASHING | PIERCING, NONE, "keen" },
  {  80,  50,  17, NONE, 1, ANY, NONE, "mighty cleaving" },
  {  89,  51,  19, NONE, 1, ANY, NONE, "spell storing" },
  {  99,  56,  21, NONE, 1, ANY, NONE, "throwing" },
  {   0,  59,  26, BANE, 2, ANY, NONE, "bane of " },
  {   0,  62,  29, NONE, 2, BLUDGEON, NONE, "disruption" },
  {   0,  65,  33, NONE, 2, ANY, NONE, "flaming burst" },
  {   0,  68,  37, NONE, 2, ANY, NONE, "icy burst" },
  {   0,  71,  41, NONE, 2, ANY, NONE, "shocking burst" },
  {   0,  76,  44, NONE, 2, ANY, NONE, "thundering" },
  {   0,  79,  47, NONE, 2, ANY, NONE, "wounding" },
  {   0,  82,  52, NONE, 2, ANY, opGOOD, "holiness" },
  {   0,  85,  57, NONE, 2, ANY, opEVIL, "unholiness" },
  {   0,  88,  62, NONE, 2, ANY, opLAWFUL, "lawfulness" },
  {   0,  91,  67, NONE, 2, ANY, opCHAOTIC, "chaos" },
  {   0,  92,  71, NONE, 4, ANY, NONE, "brilliant energy" },
  {   0,  93,  73, NONE, 4, ANY, NONE, "dancing" },
  {   0,  95,  76, NONE, 4, ANY, NONE, "speed" },
  {   0,   0,  80, NONE, 5, SLASHING, NONE, "vorpal" },
  { 100, 100, 100, NONE, 0, ANY, NONE, "-" },
  {   0,   0,   0,    0, 0,   0,    0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   other;
  int   plus;
  int   standing;
  char* name;
} rangedSpecials[] = {
  {  20,  15,   0, NONE, 1, NONE, "returning" },
  {  40,  30,   0, NONE, 1, NONE, "distance" },
  {  60,  35,  10, NONE, 1, NONE, "flaming" },
  {  80,  40,  20, NONE, 1, NONE, "shocking" },
  { 100,  45,  30, NONE, 1, NONE, "frost" },
  {   0,  50,  40, NONE, 2, NONE, "flaming burst" },
  {   0,  55,  50, NONE, 2, NONE, "icy burst" },
  {   0,  60,  60, NONE, 2, NONE, "shocking burst" },
  {   0,  66,  65, BANE, 2, NONE, "bane of " },
  {   0,  74,  70, NONE, 2, opGOOD, "holiness" },
  {   0,  82,  75, NONE, 2, opEVIL, "unholiness" },
  {   0,  90,  80, NONE, 2, opLAWFUL, "lawfulness" },
  {   0,  98,  85, NONE, 2, opCHAOTIC, "chaos" },
  {   0,   0,  90, NONE, 4, NONE, "speed" },
  {   0,   0,  97, NONE, 4, NONE, "brilliant energy" },
  {   0, 100, 100, NONE, 0, NONE, "-" },
  {   0,   0,   0,    0, 0,    0, 0 }
};


struct {
  int   dtop;
  int   standing;
  char* name;
} baneTable[] = {
  {   5, NONE, "aberrations" },
  {   8, NONE, "animals" },
  {  13, NONE, "beasts" },
  {  20, NONE, "constructs" },
  {  25, NONE, "dragons" },
  {  30, NONE, "elementals" },
  {  35, NONE, "fey" },
  {  40, NONE, "giants" },
  {  45, NONE, "magical beasts" },
  {  50, NONE, "monstrous humanoids" },
  {  53, NONE, "oozes" },
  {  58, opLAWFUL, "chaotic outsiders" },
  {  65, opGOOD, "evil outsiders" },
  {  70, opEVIL, "good outsiders" },
  {  75, opCHAOTIC, "lawful outsiders" },
  {  77, NONE, "plants" },
  {  85, NONE, "shapechangers" },
  {  92, NONE, "undead" },
  {  94, NONE, "vermin" },
  { 100, NONE, "one humanoid subtype" },
  {   0, NONE, 0 }
};


struct {
  int   mediumDtop;
  int   majorDtop;
  int   value;
  int   other;
  char* name;
} specificWeapons[] = {
  {  20,   0,    132, NONE, "sleep arrow" },
  {  40,   0,    257, NONE, "screaming bolt" },
  {  55,   4,    751, NONE, "javelin of lightning" },
  {  65,   9,   2282, BANE, "slaying arrow" },
  {  70,   0,   3302, NONE, "adamantine dagger" },
  {  72,  11,   3815, NONE, "trident of fish command" },
  {   0,  13,   4057, BANE, "greater slaying arrow" },
  {  74,  17,   9302, NONE, "dagger of venom" },
  {  76,  20,   9310, NONE, "adamantine battleaxe" },
  {  79,  25,   9815, NONE, "trident of warning" },
  {  82,  30,  10302, NONE, "assassin's dagger" },
  {  85,  35,  15310, NONE, "sword of subtlety" },
  {  88,  40,  17812, NONE, "mace of terror" },
  {  91,  45,  25315, NONE, "nine lives stealer" },
  {  94,  50,  27875, NONE, "oathbow" },
  {  96,  55,  30315, NONE, "sword of life stealing" },
  {  98,  60,  32315, NONE, "flame tongue" },
  { 100,  66,  40320, NONE, "life-drinker" },
  {   0,  72,  49350, NONE, "frost brand" },
  {   0,  78,  50320, NONE, "rapier of puncturing" },
  {   0,  81,  50335, NONE, "sun blade" },
  {   0,  83,  52315, NONE, "sword of the planes" },
  {   0,  85,  55815, NONE, "sylvan scimitar" },
  {   0,  87,  60312, NONE, "dwarven thrower" },
  {   0,  90,  75312, NONE, "mace of smiting" },
  {   0,  96, 120315, NONE, "holy avenger" },
  {   0, 100, 170560, NONE, "luck blade" },
  {   0,   0,      0,    0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char* name;
} potionTable[] = {
  {   5,   0,   0,  50, "jump" },
  {  10,   0,   0,  50, "spider climb" },
  {  19,   0,   0,  50, "cure light wounds" },
  {  20,   1,   0, 150, "love" },
  {  24,   2,   0, 150, "vision" },
  {  28,   3,   0, 150, "swimming" },
  {  32,   4,   0, 150, "hiding" },
  {  36,   5,   0, 150, "sneaking" },
  {  37,   6,   0, 150, "oil of timelessness" },
  {  42,   7,   0, 250, "reduce (at 5th level)" },
  {  47,   8,   0, 250, "enlarge (at 5th level)" },
  {  50,   9,   0, 300, "speak with animals" },
  {  53,  10,   1, 300, "clairaudience/clairvoyance" },
  {  56,  12,   2, 300, "charisma" },
  {  59,  14,   3, 300, "intelligence" },
  {  62,  16,   4, 300, "wisdom" },
  {  65,  18,   5, 300, "alter self" },
  {  68,  21,   7, 300, "blur" },
  {  71,  24,   8, 300, "darkvision" },
  {  74,  26,   9, 300, "ghoul touch" },
  {  77,  29,  10, 300, "delay poison" },
  {  80,  32,  13, 300, "endurance" },
  {  83,  40,  16, 300, "cure moderate wounds" },
  {  86,  45,  19, 300, "detect thoughts" },
  {  89,  50,  22, 300, "levitate" },
  {  91,  55,  25, 300, "aid" },
  {  93,  60,  30, 300, "invisibility" },
  {  94,  65,  35, 300, "lesser restoration" },
  {  95,  70,  40, 300, "cat's grace" },
  {  96,  75,  45, 300, "bull's strength" },
  {  97,  77,  46, 500, "truth" },
  {  98,  79,  47, 500, "glibness" },
  {  99,  84,  49, 750, "nondetection" },
  { 100,  87,  51, 750, "tongues" },
  {   0,  91,  53, 750, "water breathing" },
  {   0,  92,  55, 750, "remove paralysis" },
  {   0,  93,  57, 750, "remove blindness/deafness" },
  {   0,  94,  59, 750, "remove disease" },
  {   0,  96,  69, 750, "neutralize poison" },
  {   0,  97,  73, 750, "cure serious wounds" },
  {   0,  98,  75, 750, "fly" },
  {   0,   0,  77, 750, "protection from elements (cold)" },
  {   0,   0,  79, 750, "protection from elements (electricity)" },
  {   0,   0,  83, 750, "protection from elements (fire)" },
  {   0,   0,  85, 750, "protection from elements (acid)" },
  {   0,   0,  87, 750, "protection from elements (sonic)" },
  {   0,   0,  90, 750, "haste" },
  {   0,   0,  93, 750, "gaseous form" },
  {   0,   0,  95, 900, "oil of slipperiness" },
  {   0, 100,  98, 900, "heroism" },
  {   0,   0, 100, 900, "fire breath" },
  {   0,   0,   0,   0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   dcount;
  int   dtype;
  int   value;
  char* name;
} ringTable[] = {
  {   5,   0,   0,  0,  0,   2000, "climbing" },
  {  10,   0,   0,  0,  0,   2000, "jumping" },
  {  25,   0,   0,  0,  0,   2000, "protection +1" },
  {  30,   0,   0,  0,  0,   2100, "warmth" },
  {  40,   0,   0,  0,  0,   2200, "feather falling" },
  {  45,   0,   0,  0,  0,   2300, "swimming" },
  {  50,   0,   0,  0,  0,   2500, "sustenance" },
  {  55,   5,   0,  0,  0,   4000, "counterspells" },
  {  60,  10,   0,  0,  0,   8000, "mind shielding" },
  {  70,  20,   0,  0,  0,   8000, "protection +2" },
  {  75,  25,   0,  0,  0,   8500, "force shield" },
  {  80,  30,   1,  1, 50,   8600, "the ram" },
  {  85,  35,   2,  0,  0,   9500, "animal friendship" },
  {  90,  40,   3,  0,  0,  12000, "chameleon power" },
  {  95,  45,   4,  0,  0,  15000, "water walking" },
  { 100,  50,   6,  0,  0,  16000, "minor elemental resistance" },
  {   0,  60,  10,  0,  0,  18000, "protection +3" },
  {   0,  70,  15,  0,  0,  20000, "invisibility" },
  {   0,  75,  20,  0,  0,  20000, "wizardry (I)" },
  {   0,  80,  25,  0,  0,  24000, "major elemental resistance" },
  {   0,  82,  30,  0,  0,  25000, "x-ray vision" },
  {   0,  84,  35,  0,  0,  25000, "evasion" },
  {   0,  86,  40,  0,  0,  30000, "blinking" },
  {   0,  88,  45,  0,  0,  32000, "protection +4" },
  {   0,  90,  50,  0,  0,  40000, "wizardry (II)" },
  {   0,  92,  55,  0,  0,  40000, "freedom of movement" },
  {   0,  94,  60,  0,  0,  50000, "friend shield" },
  {   0,  96,  65,  0,  0,  50000, "protection +5" },
  {   0,  98,  70,  0,  0,  50000, "shooting stars" },
  {   0,  99,  75,  0,  0,  75000, "telekinesis" },
  {   0, 100,  80,  0,  0,  80000, "wizardry (III)" },
  {   0,   0,  84,  0,  0,  90000, "spell storing" },
  {   0,   0,  87,  0,  0,  90000, "regeneration" },
  {   0,   0,  89,  1,  3,  97950, "three wishes" },
  {   0,   0,  92,  0,  0, 100000, "wizardry (IV)" },
  {   0,   0,  94,  0,  0, 125000, "djinni calling" },
  {   0,   0,  96,  0,  0, 150000, "spell turning" },
  {   0,   0,  97,  0,  0, 200000, "air elemental command" },
  {   0,   0,  98,  0,  0, 200000, "earth elemental command" },
  {   0,   0,  99,  0,  0, 200000, "fire elemental command" },
  {   0,   0, 100,  0,  0, 200000, "water elemental command" },
  {   0,   0,   0,  0,  0,      0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char* name;
} rodTable[] = {
  {   0,   6,   0,  7500, "immovable" },
  {   0,  12,   0, 10500, "metal and mineral detection" },
  {   0,  20,   5, 11000, "cancellation" },
  {   0,  25,  10, 12000, "wonder" },
  {   0,  29,  15, 13000, "python" },
  {   0,  34,  20, 15000, "flame extinguishing" },
  {   0,  40,  27, 17000, "withering" },
  {   0,  45,  33, 19000, "viper" },
  {   0,  52,  40, 23000, "thunder and lightning" },
  {   0,  60,  50, 23500, "enemy detection" },
  {   0,  68,  55, 25000, "splendor" },
  {   0,  78,  65, 35000, "negation" },
  {   0,  90,  80, 40000, "flailing" },
  {   0,  96,  85, 50000, "absorption" },
  {   0,  99,  90, 60000, "rulership" },
  {   0, 100,  94, 61000, "security" },
  {   0,   0,  98, 70000, "lordly might" },
  {   0,   0, 100, 72000, "alertness" },
  {   0,   0,   0,     0, 0 }
};


struct {
  int dtop;
  int type;
} scrollTypes[] = {
  {  70, ARCANE },
  { 100, DIVINE },
  {   0, 0 }
};


struct {
  int minorDtop;
  int mediumDtop;
  int majorDtop;
  int level;
  int caster;
} spellLevels[] = {
  {  50,   0,   0, 1,  1 },
  {  95,   5,   0, 2,  3 },
  { 100,  65,   0, 3,  5 },
  {   0,  95,   5, 4,  7 },
  {   0, 100,  50, 5,  9 },
  {   0,   0,  70, 6, 11 },
  {   0,   0,  85, 7, 13 },
  {   0,   0,  95, 8, 15 },
  {   0,   0, 100, 9, 17 },
  {   0,   0,   0, 0,  0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char* name;
} staffTable[] = {
  {    0,  10,   0,   6500, "size alteration" },
  {    0,  20,   5,  12000, "charming" },
  {    0,  30,  15,  33000, "healing" },
  {    0,  40,  30,  29000, "fire" },
  {    0,  50,  40,  20000, "swarming insects" },
  {    0,  60,  50,  70000, "frost" },
  {    0,  70,  60,  85000, "earth and stone" },
  {    0,  80,  70,  80000, "defense" },
  {    0,  89,  80,  90000, "woodlands" },
  {    0,  95,  90, 130000, "life" },
  {    0, 100,  96, 180000, "passage" },
  {    0,   0, 100, 200000, "power" },
  {    0,   0,   0,      0, 0 }
};


struct {
  int   minorDtop;
  int   mediumDtop;
  int   majorDtop;
  int   value;
  char* name;
} wandTable[] = {
  {   5,   0,   0,   375, "detect magic" },
  {  10,   0,   0,   375, "light" },
  {  15,   0,   0,   750, "detect secret doors" },
  {  20,   0,   0,   750, "color spray" },
  {  25,   0,   0,   750, "burning hands" },
  {  30,   3,   0,   750, "charm person" },
  {  35,   6,   0,   750, "enlarge" },
  {  40,   9,   0,   750, "magic missile (1st-level caster)" },
  {  45,  12,   0,   750, "shocking grasp" },
  {  50,  15,   0,   750, "summon monster I" },
  {  55,  18,   0,   750, "cure light wounds" },
  {  58,  21,   0,  2250, "magic missile (3rd-level caster)" },
  {  59,  23,   2,  3750, "magic missile (5th-level caster)" },
  {  63,  26,   3,  4500, "levitate" },
  {  66,  29,   4,  4500, "summon monster II" },
  {  69,  32,   5,  4500, "silence" },
  {  72,  35,   6,  4500, "knock" },
  {  75,  38,   7,  4500, "daylight" },
  {  78,  41,  10,  4500, "invisibility" },
  {  81,  44,  12,  4500, "shatter" },
  {  84,  48,  15,  4500, "bull's strength" },
  {  87,  50,  17,  4500, "mirror image" },
  {  90,  53,  19,  4500, "ghoul touch" },
  {  93,  60,  21,  4500, "cure moderate wounds" },
  {  96,  63,  23,  4500, "hold person" },
  {  98,  66,  25,  4500, "Melf's acid arrow" },
  {  99,  69,  27,  4500, "web" },
  { 100,  71,  30,  4500, "darkness" },
  {   0,  72,  33,  5250, "magic missile (7th-level caster)" },
  {   0,   0,  36,  6750, "magic missile (9th-level caster)" },
  {   0,  75,  39, 11250, "fireball (3rd-level caster)" },
  {   0,  80,  41, 11250, "lightning bolt (3rd-level caster)" },
  {   0,  82,  43, 11250, "summon monster III" },
  {   0,  84,  45, 11250, "keen edge" },
  {   0,  86,  47, 11250, "major image" },
  {   0,  88,  49, 11250, "slow" },
  {   0,  90,  51, 11250, "suggestion" },
  {   0,  92,  53, 11250, "dispel magic" },
  {   0,  94,  55, 11250, "cure serious wounds" },
  {   0,  95,  57, 11250, "contagion" },
  {   0,  96,  58, 11250, "charm person" },
  {   0,  97,  59, 13500, "fireball (6th-level caster)" },
  {   0,  99,  61, 13500, "searing light (6th-level caster)" },
  {   0, 100,  63, 13500, "lightning bolt (6th-level caster)" },
  {   0,   0,  65, 18000, "fireball (8th-level caster)" },
  {   0,   0,  67, 18000, "lightning bolt (8th-level caster)" },
  {   0,   0,  69, 21000, "charm monster" },
  {   0,   0,  71, 21000, "fear" },
  {   0,   0,  73, 21000, "improved invisibility" },
  {   0,   0,  75, 21000, "polymorph self" },
  {   0,   0,  77, 21000, "polymorph other" },
  {   0,   0,  79, 21000, "ice storm" },
  {   0,   0,  81, 21000, "summon monster IV" },
  {   0,   0,  83, 21000, "wall of ice" },
  {   0,   0,  84, 21000, "wall of fire" },
  {   0,   0,  85, 21000, "ray of enfeeblement (heightened to 4th-level spell)" },
  {   0,   0,  86, 21000, "poison" },
  {   0,   0,  87, 21000, "suggestion (heightened to 4th-level spell)" },
  {   0,   0,  89, 21000, "neutralize poison" },
  {   0,   0,  90, 21000, "inflict critical wounds" },
  {   0,   0,  92, 21000, "cure critical wounds" },
  {   0,   0,  93, 21100, "restoration" },
  {   0,   0,  94, 22500, "fireball (10th-level caster)" },
  {   0,   0,  95, 22500, "lightning bolt (10th-level caster)" },
  {   0,   0,  96, 24000, "holy smite (8th-level caster)" },
  {   0,   0,  97, 24000, "chaos hammer (8th-level caster)" },
  {   0,   0,  98, 24000, "unholy blight (8th-level caster)" },
  {   0,   0,  99, 24000, "order's wrath (8th-level caster)" },
  {   0,   0, 100, 37700, "stoneskin" },
  {   0,   0,   0,     0, 0 }
};


typedef struct {
  int   dtop;
  int   value;
  char* name;
} WONDROUSITEM;

WONDROUSITEM minorItems[] = {
  {   1,   25, "dull gray Ioun stone" },
  {   2,   50, "Quaal's feather token (anchor)" },
  {   3,   90, "everburning torch" },
  {   4,  100, "Quaal's feather token (tree)" },
  {   5,  200, "Quaal's feather token (fan)" },
  {   6,  250, "dust of tracelessness" },
  {   7,  300, "Quaal's feather token (bird)" },
  {   8,  450, "Quaal's feather token (swan boat)" },
  {   9,  500, "dust of illusion" },
  {  10,  500, "necklace of prayer beads (blessing)" },
  {  11,  500, "Quaal's feather token (whip)" },
  {  12,  800, "golembane scarab (flesh)" },
  {  13,  900, "gray bag of tricks" },
  {  14,  900, "dust of dryness" },
  {  15, 1000, "bracers of armor (+1)" },
  {  16, 1000, "cloak of resistance (+1)" },
  {  17, 1000, "eyes of the eagle" },
  {  18, 1000, "goggles of minute seeing" },
  {  19, 1000, "hand of the mage" },
  {  20, 1000, "pearl of power (1st-level spell)" },
  {  21, 1000, "phylactery of faithfulness" },
  {  22, 1000, "golembane scarab (clay)" },
  {  23, 1000, "stone of alarm" },
  {  24, 1150, "pipes of the sewers" },
  {  25, 1200, "golembane scarab (stone)" },
  {  26, 1500, "brooch of shielding" },
  {  27, 1600, "golembane scarab (iron)" },
  {  28, 1650, "necklace of fireballs (Type I)" },
  {  29, 1800, "pipes of sounding" },
  {  30, 1800, "quiver of Ehlonna" },
  {  31, 1800, "golembane scarab (flesh and clay)" },
  {  32, 1900, "horseshoes of speed" },
  {  33, 2000, "amulet of natural armor (+1)" },
  {  34, 2000, "bead of force" },
  {  35, 2000, "boots of elvenkind" },
  {  36, 2000, "cloak of elvenkind" },
  {  37, 2000, "hat of disguise" },
  {  38, 2000, "Heward's handy haversack" },
  {  39, 2000, "horn of fog" },
  {  40, 2000, "slippers of spider climbing" },
  {  41, 2000, "universal solvent" },
  {  42, 2000, "vest of escape" },
  {  43, 2100, "dust of appearance" },
  {  44, 2200, "glove of storing" },
  {  45, 2400, "sovereign glue" },
  {  46, 2500, "candle of truth" },
  {  47, 2500, "bag of holding (bag 1)" },
  {  48, 2500, "boots of the winterlands" },
  {  49, 2500, "boots of striding and springing" },
  {  50, 2500, "golembane scarab (any golem)" },
  {  51, 2600, "helm of comprehending languages and reading magic" },
  {  52, 2700, "necklace of fireballs (Type II)" },
  {  53, 3000, "rust bag of tricks" },
  {  54, 3000, "chime of opening" },
  {  55, 3000, "rope of climbing" },
  {  56, 3000, "horseshoes of a zephyr" },
  {  57, 3500, "dust of disappearance" },
  {  58, 3500, "lens of detection" },
  {  59, 3800, "figurine of wondrous power (silver raven)" },
  {  60, 4000, "bracers of armor (+2)" },
  {  61, 4000, "cloak of resistance (+2)" },
  {  62, 4000, "gloves of arrow snaring" },
  {  63, 4000, "dusty rose prism Ioun stone" },
  {  64, 4000, "Keoghtom's ointment" },
  {  65, 4000, "pearl of power (2nd-level spell)" },
  {  66, 4000, "periapt of proof against poison" },
  {  67, 4000, "stone salve" },
  {  68, 4000, "gauntlets of ogre power" },
  {  69, 4000, "bracers of health (+2)" },
  {  70, 4000, "gloves of Dexterity (+2)" },
  {  71, 4000, "headband of intellect (+2)" },
  {  72, 4000, "periapt of Wisdom (+2)" },
  {  73, 4000, "cloak of Charisma (+2)" },
  {  74, 4350, "necklace of fireballs (Type III)" },
  {  75, 4500, "circlet of persuasion" },
  {  76, 4550, "bracelet of friends" },
  {  77, 4900, "incense of meditation" },
  {  78, 5000, "bag of holding (bag 2)" },
  {  79, 5000, "clear spindle Ioun stone" },
  {  80, 5000, "necklace of prayer beads (karma)" },
  {  81, 5100, "bracers of archery" },
  {  82, 5200, "eversmoking bottle" },
  {  83, 5400, "necklace of fireballs (Type IV)" },
  {  84, 5500, "Murlynd's spoon" },
  {  85, 5500, "Nolzur's marvelous pigments" },
  {  86, 5500, "wind fan" },
  {  87, 5500, "wings of flying" },
  {  88, 5800, "druid's vestment" },
  {  89, 6000, "cloak of arachnida" },
  {  90, 6000, "gloves of swimming and climbing" },
  {  91, 6000, "horn of goodness/evil" },
  {  92, 6150, "necklace of fireballs (Type V)" },
  {  93, 6300, "tan bag of tricks" },
  {  94, 6480, "minor circlet of blasting" },
  {  95, 6500, "pipes of haunting" },
  {  96, 7000, "robe of useful items" },
  {  97, 7200, "hand of glory" },
  {  98, 7400, "bag of holding (bag 3)" },
  {  99, 2000, "boots of elvenkind" },
  { 100,  900, "gray bag of tricks" },
  {   0,    0, 0 }
};


WONDROUSITEM mediumItems[] = {
  {   1,  7500, "boots of levitation" },
  {   2,  7500, "harp of charming" },
  {   3,  7500, "periapt of health" },
  {   4,  7800, "candle of invocation" },
  {   5,  8000, "amulet of natural armor (+2)" },
  {   6,  8000, "boots of speed" },
  {   7,  8000, "dark blue rhomboid Ioun stone" },
  {   8,  8000, "deep red sphere Ioun stone" },
  {   9,  8000, "incandescent blue sphere Ioun stone" },
  {  10,  8000, "pale blue rhomboid Ioun stone" },
  {  11,  8000, "pink rhomboid Ioun stone" },
  {  12,  8000, "pink and green sphere Ioun stone" },
  {  13,  8000, "scarlet and blue sphere Ioun stone" },
  {  14,  8000, "goggles of night" },
  {  15,  8100, "necklace of fireballs (Type VI)" },
  {  16,  9000, "monk's belt" },
  {  17,  9000, "bracers or armor (+3)" },
  {  18,  9000, "cloak of resistance (+3)" },
  {  19,  9000, "decanter of endless water" },
  {  20,  9000, "pearl of power (3rd-level spell)" },
  {  21,  9000, "talisman of the sphere" },
  {  22,  9100, "figurine of wondrous power (serpentine owl)" },
  {  23,  9150, "necklace of fireball (Type VII)" },
  {  24,  9200, "deck of illusions" },
  {  25,  9500, "Boccob's blessed book" },
  {  26, 10000, "bag of holding (bag 4)" },
  {  27, 10000, "figurine of wondrous power (bronze griffon)" },
  {  28, 10000, "figurine of wondrous power (ebony fly)" },
  {  29, 10000, "necklace of prayer beads (healing)" },
  {  30, 10000, "robe of blending" },
  {  31, 10000, "stone of good luck (luckstone)" },
  {  32, 10000, "stone horse (courser)" },
  {  33, 10500, "folding boat" },
  {  34, 11000, "amulet of undead turning" },
  {  35, 11500, "gauntlet of rust" },
  {  36, 12000, "winged boots" },
  {  37, 12000, "horn of blasting" },
  {  38, 12000, "vibrant purple prism Ioun stone" },
  {  39, 12000, "medallion of thoughts" },
  {  40, 12000, "pipes of pain" },
  {  41, 12960, "cape of mountebank" },
  {  42, 13000, "lyre of building" },
  {  43, 14000, "portable hole" },
  {  44, 14500, "bottle of air" },
  {  45, 14800, "stone horse (destrier)" },
  {  46, 14900, "belt of dwarvenkind" },
  {  47, 15000, "iridescent spindle Ioun stone" },
  {  48, 15000, "necklace of prayer beads (smiting)" },
  {  49, 15000, "periapt of wound closure" },
  {  50, 15000, "scabbard of keen edges" },
  {  51, 15100, "broom of flying" },
  {  52, 15100, "horn of the tritons" },
  {  53, 15200, "gem of brightness" },
  {  54, 15300, "pearl of the sirines" },
  {  55, 15500, "figurine of wondrous power (onyx dog)" },
  {  56, 15800, "chime of interruption" },
  {  57, 16000, "bracers of armor (+4)" },
  {  58, 16000, "cloak of resistance (+4)" },
  {  59, 16000, "pearl of power (4th-level spell)" },
  {  60, 16000, "belt of giant strength (+4)" },
  {  61, 16000, "gloves of Dexterity (+4)" },
  {  62, 16000, "bracers of health (+4)" },
  {  63, 16000, "headband of intellect (+4)" },
  {  64, 16000, "periapt of Wisdom (+4)" },
  {  65, 16000, "cloak of Charisma (+4)" },
  {  66, 16500, "figurine of wondrous power (golden lions)" },
  {  67, 17000, "figurine of wondrous power (marble elephant)" },
  {  68, 18000, "amulet of natural armor (+3)" },
  {  69, 18000, "carpet of flying (3 ft. by 5 ft.)" },
  {  70, 19000, "necklace of adaptation" },
  {  71, 20000, "cloak of the manta ray" },
  {  72, 20000, "pale green prism Ioun stone" },
  {  73, 20000, "pale lavender ellipsoid Ioun stone" },
  {  74, 20000, "pearly white spindle Ioun stone" },
  {  75, 21000, "figurine of wondrous power (ivory goats)" },
  {  76, 21000, "rope of entanglement" },
  {  77, 22000, "cube of frost resistance" },
  {  78, 23000, "mattock of the titans" },
  {  79, 23760, "major circlet of blasting" },
  {  80, 24000, "cloak of the bat" },
  {  81, 24000, "helm of underwater action" },
  {  82, 24500, "eyes of doom" },
  {  83, 25000, "minor cloak of displacement (20% miss chance)" },
  {  84, 25000, "cloak of resistance (+5)" },
  {  85, 25000, "mask of the skull" },
  {  86, 25000, "maul of the titans" },
  {  87, 25000, "pearl of power (5th-level spell)" },
  {  88, 25000, "bracers of armor (+5)" },
  {  89, 26000, "dimensional shackles" },
  {  90, 26000, "iron bands of Bilarro" },
  {  91, 27000, "robe of scintillating colors" },
  {  92, 27500, "manual of bodily health +1" },
  {  93, 27500, "manual of gainful exercise +1" },
  {  94, 27500, "manual of quickness in action +1" },
  {  95, 27500, "tome of clear thought +1" },
  {  96, 27500, "tome of leadership and influence +1" },
  {  97, 27500, "tome of understanding +1" },
  {  98, 28500, "figurine of wondrous power (obsidian steed)" },
  {  99, 29000, "carpet of flying (4 ft. by 6 ft.)" },
  { 100,  7500, "boots of levitation" },
  {   0,     0, 0 }
};


WONDROUSITEM majorItems[] = {
  {   2,  30000, "lantern of revealing" },
  {   4,  30000, "necklace of prayer beads (wind walking)" },
  {   6,  30000, "drums of panic" },
  {   8,  31000, "helm of telepathy" },
  {  10,  32000, "amulet of natural armor (+4)" },
  {  12,  35000, "amulet of proof against detection of location" },
  {  14,  36000, "bracers of armor (+6)" },
  {  15,  36000, "belt of giant strength (+6)" },
  {  16,  36000, "gloves of Dexterity (+6)" },
  {  17,  36000, "bracers of health (+6)" },
  {  18,  36000, "headband of intellect (+6)" },
  {  19,  36000, "periapt of Wisdom (+6)" },
  {  20,  36000, "cloak of Charisma (+6)" },
  {  22,  36000, "pearl of power (6th-level spell)" },
  {  24,  38000, "orb of storms" },
  {  26,  38000, "scarab of protection" },
  {  28,  40000, "lavender and green ellipsoid Ioun stone" },
  {  30,  40000, "ring gates" },
  {  31,  41000, "carpet of flying (5 ft. by 7 ft.)" },
  {  32,  42000, "crystal ball" },
  {  33,  48600, "helm of teleportation" },
  {  34,  49000, "bracers of armor (+7)" },
  {  35,  50000, "pearl of power (7th-level spell)" },
  {  36,  50000, "amulet of natural armor (+5)" },
  {  37,  50000, "major cloak of displacement (50% miss chance)" },
  {  38,  50000, "crystal ball with detect invisibility" },
  {  39,  50000, "horn of Valhalla" },
  {  40,  50000, "necklace of prayer beads (summons)" },
  {  41,  51000, "crystal ball with detect thoughts" },
  {  42,  52000, "cloak of etherealness" },
  {  43,  53000, "carpet of flying (6 ft. by 9 ft.)" },
  {  44,  55000, "Daern's instant fortress" },
  {  45,  55000, "manual of bodily health +2" },
  {  46,  55000, "manual of gainful exercise +2" },
  {  47,  55000, "manual of quickness in action +2" },
  {  48,  55000, "tome of clear thought +2" },
  {  49,  55000, "tome of leadership and influence +2" },
  {  50,  55000, "tome of understanding +2" },
  {  51,  56000, "eyes of charming" },
  {  52,  58000, "robe of stars" },
  {  53,  60000, "darkskull" },
  {  54,  62000, "cube of force" },
  {  55,  64000, "bracers of armor (+8)" },
  {  56,  64000, "pearl of power (8th-level spell)" },
  {  57,  70000, "crystal ball with telepathy" },
  {  58,  70000, "pearl of power (two spells)" },
  {  59,  75000, "gem of seeing" },
  {  60,  75000, "robe of the archmagi" },
  {  61,  76000, "vestments of faith" },
  {  62,  80000, "amulet of the planes" },
  {  63,  80000, "crystal ball with true seeing" },
  {  64,  81000, "pearl of power (9th-level spell)" },
  {  65,  82000, "well of many worlds" },
  {  66,  82500, "manual of bodily health +3" },
  {  67,  82500, "manual of gainful exercise +3" },
  {  68,  82500, "manual of quickness in action +3" },
  {  69,  82500, "tome of clear thought +3" },
  {  70,  82500, "tome of leadership and influence +3" },
  {  71,  82500, "tome of understanding +3" },
  {  72,  90000, "mantle of spell resistance" },
  {  73,  90000, "robe of eyes" },
  {  74,  92000, "mirror of opposition" },
  {  75,  93000, "chaos diamond" },
  {  76,  98000, "eyes of petrification" },
  {  77, 100000, "bowl of commanding water elementals" },
  {  78, 100000, "brazier of commanding fire elementals" },
  {  79, 100000, "censer of controlling air elementals" },
  {  80, 100000, "stone of controlling earth elementals" },
  {  81, 110000, "manual of bodily health +4" },
  {  82, 110000, "manual of gainful exercise +4" },
  {  83, 110000, "manual of quickness in action +4" },
  {  84, 110000, "tome of clear thought +4" },
  {  85, 110000, "tome of leadership and influence +4" },
  {  86, 110000, "tome of understanding +4" },
  {  87, 130000, "apparatus of Kwalish" },
  {  88, 137500, "manual of bodily health +5" },
  {  89, 137500, "manual of gainful exercise +5" },
  {  90, 137500, "manual of quickness in action +5" },
  {  91, 137500, "tome of clear thought +5" },
  {  92, 137500, "tome of leadership and influence +5" },
  {  93, 137500, "tome of understanding +5" },
  {  94, 150000, "efreeti bottle" },
  {  95, 152000, "mirror of life trapping" },
  {  96, 156000, "cubic gate" },
  {  97, 157000, "helm of brilliance" },
  {  98, 170000, "iron flask" },
  {  99, 175000, "mirror of mental prowess" },
  { 100,  36000, "belt of giant strength (+6)" },
  {   0,      0, 0 }
};


typedef struct {
  int   dtop;
  int   targeted;
  int   value;
  char* name;
} SPELL;


SPELL arcane1[] = {
  {   5,  0,  25, "burning hands" },
  {  10,  0,  25, "change self" },
  {  15,  1,  25, "charm person" },
  {  18,  0,  25, "color spray" },
  {  22,  0,  25, "detect secret doors" },
  {  25,  0,  25, "detect undead" },
  {  28,  1,  25, "enlarge" },
  {  31,  0,  25, "erase" },
  {  36,  0,  25, "feather fall" },
  {  39,  0,  25, "grease" },
  {  44,  0, 125, "identify" },
  {  47,  0,  25, "jump" },
  {  51,  0,  25, "mage armor" },
  {  54,  0,  25, "magic weapon" },
  {  57,  0,  25, "mount" },
  {  60,  1,  25, "ray of enfeeblement" },
  {  63,  1,  25, "reduce" },
  {  66,  0,  25, "shield" },
  {  69,  1,  25, "shocking grasp" },
  {  73,  0,  25, "silent image" },
  {  78,  1,  25, "sleep" },
  {  81,  0,  25, "spider climb" },
  {  84,  0,  25, "summon monster I" },
  {  87,  0,  25, "Tenser's floating disk" },
  {  92,  0,  25, "unseen servant" },
  {  95,  0,  25, "ventriloquism" },
  { 100,  0,  25, "magic missile" },
  {   0,  0,   0, 0 }
};
 

SPELL arcane2[] = {
  {   3,  0, 175, "arcane lock" },
  {   8,  1, 150, "blindness/deafness" },
  {  13,  0, 150, "blur" },
  {  18,  0, 150, "bull's strength" },
  {  22,  0, 150, "cat's grace" },
  {  25,  0, 150, "darkvision" },
  {  30,  0, 150, "detect thoughts" },
  {  33,  0, 150, "flaming sphere" },
  {  38,  0, 150, "invisibility" },
  {  41,  0, 150, "knock" },
  {  46,  0, 150, "levitate" },
  {  51,  0, 150, "locate object" },
  {  54,  1, 150, "Melf's acid arrow" },
  {  59,  0, 150, "minor image" },
  {  64,  0, 150, "mirror image" },
  {  69,  0, 150, "misdirection" },
  {  72,  0, 150, "protection from arrows" },
  {  77,  0, 150, "see invisibility" },
  {  80,  0, 150, "spectral hand" },
  {  83,  0, 150, "stinking cloud" },
  {  87,  0, 150, "summon monster II" },
  {  92,  0, 150, "summon swarm" },
  {  95,  1, 150, "web" },
  { 100,  1, 150, "ghoul touch" },
  {   0,  0,   0, 0 }
};


SPELL arcane3[] = {
  {   5,  0, 375, "blink" },
  {  10,  0, 375, "clairaudience/clairvoyance" },
  {  15,  1, 375, "dispel magic" },
  {  20,  0, 375, "displacement" },
  {  25,  1, 375, "fireball" },
  {  28,  1, 375, "flame arrow" },
  {  31,  0, 375, "fly" },
  {  33,  1, 375, "gaseous form" },
  {  36,  0, 375, "greater magic weapon" },
  {  39,  0, 375, "halt undead" },
  {  42,  0, 375, "haste" },
  {  45,  1, 375, "hold person" },
  {  47,  0, 375, "invisibility sphere" },
  {  53,  1, 375, "lightning bolt" },
  {  54,  0, 375, "magic circle against chaos" },
  {  55,  0, 375, "magic circle against evil" },
  {  56,  0, 375, "magic circle against good" },
  {  57,  0, 375, "magic circle against law" },
  {  60,  0, 425, "nondetection" },
  {  65,  0, 375, "slow" },
  {  70,  0, 375, "sepia snake sigil" },
  {  75,  0, 375, "suggestion" },
  {  79,  0, 375, "tongues" },
  {  87,  1, 375, "vampiric touch" },
  {  90,  0, 375, "water breathing" },
  { 100,  0, 375, "keen edge" },
  {   0,  0,   0, 0 }
};


SPELL arcane4[] = {
  {   5,  1, 700, "charm monster" },
  {  10,  1, 700, "confusion" },
  {  15,  1, 700, "contagion" },
  {  20,  0, 700, "detect scrying" },
  {  23,  1, 700, "dimensional anchor" },
  {  28,  1, 700, "dimension door" },
  {  33,  1, 700, "emotion" },
  {  36,  1, 700, "enervation" },
  {  39,  0, 700, "Evard's black tentacles" },
  {  44,  1, 700, "fear" },
  {  47,  0, 700, "fire shield" },
  {  50,  0, 700, "ice storm" },
  {  55,  0, 700, "improved invisibility" },
  {  58,  0, 700, "lesser geas" },
  {  61,  0, 700, "minor globe of invulnerability" },
  {  67,  0, 700, "phantasmal killer" },
  {  70,  1, 700, "polymorph other" },
  {  73,  0, 700, "polymorph self" },
  {  76,  0, 700, "remove curse" },
  {  79,  0, 700, "shadow conjuration" },
  {  82,  0, 950, "stoneskin" },
  {  84,  0, 700, "summon monster IV" },
  {  87,  0, 700, "wall of fire" },
  {  90,  0, 700, "wall of ice" },
  {  95,  0, 700, "shout" },
  { 100,  0, 700, "bestow curse" },
  {   0,  0,   0, 0 }
};


SPELL arcane5[] = {
  {   4,  0, 1125, "Bigby's interposing hand" },
  {   8,  0, 1125, "cloudkill" },
  {  13,  0, 1125, "cone of cold" },
  {  17,  1, 1125, "dismissal" },
  {  21,  1, 1125, "dominate person" },
  {  24,  1, 1125, "feeblemind" },
  {  27,  0, 1125, "greater shadow conjuration" },
  {  31,  1, 1125, "hold monster" },
  {  35,  0, 1125, "major creation" },
  {  40,  0, 1125, "mind fog" },
  {  44,  0, 1125, "passwall" },
  {  49,  0, 1125, "persistant image" },
  {  53,  0, 1125, "shadow evocation" },
  {  56,  0, 1125, "stone shape" },
  {  60,  0, 1125, "summon monster V" },
  {  64,  0, 1125, "telekinesis" },
  {  69,  0, 1125, "teleport" },
  {  73,  0, 1125, "transmute mud to rock" },
  {  77,  0, 1125, "transmute rock to mud" },
  {  81,  0, 1125, "wall of force" },
  {  86,  0, 1175, "wall of iron" },
  {  90,  0, 1125, "wall of stone" },
  {  95,  0, 1125, "permanency" },
  { 100,  0, 1125, "Mordenkainen's faithful hound" },
  {   0,  0,    0, 0 }
};


SPELL arcane6[] = {
  {   4,  0, 1650, "acid fog" },
  {   7,  0, 1650, "analyze dweomer" },
  {  11,  0, 1650, "antimagic field" },
  {  15,  0, 1650, "Bigby's forceful hand" },
  {  19,  1, 1650, "chain lightning" },
  {  23,  0, 2150, "circle of death" },
  {  26,  0, 1650, "control water" },
  {  30,  1, 1650, "disintegrate" },
  {  33,  0, 1650, "eyebite" },
  {  37,  1, 1650, "flesh to stone" },
  {  41,  0, 1650, "globe of invulnerability" },
  {  45,  0, 1650, "greater shadow evocation" },
  {  49,  0, 1650, "mass suggestion" },
  {  52,  0, 1650, "mislead" },
  {  57,  0, 1650, "move earth" },
  {  61,  0, 1650, "Otiluke's freezing sphere" },
  {  65,  0, 1650, "programmed image" },
  {  70,  0, 1650, "project image" },
  {  75,  1, 1650, "repulsion" },
  {  78,  0, 1650, "shades" },
  {  82,  0, 1650, "stone to flesh" },
  {  86,  0, 1650, "summon monster VI" },
  {  90,  0, 1650, "true seeing" },
  {  95,  0, 1900, "contingency" },
  { 100,  0, 2000, "legend lore" },
  {   0,  0,    0, 0 }
};


SPELL arcane7[] = {
  {   5,  0, 2275, "Bigby's grasping hand" },
  {  10,  0, 2275, "control undead" },
  {  15,  1, 2275, "delayed blast fireball" },
  {  20,  0, 2275, "ethereal jaunt" },
  {  25,  1, 2275, "finger of death" },
  {  30,  0, 3775, "forcecage" },
  {  35,  0, 3775, "limited wish" },
  {  40,  0, 2275, "mass invisibility" },
  {  45,  0, 2275, "Mordenkainen's sword" },
  {  50,  1, 2275, "Power word, stun" },
  {  55,  0, 2275, "prismatic spray" },
  {  60,  0, 2275, "reverse gravity" },
  {  65,  0, 2275, "sequester" },
  {  70,  0, 2275, "spell turning" },
  {  75,  0, 2275, "summon monster VII" },
  {  80,  0, 2275, "teleport without error" },
  {  85,  0, 3025, "vanish" },
  {  90,  0, 2275, "vision" },
  {  95,  0, 2275, "Mordenkainen's magnificent mansion" },
  { 100,  0, 2275, "plane shift" },
  {   0,  0,    0, 0 }
};


SPELL arcane8[] = {
  {   3,  0,  3000, "antipathy" },
  {   8,  0,  3000, "Bigby's clenched fist" },
  {  13,  0,  4000, "clone" },
  {  18,  0,  3000, "demand" },
  {  23,  1,  3000, "horrid wilting" },
  {  28,  0,  3000, "incendiary cloud" },
  {  33,  0,  3000, "mass charm" },
  {  38,  1,  3000, "maze" },
  {  43,  0,  3000, "mind blank" },
  {  48,  0,  3000, "Otiluke's telekinetic sphere" },
  {  53,  1,  3000, "Otto's irresistible dance" },
  {  58,  1,  3000, "polymorph any object" },
  {  63,  1,  3000, "power world, blind" },
  {  68,  0,  3000, "prismatic wall" },
  {  73,  0,  3000, "protection from spells" },
  {  78,  0,  3000, "screen" },
  {  83,  0,  3000, "summon monster VII" },
  {  88,  0,  3000, "sunburst" },
  {  90,  0,  4500, "sympathy" },
  {  95,  0, 13000, "symbol" },
  { 100,  0,  5000, "binding" },
  {   0,  0,     0, 0 }
};


SPELL arcane9[] = {
  {   7,  0,  3825, "Bigby's crushing hand" },
  {  14,  1,  3825, "energy drain" },
  {  21,  1,  3825, "imprisonment" },
  {  28,  1,  3825, "meteor swarm" },
  {  35,  0,  3825, "Mordenkainen's disjunction" },
  {  42,  0,  3825, "teleportation circle" },
  {  49,  0,  3825, "prismatic sphere" },
  {  56,  0,  3825, "shapechange" },
  {  63,  0,  3825, "summon monster IX" },
  {  69,  0,  3825, "time stop" },
  {  76,  0,  3825, "wail of the banshee" },
  {  83,  0,  3825, "weird" },
  {  90,  0, 28825, "wish" },
  {  95,  0,  3825, "gate" },
  { 100,  0,  3825, "foresight" },
  {   0,  0,     0, 0 }
};


SPELL divine1[] = {
  {   5,  0, 25, "bless" },
  {  10,  0, 25, "calm animals" },
  {  14,  0, 25, "command" },
  {  19,  0, 25, "cure light wounds" },
  {  22,  0, 25, "detect chaos" },
  {  25,  0, 25, "detect evil" },
  {  28,  0, 25, "detect good" },
  {  31,  0, 25, "detect law" },
  {  34,  0, 25, "detect snares and pits" },
  {  39,  0, 25, "doom" },
  {  44,  0, 25, "entangle" },
  {  49,  0, 25, "faerie fire" },
  {  54,  1, 25, "inflict light wounds" },
  {  59,  0, 25, "invisibility to animals" },
  {  64,  0, 25, "invisibility to undead" },
  {  67,  0, 25, "magic fang" },
  {  70,  0, 25, "magic stone" },
  {  73,  0, 25, "magic weapon" },
  {  77,  0, 25, "sanctuary" },
  {  82,  0, 25, "shillelagh" },
  {  86,  0, 25, "summon monster I" },
  {  90,  0, 25, "summon nature's ally I" },
  {  95,  0, 25, "pass without trace" },
  { 100,  0, 25, "endure elements" },
  {   0,  0,  0, 0 }
};


SPELL divine2[] = {
  {   5,  0, 150, "aid" },
  {  10,  0, 150, "augury" },
  {  15,  0, 150, "barkskin" },
  {  20,  0, 150, "bull's strength" },
  {  25,  0, 150, "charm person or animal" },
  {  28,  0, 150, "chill metal" },
  {  31,  0, 150, "cure moderate wounds" },
  {  36,  0, 150, "delay poison" },
  {  39,  0, 150, "flame blade" },
  {  42,  0, 150, "flaming sphere" },
  {  47,  0, 150, "heat metal" },
  {  50,  0, 150, "hold animal" },
  {  55,  1, 150, "hold person" },
  {  58,  1, 150, "inflict moderate wounds" },
  {  63,  0, 150, "lesser restoration" },
  {  67,  1, 150, "silence" },
  {  70,  0, 150, "speak with animals" },
  {  75,  0, 150, "spiritual weapon" },
  {  79,  0, 150, "summon monster II" },
  {  83,  0, 150, "summon nature's ally II" },
  {  85,  0, 150, "summon swarm" },
  {  90,  0, 150, "undetectable alignment" },
  {  95,  0, 150, "find traps" },
  { 100,  0, 150, "zone of truth" },
  {   0,  0,   0, 0 }
};


SPELL divine3[] = {
  {   2,  0, 375, "call lightning" },
  {   9,  0, 375, "cure serious wounds" },
  {  13,  1, 375, "dispel magic" },
  {  15,  1, 375, "dominate animal" },
  {  17,  0, 375, "greater magic fang" },
  {  19,  1, 375, "inflict serious wounds" },
  {  22,  0, 375, "invisibility purge" },
  {  26,  0, 375, "locate object" },
  {  28,  0, 375, "magic circle against chaos" },
  {  30,  0, 375, "magic circle against evil" },
  {  32,  0, 375, "magic circle against good" },
  {  34,  0, 375, "magic circle against law" },
  {  38,  0, 375, "negative energy protection" },
  {  41,  0, 375, "neutralize poison" },
  {  43,  0, 375, "plant growth" },
  {  46,  0, 375, "prayer" },
  {  51,  0, 375, "protection from elements" },
  {  53,  0, 375, "remove blindness/deafness" },
  {  56,  0, 375, "remove curse" },
  {  59,  0, 375, "remove disease" },
  {  62,  0, 375, "searing light" },
  {  65,  0, 375, "speak with dead" },
  {  67,  0, 375, "spike growth" },
  {  72,  0, 375, "stone shape" },
  {  75,  0, 375, "summon monster III" },
  {  78,  0, 375, "summon nature's ally III" },
  {  80,  0, 375, "water breathing" },
  {  90,  0, 375, "water walk" },
  {  95,  0, 375, "meld into stone" },
  { 100,  0, 375, "deeper darkness" },
  {   0,  0,   0, 0 }
};


SPELL divine4[] = {
  {   2,  0, 700, "antiplant shell" },
  {   5,  0, 700, "control water" },
  {  12,  0, 700, "cure critical wounds" },
  {  19,  0, 700, "discern lies" },
  {  24,  1, 700, "dispel magic" },
  {  27,  0, 700, "divine power" },
  {  34,  0, 700, "flame strike" },
  {  41,  0, 700, "freedom of movement" },
  {  47,  0, 700, "giant vermin" },
  {  50,  0, 700, "greater magic weapon" },
  {  53,  1, 700, "inflict critical wounds" },
  {  55,  0, 700, "lesser planar ally" },
  {  62,  0, 700, "neutralize poison" },
  {  66,  0, 700, "quench" },
  {  68,  0, 800, "restoration" },
  {  71,  0, 700, "rusting grasp" },
  {  74,  0, 700, "spell immunity" },
  {  76,  0, 700, "spike stones" },
  {  80,  0, 700, "summon monster IV" },
  {  82,  0, 700, "summon nature's ally IV" },
  {  90,  0, 700, "tongues" },
  {  95,  0, 700, "death ward" },
  { 100,  0, 700, "sleet storm" },
  {   0,  0,   0, 0 }
};


SPELL divine5[] = {
  {   7,  0, 1125, "break enchantment" },
  {  13,  0, 1625, "commune" },
  {  15,  0, 1125, "control winds" },
  {  22,  0, 1125, "cure critical wounds" },
  {  26,  0, 1125, "dispel evil" },
  {  29,  0, 1125, "dispel good" },
  {  35,  0, 1125, "flame strike" },
  {  38,  0, 1125, "greater command" },
  {  40,  0, 6125, "hallow" },
  {  43,  0, 1125, "healing circle" },
  {  45,  0, 1125, "ice storm" },
  {  50,  0, 1125, "insect plague" },
  {  57,  0, 1625, "raise dead" },
  {  60,  0, 1125, "righteous might" },
  {  63,  0, 1125, "slay living" },
  {  65,  0, 1125, "spell resistance" },
  {  67,  0, 1125, "summon monster V" },
  {  69,  0, 1125, "summon nature's ally V" },
  {  72,  0, 1125, "transmute rock to mud" },
  {  74,  0, 1325, "true seeing" },
  {  75,  0, 6125, "unhallow" },
  {  78,  0, 1125, "wall of fire" },
  {  80,  0, 1125, "wall of stone" },
  {  90,  0, 1125, "wall of thorns" },
  {  95,  0, 1125, "plane shift" },
  { 100,  0, 1125, "tree stride" },
  {   0,  0,    0, 0 }
};


SPELL divine6[] = {
  {   8,  0, 1650, "antilife shell" },
  {  14,  0, 1650, "blade barrier" },
  {  19,  0, 1650, "find the path" },
  {  23,  0, 1650, "fire seeds" },
  {  28,  0, 1650, "geas/quest" },
  {  34,  1, 1650, "harm" },
  {  41,  1, 1650, "heal" },
  {  47,  0, 1650, "heroes' feast" },
  {  55,  0, 1650, "planar ally" },
  {  57,  0, 1650, "repel wood" },
  {  60,  0, 1650, "stone tell" },
  {  68,  0, 1650, "summon monster VI" },
  {  71,  0, 1650, "transport via plants" },
  {  77,  0, 1650, "wall of stone" },
  {  80,  0, 1650, "wind walk" },
  {  90,  0, 1650, "word of recall" },
  { 100,  0, 1800, "create undead" },
  {   0,  0,    0, 0 }
};


SPELL divine7[] = {
  {  11,  0, 2275, "control weather" },
  {  18,  0, 2275, "creeping doom" },
  {  25,  0, 2275, "destruction" },
  {  32,  0, 2275, "dictum" },
  {  36,  0, 2275, "fire storm" },
  {  40,  0, 4775, "greater restoration" },
  {  47,  0, 2275, "holy word" },
  {  54,  0, 2275, "regenerate" },
  {  61,  0, 2275, "repulsion" },
  {  68,  0, 2275, "resurrection" },
  {  72,  0, 2275, "summon monster VII" },
  {  76,  0, 2275, "transmute metal to wood" },
  {  80,  0, 2575, "true seeing" },
  {  90,  0, 2275, "word of chaos" },
  {  95,  0, 3775, "refuge" },
  { 100,  0, 2275, "blasphemy" },
  {   0,  0,    0, 0 }
};


SPELL divine8[] = {
  {   6,  0, 3000, "antimagic field" },
  {  12,  0, 3000, "creeping doom" },
  {  18,  0, 3000, "discern location" },
  {  25,  0, 3000, "earthquake" },
  {  30,  1, 3000, "finger of death" },
  {  35,  0, 3000, "fire storm" },
  {  44,  0, 3000, "holy aura" },
  {  50,  0, 3000, "mass heal" },
  {  56,  0, 3000, "repel metal or stone" },
  {  62,  0, 3000, "reverse gravity" },
  {  68,  0, 3000, "summon monster VIII" },
  {  74,  0, 3000, "sunburst" },
  {  80,  0, 3000, "unholy aura" },
  {  90,  0, 3000, "whirlwind" },
  { 100,  0, 3000, "animal shapes" },
  {   0,  0,    0, 0 }
};


SPELL divine9[] = {
  {   7,  0,  3825, "earthquake" },
  {  14,  0,  3825, "elemental swarm" },
  {  26,  1,  3825, "energy drain" },
  {  38,  1,  3825, "implosion" },
  {  50,  0, 28825, "miracle" },
  {  57,  0,  3825, "shapechange" },
  {  68,  0,  3825, "storm of vengeance" },
  {  80,  0,  3825, "summon monster IX" },
  {  90,  0,  8825, "true resurrection" },
  {  93,  0,  5825, "soul bind" },
  {  95,  0,  3825, "gate" },
  { 100,  0,  3825, "antipathy" },
  {   0,  0,     0, 0 }
};


struct {
  SPELL *arcane[9];
  SPELL *divine[9];
} spellTable = {
  { arcane1, arcane2, arcane3, arcane4, arcane5, arcane6, arcane7, arcane8, arcane9 },
  { divine1, divine2, divine3, divine4, divine5, divine6, divine7, divine8, divine9 }
};
 

struct {
  int   dtop;
  int   twoBonus;
  int   primary;
  int   extraordinary;
  int   egoBonus;
  int   value;
  char* communication;
  char* other;
} intTable[] = {
  {  34,  5, 1, 0, 0, 10000, "Semiempathy", "" },
  {  59,  6, 2, 0, 0, 15000, "Empathy", "" },
  {  79,  7, 2, 0, 0, 17500, "Speech", "" },
  {  91,  8, 3, 0, 0, 25000, "Speech", "" },
  {  97,  9, 3, 0, 1, 32000, "Speech", "reads all languages spoken" },
  {  98, 10, 3, 1, 2, 55000, "Speech, telepathy", "reads all languages spoken" },
  {  99, 11, 3, 2, 3, 78000, "Speech, telepathy", "reads all languages and magic" },
  { 100, 12, 4, 2, 3, 90000, "Speech, telepathy", "reads all languages and magic" },
  {   0,  0, 0, 0, 0,     0, 0 }
};


struct {
  int   die;
  char* hi;
  char* med;
  char* lo;
} intAbilities[] = {
  { 1, "Int", "Cha", "Wis" },
  { 2, "Int", "Wis", "Cha" },
  { 3, "Wis", "Int", "Cha" },
  { 4, "Cha", "Int", "Wis" },
  { 0, 0, 0, 0 }
};


struct {
  int   dtop;
  int   standing;
  char* alignment;
} alignments[] = {
  {   5, opCHAOTIC | opGOOD, "Chaotic good" },
  {  15, opCHAOTIC, "Chaotic neutral" },
  {  20, opCHAOTIC | opEVIL, "Chaotic evil" },
  {  25, opEVIL, "Neutral evil" },
  {  30, opLAWFUL | opEVIL, "Lawful evil" },
  {  55, opLAWFUL | opGOOD, "Lawful good" },
  {  60, opLAWFUL, "Lawful neutral" },
  {  80, opGOOD, "Neutral good" },
  { 100, NONE, "Neutral" },
  {   0, 0 }
};


struct {
  int   dtop;
  int   allowDup;
  char* name;
} primaryAbilities[] = {
  {   4, 1, "Item can Intuit Direction (10 ranks)" },
  {   8, 1, "Item can Sense Motive (10 ranks)" },
  {  12, 0, "Wielder has free use of Combat Reflexes" },
  {  16, 0, "Wielder has free use of Blind-Fight" },
  {  20, 0, "Wielder has free use of Improved Initiative" },
  {  24, 0, "Wielder has free use of Mobility" },
  {  28, 0, "Wielder has free use of Sunder" },
  {  32, 0, "Wielder has free use of Expertise" },
  {  39, 0, "Detect opposing alignment at will" },
  {  42, 0, "Find traps at will" },
  {  47, 0, "Detect secret doors at will" },
  {  54, 0, "Detect magic at will" },
  {  57, 0, "Wielder has free use of uncanny dodge (as a 5th-level barbarian)" },
  {  60, 0, "Wielder has free use of evasion" },
  {  65, 0, "Wielder can see invisible at will" },
  {  70, 1, "Cure light wounds (1d8+5) on wielder 1/day" },
  {  75, 1, "Feather fall on wielder 1/day" },
  {  76, 1, "Locate object in a 120-ft. radius" },
  {  77, 0, "Wielder does not need to sleep" },
  {  78, 0, "Wielder does not need to breathe" },
  {  79, 1, "Jump for 20 minutes on wielder 1/day" },
  {  80, 1, "Spider climb for 20 minutes on wielder 1/day" },
  {  90, 1, "-" }, /* roll twice more */
  { 100, 1, "*" }, /* roll on extraordinary abilities table */
  {   0, 0 }
};


struct {
  int   dtop;
  int   allowDup;
  char* name;
} extraAbilities[] = {
  {   5, 1, "Charm person (DC 11) on contact, 3/day" },
  {  10, 1, "Clairaudience/clairvoyance (100-ft. range, 1 minute per use), 3/day" },
  {  15, 1, "Magic missile (200-ft. range, 3 missiles), 3/day" },
  {  20, 1, "Shield on wielder, 3/day" },
  {  25, 1, "Detect thoughts (100-ft. range, 1 minute per use), 3/day" },
  {  30, 1, "Levitation (wielder only, 10 minute duration), 3/day" },
  {  35, 1, "Invisibility (wielder only, up to 30 minutes per use), 3/day" },
  {  40, 1, "Fly (30 minutes per use), 2/day" },
  {  45, 1, "Lightning bolt (8d6 points of damage, 200-ft. range, DC 13), 1/day" },
  {  50, 1, "Summon monster III, 1/day" },
  {  55, 1, "Telepathy (100 ft. range), 2/day" },
  {  60, 1, "Cat's grace (wielder only), 1/day" },
  {  65, 1, "Bull's strength (wielder only), 1/day" },
  {  70, 1, "Haste (wielder only, 10 rounds), 1/day" },
  {  73, 1, "Telekinesis (250 lb. maximum, 1 minute each use), 2/day" },
  {  76, 1, "Heal, 1/day" },
  {  77, 1, "Teleport, 600 lb. maximum, 1/day" },
  {  78, 1, "Globe of invulnerability, 1/day" },
  {  79, 1, "Stoneskin (wielder only, 10 minutes per use), 2/day" },
  {  80, 1, "Feeblemind by touch, 2/day" },
  {  81, 0, "True seeing, at will" },
  {  82, 1, "Wall of force, 1/day" },
  {  83, 1, "Summon monster VI, 1/day" },
  {  84, 1, "Finger of death (100 ft. range, DC 17), 1/day" },
  {  85, 0, "Passwall, at will" },
  {  90, 1, "-" }, /* roll twice again */
  { 100, 0, "*" }, /* roll again, plus roll on item purpose table */
  {   0, 0 } 
};


struct {
  int   dtop;
  int   other; 
  int   standing;
  char* purpose;
} itemPurposes[] = {
  {  20, NONE, NONE, "Defeat/slay diametrically opposed alignment" },
  {  30, NONE, NONE, "Defeat/slay arcane spellcasters (including spellcasting monsters and those that use spell-like abilities" },
  {  40, NONE, NONE, "Defeat/slay divine spellcasters (including divine entities and servitors)" },
  {  50, NONE, NONE, "Defeat/slay nonspellcasters" },
  {  55, NONE, NONE, "Defeat/slay a type of creature (see MM)" },
  {  60, NONE, NONE, "Defeat/slay a particular kind of creature" },
  {  70, NONE, NONE, "Defend a particular race or kind of creature" },
  {  80, NONE, NONE, "Defeat/slay the servants of a specific deity" },
  {  90, NONE, NONE, "Defend the servants and interests of a specific deity" },
  {  95, NONE, opEVIL | opCHAOTIC, "Defeat/slay all (other than the item and the wielder)" },
  { 100, NONE, opGOOD, "Defeat/slay evil creatures and items" },
  {   0,    0, 0 }
};


struct {
  int   dtop;
  char* name;
} itemPurposeAbilities[] = {
  {  10, "Blindness (DC 12) for 2d6 rounds" },
  {  20, "Confusion (DC 14) for 2d6 rounds" },
  {  25, "Fear (DC 14) for 1d4 rounds" },
  {  55, "Hold monster (DC 14) for 1d4 rounds" },
  {  65, "Slay living (DC 15)" },
  {  75, "Disintegrate (DC 16)" },
  {  80, "True resurrection on wielder, one time only" },
  { 100, "+2 luck bonus to all saving throws, +2 deflection AC bonus, spell resistance 15" },
  {   0, 0 }
};


static int opposes( int standing1, int standing2 ) {
  if( ( standing1 & opGOOD ) != 0 && ( standing2 & opEVIL ) != 0 ) {
    return 1;
  }
  if( ( standing1 & opEVIL ) != 0 && ( standing2 & opGOOD ) != 0 ) {
    return 1;
  }
  if( ( standing1 & opLAWFUL ) != 0 && ( standing2 & opCHAOTIC ) != 0 ) {
    return 1;
  }
  if( ( standing1 & opCHAOTIC ) != 0 && ( standing2 & opLAWFUL ) != 0 ) {
    return 1;
  }

  return 0;
}


void addNewTreasure( TREASUREOPTS *opts, char* desc, int value ) {
  TREASUREITEM *t;

  t = (TREASUREITEM*)malloc( sizeof( TREASUREITEM ) );
  strcpy( t->desc, desc );
  t->value = value;
  t->next = 0;

  if( opts->treasureTail == NULL ) {
    opts->treasureList = opts->treasureTail = t;
  } else {
    opts->treasureTail->next = t;
    opts->treasureTail = t;
  }
}


void cleanupTreasure( TREASUREOPTS *opts ) {
  TREASUREITEM* t;
  TREASUREITEM* n;

  t = opts->treasureList;
  while( t != NULL ) {
    n = t->next;
    free( t );
    t = n;
  }
}


/* swap two integer values, used by determineIntelligence */
static void swap( int* i1, int* i2 ) {
  int t;

  t = *i1;
  *i1 = *i2;
  *i2 = t;
}


/* determines the characteristics of an intelligent item.  the 'enh'
 * value is the enhancement value of the item (including special abilities)
 * and 'value' is a pointer to an integer that holds the current gp value
 * of the item.  The routine returns a static pointer to the text description
 * of the intelligent item.
 */

static char* determineIntelligence( int enh, int* standing, int* value ) {
  int hi, med, lo;
  int d; 
  int i;
  int j;
  int k;
  int valid;
  int ilevl = 0;
  int primary;
  int extra;
  int hasPurpose;
  int ego;
  static char buffer[1024];
  char *slist[30];

  ego = enh;

  /* ego, according to the DMG, is an amalgamation of the properties of the
   * item: enhancement bonuses, special abilities, ability score bonuses,
   * purpose, primary abilities, extraordinary abilities, etc. */

  buffer[0] = 0;

  d = rollDice( 1, 100 );
  for( i = 0; intTable[i].communication != 0; i++ ) {
    if( d <= intTable[i].dtop ) {
      ilevl = i;
      break;
    }
  }

  ego += intTable[i].egoBonus;
  *value += intTable[i].value;

  hi = rollDice( 2, 6 ) + intTable[i].twoBonus;
  med = rollDice( 2, 6 ) + intTable[i].twoBonus;
  lo = rollDice( 3, 6 );

  if( hi < lo ) {
    swap( &hi, &lo );
  }
  if( hi < med ) {
    swap( &hi, &med );
  }
  if( med < lo ) {
    swap( &med, &lo );
  }

  d = ( hi - 10 ) / 2;
  d = ( d < 0 ? 0 : d );
  ego += d;
  d = ( med - 10 ) / 2;
  d = ( d < 0 ? 0 : d );
  ego += d;
  d = ( lo - 10 ) / 2;
  d = ( d < 0 ? 0 : d );
  ego += d;

  d = rollDice( 1, 4 ) - 1;
  sprintf( buffer, " - %s %d, %s %d, %s %d ", 
           intAbilities[d].hi, hi,
           intAbilities[d].med, med,
           intAbilities[d].lo, lo );
  strcat( buffer, "communicates by " );
  strcat( buffer, intTable[ilevl].communication );
  if( intTable[ilevl].other[0] != '\0' ) {
    strcat( buffer, " and " );
    strcat( buffer, intTable[ilevl].other );
  } 
  strcat( buffer, ", " );

  do {
    j = 0;
    d = rollDice( 1, 100 );
    for( i = 0; alignments[i].alignment != 0; i++ ) {
      if( d <= alignments[i].dtop ) {
        if( opposes( *standing, alignments[i].standing ) ) {
          j = 1;
          break;
        }
        strcat( buffer, alignments[i].alignment );
        (*standing) |= alignments[i].standing;
        break;
      }
    }
  } while( j );

  primary = intTable[ilevl].primary;
  extra = intTable[ilevl].extraordinary;
  hasPurpose = 0;

  for( i = 0; i < primary; i++ ) {
    do {
      valid = 1;
      d = rollDice( 1, 100 );
      for( j = 0; primaryAbilities[j].name != 0; j++ ) {
        if( d <= primaryAbilities[j].dtop ) {
          if( primaryAbilities[j].name[0] == '-' ) {
            slist[i] = strdup( primaryAbilities[j].name );
            primary += 2;
          } else if( primaryAbilities[j].name[0] == '*' ) {
            slist[i] = strdup( primaryAbilities[j].name );
            extra++;
          } else {
            for( k = 0; k < i; k++ ) {
              if( !primaryAbilities[j].allowDup && ( strcmp( slist[k], primaryAbilities[j].name ) == 0 ) ) {
                valid = 0;
                break;
              }
            }
            if( !valid ) {
              break;
            }
            ego++;
            strcat( buffer, "\n - " );
            strcat( buffer, primaryAbilities[j].name );
            slist[i] = strdup( primaryAbilities[j].name );
          }
          break;
        }
      }
    } while( !valid );
  }

  for( i = 0; i < primary; i++ ) {
    free( slist[i] );
  }

  for( i = 0; i < extra; i++ ) {
    do {
      valid = 1;
      d = rollDice( 1, 100 );
      for( j = 0; extraAbilities[j].name != 0; j++ ) {
        if( d <= extraAbilities[j].dtop ) {
          if( extraAbilities[j].name[0] == '-' ) {
            extra += 2;
            slist[i] = strdup( extraAbilities[j].name );
          } else if( extraAbilities[j].name[0] == '*' ) {
            extra++;
            hasPurpose = 1;
            slist[i] = strdup( extraAbilities[j].name );
          } else {
            for( k = 0; k < i; k++ ) {
              if( !extraAbilities[j].allowDup && ( strcmp( slist[k], extraAbilities[j].name ) == 0 ) ) {
                valid = 0;
                break;
              }
            }
            if( !valid ) {
              break;
            }
            ego += 2;
            strcat( buffer, "\n - " );
            strcat( buffer, extraAbilities[j].name );
            slist[i] = strdup( extraAbilities[j].name );
          }
          break;
        }
      }
    } while( !valid );
  }

  for( i = 0; i < extra; i++ ) {
    free( slist[i] );
  }

  if( hasPurpose ) {
    ego += 4;

    strcat( buffer, "\n - Purpose: " );
    do {
      j = 0;
      d = rollDice( 1, 100 );
      for( i = 0; itemPurposes[i].purpose != 0; i++ ) {
        if( d <= itemPurposes[i].dtop ) {
          if( opposes( *standing, itemPurposes[i].standing ) ) {
            j = 1;
            break;
          }
          strcat( buffer, itemPurposes[i].purpose );
          strcat( buffer, "\n" );
          break;
        }
      }
    } while( j );

    d = rollDice( 1, 100 );
    strcat( buffer, "   " );
    for( i = 0; itemPurposeAbilities[i].name != 0; i++ ) {
      if( d <= itemPurposeAbilities[i].dtop ) {
        strcat( buffer, itemPurposeAbilities[i].name );
        break;
      }
    }
  }

  sprintf( &(buffer[strlen(buffer)]), "\n - Ego: %d", ego );

  return buffer;
}


char* randomSpell( int type, int level, int *value ) {
  int d;
  int i;
  SPELL **table;

  if( ( level < 1 ) || ( level > 9 ) ) {
    return "";
  }

  d = rollDice( 1, 100 );
  if( type == ARCANE ) {
    table = spellTable.arcane;
  } else {
    table = spellTable.divine;
  }

  for( i = 0; table[level-1][i].name != 0; i++ ) {
    if( d <= table[level-1][i].dtop ) {
      *value += table[level-1][i].value;
      return table[level-1][i].name;
    }
  }

  return "";
}


static char* randomItem( char** names ) {
  int len;

  len = (int)names[0];

  if( len < 1 ) {
    return "";
  }
    
  return names[ rand() % len + 1 ];
}


void generateSpecificArmor( TREASUREOPTS *opts, int level ) {
  int d;
  int i;
  int *top;              

  d = rollDice( 1, 100 );
  for( i = 0; specificArmors[i].armor != 0; i++ ) {
    if( level == MEDIUM ) {
      top = &(specificArmors[i].mediumDtop);
    } else {
      top = &(specificArmors[i].majorDtop);
    }

    if( d <= *top ) {
      addNewTreasure( opts, specificArmors[i].armor, specificArmors[i].value*100 );
      return;
    }
  }
}


void generateSpecificShield( TREASUREOPTS *opts, int level ) {
  int d;
  int i;
  int *top;

  d = rollDice( 1, 100 );
  for( i = 0; specificShields[i].shield != 0; i++ ) {
    if( level == MEDIUM ) {
      top = &(specificShields[i].mediumDtop);
    } else {
      top = &(specificShields[i].majorDtop);
    }

    if( d <= *top ) {
      addNewTreasure( opts, specificShields[i].shield, specificShields[i].value*100 );
      return;
    }
  }
}


static char* getShieldSpecial( int level, int* plusses ) {
  int d;
  int i;
  int *top;

  do {
    d = rollDice( 1, 100 );
    for( i = 0; shieldSpecials[ i ].ability != 0; i++) {
      if( level == MINOR ) {
        top = &(shieldSpecials[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(shieldSpecials[i].mediumDtop);
      } else {
        top = &(shieldSpecials[i].majorDtop);
      }

      if( d <= *top ) {
        if( *plusses + shieldSpecials[i].plus > 10 ) {
          break;
        }
        (*plusses) += shieldSpecials[i].plus;
        return shieldSpecials[i].ability;
      }
    }
  } while( 1 );

  return "[unknown!]";
}


static char* getArmorSpecial( int level, int* plusses ) {
  int d;
  int i;
  int *top;

  do {
    d = rollDice( 1, 100 );
    for( i = 0; armorSpecials[ i ].ability != 0; i++) {
      if( level == MINOR ) {
        top = &(armorSpecials[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(armorSpecials[i].mediumDtop);
      } else {
        top = &(armorSpecials[i].majorDtop);
      }

      if( d <= *top ) {
        if( *plusses + armorSpecials[i].plus > 10 ) {
          break;
        }
        (*plusses) += armorSpecials[i].plus;
        return armorSpecials[i].ability;
      }
    }
  } while( 1 );

  return "[unknown!]";
}


void generateArmor( TREASUREOPTS *opts, int level ) {
  int  i;
  int  d;
  int  j;
  int *top;
  int  enh;
  int  specCnt;
  int  armor;
  int  tries;
  char *p;
  char desc[1024];
  char *slist[30];
  int  value;
  int  standing;

  enh = 0;
  specCnt = 0;
  armor = 0;
  desc[0] = 0;
  value = 0;
  standing = NONE;

  do {
    d = rollDice( 1, 100 );
    for( i = 0; armorTable[i].type != 0; i++ ) {
      if( level == MINOR ) {
        top = &(armorTable[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(armorTable[i].mediumDtop);
      } else {
        top = &(armorTable[i].majorDtop);
      }

      if( d <= *top ) {
        break;
      }
    }

    switch( armorTable[i].type ) {
      case ENHANCED:
        enh = armorTable[i].enhancement;
        armor = armorTable[i].armor;
        break;
      case SPECIFIC:
        if( armorTable[i].armor ) {
          generateSpecificArmor( opts, level );
        } else {
          generateSpecificShield( opts, level );
        }
        return;
      case SPECIAL:
        specCnt++;
        break;
    }
  } while( enh < 1 );

  sprintf( desc, "+%d ", enh );

  d = rollDice( 1, 100 );
  if( armor ) {
    for( i = 0; armorTypes[i].name != 0; i++ ) {
      if( d <= armorTypes[i].dtop ) {
        value = armorTypes[i].value;
        strcat( desc, armorTypes[i].name );
        strcat( desc, " " );
        break;
      }
    }
  } else {
    for( i = 0; shieldTypes[i].name != 0; i++ ) {
      if( d <= shieldTypes[i].dtop ) {
        value = shieldTypes[i].value;
        strcat( desc, shieldTypes[i].name );
        strcat( desc, " " );
        break;
      }
    }
  }

  if( specCnt > 0 ) {
    strcat( desc, "of " );

    for( i = 0; i < specCnt; i++ ) {
      if( enh >= 10 ) {
        specCnt = i;
        break;
      }

      tries = 0;
      do {
        tries++;
        if( armor ) {
          p = getArmorSpecial( level, &enh );
        } else {
          p = getShieldSpecial( level, &enh );
        }

        d = 0;
        for( j = 0; j < i; j++ ) {
          if( strcmp( slist[j], p ) == 0 ) {
            d = 1;
            break;
          }
        }
      } while( ( d ) && ( tries < 10 ) );

      if( tries >= 10 ) {
        desc[strlen(desc)-2] = 0;
        specCnt = i;
        break;
      }

      slist[i] = strdup( p );

      if( *p == '-' ) {
        specCnt += 2;
      } else {
        strcat( desc, p );
        if( ( i < specCnt-1 ) && ( enh < 10 ) ) {
          strcat( desc, ", " );
        }
      }
    }
  }

  value += ( enh * enh * 1000 );

  if( opts->forceIntelligent || ( rollDice( 1, 100 ) == 1 ) ) {
    strcat( desc, "\n" );
    strcat( desc, determineIntelligence( enh, &standing, &value ) );
  }

  for( i = 0; i < specCnt; i++ ) {
    free( slist[i] );
  }

  addNewTreasure( opts, desc, value*100 );
}


static char* getBane( int* standing ) {
  int d;
  int i;

  do {
    d = rollDice( 1, 100 );
    for( i = 0; baneTable[i].name != 0; i++ ) {
      if( d <= baneTable[i].dtop ) {
        if( opposes( *standing, baneTable[i].standing ) ) {
          break;
        }
        return baneTable[i].name;
      }
    }
  } while( 1 );

  return "unknown";
}


void generateSpecificWeapon( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[ 1024 ];
  int  standing = 0;

  d = rollDice( 1, 100 );
  for( i = 0; specificWeapons[i].name != 0; i++ ) {
    if( level == MEDIUM ) {
      top = &(specificWeapons[i].mediumDtop);
    } else {
      top = &(specificWeapons[i].majorDtop);
    }

    if( d <= *top ) {
      strcpy( desc, specificWeapons[i].name );
      switch( specificWeapons[i].other ) {
        case BANE:
          strcat( desc, " - " );
          strcat( desc, getBane( &standing ) );
          break;
      }
      addNewTreasure( opts, desc, specificWeapons[i].value*100 );
      return;
    }
  }
}


static char* getMeleeSpecial( int level, int restrict, int* standing, int* plusses ) {
  int  d;
  int  i;
  int* top;
  static char buffer[256];
 
  do {
    d = rollDice( 1, 100 );
    for( i = 0; meleeSpecials[i].name != 0; i++ ) {
      if( level == MINOR ) {
        top = &(meleeSpecials[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(meleeSpecials[i].mediumDtop);
      } else {
        top = &(meleeSpecials[i].majorDtop);
      }

      if( d <= *top ) {
        if( ( restrict & meleeSpecials[i].restrict ) != 0 ) {
          if( *plusses + meleeSpecials[i].plus > 10 ) {
            break;
          }
          if( opposes( *standing, meleeSpecials[i].standing ) ) {
            break;
          }
          (*plusses) += meleeSpecials[i].plus;
          (*standing) |= meleeSpecials[i].standing;
          strcpy( buffer, meleeSpecials[i].name );
          switch( meleeSpecials[i].other ) {
    	    case BANE:
  	      strcat( buffer, getBane( standing ) );
	      break;
          }
          return buffer;
        } else {
          break;
        }
      }
    }
  } while( 1 );

  return "[!unknown]";
}


static char* getRangedSpecial( int level, int* standing, int* plusses ) {
  int  d;
  int  i;
  int* top;
  static char buffer[256];

  do {
    d = rollDice( 1, 100 );
    for( i = 0; rangedSpecials[i].name != 0; i++ ) {
      if( level == MINOR ) {
        top = &(rangedSpecials[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(rangedSpecials[i].mediumDtop);
      } else {
        top = &(rangedSpecials[i].majorDtop);
      }
  
      if( d <= *top ) {
        if( *plusses + rangedSpecials[i].plus > 10 ) {
          break;
        }
        if( opposes( *standing, rangedSpecials[i].standing ) ) {
          break;
        }
        (*plusses) += rangedSpecials[i].plus;
        (*standing) |= rangedSpecials[i].standing;
        strcpy( buffer, rangedSpecials[i].name );
        switch( rangedSpecials[i].other ) {
          case BANE:
            strcat( buffer, getBane( standing ) );
            break;
        }
        return buffer;
      }
    }
  } while( 1 );

  return "[!unknown]";
}


void generateWeapon( TREASUREOPTS *opts, int level ) {
  int   i;
  int   d;
  int   j;
  int*  top;
  int   enh;
  int   spcCnt;
  int   melee;
  int   restrict = 0;
  int   intelligent;
  int   tries;
  char* p;
  char  desc[1024];
  char* slist[30];
  int   value;
  int   standing;

  enh = 0;
  spcCnt = 0;
  melee = 0;
  intelligent = 0;
  value = 0;
  standing = NONE;

  do {
    d = rollDice( 1, 100 );
    for( i = 0; weaponTable[i].type != 0; i++ ) {
      if( level == MINOR ) {
        top = &(weaponTable[i].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(weaponTable[i].mediumDtop);
      } else {
        top = &(weaponTable[i].majorDtop);
      }

      if( d <= *top ) {
        break;
      }
    }

    switch( weaponTable[i].type ) {
      case ENHANCED:
        enh = weaponTable[i].enhancement;
        break;
      case SPECIFIC:
        generateSpecificWeapon( opts, level );
        return;
      case SPECIAL:
        spcCnt++;
        break;
    }
  } while( enh < 1 );

  d = rollDice( 1, 100 );
  for( i = 0; weaponTypes[ i ].type != 0; i++ ) {
    if( d <= weaponTypes[ i ].dtop ) {
      break;
    }
  }

  sprintf( desc, "+%d", enh );

  d = rollDice( 1, 100 );
  if( d <= 30 ) {
    strcat( desc, " light-emitting" );
  }

  switch( weaponTypes[ i ].type ) {
    case COMMONMW:
      melee = 1;
      if( rollDice( 1, 100 ) <= 15 ) {
        intelligent = 1;
      }

      d = rollDice( 1, 100 );
      for( i = 0; commonMeleeWeapons[i].name != 0; i++ ) {
        if( d <= commonMeleeWeapons[i].dtop ) {
          strcat( desc, " " );
          strcat( desc, commonMeleeWeapons[i].name );
          restrict = commonMeleeWeapons[i].type;
          value += commonMeleeWeapons[i].value;
          break;
        }
      }
      break;
    case UNCOMMNW:
      d = rollDice( 1, 100 );
      for( i = 0; uncommonWeapons[i].name != 0; i++ ) {
        if( d <= uncommonWeapons[i].dtop ) {
          melee = ( uncommonWeapons[i].type == MELEE );
          if( melee ) {
            if( rollDice( 1, 100 ) <= 15 ) {
              intelligent = 1;
            }
          } else {
            if( rollDice( 1, 100 ) <= 5 ) {
              intelligent = 1;
            }
          }

          strcat( desc, " " );
          strcat( desc, uncommonWeapons[i].name );
          restrict = uncommonWeapons[i].dtype;
          value += uncommonWeapons[i].value;
          break;
        }
      }
      break;
    case COMMONRW:
      melee = 0;
      d = rollDice( 1, 200 );

      if( ( d > 100 ) && ( rollDice( 1, 100 ) <= 5 ) ) {
        intelligent = 1;
      }

      for( i = 0; commonRangedWeapons[i].name != 0; i++ ) {
        if( d <= commonRangedWeapons[i].dtop ) {
          strcat( desc, " " );
          strcat( desc, commonRangedWeapons[i].name );
          value += commonRangedWeapons[i].value;
          break;
        }
      }
      break;
  }

  if( spcCnt > 0 ) {
    strcat( desc, ", " );
    for( i = 0; i < spcCnt; i++ ) {
      if( enh >= 10 ) {
        spcCnt = i;
        break;
      }

      tries = 0;
      do {
        tries++;
        if( melee ) {
      	  p = getMeleeSpecial( level, restrict, &standing, &enh );
        } else {
  	      p = getRangedSpecial( level, &standing, &enh );
        }

        d = 0;
        for( j = 0; j < i; j++ ) {
          if( strcmp( slist[j], p ) == 0 ) {
            d = 1;
            break;
          }
        }
      } while( ( d ) && ( tries < 10 ) );

      if( tries >= 10 ) {
        spcCnt = i;
        desc[strlen(desc)-2] = 0; 
        break;
      }

      slist[i] = strdup( p ); 

      if( *p == '-' ) {
        spcCnt += 2;
      } else {
        strcat( desc, p );
        if( ( i < spcCnt-1 ) && ( enh < 10 ) ) {
          strcat( desc, ", " );
        }
      }
    }
  }

  value += ( enh * enh * 2000 );

  for( i = 0; i < spcCnt; i++ ) {
    free( slist[i] );
  }

  if( opts->forceIntelligent || intelligent ) {
    strcat( desc, "\n" );
    strcat( desc, determineIntelligence( enh, &standing, &value ) );
  }

  addNewTreasure( opts, desc, value*100 );
}


void generatePotion( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[1024];

  strcpy( desc, "potion: " );

  d = rollDice( 1, 100 );
  for( i = 0; potionTable[i].name != 0; i++ ) {
    if( level == MINOR ) {
      top = &(potionTable[i].minorDtop);
    } else if( level == MEDIUM ) {
      top = &(potionTable[i].mediumDtop);
    } else {
      top = &(potionTable[i].majorDtop);
    }

    if( d <= *top ) {
      strcat( desc, potionTable[i].name );
      addNewTreasure( opts, desc, potionTable[i].value*100 );
      return;
    }
  }
}


void generateRing( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[1024];
  int value;
  int standing;

  desc[0] = 0;
  standing = NONE;

  d = rollDice( 1, 100 );
  for( i = 0; ringTable[i].name != 0; i++ ) {
    if( level == MINOR ) {
      top = &(ringTable[i].minorDtop);
    } else if( level == MEDIUM ) {
      top = &(ringTable[i].mediumDtop);
    } else {
      top = &(ringTable[i].majorDtop);
    }

    if( d <= *top ) {
      strcat( desc, "ring of " );
      strcat( desc, ringTable[i].name );
      if( ringTable[i].dcount > 0 ) {
        sprintf( &desc[strlen(desc)], " (%d charges)", rollDice( ringTable[i].dcount, ringTable[i].dtype ) );
      }

      value = ringTable[i].value;

      if( opts->forceIntelligent || ( rollDice( 1, 100 ) == 1 ) ) {
        strcat( desc, "\n" );
        strcat( desc, determineIntelligence( 1, &standing, &value ) );
      }

      addNewTreasure( opts, desc, value*100 );
      return;
    }
  }
}


void generateRod( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[1024];
  int value;
  int standing;

  desc[0] = 0;
  standing = 0;

  d = rollDice( 1, 100 );
  for( i = 0; rodTable[i].name != 0; i++ ) {
    if( level == MINOR ) {
      top = &(rodTable[i].minorDtop);
    } else if( level == MEDIUM ) {
      top = &(rodTable[i].mediumDtop);
    } else {
      top = &(rodTable[i].majorDtop);
    }

    if( d <= *top ) {
      strcat( desc, "rod: " );
      strcat( desc, rodTable[i].name );

      value = rodTable[i].value;

      if( opts->forceIntelligent || ( rollDice( 1, 100 ) == 1 ) ) {
        strcat( desc, "\n" );
        strcat( desc, determineIntelligence( 1, &standing, &value ) );
      }

      addNewTreasure( opts, desc, value*100 );
      return;
    }
  }
}


void generateScroll( TREASUREOPTS *opts, int level ) {
  int i;
  int j;
  int d;
  int type;
  int min;
  int max;
  int count;
  int *top;
  char desc[1024];
  int  value;

  type = 0;
  d = rollDice( 1, 100 );
  for( i = 0; scrollTypes[i].type != 0; i++ ) {
    if( d <= scrollTypes[i].dtop ) {
      type = scrollTypes[i].type;
      break;
    }
  }

  min = 1;
  if( level == MINOR ) {
    max = 3;
  } else if( level == MEDIUM ) {
    max = 4;
  } else {
    max = 6;
  }

  value = 0;

  sprintf( desc, "scroll (%s)\n", ( type == ARCANE ? "arcane" : "divine" ) );
  count = rollDice( min, max/min );
  for( i = 0; i < count; i++ ) {
    d = rollDice( 1, 100 );
    for( j = 0; spellLevels[j].level != 0; j++ ) {
      if( level == MINOR ) {
        top = &(spellLevels[j].minorDtop);
      } else if( level == MEDIUM ) {
        top = &(spellLevels[j].mediumDtop);
      } else {
        top = &(spellLevels[j].majorDtop);
      }

      if( d <= *top ) {
        sprintf( &desc[strlen(desc)], "  - %s (l%d, cl%d)", randomSpell( type, spellLevels[j].level, &value ), spellLevels[j].level, spellLevels[j].caster );
        if( i < count-1 ) {
          strcat( desc, "\n" );
        }
        break;
      }
    }
  }
  addNewTreasure( opts, desc, value*100 );
}


void generateStaff( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[1024];

  d = rollDice( 1, 100 );
  for( i = 0; staffTable[i].name != 0; i++ ) {
    if( level == MINOR ) {
      top = &(staffTable[i].minorDtop);
    } else if( level == MEDIUM ) {
      top = &(staffTable[i].mediumDtop);
    } else {
      top = &(staffTable[i].majorDtop);
    }

    if( d <= *top ) {
      sprintf( desc, "staff of %s (%d charges)", staffTable[i].name, rollDice( 1, 50 ) );
      addNewTreasure( opts, desc, staffTable[i].value*100 );
      return;
    }
  }
}


void generateWand( TREASUREOPTS *opts, int level ) {
  int  d;
  int  i;
  int* top;
  char desc[1024];

  d = rollDice( 1, 100 );
  for( i = 0; wandTable[i].name != 0; i++ ) {
    if( level == MINOR ) {
      top = &(wandTable[i].minorDtop);
    } else if( level == MEDIUM ) {
      top = &(wandTable[i].mediumDtop);
    } else {
      top = &(wandTable[i].majorDtop);
    }

    if( d <= *top ) {
      sprintf( desc, "wand of %s (%d charges)", wandTable[i].name, rollDice( 1, 50 ) );
      addNewTreasure( opts, desc, wandTable[i].value*100 );
      return;
    }
  }
}


void generateWondrousItem( TREASUREOPTS *opts, int level ) {
  WONDROUSITEM* list;
  int d;
  int i;

  if( level == MINOR ) {
    list = minorItems;
  } else if( level == MEDIUM ) {
    list = mediumItems;
  } else {
    list = majorItems;
  }

  d = rollDice( 1, 100 );
  for( i = 0; list[ i ].name != 0; i++ ) {
    if( d <= list[ i ].dtop ) {
      addNewTreasure( opts, list[i].name, list[i].value*100 );
      break;
    }
  }
}


int generateTreasure( TREASUREOPTS *opts, int level, int column, int rollHigher, float mod ) {
  int d;
  int count;
  int i;
  int j;
  int k;
  int level_column;
  int value;
  treasureEntry *t;
  char desc[1024];
  float f;

  if( (int)(mod*1000) == 0 ) {
    return 0;
  }

  level_column = ( level * 15 ) + ( column * 5 );

  d = rollDice( 1, 100 );
  t = NULL;
  for( i = 0; i < 5; i++ ) {
    if( d <= treasureTable[ level_column + i ].pcap ) {
      t = &( treasureTable[ level_column + i ] );
      break;
    }
  }

  count = rollDice( t->tdc, t->tdt ) * t->mul;
  f = mod * count;

  if( ( f < 1 ) && ( f >= 0.5 ) ) {
    count = 1;
  } else if( f < 0.5 ) {
    return 0;
  } else {
    count = (int)f;
  }    
    
  switch( t->ttype ) {
    case NONE:
      break;
    case CP:
      commify( desc, count );
      strcat( desc, " copper coins" );
      addNewTreasure( opts, desc, count );
      break;
    case SP:
      commify( desc, count );
      strcat( desc, " silver coins" );
      addNewTreasure( opts, desc, count*10 );
      break;
    case GP:
      commify( desc, count );
      strcat( desc, " gold coins" );
      addNewTreasure( opts, desc, count*100 );
      break;
    case PP:
      commify( desc, count );
      strcat( desc, " platinum coins" );
      addNewTreasure( opts, desc, count*1000 );
      break;
    case GEMS:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; gemTable[ j ].dcap > 0; j++ ) {
          if( d <= gemTable[ j ].dcap ) {
            d = rollDice( gemTable[ j ].dcount, gemTable[ j ].dtype ) * gemTable[ j ].dmul;
            strcpy( desc, "gemstone - " );
            strcat( desc, randomItem( gemTable[ j ].names ) );
            addNewTreasure( opts, desc, d*100 );
            break;
          }
        }
      }
      break;
    case ART:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; artTable[ j ].dcap > 0; j++ ) {
          if( d <= artTable[ j ].dcap ) {
            d = rollDice( artTable[ j ].dcount, artTable[ j ].dtype ) * artTable[ j ].dmul;
            addNewTreasure( opts, randomItem( artTable[ j ].names ), d*100 );
            break;
          }
        }
      }
      break;
    case MUNDANE:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; mundaneTable[ j ].name != 0; j++ ) {
          if( d <= mundaneTable[ j ].dcap ) {
            strcpy( desc, mundaneTable[ j ].name );

            value = mundaneTable[ j ].value;
            switch( mundaneTable[ j ].other ) {
              case COMMONMW:
                d = rollDice( 1, 100 );
                for( k = 0; commonMeleeWeapons[k].name != 0; k++ ) {
                  if( d <= commonMeleeWeapons[k].dtop ) {
                    strcat( desc, commonMeleeWeapons[k].name );
                    value += commonMeleeWeapons[k].value;
                    break;
                  }
                }
                break;
              case UNCOMMNW:
                d = rollDice( 1, 100 );
                for( k = 0; uncommonWeapons[k].name != 0; k++ ) {
                  if( d <= uncommonWeapons[k].dtop ) {
                    strcat( desc, uncommonWeapons[k].name );
                    value += uncommonWeapons[k].value;
                    break;
                  }
                }
                break;
              case COMMONRW:
                d = rollDice( 1, 200 );
                for( k = 0; commonRangedWeapons[k].name != 0; k++ ) {
                  if( d <= commonRangedWeapons[k].dtop ) {
                    strcat( desc, commonRangedWeapons[k].name );
                    value += commonRangedWeapons[k].value;
                    break;
                  }
                }
                break;
            }

            if( mundaneTable[ j ].dtype > 1 ) {
              d = rollDice( mundaneTable[ j ].dcount, mundaneTable[ j ].dtype ) * mundaneTable[ j ].dmul;
              sprintf( &desc[strlen(desc)], " (%d)", d );
            } else {
              d = 1;
            }

            value *= d;
            addNewTreasure( opts, desc, value*100 );
            break;
          }
        }
      }
      break;
    case MINOR:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; magicTable[ j ].type != 0; j++ ) {
          if( d <= magicTable[ j ].minorDtop ) {
            switch( magicTable[ j ].type ) {
              case ARMOR:
                generateArmor( opts, MINOR );
                break;
              case WEAPONS:
                generateWeapon( opts, MINOR );
                break;
              case POTIONS:
                generatePotion( opts, MINOR );
                break;
              case RINGS:
                generateRing( opts, MINOR );
                break;
              case RODS:
                generateRod( opts, MINOR );
                break;
              case SCROLLS:
                generateScroll( opts, MINOR );
                break;
              case STAFFS:
                generateStaff( opts, MINOR );
                break;
              case WANDS:
                generateWand( opts, MINOR );
                break;
              case WONDROUS:
                generateWondrousItem( opts, MINOR );
                break;
            }
            break;
          }
        }
      }
      break;
    case MEDIUM:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; magicTable[ j ].type != 0; j++ ) {
          if( d <= magicTable[ j ].mediumDtop ) {
            switch( magicTable[ j ].type ) {
              case ARMOR:
                generateArmor( opts, MEDIUM );
                break;
              case WEAPONS:
                generateWeapon( opts, MEDIUM );
                break;
              case POTIONS:
                generatePotion( opts, MEDIUM );
                break;
              case RINGS:
                generateRing( opts, MEDIUM );
                break;
              case RODS:
                generateRod( opts, MEDIUM );
                break;
              case SCROLLS:
                generateScroll( opts, MEDIUM );
                break;
              case STAFFS:
                generateStaff( opts, MEDIUM );
                break;
              case WANDS:
                generateWand( opts, MEDIUM );
                break;
              case WONDROUS:
                generateWondrousItem( opts, MEDIUM );
                break;
            }
            break;
          }
        }
      }
      break;
    case MAJOR:
      for( i = 0; i < count; i++ ) {
        d = rollDice( 1, 100 );
        for( j = 0; magicTable[ j ].type != 0; j++ ) {
          if( d <= magicTable[ j ].majorDtop ) {
            switch( magicTable[ j ].type ) {
              case ARMOR:
                generateArmor( opts, MAJOR );
                break;
              case WEAPONS:
                generateWeapon( opts, MAJOR );
                break;
              case POTIONS:
                generatePotion( opts, MAJOR );
                break;
              case RINGS:
                generateRing( opts, MAJOR );
                break;
              case RODS:
                generateRod( opts, MAJOR );
                break;
              case SCROLLS:
                generateScroll( opts, MAJOR );
                break;
              case STAFFS:
                generateStaff( opts, MAJOR );
                break;
              case WANDS:
                generateWand( opts, MAJOR );
                break;
              case WONDROUS:
                generateWondrousItem( opts, MAJOR );
                break;
            }
            break;
          }
        }
      }
      break;
  }

  if( rollHigher && ( d >= 96 ) ) {
    level++;
    if( level >= LEVEL_COUNT ) {
      level = LEVEL_COUNT-1;
    }
    count += generateTreasure( opts, level, column, 0, mod );
  }

  return count;
}


void generateRandomTreasureEx( TREASUREOPTS *opts, int level, float* mods ) {
  generateTreasure( opts, level, 0, 1, mods[0] );
  generateTreasure( opts, level, 1, 1, mods[1] );
  generateTreasure( opts, level, 2, 1, mods[2] );
}


void generateRandomTreasure( TREASUREOPTS* opts, int level ) {
  float mods[3];

  mods[0] = mods[1] = mods[2] = 1.0;
  generateRandomTreasureEx( opts, level, mods );
}
