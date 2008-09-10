/* ---------------------------------------------------------------------- *
 * This file is in the public domain, and may be used, modified, and
 * distributed without restriction.
 * ---------------------------------------------------------------------- *
 * JBDungeonData
 *
 * Author: Jamis Buck <jamis@jamisbuck.org>
 * Homepage: http://github.com/jamis/dnd-dungeon
 * ---------------------------------------------------------------------- */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "jbdungeon.h"
#include "jbdungeondata.h"

#include "dndconst.h"
#include "dndutil.h"
#include "gameutil.h"

#include "npcEngine.h"
#include "treasureEngine.h"

#define SPELLTRAP             ((char*)14)
#define DRAGON                ((char*)11)

#define MONSTER               ( 0x0001 )
#define FEATURE               ( 0x0002 )
#define TREASURE              ( 0x0004 )
#define TRAP                  ( 0x0008 )

#define REROLL_ONCE           ( 0x10000000 )
#define REROLL_ANY            ( 0x20000000 )  

/* door and wall attribute descriptions */

static struct {
  long  flag;
  char* desc;
} s_flagDescriptions[] = {
  { dtWOODEN,          "wooden" },
  { dtSTONE,           "stone" },
  { dtIRON,            "iron" },
  { cdtILLUSORYWALL,   "illusory wall" },
  { cdtFALSEWALL,      "false wall" },
  { dtSIMPLE,          "simple" },
  { dtGOOD,            "good" },
  { dtSTRONG,          "strong" },
  { dtFREE,            "free" },
  { dtSTUCK,           "stuck" },
  { dtLOCKED,          "locked" },
  { dtSIDESLIDE,       "side-sliding" },
  { dtDOWNSLIDE,       "down-sliding" },
  { dtUPSLIDE,         "up-sliding" },
  { dtMAGIC,           "magically reinforced" },
  { sdtROTATINGWALL,   "rotating wall" },
  { sdtPASSWALL,       "passwall" },
  { sdtPUSHBRICK,      "push-brick trigger" },
  { sdtMAGICWORD,      "magic word trigger" },
  { sdtGESTURE,        "gesture trigger" },
  { sdtPRESSUREPLATE,  "pressure-plate trigger" },
  { cdtBEHINDTAPESTRY, "behind tapestry" },
  { cdtBEHINDRUBBISH,  "behind rubbish" },
  { dtTRAPPED,         "trapped" },
  { 0,           0 }
};


typedef struct __doortype__ DOORTYPE;
struct __doortype__ {
  int  dtop;
  long data;
};

/* "normal" door types, by percentage chance of occuring */

static DOORTYPE s_doorTypes[] = {
  {   8, dtWOODEN|dtSIMPLE|dtFREE },
  {   9, dtWOODEN|dtSIMPLE|dtFREE|dtTRAPPED },
  {  23, dtWOODEN|dtSIMPLE|dtSTUCK },
  {  24, dtWOODEN|dtSIMPLE|dtSTUCK|dtTRAPPED },
  {  29, dtWOODEN|dtSIMPLE|dtLOCKED },
  {  30, dtWOODEN|dtSIMPLE|dtLOCKED|dtTRAPPED },
  {  35, dtWOODEN|dtGOOD|dtFREE },
  {  36, dtWOODEN|dtGOOD|dtFREE|dtTRAPPED },
  {  44, dtWOODEN|dtGOOD|dtSTUCK },
  {  45, dtWOODEN|dtGOOD|dtSTUCK|dtTRAPPED },
  {  49, dtWOODEN|dtGOOD|dtLOCKED },
  {  50, dtWOODEN|dtGOOD|dtLOCKED|dtTRAPPED },
  {  55, dtWOODEN|dtSTRONG|dtFREE },
  {  56, dtWOODEN|dtSTRONG|dtFREE|dtTRAPPED },
  {  64, dtWOODEN|dtSTRONG|dtSTUCK },
  {  65, dtWOODEN|dtSTRONG|dtSTUCK|dtTRAPPED },
  {  69, dtWOODEN|dtSTRONG|dtLOCKED },
  {  70, dtWOODEN|dtSTRONG|dtLOCKED|dtTRAPPED },
  {  71, dtSTONE|dtFREE },
  {  72, dtSTONE|dtFREE|dtTRAPPED },
  {  75, dtSTONE|dtSTUCK },
  {  76, dtSTONE|dtSTUCK|dtTRAPPED },
  {  79, dtSTONE|dtLOCKED },
  {  80, dtSTONE|dtLOCKED|dtTRAPPED },
  {  81, dtIRON|dtFREE },
  {  82, dtIRON|dtFREE|dtTRAPPED },
  {  85, dtIRON|dtSTUCK },
  {  89, dtIRON|dtLOCKED },
  {  90, dtIRON|dtLOCKED|dtTRAPPED },
  {  93, dtSIDESLIDE | REROLL_ONCE },
  {  96, dtDOWNSLIDE | REROLL_ONCE },
  {  99, dtUPSLIDE | REROLL_ONCE },
  { 100, dtMAGIC | REROLL_ONCE },
  {   0, 0 }
};


/* secret door types, by percentage chance of occuring */

static DOORTYPE s_secretDoorTypes[] = {
  {   4, sdtROTATINGWALL|sdtPUSHBRICK },
  {   5, sdtROTATINGWALL|sdtPUSHBRICK|dtTRAPPED },
  {   9, sdtROTATINGWALL|sdtMAGICWORD },
  {  10, sdtROTATINGWALL|sdtMAGICWORD|dtTRAPPED },
  {  14, sdtROTATINGWALL|sdtGESTURE },
  {  15, sdtROTATINGWALL|sdtGESTURE|dtTRAPPED },
  {  19, sdtROTATINGWALL|sdtPRESSUREPLATE },
  {  20, sdtROTATINGWALL|sdtPRESSUREPLATE|dtTRAPPED },
  {  24, sdtPASSWALL|sdtPUSHBRICK },
  {  25, sdtPASSWALL|sdtPUSHBRICK|dtTRAPPED },
  {  29, sdtPASSWALL|sdtMAGICWORD },
  {  30, sdtPASSWALL|sdtMAGICWORD|dtTRAPPED },
  {  34, sdtPASSWALL|sdtGESTURE },
  {  35, sdtPASSWALL|sdtGESTURE|dtTRAPPED },
  {  39, sdtPASSWALL|sdtPRESSUREPLATE },
  {  40, sdtPASSWALL|sdtPRESSUREPLATE|dtTRAPPED },
  {  44, dtSIDESLIDE|sdtPUSHBRICK },
  {  45, dtSIDESLIDE|sdtPUSHBRICK|dtTRAPPED },
  {  49, dtSIDESLIDE|sdtMAGICWORD },
  {  50, dtSIDESLIDE|sdtMAGICWORD|dtTRAPPED },
  {  54, dtSIDESLIDE|sdtGESTURE },
  {  55, dtSIDESLIDE|sdtGESTURE|dtTRAPPED },
  {  59, dtSIDESLIDE|sdtPRESSUREPLATE },
  {  60, dtSIDESLIDE|sdtPRESSUREPLATE|dtTRAPPED },
  {  64, dtUPSLIDE|sdtPUSHBRICK },
  {  65, dtUPSLIDE|sdtPUSHBRICK|dtTRAPPED },
  {  69, dtUPSLIDE|sdtMAGICWORD },
  {  70, dtUPSLIDE|sdtMAGICWORD|dtTRAPPED },
  {  74, dtUPSLIDE|sdtGESTURE },
  {  75, dtUPSLIDE|sdtGESTURE|dtTRAPPED },
  {  79, dtUPSLIDE|sdtPRESSUREPLATE },
  {  80, dtUPSLIDE|sdtPRESSUREPLATE|dtTRAPPED },
  {  84, dtDOWNSLIDE|sdtPUSHBRICK },
  {  85, dtDOWNSLIDE|sdtPUSHBRICK|dtTRAPPED },
  {  89, dtDOWNSLIDE|sdtMAGICWORD },
  {  90, dtDOWNSLIDE|sdtMAGICWORD|dtTRAPPED },
  {  94, dtDOWNSLIDE|sdtGESTURE },
  {  95, dtDOWNSLIDE|sdtGESTURE|dtTRAPPED },
  {  99, dtDOWNSLIDE|sdtPRESSUREPLATE },
  { 100, dtDOWNSLIDE|sdtPRESSUREPLATE|dtTRAPPED },
  {   0, 0 }
};

/* concealed door types, by percentage chance of occuring */

static DOORTYPE s_concealedDoorTypes[] = {
  {  35, cdtILLUSORYWALL },
  {  36, cdtILLUSORYWALL|dtTRAPPED },
  {  70, cdtFALSEWALL|dtFREE },
  {  71, cdtFALSEWALL|dtFREE|dtTRAPPED },
  {  99, cdtFALSEWALL|dtSTUCK },
  { 100, cdtFALSEWALL|dtSTUCK|dtTRAPPED },
  {   0, 0 }
};


/* room contents, by percentage chance of occuring.  Note that is directly
 * from the table in the DMG. */

static struct {
  int  dtop;
  int  contents;
} s_roomContents[] = {
  {  18, MONSTER },
  {  44, MONSTER | FEATURE },
  {  45, MONSTER | TREASURE },
  {  46, MONSTER | TRAP },
  {  47, MONSTER | FEATURE | TREASURE },
  {  48, MONSTER | FEATURE | TRAP },
  {  49, MONSTER | TREASURE | TRAP },
  {  50, MONSTER | FEATURE | TREASURE | TRAP },
  {  76, FEATURE },
  {  77, FEATURE | TREASURE },
  {  78, FEATURE | TRAP },
  {  79, FEATURE | TREASURE | TRAP },
  {  80, TREASURE },
  {  81, TREASURE | TRAP },
  {  82, TRAP },
  { 100, 0 },
  {   0, 0 }
};


typedef struct __traplist__ TRAPLIST;
struct __traplist__ {
  int   dtop;
  int   forDoor;
  char* trap;
};


typedef struct __featurelist__ FEATURELIST;
struct __featurelist__ {
  int   dtop;
  char* feature;
};


/* trap types (CR 1-3), by percentage chance of occuring.  Note that is 
 * directly from the table in the DMG. */

TRAPLIST traps1[] = {
  {  10, 1, "arrow trap (CR1)" },
  {  15, 1, "spear trap (CR2)" },
  {  25, 1, "pit trap (20 ft. deep) (CR1)" },
  {  35, 1, "spiked pit trap (20 ft. deep) (CR2)" },
  {  45, 1, "pit trap (40 ft. deep) (CR2)" },
  {  50, 1, "spiked pit trap (40 ft. deep) (CR3)" },
  {  55, 1, "pit trap (30 ft. deep) (CR3)" },
  {  65, 1, "poison needle trap (CR2)" },
  {  70, 1, "hail of needles (CR1)" },
  {  75, 1, "scything blade trap (CR1)" },
  {  80, 1, "large net trap (CR1)" },
  {  85, 1, "portculis trap (CR2)" },
  {  90, 1, "flame jet (CR2)" },
  {  95, 1, "lightning blast (CR3)" },
  { 100, 0, "illusion over spiked pit (CR3)" },
  {   0, 0, 0 }
};

/* trap types (CR 4+), by percentage chance of occuring.  Note that is 
 * directly from the table in the DMG. */

TRAPLIST traps2[] = {
  {  10, 1, "spiked pit trap (60 ft. deep) (CR4)" },
  {  20, 1, "pit trap (80 ft. deep) (CR4)" },
  {  25, 1, "spiked pit trap (80 ft. deep) (CR5)" },
  {  30, 1, "pit trap (100 ft. deep) (CR5)" },
  {  35, 1, "spiked pit trap (100 ft. deep) (CR6)" },
  {  38, 1, "crushing wall trap (CR10)" },
  {  43, 1, "falling block trap (CR5)" },
  {  45, 1, "poison gas trap (CR10)" },
  {  50, 1, "flooding room trap (CR5)" },
  {  55, 1, "globe of cold (CR4)" },
  {  60, 1, "electrified floor (CR4)" },
  {  65, 1, "air sucked out of room (CR5)" },
  {  70, 1, "floor transforms into acid (CR6)" },
  { 100, 1, SPELLTRAP },
  {   0, 0, 0 }
};


/* possible spells for use in traps, by percentage chance of occuring.  Note
 * that is directly from the table in the DMG. */

static struct {
  int dtop;
  int spell;
} s_spellsForTraps[] = {
  {   1, spACIDFOG },
  {   2, spALARM },
  {   3, spANIMATEOBJECTS },
  {   4, spANTIMAGICFIELD },
  {   5, spBIGBYSCLENCHEDFIST },
  {   6, spBIGBYSFORCEFULHAND },
  {   7, spBIGBYSGRASPINGHAND },
  {   8, spBINDING },
  {   9, spBLADEBARRIER },
  {  10, spBLINDNESSDEAFNESS },
  {  11, spCIRCLEOFDEATH },
  {  12, spCOLORSPRAY },
  {  13, spCONFUSION },
  {  14, spCONTAGION },
  {  15, spDARKNESS },
  {  16, spDISINTEGRATE },
  {  17, spDISPELGOOD },
  {  18, spDISPELMAGIC },
  {  19, spDOMINATEPERSON },
  {  20, spDOOM },
  {  21, spENERGYDRAIN },
  {  22, spENERVATION },
  {  23, spENLARGE },
  {  24, spEXPLOSIVERUNES },
  {  25, spEYEBITE },
  {  26, spFALSEVISION },
  {  27, spFEAR },
  {  28, spFEEBLEMIND },
  {  29, spFIREBALL },
  {  30, spFIRETRAP },
  {  31, spFLAMINGSPHERE },
  {  32, spFLESHTOSTONE },
  {  33, spFORBIDDANCE },
  {  34, spFORCECAGE },
  {  35, spGATE },
  {  36, spGEASQUEST },
  {  37, spGIANTVERMIN },
  {  38, spGLYPHOFWARDING },
  {  39, spGREASE },
  {  40, spHARM },
  {  41, spHOLDMONSTER },
  {  42, spHOLDPERSON },
  {  43, spIMPRISONMENT },
  {  44, spINFLICTCRITICALWOUNDS },
  {  45, spINFLICTLIGHTWOUNDS },
  {  46, spINFLICTMODERATEWOUNDS },
  {  47, spINFLICTSERIOUSWOUNDS },
  {  48, spINVISIBILITY },
  {  49, spLEVITATE },
  {  50, spLIGHTNINGBOLT },
  {  51, spMAGICJAR },
  {  52, spMAGICMISSILE },
  {  53, spMASSSUGGESTION },
  {  54, spMELFSACIDARROW },
  {  55, spMINDFOG },
  {  56, spMORDENKAINENSDISJUNCTION },
  {  57, spNIGHTMARE },
  {  58, spOTILUKESTELEKINETICSPHERE },
  {  59, spPERMANENCY },
  {  60, spPERMANENTIMAGE },
  {  61, spPLANESHIFT },
  {  62, spPOLYMORPHOTHER },
  {  63, spPOWERWORDKILL },
  {  64, spPRISMATICSPRAY },
  {  65, spPROGRAMMEDIMAGE },
  {  66, spREDUCE },
  {  67, spREPULSION },
  {  68, spREVERSEGRAVITY },
  {  69, spSCREEN },
  {  70, spSEPIASNAKESIGIL },
  {  71, spSHATTER },
  {  72, spSILENCE },
  {  73, spSLAYLIVING },
  {  74, spSLOW },
  {  75, spSPELLTURNING },
  {  76, spSUGGESTION },
  {  77, spSUMMONMONSTERI },
  {  78, spSUMMONMONSTERII },
  {  79, spSUMMONMONSTERIII },
  {  80, spSUMMONMONSTERIV },
  {  81, spSUMMONMONSTERIX },
  {  82, spSUMMONMONSTERV },
  {  83, spSUMMONMONSTERVI },
  {  84, spSUMMONMONSTERVII },
  {  85, spSUMMONMONSTERVIII },
  {  86, spSUMMONMONSTERIX },
  {  87, spSYMBOL },
  {  88, spTASHASHIDEOUSLAUGHTER },
  {  89, spTELEKINESIS },
  {  90, spTELEPORT },
  {  91, spTEMPORALSTASIS },
  {  92, spTRAPTHESOUL },
  {  93, spVANISH },
  {  94, spWALLOFFIRE },
  {  95, spWALLOFFORCE },
  {  96, spWALLOFIRON },
  {  97, spWALLOFSTONE },
  {  98, spWEB },
  {  99, spWEIRD },
  { 100, spWORDOFCHAOS },
  {   0, 0 }
};


/* major features of rooms, by percentage chance of occuring.  Note
 * that is directly from the table in the DMG. */

static FEATURELIST s_majorFeatures[] = {
  {   1, "alcove" },
  {   2, "altar" },
  {   3, "arch" },
  {   4, "arrow slit (wall)/murder hole (ceiling)" },
  {   5, "balcony" },
  {   6, "barrel" },
  {   7, "bed" },
  {   8, "bench" },
  {   9, "bookcase" },
  {  10, "brazier" },
  {  11, "cage" },
  {  12, "caldron" },
  {  13, "carpet" },
  {  14, "carving" },
  {  15, "casket" },
  {  16, "catwalk" },
  {  17, "chair" },
  {  18, "chandelier" },
  {  19, "charcoal bin" },
  {  20, "chasm" },
  {  21, "chest" },
  {  22, "chest of drawers" },
  {  23, "chute" },
  {  24, "coat rack" },
  {  25, "collapsed wall" },
  {  26, "crate" },
  {  27, "cupboard" },
  {  28, "curtain" },
  {  29, "divan" },
  {  30, "dome" },
  {  31, "door (broken)" },
  {  32, "dung heap" },
  {  33, "evil symbol" },
  {  34, "fallen stones" },
  {  35, "firepit" },
  {  36, "fireplace" },
  {  37, "font" },
  {  38, "forge" },
  {  39, "fountain" },
  {  40, "furniture (broken)" },
  {  41, "gong" },
  {  42, "hay (pile)" },
  {  43, "hole" },
  {  44, "hole (blasted)" },
  {  45, "idol" },
  {  46, "iron bars" },
  {  47, "iron maiden" },
  {  48, "kiln" },
  {  49, "ladder" },
  {  50, "ledge" },
  {  51, "loom" },
  {  52, "loose masonry" },
  {  53, "manacles" },
  {  54, "manger" },
  {  55, "mirror" },
  {  56, "mosaic" },
  {  57, "mound of rubble" },
  {  58, "oven" },
  {  59, "overhang" },
  {  60, "painting" },
  {  61, "partially collapsed ceiling" },
  {  62, "pedestal" },
  {  63, "peephole" },
  {  64, "pillar" },
  {  65, "pillory" },
  {  66, "pit (shallow)" },
  {  67, "platform" },
  {  68, "pool" },
  {  69, "portcullis" },
  {  70, "rack" },
  {  71, "ramp" },
  {  72, "recess" },
  {  73, "relief" },
  {  74, "sconce" },
  {  75, "screen" },
  {  76, "shaft" },
  {  77, "shelf" },
  {  78, "shrine" },
  {  79, "spinning wheel" },
  {  80, "stall or pen" },
  {  81, "statue" },
  {  82, "statue (toppled)" },
  {  83, "steps" },
  {  84, "stool" },
  {  85, "stuffed beast" },
  {  86, "sunken area" },
  {  87, "table (large)" },
  {  88, "table (small)" },
  {  89, "tapestry" },
  {  90, "throne" },
  {  91, "trash (pile)" },
  {  92, "tripod" },
  {  93, "trough" },
  {  94, "tub" },
  {  95, "wall basin" },
  {  96, "wardrobe" },
  {  97, "weapon rack" },
  {  98, "well" },
  {  99, "winch and pulley" },
  { 100, "workbench" },
  {   0, 0 }
};


/* minor features of rooms, by percentage chance of occuring.  Note
 * that is directly from the table in the DMG. */

static FEATURELIST s_minorFeatures[] = {
  {   1, "anvil" },
  {   2, "ash" },
  {   3, "backpack" },
  {   4, "bale (straw)" },
  {   5, "bellows" },
  {   6, "belt" },
  {   7, "bits of fur" },
  {   8, "blanket" },
  {   9, "bloodstain" },
  {  10, "bones (humanoid)" },
  {  11, "bones (nonhumanoid)" },
  {  12, "books" },
  {  13, "boots" },
  {  14, "bottle" },
  {  15, "box" },
  {  16, "branding iron" },
  {  17, "broken glass" },
  {  18, "bucket" },
  {  19, "candle" },
  {  20, "candelabra" },
  {  21, "cards (playing cards)" },
  {  22, "chains" },
  {  23, "claw marks" },
  {  24, "cleaver" },
  {  25, "clothing" },
  {  26, "cobwebs" },
  {  27, "cold spot" },
  {  28, "corpse (adventurer)" },
  {  29, "corpse (monster)" },
  {  30, "cracks" },
  {  31, "dice" },
  {  32, "discarded weapons" },
  {  33, "dishes" },
  {  34, "dipping water" },
  {  35, "drum" },
  {  36, "dust" },
  {  37, "engraving" },
  {  38, "equipment (broken)" },
  {  39, "equipment (usable)" },
  {  40, "flask" },
  {  41, "flint and tinder" },
  {  42, "foodstuffs (spoiled)" },
  {  43, "foodstuffs (edible)" },
  {  44, "fungus" },
  {  45, "grinder" },
  {  46, "hook" },
  {  47, "horn" },
  {  48, "hourglass" },
  {  49, "insects" },
  {  50, "jar" },
  {  51, "keg" },
  {  52, "key" },
  {  53, "lamp" },
  {  54, "lantern" },
  {  55, "markings" },
  {  56, "mold" },
  {  57, "mud" },
  {  58, "mug" },
  {  59, "musical instrument" },
  {  60, "mysterious stain" },
  {  61, "nest (animal)" },
  {  62, "odor (unidentifiable)" },
  {  63, "oil (fuel)" },
  {  64, "oil (scented)" },
  {  65, "paint" },
  {  66, "paper" },
  {  67, "pillows" },
  {  68, "pipe (smoking pipe)" },
  {  69, "pole" },
  {  70, "pot" },
  {  71, "pottery shard" },
  {  72, "pouch" },
  {  73, "puddle (water)" },
  {  74, "rags" },
  {  75, "razor" },
  {  76, "rivulet" },
  {  77, "ropes" },
  {  78, "runes" },
  {  79, "sack" },
  {  80, "scattered stones" },
  {  81, "scorch marks" },
  {  82, "scroll (nonmagical)" },
  {  83, "scroll case (empty)" },
  {  84, "skull" },
  {  85, "slime" },
  {  86, "sound (unexplained)" },
  {  87, "spices" },
  {  88, "spike" },
  {  89, "teeth" },
  {  90, "tongs" },
  {  91, "tools" },
  {  92, "torch (stub)" },
  {  93, "tray" },
  {  94, "trophy" },
  {  95, "twine" },
  {  96, "urn" },
  {  97, "utensils" },
  {  98, "whetstone" },
  {  99, "wood (scraps)" },
  { 100, "words (scrawled)" },
  {   0, 0 }
};


typedef struct __dungeonlevel__ DUNGEONLEVEL;
struct __dungeonlevel__ {
  int dtop;
  int level;
  int mul;
  int div;
};

/* encounter modifiers by dungeon level, by percentage chance.  See the
 * DMG for a fuller description of these tables */

static DUNGEONLEVEL one[] = {
  {  70, 1, 1, 1 },
  {  90, 2, 1, 2 },
  { 100, 3, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL two[] = {
  {  20, 1, 2, 1 },
  {  70, 2, 1, 1 },
  {  80, 3, 2, 3 },
  {  90, 4, 1, 2 },
  { 100, 5, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL three[] = {
  {  10, 1, 3, 1 },
  {  30, 2, 3, 2 },
  {  70, 3, 1, 1 },
  {  80, 4, 2, 3 },
  {  90, 5, 1, 2 },
  { 100, 6, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL four[] = {
  {  10, 1, 4, 1 },
  {  20, 2, 2, 1 },
  {  30, 3, 3, 2 },
  {  70, 4, 1, 1 },
  {  80, 5, 2, 3 },
  {  90, 6, 1, 2 },
  { 100, 7, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL five[] = {
  {   5, 2, 4, 1 },
  {  10, 2, 3, 1 },
  {  20, 3, 2, 1 },
  {  30, 4, 3, 2 },
  {  70, 5, 1, 1 },
  {  80, 6, 2, 3 },
  {  90, 7, 1, 2 },
  { 100, 8, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL six[] = {
  {   5, 2, 4, 1 },
  {  10, 3, 3, 1 },
  {  20, 4, 2, 1 },
  {  30, 5, 3, 2 },
  {  70, 6, 1, 1 },
  {  80, 7, 2, 3 },
  {  90, 8, 1, 2 },
  { 100, 9, 1, 3 },
  {   0, 0, 0, 0 }
};

static DUNGEONLEVEL seven[] = {
  {   5,  3, 4, 1 },
  {  10,  4, 3, 1 },
  {  20,  5, 2, 1 },
  {  30,  6, 3, 2 },
  {  70,  7, 1, 1 },
  {  80,  8, 2, 3 },
  {  90,  9, 1, 2 },
  { 100, 10, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL eight[] = {
  {   5,  4, 4, 1 },
  {  10,  5, 3, 1 },
  {  20,  6, 2, 1 },
  {  30,  7, 3, 2 },
  {  70,  8, 1, 1 },
  {  80,  9, 2, 3 },
  {  90, 10, 1, 2 },
  { 100, 11, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL nine[] = {
  {   5,  5, 4, 1 },
  {  10,  6, 3, 1 },
  {  20,  7, 2, 1 },
  {  30,  8, 3, 2 },
  {  70,  9, 1, 1 },
  {  80, 10, 2, 3 },
  {  90, 11, 1, 2 },
  { 100, 12, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL ten[] = {
  {   5,  6, 4, 1 },
  {  10,  7, 3, 1 },
  {  20,  8, 2, 1 },
  {  30,  9, 3, 2 },
  {  70, 10, 1, 1 },
  {  80, 11, 2, 3 },
  {  90, 12, 1, 2 },
  { 100, 13, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL eleven[] = {
  {   5,  7, 4, 1 },
  {  10,  8, 3, 1 },
  {  20,  9, 2, 1 },
  {  30, 10, 3, 2 },
  {  70, 11, 1, 1 },
  {  80, 12, 2, 3 },
  {  90, 13, 1, 2 },
  { 100, 14, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL twelve[] = {
  {   5,  8, 4, 1 },
  {  10,  9, 3, 1 },
  {  20, 10, 2, 1 },
  {  30, 11, 3, 2 },
  {  70, 12, 1, 1 },
  {  80, 13, 2, 3 },
  {  90, 14, 1, 2 },
  { 100, 15, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL thirteen[] = {
  {   5,  9, 4, 1 },
  {  10, 10, 3, 1 },
  {  20, 11, 2, 1 },
  {  30, 12, 3, 2 },
  {  70, 13, 1, 1 },
  {  80, 14, 2, 3 },
  {  90, 15, 1, 2 },
  { 100, 16, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL fourteen[] = {
  {   5, 10, 4, 1 },
  {  10, 11, 3, 1 },
  {  20, 12, 2, 1 },
  {  30, 13, 3, 2 },
  {  70, 14, 1, 1 },
  {  80, 15, 2, 3 },
  {  90, 16, 1, 2 },
  { 100, 17, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL fifteen[] = {
  {   5, 11, 4, 1 },
  {  10, 12, 3, 1 },
  {  20, 13, 2, 1 },
  {  30, 14, 3, 2 },
  {  70, 15, 1, 1 },
  {  80, 16, 2, 3 },
  {  90, 17, 1, 2 },
  { 100, 18, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL sixteen[] = {
  {   5, 12, 4, 1 },
  {  10, 13, 3, 1 },
  {  20, 14, 2, 1 },
  {  30, 15, 3, 2 },
  {  70, 16, 1, 1 },
  {  80, 17, 2, 3 },
  {  90, 18, 1, 2 },
  { 100, 19, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL seventeen[] = {
  {   5, 13, 4, 1 },
  {  10, 14, 3, 1 },
  {  20, 15, 2, 1 },
  {  30, 16, 3, 2 },
  {  70, 17, 1, 1 },
  {  80, 18, 2, 3 },
  {  90, 19, 1, 2 },
  { 100, 20, 1, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL eighteen[] = {
  {   5, 14, 4, 1 },
  {  10, 15, 3, 1 },
  {  20, 16, 2, 1 },
  {  30, 17, 3, 2 },
  {  70, 18, 1, 1 },
  {  80, 19, 2, 3 },
  { 100, 20, 1, 2 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL nineteen[] = {
  {   5, 15, 4, 1 },
  {  10, 16, 3, 1 },
  {  20, 17, 2, 1 },
  {  30, 18, 3, 2 },
  {  80, 19, 1, 1 },
  { 100, 20, 2, 3 },
  {   0,  0, 0, 0 }
};

static DUNGEONLEVEL twenty[] = {
  {   5, 16, 4, 1 },
  {  10, 17, 3, 1 },
  {  20, 18, 2, 1 },
  {  30, 19, 3, 2 },
  { 100, 20, 1, 1 },
  {   0,  0, 0, 0 }
};

DUNGEONLEVEL* s_masterMonsterTable[] =
  { 0, one, two, three, four, five, six, seven, eight, nine, ten, eleven,
    twelve, thirteen, fourteen, fifteen, sixteen, seventeen, eighteen,
    nineteen, twenty };

typedef struct __dungeonmonsters__ DUNGEONMONSTERS;
struct __dungeonmonsters__ {
  int    dtop;
  int    dcount;
  int    dtype;
  int    dmod;
  double treasureMod;
  int    levelMod;
  int    npcRace;
  int    npcClass;
  int    npcMinLevel;
  int    npcMaxLevel;
  char*  monster;
};

/* encounter table by dungeon level, by percentage chance of encountering
 * that type of monster.  See the DMG for a fuller description of these
 * tables */

DUNGEONMONSTERS mOne[] = {
  {   4, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "centipede, medium-size monstrous (vermin)" },
  {   9, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dire rat" },
  {  14, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "fire beetle, giant (vermin)" },
  {  17, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "scorpion, small monstrous (vermin)" },
  {  20, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "spider, small monstrous (vermin)" },
  {  25, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, DRAGON },
  {  30, 1, 3, 0, 0.8, 1, rcDWARF_HILL, npcWARRIOR, 1, 1, 0 },
  {  35, 1, 3, 0, 0.8, 1, rcELF_HIGH, npcWARRIOR, 1, 1, 0 },
  {  40, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 1, 1, 0 },
  {  45, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "darkmantle" },
  {  55, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "krenshar" },
  {  60, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "lemure (devil)" },
  {  65, 1, 4, 2, 0.8, 1, 0, 0, 0, 0, "goblin" },
  {  70, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, "hobgoblin" },
  {  70, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "goblin" },
  {  80, 1, 6, 3, 0.8, 1, 0, 0, 0, 0, "kobold" },
  {  90, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "skeleton, medium-size [human]" },
  { 100, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "zombie, medium-size [human]" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mTwo[] = {
  {   5, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "centipede, large monstrous (vermin)" },
  {  10, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "giant ant (vermin)" },
  {  15, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "scorpion, medium-size monstrous (vermin)" },
  {  20, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "spider, medium-size monstrous (vermin)" },
  {  25, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  30, 1, 4, 2, 0.8, 2, rcELF_HIGH, npcWARRIOR, 1, 2, 0 },
  {  35, 1, 3, 0, 0.0, 0, raceANY_DMG, classANY_NPC, 1, 1, 0 },
  {  37, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "choker" },
  {  42, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "ethereal marauder" },
  {  45, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "shrieker" },
  {  50, 1, 4, 2, 0.8, 2, 0, 0, 0, 0, "formian workers" },
  {  55, 1, 4, 2, 0.8, 2, 0, 0, 0, 0, "hobgoblin" },
  {  60, 1, 3, 0, 0.8, 2, 0, 0, 0, 0, "hobgoblin" },
  {  60, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "goblin" },
  {  70, 1, 3, 0, 0.8, 2, 0, 0, 0, 0, "lizardfolk" },
  {  80, 1, 4, 2, 0.8, 2, 0, 0, 0, 0, "orc" },
  {  90, 1, 4, 2, 0.5, 0, 0, 0, 0, 0, "zombie, medium-size [human]" },
  { 100, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "ghoul" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mThree[] = {
  {   2, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "bombardier beetle, giant (vermin)" },
  {   4, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "centipede, huge monstrous (vermin)" },
  {   6, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "dire badger" },
  {   8, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "dire bat" },
  {  11, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "gelatinous cube (ooze)" },
  {  13, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "praying mantis, giant (vermin)" },
  {  14, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "scorpion, large monstrous (vermin)" },
  {  15, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "spider, large monstrous (vermin)" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  25, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "imp (devil)" },
  {  30, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "wererat (lycanthrope)" },
  {  30, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "dire rat" },
  {  35, 1, 6, 3, 0.8, 3, rcDWARF_HILL, npcWARRIOR, 1, 2, 0 },
  {  40, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 1, 1, 0 },
  {  44, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "dretch (demon)" },
  {  48, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "ethereal filcher" },
  {  52, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "phantom fungus" },
  {  56, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "thoqquas" },
  {  60, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "vargouilles" },
  {  62, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "bugbear" },
  {  62, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "goblin" },
  {  67, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "gnoll" },
  {  69, 1, 4, 2, 0.8, 3, 0, 0, 0, 0, "goblin" },
  {  69, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "wolf" },
  {  71, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "hobgoblin" },
  {  71, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "wolf" },
  {  75, 1, 6, 3, 0.8, 3, 0, 0, 0, 0, "kobold" },
  {  75, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "dire weasel" },
  {  80, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "troglodyte" },
  {  90, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "shadow" },
  { 100, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "skeleton, large [ogre]" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};


DUNGEONMONSTERS mFour[] = {
  {   4, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "ankheg" },
  {   8, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "dire weasel" },
  {  12, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "ooze, gray" },
  {  15, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "snake, huge viper (animal)" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  23, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "formian warrior" },
  {  23, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "formian worker" },
  {  26, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "imp (devil)" },
  {  26, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "lemure (devil)" },
  {  30, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "quasit (devil)" },
  {  35, 1, 3, 0, 0.5, 2, 0, 0, 0, 0, "lantern archon (celestial)" },
  {  40, 1, 3, 0, 0.0, 0, raceANY_DMG, classANY_PC, 2, 2, 0 },
  {  45, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "carrion crawler" },
  {  50, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "mimic" },
  {  55, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "rust monster" },
  {  60, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "violet fungi" },
  {  62, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "bugbear" },
  {  62, 1, 6, 3, 0.0, 0, 0, 0, 0, 0, "hobgoblin" },
  {  65, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ettercap" },
  {  67, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "gnoll" },
  {  67, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "hyena [treat as wolf (animal)]" },
  {  70, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "lizardfolk" },
  {  70, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "giant lizard (animal)" },
  {  73, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "magmin" },
  {  76, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ogre" },
  {  76, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "orc" },
  {  78, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "orc" },
  {  78, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "dire boar" },
  {  80, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "worg" },
  {  80, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "goblin" },
  {  85, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "allip" },
  {  90, 1, 1, 0, 0.5, 0, raceANY_DMG, classANY_PC, 1, 3, "ghost" },
  {  95, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "vampire spawn" },
  { 100, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "wight" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mFive[] = {
  {   2, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "ant, giant soldier (vermin)" },
  {   2, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "ant, giant worker (vermin)" },
  {   5, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "dire wolverine" },
  {   9, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "ochre jelly" },
  {  11, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "snake, giant constrictor (animal)" },
  {  12, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "spiders, huge monstrous (vermin)" },
  {  15, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "spider eater" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  23, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "doppleganger" },
  {  25, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "green hag (hag)" },
  {  27, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "mephits" },
  {  30, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "wererat (lycanthrope)" },
  {  35, 1, 3, 1, 0.5, 2, 0, 0, 0, 0, "blink dog" },
  {  40, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 2, 2, 0 },
  {  43, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "cockatrice" },
  {  47, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "gibbering mouther" },
  {  50, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "grick" },
  {  52, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra, 1d3+4 heads" },
  {  55, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "nightmare" },
  {  58, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "shocker lizard" },
  {  60, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "violet fungus" },
  {  60, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "shrieker" },
  {  64, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "azer" },
  {  67, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "bugbear" },
  {  69, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ettercap" },
  {  69, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "spider, medium-size monstous (vermin)" },
  {  72, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "orge" },
  {  75, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "salamanders, small" },
  {  77, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "troglodyte" },
  {  77, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "giant lizard (animal) [immune to stench]" },
  {  80, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "worg" },
  {  85, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "ghast" },
  {  85, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "ghoul" },
  {  90, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "mummy" },
  {  95, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "skeleton, huge [giant]" },
  { 100, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "wraith" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mSix[] = {
  {   2, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "digester" },
  {   4, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dire ape" },
  {   6, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dire wolf" },
  {   7, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "giant stag beetle (vermin)" },
  {   9, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "giant wasp (vermin)" },
  {  12, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "owlbear" },
  {  15, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "shambling mound" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  22, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "annis (hag)" },
  {  25, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "harpy" },
  {  26, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "quasit (demon)" },
  {  26, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "dretch (demon)" },
  {  28, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "wereboar (lycanthrope)" },
  {  30, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "werewolf (lycanthrope)" },
  {  35, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "werebear (lycanthrope)" },
  {  40, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 3, 3, 0 },
  {  43, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "arrowhawk, small" },
  {  46, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "basilisk" },
  {  50, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "displacer beast" },
  {  53, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "gargoyle" },
  {  56, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "hell hound" },
  {  59, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "howler" },
  {  62, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "otyughs" },
  {  65, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "ravid" },
  {  65, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "animated object, large" },
  {  67, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "xorn, small" },
  {  70, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "yeth hounds" },
  {  77, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ettin" },
  {  77, 1, 6, 3, 0.0, 0, 0, 0, 0, 0, "orc" },
  {  82, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "ogre" },
  {  82, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "boar (animal)" },
  {  90, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "weretiger (lycanthrope)" },
  { 100, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "zombie, huge [giant]" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mSeven[] = {
  {   4, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "black pudding (ooze)" },
  {   5, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "centipede, gargantuan monstrous (vermin)" },
  {   8, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "criosphinx (sphinx)" },
  {  10, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dire boar" },
  {  14, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "remorhaz" },
  {  15, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "scorpion, huge monstrous (vermin)" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  22, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "aranea" },
  {  24, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "barghest, medium-size" },
  {  26, 1, 3, 0, 0.8, 3, 0, 0, 0, 0, "djinn" },
  {  28, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "formian taskmaster" },
  {  28, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "minotaur" },
  {  30, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "jann (genie)" },
  {  35, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "hound archon (celestial)" },
  {  40, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 4, 4, 0 },
  {  45, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "cloaker" },
  {  48, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "cryohydra, 1d3+4 heads (hydra)" },
  {  52, 1, 4, 2, 0.8, 3, 0, 0, 0, 0, "formian warrior" },
  {  57, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "invisible stalker" },
  {  60, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "pyrohydra, 1d3+4 heads (hydra)" },
  {  65, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "bugbear" },
  {  65, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "wolf" },
  {  70, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ettin" },
  {  70, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "brown bear (animal)" },
  {  75, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "minotaur" },
  {  80, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "salamander, medium-size" },
  {  80, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "salamander, small" },
  {  90, 1, 1, 0, 0.5, 0, raceANY_DMG, classANY_PC, 5, 8, "ghost" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 5, 6, "vampire" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mEight[] = {
  {   3, 1, 6, 5, 0.2, 0, 0, 0, 0, 0, "ant, giant soldier (vermin)" },
  {   8, 1, 6, 5, 0.2, 0, 0, 0, 0, 0, "dire bat" },
  {  10, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "spider, gargantuan monstrous (vermin)" },
  {  20, 1, 1, 0, 0.8, 4, 0, 0, 0, 0, DRAGON },
  {  22, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "aboleth" },
  {  22, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "skum" },
  {  24, 1, 3, 1, 0.8, 3, 0, 0, 0, 0, "barghest, large" },
  {  26, 1, 2, 0, 0.8, 3, 0, 0, 0, 0, "erinyes (devil)" },
  {  28, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "medusa" },
  {  28, 1, 6, 3, 0.0, 0, 0, 0, 0, 0, "grimlock" },
  {  30, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "mind flayer" },
  {  33, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "ogre mage" },
  {  35, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "yuan-ti halfblood" },
  {  35, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "yuan-ti pureblood" },
  {  40, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, "lammasu" },
  {  45, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 5, 5, 0 },
  {  47, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "achaierai" },
  {  48, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "arrowhawk, medium-size" },
  {  50, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "girallon" },
  {  52, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "golem, flesh" },
  {  54, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "render" },
  {  56, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "hieracosphinx (sphinx)" },
  {  59, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra (1d3+7 heads)" },
  {  60, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra, lernaean (1d3+4 heads)" },
  {  62, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "phase spider" },
  {  64, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "rast" },
  {  66, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "shadow mastiff" },
  {  70, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "xorn, medium-size" },
  {  74, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "drider" },
  {  74, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "spider, large monstrous (vermin)" },
  {  78, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "ettin" },
  {  82, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "manticore" },
  {  86, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "salamander, medium-size" },
  {  90, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "troll" },
  { 100, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "spectre" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mNine[] = {
  {   5, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "bulette" },
  {  10, 1, 4, 2, 0.2, 0, 0, 0, 0, 0, "dire lion" },
  {  20, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, DRAGON },
  {  21, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "bebilith (demon)" },
  {  22, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "lamia" },
  {  24, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "mind flayer" },
  {  24, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 3, 3, 0 },
  {  26, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "night hag" },
  {  28, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "ogre mage" },
  {  28, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "ogre" },
  {  30, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "rakshasa" },
  {  32, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "succubus" },
  {  33, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "xill, barbaric" },
  {  34, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "xill, civilized" },
  {  35, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "yuan-ti, abomination" },
  {  35, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "yuan-ti, halfblood" },
  {  40, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "androsphinx (sphinx)" },
  {  45, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 6, 6, 0 },
  {  47, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "behir" },
  {  49, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "belker" },
  {  50, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "cryohydra, 1d3+6 heads (hydra)" },
  {  52, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "delver" },
  {  54, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "dragon turtle" },
  {  55, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "pyrohydra, 1d3+6 heads (hydra)" },
  {  57, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "will-o'-wisp" },
  {  60, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "wyvern" },
  {  64, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "barbazu (devil)" },
  {  64, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "osyluth (devil)" },
  {  68, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "giant, hill" },
  {  68, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "dire wolf" },
  {  72, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "kyton (devil)" },
  {  76, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "osyluth (devil)" },
  {  80, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "troll" },
  {  80, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "dire boar" },
  {  90, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "bodak" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 7, 8, "vampire" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mTen[] = {
  {   5, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dire bear" },
  {  15, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, DRAGON },
  {  17, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "aboleth" },
  {  19, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "athach" },
  {  21, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "formian myrmarch" },
  {  24, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "medusa" },
  {  26, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "naga, water" },
  {  28, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "night hag" },
  {  28, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "nightmare" },
  {  30, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "salamander, large" },
  {  30, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "salamander, medium-size" },
  {  32, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "yuan-ti abomination" },
  {  37, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "lillend" },
  {  47, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 7, 7, 0 },
  {  49, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "chaos beast" },
  {  51, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "chimera" },
  {  53, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "chuul" },
  {  54, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "cryohydra, lernaean, 1d4+4 heads (hydra)" },
  {  56, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "dragonne" },
  {  58, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "hellcat (devil)" },
  {  59, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra, 1d3+9 heads" },
  {  60, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "phasm" },
  {  61, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "pyrohydra, lernaean, 1d4+4 heads (hydra)" },
  {  63, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "retriever (demon)" },
  {  65, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "slaad, red" },
  {  67, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "umber hulk" },
  {  71, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "barbazu (devil)" },
  {  75, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "drider" },
  {  79, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "giant, frost" },
  {  79, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "winter wolf" },
  {  83, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "giant, stone" },
  {  83, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "dire bear" },
  {  87, 1, 3, 1, 0.8, 2, 0, 0, 0, 0, "giant, hill" },
  {  90, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, "hamatula (devil)" },
  {  90, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "barbazu (devil)" },
  { 100, 1, 1, 0, 0.5, 0, raceANY_DMG, classANY_PC, 7, 9, "ghost" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mEleven[] = {
  {   5, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "dire tiger" },
  {  15, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, DRAGON },
  {  18, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, "green hag" },
  {  18, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "annis (hag)" },
  {  18, 1, 1, 0, 0.0, 0, 0, 0, 0, 0, "sea hag" },
  {  18, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "ogre" },
  {  18, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "giant, hill" },
  {  21, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "efreet" },
  {  24, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, "formian myrmarch" },
  {  24, 1, 6, 3, 0.0, 0, 0, 0, 0, 0, "formian warrior" },
  {  27, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "gynosphinx (sphinx)" },
  {  30, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "naga, dark" },
  {  35, 1, 3, 0, 0.8, 1, 0, 0, 0, 0, "avoral guardian (celestial)" },
  {  45, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 8, 8, 0 },
  {  48, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "arrowhawk, large" },
  {  51, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "destrachan" },
  {  54, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "golem, clay" },
  {  57, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "gorgon" },
  {  59, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra, lernaean, 1d3+7 heads" },
  {  62, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "slaad, blue" },
  {  65, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "xorn, large" },
  {  70, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, "giant, fire" },
  {  70, 1, 6, 3, 0.0, 0, 0, 0, 0, 0, "hell hound" },
  {  75, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "giant, stone" },
  {  80, 1, 3, 1, 0.8, 1, 0, 0, 0, 0, "hamatula (devil)" },
  {  90, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "devourer" },
  { 100, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "mohrg" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mTwelve[] = {
  {   4, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "purple worm" },
  {   5, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "scorpion, colossal monstrous (vermin)" },
  {  15, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, DRAGON },
  {  20, 1, 4, 2, 0.5, 1, 0, 0, 0, 0, "mind flayer (inquisition)" },
  {  25, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "naga, spirit" },
  {  30, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "slaad, green" },
  {  35, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "giant, cloud [good]" },
  {  35, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "dire lion" },
  {  50, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 9, 9, 0 },
  {  55, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "cryohydra, 1d3+9 heads (hydra)" },
  {  60, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "golem, stone" },
  {  65, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "pyrohydra, 1d3+9 heads (hydra)" },
  {  70, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "yrthak" },
  {  75, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "cornugon (devil)" },
  {  75, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "hamatula (devil)" },
  {  80, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "giant, cloud [evil]" },
  {  80, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "dire lion" },
  {  85, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "giant, frost" },
  {  90, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "salamander, large" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 9, 11, "vampire" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mThirteen[] = {
  {  15, 1, 1, 0, 0.8, 3, 0, 0, 0, 0, DRAGON },
  {  20, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "beholder" },
  {  30, 3, 1, 0, 0.5, 1, 0, 0, 0, 0, "night hag" },
  {  30, 3, 1, 0, 0.0, 0, 0, 0, 0, 0, "nightmare" },
  {  35, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "slaad, gray" },
  {  40, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "couatl" },
  {  45, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "naga, guardian" },
  {  60, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 10, 10, 0 },
  {  67, 1, 2, 0, 0.2, 0, 0, 0, 0, 0, "frost worm" },
  {  73, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "hydra, lernaean, 1d3+9 heads" },
  {  80, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "roper" },
  {  90, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "cornugon (devil)" },
  { 100, 1, 1, 0, 0.5, 0, raceANY_DMG, classANY_PC, 10, 13, "ghost" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mFourteen[] = {
  {  15, 1, 1, 0, 0.8, 2, 0, 0, 0, 0, DRAGON },
  {  25, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "beholder" },
  {  25, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 8, 8, "charmed" },
  {  35, 1, 2, 0, 0.5, 1, 0, 0, 0, 0, "slaad, death" },
  {  40, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "giant, cloud [good]" },
  {  55, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 11, 11, 0 },
  {  60, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "cryohydra, lernaean, 1d4+8 heads (hydra)" },
  {  65, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "golem, iron" },
  {  70, 1, 1, 0, 0.2, 0, 0, 0, 0, 0, "pyrohydra, lernaean, 1d4+8 heads (hydra)" },
  {  80, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "giant, cloud [evil]" },
  {  90, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "giant, storm" },
  {  90, 1, 4, 2, 0.0, 0, 0, 0, 0, 0, "griffon" },
  {  91, 1, 1, 0, 0.0, 0, raceANY_DMG, pcCLERIC, 11, 13, "lich" },
  {  94, 1, 1, 0, 0.0, 0, raceANY_DMG, pcSORCERER, 11, 13, "lich" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, pcWIZARD, 11, 13, "lich" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mFifteen[] = {
  {  20, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, DRAGON },
  {  30, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "beholder" },
  {  40, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "slaad, death" },
  {  40, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "slaad, green" },
  {  45, 1, 3, 0, 0.8, 0, 0, 0, 0, 0, "ghaele (celestial)" },
  {  70, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 12, 12, 0 },
  {  80, 1, 2, 0, 0.5, 0, 0, 0, 0, 0, "hezrous" },
  {  90, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "gelugon (devil)" },
  {  90, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "cornugon (devil)" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 12, 14, "vampire" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mSixteen[] = {
  {  20, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, DRAGON },
  {  30, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "pit fiend (devil)" },
  {  35, 1, 3, 0, 0.5, 1, 0, 0, 0, 0, "astral deva (celestial)" },
  {  60, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 13, 13, 0 },
  {  70, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "gelugon (devil)" },
  {  80, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "giant, storm" },
  {  90, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "vrock (demon)" },
  { 100, 1, 1, 0, 0.2, 0, raceANY_DMG, classANY_PC, 13, 15, "ghost" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mSeventeen[] = {
  {  20, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, DRAGON },
  {  30, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "marilith (demon)" },
  {  35, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "trumpet archon (celestial)" },
  {  60, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 14, 14, 0 },
  {  70, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "glabrezu (demon)" },
  {  80, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "hezrou (demon)" },
  {  81, 1, 1, 0, 0.0, 0, raceANY_DMG, pcCLERIC, 14, 16, "lich" },
  {  84, 1, 1, 0, 0.0, 0, raceANY_DMG, pcSORCERER, 14, 16, "lich" },
  {  90, 1, 1, 0, 0.0, 0, raceANY_DMG, pcWIZARD, 14, 16, "lich" },
  { 100, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "nightwing (nightshade)" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mEighteen[] = {
  {  20, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, DRAGON },
  {  30, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "balor (demon)" },
  {  40, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "pit fiend (devil)" },
  {  40, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "gelugon (devil)" },
  {  45, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "planetar (celestial)" },
  {  70, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 15, 15, 0 },
  {  80, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "glabrezu (demon)" },
  {  90, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 15, 17, "vampire" },
  { 100, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "nightwalker (nightshade)" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mNineteen[] = {
  {  20, 1, 1, 0, 0.8, 1, 0, 0, 0, 0, DRAGON },
  {  30, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "marilith (demon)" },
  {  30, 1, 3, 0, 0.0, 0, 0, 0, 0, 0, "glabrezu (demon)" },
  {  40, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "pit fiend (devil)" },
  {  45, 1, 1, 0, 0.5, 1, 0, 0, 0, 0, "solar (celestial)" },
  {  70, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 16, 16, 0 },
  {  80, 1, 3, 1, 0.5, 1, 0, 0, 0, 0, "nalfeshnee (demon)" },
  {  90, 1, 1, 0, 0.2, 0, raceANY_DMG, classANY_PC, 16, 18, "ghost" },
  { 100, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "nightcrawler (nightshade)" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};

DUNGEONMONSTERS mTwenty[] = {
  {  20, 1, 1, 0, 0.8, 0, 0, 0, 0, 0, DRAGON },
  {  30, 1, 3, 0, 0.5, 0, 0, 0, 0, 0, "balor (demon)" },
  {  40, 1, 3, 1, 0.5, 0, 0, 0, 0, 0, "marilith (demon)" },
  {  45, 1, 1, 0, 0.5, 0, 0, 0, 0, 0, "solar (celestial)" },
  {  45, 1, 2, 0, 0.0, 0, 0, 0, 0, 0, "planetar (celestial)" },
  {  55, 1, 3, 1, 0.0, 0, raceANY_DMG, classANY_PC, 17, 17, 0 },
  {  60, 1, 3, 0, 0.0, 0, raceANY_DMG, classANY_PC, 18, 18, 0 },
  {  65, 1, 2, 0, 0.0, 0, raceANY_DMG, classANY_PC, 19, 19, 0 },
  {  70, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 20, 20, 0 },
  {  80, 1, 3, 1, 0.2, 0, 0, 0, 0, 0, "nalfeshnee (demon)" },
  {  80, 1, 3, 1, 0.0, 0, 0, 0, 0, 0, "hezrou (demon)" },
  {  85, 1, 1, 0, 0.2, 0, raceANY_DMG, classANY_PC, 19, 20, "ghost" },
  {  86, 1, 1, 0, 0.0, 0, raceANY_DMG, pcCLERIC, 17, 20, "lich" },
  {  88, 1, 1, 0, 0.0, 0, raceANY_DMG, pcSORCERER, 17, 20, "lich" },
  {  90, 1, 1, 0, 0.0, 0, raceANY_DMG, pcWIZARD, 17, 20, "lich" },
  {  95, 1, 3, 0, 0.2, 0, 0, 0, 0, 0, "nightcrawler (nightshade)" },
  { 100, 1, 1, 0, 0.0, 0, raceANY_DMG, classANY_PC, 18, 20, "lich" },
  {   0, 0, 0, 0, 0.0, 0, 0, 0, 0, 0, 0 }
};


DUNGEONMONSTERS* s_monsterTable[] = {
  0, mOne, mTwo, mThree, mFour, mFive, mSix, mSeven, mEight, mNine, mTen,
  mEleven, mTwelve, mThirteen, mFourteen, mFifteen, mSixteen, mSeventeen,
  mEighteen, mNineteen, mTwenty
};

/* dragon ages */

#define daWYRMLING         "wyrmling"
#define daVERYYOUNG        "very young"
#define daYOUNG            "young"
#define daJUVENILE         "juvenile"
#define daYOUNGADULT       "young adult"
#define daADULT            "adult"
#define daMATUREADULT      "mature adult"
#define daOLD              "old"
#define daVERYOLD          "very old"
#define daANCIENT          "ancient"
#define daWYRM             "wyrm"
#define daGREATWYRM        "great wyrm"

/* random dragon table, by percentage chance of that dragon occuring.  See
 * the DMG for a fuller description of this table */

static struct {
  int   dtop;
  char* dragonType;
  char* age[21]; 
} s_dragonTable[] = {
  {  16, "white",  { 0, daWYRMLING, daVERYYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daOLD, daVERYOLD, daVERYOLD, daANCIENT, daWYRM, daGREATWYRM, daGREATWYRM } },
  {  32, "black",  { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD, daVERYOLD, daANCIENT, daWYRM, daGREATWYRM } },
  {  48, "green",  { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD, daANCIENT, daANCIENT } },
  {  64, "blue",   { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD, daANCIENT, daANCIENT } },
  {  80, "red",    { 0, daWYRMLING, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD } },
  {  84, "brass",  { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD, daVERYOLD, daANCIENT, daWYRM } },
  {  88, "copper", { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daOLD, daVERYOLD, daANCIENT } },
  {  91, "bronze", { 0, daWYRMLING, daWYRMLING, daVERYYOUNG, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD, daANCIENT } },
  {  96, "silver", { 0, daWYRMLING, daWYRMLING, daWYRMLING, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD, daVERYOLD } },
  { 100, "gold",   { 0, daWYRMLING, daWYRMLING, daWYRMLING, daWYRMLING, daVERYYOUNG, daVERYYOUNG, daYOUNG, daYOUNG, daJUVENILE, daJUVENILE, daYOUNGADULT, daYOUNGADULT, daYOUNGADULT, daADULT, daADULT, daMATUREADULT, daMATUREADULT, daMATUREADULT, daOLD, daOLD } },
  {   0, 0,        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

typedef int (*CONTENTHANDLER)( JBDungeonRoomDatum*, int );

/* "handlers" to be used in generating room descriptions */

int monsterHandler( JBDungeonRoomDatum* room, int level );
int featureHandler( JBDungeonRoomDatum* room, int level );
int trapHandler( JBDungeonRoomDatum* room, int level );
int treasureHandler( JBDungeonRoomDatum* room, int level );

/* handlers to use to determine the contents of a given room */

static struct {
  int            type;
  CONTENTHANDLER handler;
} s_contentHandlers[] = {
  { MONSTER,  monsterHandler },
  { FEATURE,  featureHandler },
  { TREASURE, treasureHandler },
  { TRAP,     trapHandler },
  { 0,        0 }
};


void JBDungeonWallDatum::getDatumDescription( char* desc ) {
  int  comma;
  int  i;

  comma = 0;
  *desc = 0;

  if( descriptor != 0 ) {
    strcat( desc, descriptor );
    strcat( desc, " " );
  }

  for( i = 0; s_flagDescriptions[ i ].desc != 0; i++ ) {
    if( ( type & s_flagDescriptions[ i ].flag ) != 0 ) {
      if( comma ) {
        strcat( desc, ", " );
      }
      strcat( desc, s_flagDescriptions[ i ].desc );
      comma = 1;
    }
  }

  if( trap != 0 ) {
    strcat( desc, " [trap: " );
    strcat( desc, trap );
    strcat( desc, "]" );
  }
}


JBDungeonRoomDatum::~JBDungeonRoomDatum() {
  int i;

  if( monsterCount > 0 ) {
    for( i = 0; i < monsterCount; i++ ) {
      delete[] monsters[ i ];
    }
    delete[] monsters;
  }

  if( featureCount > 0 ) {
    for( i = 0; i < featureCount; i++ ) {
      delete[] features[ i ];
    }
    delete[] features;
  }

  cleanupTreasure( &treasure );
  cleanupTreasure( &monsterTreasure );

  if( trap != 0 ) {
    delete[] trap;
  }
}


void JBDungeonRoomDatum::addMonster( char* desc ) {
  char** temp;
  int    len;

  temp = new char*[ monsterCount + 1 ];
  if( monsterCount > 0 ) {
    memcpy( temp, monsters, sizeof( char* ) * monsterCount );
    delete[] monsters;
  }
  monsters = temp;

  len = strlen( desc ) + 1;
  monsters[ monsterCount ] = new char[ len ];
  strcpy( monsters[ monsterCount ], desc );

  monsterCount++;
}


void JBDungeonRoomDatum::addFeature( char* desc ) {
  char** temp;
  int    len;

  temp = new char*[ featureCount + 1 ];
  if( featureCount > 0 ) {
    memcpy( temp, features, sizeof( char* ) * featureCount );
    delete[] features;
  }
  features = temp;

  len = strlen( desc ) + 1;
  features[ featureCount ] = new char[ len ];
  strcpy( features[ featureCount ], desc );

  featureCount++;
}


void JBDungeonRoomDatum::getDatumDescription( char* desc ) {
  int i;
  char temp[4096];
  TREASUREITEM* n;
  int output;

  output = 0;

  *desc = 0;
  if( monsterCount > 0 ) {
    output = 1;
    strcat( desc, "$" DUNGEON_DATA_DELIM );
    strcat( desc, "!Monsters" DUNGEON_DATA_DELIM );

    for( i = 0; i < monsterCount; i++ ) {
      strcat( desc, monsters[ i ] );
      strcat( desc, DUNGEON_DATA_DELIM );
    }

    if( monsterTreasure.treasureList != 0 ) {
      strcat( desc, "$" DUNGEON_DATA_DELIM );
      strcat( desc, "!Treasure" DUNGEON_DATA_DELIM );
      for( n = monsterTreasure.treasureList; n != 0; n = n->next ) {
        strcat( desc, n->desc );
        sprintf( temp, " (%d gp)", n->value/100 );
        strcat( desc, temp );
        strcat( desc, DUNGEON_DATA_DELIM );
      }
    }
  }

  if( featureCount > 0 ) {
    output = 1;
    strcat( desc, "$" DUNGEON_DATA_DELIM );
    strcat( desc, "!Features" DUNGEON_DATA_DELIM );

    for( i = 0; i < featureCount; i++ ) {
      strcat( desc, features[ i ] );
      strcat( desc, DUNGEON_DATA_DELIM );
    }
  }

  if( treasure.treasureList != 0 ) {
    output = 1;
    strcat( desc, "$" DUNGEON_DATA_DELIM );
    sprintf( temp, "!Hidden Treasure (Search DC %d)" DUNGEON_DATA_DELIM, 20 + dungeonLevel );
    strcat( desc, temp );

    for( n = treasure.treasureList; n != 0; n = n->next ) {
      sprintf( temp, "%s (%d gp)" DUNGEON_DATA_DELIM, n->desc, n->value/100 );
      strcat( desc, temp );
    }
  }

  if( trap != 0 ) {
    output = 1;
    strcat( desc, "$" DUNGEON_DATA_DELIM );
    strcat( desc, "!Trap" DUNGEON_DATA_DELIM );
    strcat( desc, trap );
    strcat( desc, DUNGEON_DATA_DELIM );
  }

  if( !output ) {
    strcat( desc, "$" DUNGEON_DATA_DELIM );
    strcat( desc, "Empty" DUNGEON_DATA_DELIM );
  }
}


JBDungeonDescription::JBDungeonDescription( JBDungeon* dungeon, int level ) {
  m_describeRooms( dungeon, level );
}


JBDungeonDescription::~JBDungeonDescription() {
}


void JBDungeonDescription::m_describeRooms( JBDungeon* dungeon, int level ) {
  int count;
  int i;

  count = dungeon->getRoomCount();
  for( i = 0; i < count; i++ ) {
    m_describeRoom( dungeon, dungeon->getRoom( i ), level );
  }
}


char* getRandomTrap( int level, int forDoor ) {
  TRAPLIST* list;
  int       d;
  int       i;
  int       valid;

  static char buffer[ 256 ];

  buffer[0] = 0;

  if( level <= 3 ) {
    list = traps1;
  } else {
    list = traps2;
  }

  do {
    valid = 1;
    d = rollDice( 1, 100 );
    for( i = 0; list[ i ].dtop != 0; i++ ) {
      if( d <= list[ i ].dtop ) {
        if( !(list[ i ].forDoor) && ( forDoor ) ) {
          valid = 0;
          break;
        }

        if( list[ i ].trap == SPELLTRAP ) {
          d = rollDice( 1, 100 );
          for( i = 0; s_spellsForTraps[ i ].dtop != 0; i++ ) {
            if( d <= s_spellsForTraps[ i ].dtop ) {
              char* name;

              name = dndGetSpellName( s_spellsForTraps[ i ].spell );

              sprintf( buffer, "spell (%s)", name );
              break;
            }
          }
        } else {
          strcpy( buffer, list[ i ].trap );
        }
        break;
      }
    }
  } while( !valid );

  sprintf( &(buffer[strlen(buffer)]), " (Find/Disable DC %d)", 20+level );

  return buffer;
}


long getDoorType( int allowReroll, DOORTYPE* doors ) {
  int  d;
  int  i;
  int  valid;
  long value = 0;

  do {
    d = rollDice( 1, 100 );
    valid = 0;
    for( i = 0; doors[ i ].dtop != 0; i++ ) {
      if( d <= doors[ i ].dtop ) {
        value = doors[ i ].data;
        if( ( value & REROLL_ONCE ) != 0 ) {
          if( allowReroll ) {
            value = ( value | getDoorType( 0, doors ) ) & ~REROLL_ONCE;
            valid = 1;
          }
        } else {
          valid = 1;
        }
        break;
      }      
    }
  } while( !valid );

  return value;
}


void JBDungeonDescription::m_describeRoom( JBDungeon* dungeon, JBDungeonRoom* room, int level ) {
  int i;
  int d;
  int j;
  JBDungeonRoomDatum *rdatum;
  char* trap;
  long data;

  rdatum = new JBDungeonRoomDatum();
  rdatum->dungeonLevel = level;
  rdatum->dataPath = dungeon->getDataPath();

  room->data = rdatum;

  for( i = 0; i < room->wallCount; i++ ) {
    if( room->walls[ i ]->type == JBDungeonWall::c_WALL ) {
    } else if( room->walls[ i ]->type == JBDungeonWall::c_DOOR ) {
      if( room->walls[ i ]->data == 0 ) {
        data = getDoorType( 1, s_doorTypes );
        trap = ( ( data & dtTRAPPED ) != 0 ? getRandomTrap( level, 1 ) : 0 );
        room->walls[ i ]->data = new JBDungeonWallDatum( data, trap, 0 );
      }
    } else if( room->walls[ i ]->type == JBDungeonWall::c_SECRETDOOR ) {
      if( room->walls[ i ]->data == 0 ) {
        data = getDoorType( 1, s_secretDoorTypes );
        trap = ( ( data & dtTRAPPED ) != 0 ? getRandomTrap( level, 1 ) : 0 );
        room->walls[ i ]->data = new JBDungeonWallDatum( data, trap, "(secret)" );
      }
    } else if( room->walls[ i ]->type == JBDungeonWall::c_CONCEALEDDOOR ) {
      if( room->walls[ i ]->data == 0 ) {
        if( rand() % 3 != 0 ) {
          data = getDoorType( 1, s_doorTypes );
          if( rand() % 2 == 0 ) {
            data |= cdtBEHINDRUBBISH;
          } else {
            data |= cdtBEHINDTAPESTRY;
          }
        } else {
          data = getDoorType( 1, s_concealedDoorTypes );
        }

        trap = ( ( data & dtTRAPPED ) != 0 ? getRandomTrap( level, 1 ) : 0 );
        room->walls[ i ]->data = new JBDungeonWallDatum( data, trap, "(concealed)" );
      }
    }
  }

  d = rollDice( 1, 100 );
  for( i = 0; s_roomContents[ i ].dtop != 0; i++ ) {
    if( d <= s_roomContents[ i ].dtop ) {
      for( j = 0; s_contentHandlers[ j ].type != 0; j++ ) {
        if( ( s_contentHandlers[ j ].type & s_roomContents[ i ].contents ) != 0 ) {
          s_contentHandlers[ j ].handler( rdatum, level );
        }
      }
      break;
    }
  }
}


char* getRandomDragon( int level ) {
  int i;
  int d;
  static char buffer[128];

  d = rollDice( 1, 100 );
  for( i = 0; s_dragonTable[ i ].dtop != 0; i++ ) {
    if( d <= s_dragonTable[ i ].dtop ) {
      strcpy( buffer, "~B" );
      strcat( buffer, s_dragonTable[ i ].age[ level ] );
      strcat( buffer, " " );
      strcat( buffer, s_dragonTable[ i ].dragonType );
      strcat( buffer, " " );
      strcat( buffer, "dragon" );
      strcat( buffer, "~b" );
      return buffer;
    }
  }

  return "";
}


int monsterHandler( JBDungeonRoomDatum* room, int level ) {
  DUNGEONLEVEL* masterData;
  DUNGEONMONSTERS* monsters;
  int           i;
  int           d;
  int           lvl;
  int           mul;
  int           div;
  int           k;
  int           x;
  int           y;
  char          buffer[512];

  masterData = s_masterMonsterTable[ level ];
  d = rollDice( 1, 100 );
  for( i = 0; masterData[i].dtop != 0; i++ ) {
    if( d <= masterData[ i ].dtop ) {
      lvl = masterData[ i ].level;
      mul = masterData[ i ].mul;
      div = masterData[ i ].div;

      monsters = s_monsterTable[ lvl ];
      d = rollDice( 1, 100 );
      for( i = 0; monsters[ i ].dtop != 0; i++ ) {
        if( d <= monsters[ i ].dtop ) {
          k = monsters[ i ].dtop;
          while( k == monsters[ i ].dtop ) {
            x = rollDice( monsters[ i ].dcount, monsters[ i ].dtype ) + monsters[ i ].dmod;
            x = ( x * mul ) / div;
            if( x < 1 ) {
              x = 1;
            }

            if( monsters[ i ].monster == DRAGON ) {
              /* if the monster is a dragon, add a random dragon based on the
               * modified level of the monster to be generated. */

              room->addMonster( getRandomDragon( lvl ) );

            } else if( monsters[ i ].npcMinLevel > 0 ) {
              /* if the monster is an NPC, use the NPC generator engine to
               * calculate the NPC's stats. */

              NPCGENERATOROPTS opts;
              NPCGENERATOROPTS opts2;
              NPCSTATBLOCKOPTS statOpts;
              NPC* npc;
              char buffer[4096];

              /* set the parameters for stat-block output */
              memset( &statOpts, 0, sizeof( statOpts ) );
              statOpts.acBreakdown = 1;
              statOpts.initBreakdown = 1;
              statOpts.languages = 1;
              statOpts.skillsAndFeats = 1;
              statOpts.possessions = 1;
              statOpts.spells = 1;
              statOpts.richFormatting = 1;

              /* set the parameters for NPC generation */
              memset( &opts, 0, sizeof( opts ) );
              opts.raceType = npcGetRandomRace( monsters[ i ].npcRace );
              opts.classType[0] = monsters[ i ].npcClass;
              opts.level[0] = selectBetween( monsters[ i ].npcMinLevel, monsters[ i ].npcMaxLevel );
              opts.filePath = room->dataPath;
              opts.abilityScoreStrategy = npcAbScoreStrategy_BestOf4d6;

              for( y = 0; y < x; y++ ) {
                memcpy( &opts2, &opts, sizeof( opts2 ) );

                npc = npcGenerateNPC( &opts2 );

                /* if there is a further description indicated (ghost, lich, etc.),
                 * prepend it to the NPC description. */

                buffer[0] = 0;
                if( monsters[ i ].monster != 0 ) {
                  strcpy( buffer, "(" );
                  strcat( buffer, monsters[ i ].monster );
                  strcat( buffer, ") " );
                }

                npcBuildStatBlock( npc, &statOpts, &(buffer[strlen(buffer)]), sizeof( buffer ) );
                strcat( buffer, "~n" );

                room->addMonster( buffer );

                npcDestroyNPC( npc );
              }
            } else {
              /* otherwise, it's exactly what the description says it is */

              sprintf( buffer, "~B%s~b (%d)", monsters[ i ].monster, x );
              room->addMonster( buffer );
            }

            /* if the monster has treasure, compute it using the treasure 
             * generator engine. */
            if( monsters[ i ].treasureMod > 0 ) {
              float mods[3];

              mods[0] = mods[1] = mods[2] = (float)monsters[ i ].treasureMod;
              generateRandomTreasureEx( &(room->monsterTreasure), lvl+monsters[i].levelMod-1, mods );
            }

            i++;
          }
          break;
        }
      }
      break;
    }
  }

  return 0;
}


int featureHandler( JBDungeonRoomDatum* room, int level ) {
  FEATURELIST* lists[2];
  int          i;
  int          d;
  int          j;
  int          k;

  lists[0] = s_minorFeatures;
  lists[1] = s_majorFeatures;
  for( i = 0; i < 2; i++ ) {
    for( j = rollDice( 1, 4 ); j > 0; j-- ) {
      d = rollDice( 1, 100 );
      for( k = 0; lists[ i ][ k ].dtop != 0; k++ ) {
        if( d <= lists[ i ][ k ].dtop ) {
          room->addFeature( lists[ i ][ k ].feature );
          break;
        }
      }
    }    
  }

  return 0;
}


int trapHandler( JBDungeonRoomDatum* room, int level ) {
  char* trap;

  trap = getRandomTrap( level, 0 );

  room->trap = new char[ strlen(trap) + 1 ];
  strcpy( room->trap, trap );

  return 0;
}


int treasureHandler( JBDungeonRoomDatum* room, int level ) {
  do {
    /* if there is hidden treasure, use the treasure generator engine to
     * compute the actual contents of the treasure. */

    generateRandomTreasure( &(room->treasure), level-1 );
  } while( room->treasure.treasureList == 0 );
  return 0;
}
