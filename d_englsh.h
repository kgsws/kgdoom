#ifndef __D_ENGLSH__
#define __D_ENGLSH__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR	"Development mode ON.\n"
#define D_CDROM	"CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//	M_Menu.C
//
#define PRESSKEY 	"press a key."
#define PRESSYN 	"press y or n."
#define QUITMSG	"are you sure you want to\nquit this great game?"
#define LOADNET 	"you can't do load while in a net game!\n\n"PRESSKEY
#define QLOADNET	"you can't quickload during a netgame!\n\n"PRESSKEY
#define QSAVESPOT	"you haven't picked a quicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD 	"you can't save if you aren't playing!\n\n"PRESSKEY
#define QSPROMPT 	"quicksave over your game named\n\n'%s'?\n\n"PRESSYN
#define QLPROMPT	"do you want to quickload the game named\n\n'%s'?\n\n"PRESSYN

#define NEWGAME	\
"you can't start a new game\n"\
"while in a network game.\n\n"PRESSKEY

#define NIGHTMARE	\
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\n"PRESSYN

#define SWSTRING	\
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\n"PRESSKEY

#define MSGOFF	"Messages OFF"
#define MSGON		"Messages ON"
#define NETEND	"you can't end a netgame!\n\n"PRESSKEY
#define ENDGAME	"are you sure you want to end the game?\n\n"PRESSYN

#define DOSY		"(press y to quit)"

#define DETAILHI	"High detail"
#define DETAILLO	"Low detail"
#define GAMMALVL0	"Gamma correction OFF"
#define GAMMALVL1	"Gamma correction level 1"
#define GAMMALVL2	"Gamma correction level 2"
#define GAMMALVL3	"Gamma correction level 3"
#define GAMMALVL4	"Gamma correction level 4"
#define EMPTYSTRING	"empty slot"

//
//	P_inter.C
//
#define GOTARMOR	"Picked up the armor."
#define GOTMEGA	"Picked up the MegaArmor!"
#define GOTHTHBONUS	"Picked up a health bonus."
#define GOTARMBONUS	"Picked up an armor bonus."
#define GOTSTIM	"Picked up a stimpack."
#define GOTMEDINEED	"Picked up a medikit that you REALLY need!"
#define GOTMEDIKIT	"Picked up a medikit."
#define GOTSUPER	"Supercharge!"

#define GOTBLUECARD	"Picked up a blue keycard."
#define GOTYELWCARD	"Picked up a yellow keycard."
#define GOTREDCARD	"Picked up a red keycard."
#define GOTBLUESKUL	"Picked up a blue skull key."
#define GOTYELWSKUL	"Picked up a yellow skull key."
#define GOTREDSKULL	"Picked up a red skull key."

#define GOTINVUL	"Invulnerability!"
#define GOTBERSERK	"Berserk!"
#define GOTINVIS	"Partial Invisibility"
#define GOTSUIT	"Radiation Shielding Suit"
#define GOTMAP	"Computer Area Map"
#define GOTVISOR	"Light Amplification Visor"
#define GOTMSPHERE	"MegaSphere!"

#define GOTCLIP	"Picked up a clip."
#define GOTCLIPBOX	"Picked up a box of bullets."
#define GOTROCKET	"Picked up a rocket."
#define GOTROCKBOX	"Picked up a box of rockets."
#define GOTCELL	"Picked up an energy cell."
#define GOTCELLBOX	"Picked up an energy cell pack."
#define GOTSHELLS	"Picked up 4 shotgun shells."
#define GOTSHELLBOX	"Picked up a box of shotgun shells."
#define GOTBACKPACK	"Picked up a backpack full of ammo!"

#define GOTBFG9000	"You got the BFG9000!  Oh, yes."
#define GOTCHAINGUN	"You got the chaingun!"
#define GOTCHAINSAW	"A chainsaw!  Find some meat!"
#define GOTLAUNCHER	"You got the rocket launcher!"
#define GOTPLASMA	"You got the plasma gun!"
#define GOTSHOTGUN	"You got the shotgun!"
#define GOTSHOTGUN2	"You got the super shotgun!"

//
// P_Doors.C
//
#define PD_BLUEO	"You need a blue key to activate this object"
#define PD_REDO	"You need a red key to activate this object"
#define PD_YELLOWO	"You need a yellow key to activate this object"
#define PD_BLUEK	"You need a blue key to open this door"
#define PD_REDK	"You need a red key to open this door"
#define PD_YELLOWK	"You need a yellow key to open this door"

//
//	G_game.C
//
#define GGSAVED	"game saved."

//
//	HU_stuff.C
//
#define HUSTR_MSGU	"[Message unsent]"

#define HUSTR_E1M1	"E1M1: Hangar"
#define HUSTR_E1M2	"E1M2: Nuclear Plant"
#define HUSTR_E1M3	"E1M3: Toxin Refinery"
#define HUSTR_E1M4	"E1M4: Command Control"
#define HUSTR_E1M5	"E1M5: Phobos Lab"
#define HUSTR_E1M6	"E1M6: Central Processing"
#define HUSTR_E1M7	"E1M7: Computer Station"
#define HUSTR_E1M8	"E1M8: Phobos Anomaly"
#define HUSTR_E1M9	"E1M9: Military Base"

#define HUSTR_E2M1	"E2M1: Deimos Anomaly"
#define HUSTR_E2M2	"E2M2: Containment Area"
#define HUSTR_E2M3	"E2M3: Refinery"
#define HUSTR_E2M4	"E2M4: Deimos Lab"
#define HUSTR_E2M5	"E2M5: Command Center"
#define HUSTR_E2M6	"E2M6: Halls of the Damned"
#define HUSTR_E2M7	"E2M7: Spawning Vats"
#define HUSTR_E2M8	"E2M8: Tower of Babel"
#define HUSTR_E2M9	"E2M9: Fortress of Mystery"

#define HUSTR_E3M1	"E3M1: Hell Keep"
#define HUSTR_E3M2	"E3M2: Slough of Despair"
#define HUSTR_E3M3	"E3M3: Pandemonium"
#define HUSTR_E3M4	"E3M4: House of Pain"
#define HUSTR_E3M5	"E3M5: Unholy Cathedral"
#define HUSTR_E3M6	"E3M6: Mt. Erebus"
#define HUSTR_E3M7	"E3M7: Limbo"
#define HUSTR_E3M8	"E3M8: Dis"
#define HUSTR_E3M9	"E3M9: Warrens"

#define HUSTR_E4M1	"E4M1: Hell Beneath"
#define HUSTR_E4M2	"E4M2: Perfect Hatred"
#define HUSTR_E4M3	"E4M3: Sever The Wicked"
#define HUSTR_E4M4	"E4M4: Unruly Evil"
#define HUSTR_E4M5	"E4M5: They Will Repent"
#define HUSTR_E4M6	"E4M6: Against Thee Wickedly"
#define HUSTR_E4M7	"E4M7: And Hell Followed"
#define HUSTR_E4M8	"E4M8: Unto The Cruel"
#define HUSTR_E4M9	"E4M9: Fear"

#define HUSTR_1	"level 1: entryway"
#define HUSTR_2	"level 2: underhalls"
#define HUSTR_3	"level 3: the gantlet"
#define HUSTR_4	"level 4: the focus"
#define HUSTR_5	"level 5: the waste tunnels"
#define HUSTR_6	"level 6: the crusher"
#define HUSTR_7	"level 7: dead simple"
#define HUSTR_8	"level 8: tricks and traps"
#define HUSTR_9	"level 9: the pit"
#define HUSTR_10	"level 10: refueling base"
#define HUSTR_11	"level 11: 'o' of destruction!"

#define HUSTR_12	"level 12: the factory"
#define HUSTR_13	"level 13: downtown"
#define HUSTR_14	"level 14: the inmost dens"
#define HUSTR_15	"level 15: industrial zone"
#define HUSTR_16	"level 16: suburbs"
#define HUSTR_17	"level 17: tenements"
#define HUSTR_18	"level 18: the courtyard"
#define HUSTR_19	"level 19: the citadel"
#define HUSTR_20	"level 20: gotcha!"

#define HUSTR_21	"level 21: nirvana"
#define HUSTR_22	"level 22: the catacombs"
#define HUSTR_23	"level 23: barrels o' fun"
#define HUSTR_24	"level 24: the chasm"
#define HUSTR_25	"level 25: bloodfalls"
#define HUSTR_26	"level 26: the abandoned mines"
#define HUSTR_27	"level 27: monster condo"
#define HUSTR_28	"level 28: the spirit world"
#define HUSTR_29	"level 29: the living end"
#define HUSTR_30	"level 30: icon of sin"

#define HUSTR_31	"level 31: wolfenstein"
#define HUSTR_32	"level 32: grosse"

#define PHUSTR_1	"level 1: congo"
#define PHUSTR_2	"level 2: well of souls"
#define PHUSTR_3	"level 3: aztec"
#define PHUSTR_4	"level 4: caged"
#define PHUSTR_5	"level 5: ghost town"
#define PHUSTR_6	"level 6: baron's lair"
#define PHUSTR_7	"level 7: caughtyard"
#define PHUSTR_8	"level 8: realm"
#define PHUSTR_9	"level 9: abattoire"
#define PHUSTR_10	"level 10: onslaught"
#define PHUSTR_11	"level 11: hunted"

#define PHUSTR_12	"level 12: speed"
#define PHUSTR_13	"level 13: the crypt"
#define PHUSTR_14	"level 14: genesis"
#define PHUSTR_15	"level 15: the twilight"
#define PHUSTR_16	"level 16: the omen"
#define PHUSTR_17	"level 17: compound"
#define PHUSTR_18	"level 18: neurosphere"
#define PHUSTR_19	"level 19: nme"
#define PHUSTR_20	"level 20: the death domain"

#define PHUSTR_21	"level 21: slayer"
#define PHUSTR_22	"level 22: impossible mission"
#define PHUSTR_23	"level 23: tombstone"
#define PHUSTR_24	"level 24: the final frontier"
#define PHUSTR_25	"level 25: the temple of darkness"
#define PHUSTR_26	"level 26: bunker"
#define PHUSTR_27	"level 27: anti-christ"
#define PHUSTR_28	"level 28: the sewers"
#define PHUSTR_29	"level 29: odyssey of noises"
#define PHUSTR_30	"level 30: the gateway of hell"

#define PHUSTR_31	"level 31: cyberden"
#define PHUSTR_32	"level 32: go 2 it"

#define THUSTR_1	"level 1: system control"
#define THUSTR_2	"level 2: human bbq"
#define THUSTR_3	"level 3: power control"
#define THUSTR_4	"level 4: wormhole"
#define THUSTR_5	"level 5: hanger"
#define THUSTR_6	"level 6: open season"
#define THUSTR_7	"level 7: prison"
#define THUSTR_8	"level 8: metal"
#define THUSTR_9	"level 9: stronghold"
#define THUSTR_10	"level 10: redemption"
#define THUSTR_11	"level 11: storage facility"

#define THUSTR_12	"level 12: crater"
#define THUSTR_13	"level 13: nukage processing"
#define THUSTR_14	"level 14: steel works"
#define THUSTR_15	"level 15: dead zone"
#define THUSTR_16	"level 16: deepest reaches"
#define THUSTR_17	"level 17: processing area"
#define THUSTR_18	"level 18: mill"
#define THUSTR_19	"level 19: shipping/respawning"
#define THUSTR_20	"level 20: central processing"

#define THUSTR_21	"level 21: administration center"
#define THUSTR_22	"level 22: habitat"
#define THUSTR_23	"level 23: lunar mining project"
#define THUSTR_24	"level 24: quarry"
#define THUSTR_25	"level 25: baron's den"
#define THUSTR_26	"level 26: ballistyx"
#define THUSTR_27	"level 27: mount pain"
#define THUSTR_28	"level 28: heck"
#define THUSTR_29	"level 29: river styx"
#define THUSTR_30	"level 30: last call"

#define THUSTR_31	"level 31: pharaoh"
#define THUSTR_32	"level 32: caribbean"

#define HUSTR_CHATMACRO1	"I'm ready to kick butt!"
#define HUSTR_CHATMACRO2	"I'm OK."
#define HUSTR_CHATMACRO3	"I'm not looking too good!"
#define HUSTR_CHATMACRO4	"Help!"
#define HUSTR_CHATMACRO5	"You suck!"
#define HUSTR_CHATMACRO6	"Next time, scumbag..."
#define HUSTR_CHATMACRO7	"Come here!"
#define HUSTR_CHATMACRO8	"I'll take care of it."
#define HUSTR_CHATMACRO9	"Yes"
#define HUSTR_CHATMACRO0	"No"

#define HUSTR_TALKTOSELF1	"You mumble to yourself"
#define HUSTR_TALKTOSELF2	"Who's there?"
#define HUSTR_TALKTOSELF3	"You scare yourself"
#define HUSTR_TALKTOSELF4	"You start to rave"
#define HUSTR_TALKTOSELF5	"You've lost it..."

#define HUSTR_MESSAGESENT	"[Message Sent]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define HUSTR_PLRGREEN	"Green: "
#define HUSTR_PLRINDIGO	"Indigo: "
#define HUSTR_PLRBROWN	"Brown: "
#define HUSTR_PLRRED		"Red: "

#define HUSTR_KEYGREEN	'g'
#define HUSTR_KEYINDIGO	'i'
#define HUSTR_KEYBROWN	'b'
#define HUSTR_KEYRED	'r'

//
//	AM_map.C
//

#define AMSTR_FOLLOWON	"Follow Mode ON"
#define AMSTR_FOLLOWOFF	"Follow Mode OFF"

#define AMSTR_GRIDON	"Grid ON"
#define AMSTR_GRIDOFF	"Grid OFF"

#define AMSTR_MARKEDSPOT	"Marked Spot"
#define AMSTR_MARKSCLEARED	"All Marks Cleared"

//
//	ST_stuff.C
//

#define STSTR_MUS		"Music Change"
#define STSTR_NOMUS		"IMPOSSIBLE SELECTION"
#define STSTR_DQDON		"Degreelessness Mode On"
#define STSTR_DQDOFF	"Degreelessness Mode Off"

#define STSTR_KFAADDED	"Very Happy Ammo Added"
#define STSTR_FAADDED	"Ammo (no keys) Added"

#define STSTR_NCON		"No Clipping Mode ON"
#define STSTR_NCOFF		"No Clipping Mode OFF"

#define STSTR_BEHOLD	"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define STSTR_BEHOLDX	"Power-up Toggled"

#define STSTR_CHOPPERS	"... doesn't suck - GM"
#define STSTR_CLEV		"Changing Level..."

//
//	F_Finale.C
//
#define E1TEXT \
"Once you beat the big badasses and\0"\
"clean out the moon base you're supposed\0"\
"to win, aren't you? Aren't you? Where's\0"\
"your fat reward and ticket home? What\0"\
"the hell is this? It's not supposed to\0"\
"end this way!\0"\
"\0" \
"It stinks like rotten meat, but looks\0"\
"like the lost Deimos base.  Looks like\0"\
"you're stuck on The Shores of Hell.\0"\
"The only way out is through.\0"\
"\0"\
"To continue the DOOM experience, play\0"\
"The Shores of Hell and its amazing\0"\
"sequel, Inferno!\0"


#define E2TEXT \
"You've done it! The hideous cyber-\0"\
"demon lord that ruled the lost Deimos\0"\
"moon base has been slain and you\0"\
"are triumphant! But ... where are\0"\
"you? You clamber to the edge of the\0"\
"moon and look down to see the awful\0"\
"truth.\0" \
"\0"\
"Deimos floats above Hell itself!\0"\
"You've never heard of anyone escaping\0"\
"from Hell, but you'll make the bastards\0"\
"sorry they ever heard of you! Quickly,\0"\
"you rappel down to  the surface of\0"\
"Hell.\0"\
"\0" \
"Now, it's on to the final chapter of\0"\
"DOOM! -- Inferno.\0"


#define E3TEXT \
"The loathsome spiderdemon that\0"\
"masterminded the invasion of the moon\0"\
"bases and caused so much death has had\0"\
"its ass kicked for all time.\0"\
"\0"\
"A hidden doorway opens and you enter.\0"\
"You've proven too tough for Hell to\0"\
"contain, and now Hell at last plays\0"\
"fair -- for you emerge from the door\0"\
"to see the green fields of Earth!\0"\
"Home at last.\0" \
"\0"\
"You wonder what's been happening on\0"\
"Earth while you were battling evil\0"\
"unleashed. It's good that no Hell-\0"\
"spawn could have come through that\0"\
"door with you ...\0"


#define E4TEXT \
"the spider mastermind must have sent forth\0"\
"its legions of hellspawn before your\0"\
"final confrontation with that terrible\0"\
"beast from hell.  but you stepped forward\0"\
"and brought forth eternal damnation and\0"\
"suffering upon the horde as a true hero\0"\
"would in the face of something so evil.\0"\
"\0"\
"besides, someone was gonna pay for what\0"\
"happened to daisy, your pet rabbit.\0"\
"\0"\
"but now, you see spread before you more\0"\
"potential pain and gibbitude as a nation\0"\
"of demons run amok among our cities.\0"\
"\0"\
"next stop, hell on earth!\0"


// after level 6, put this:

#define C1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\0" \
"STARPORT. BUT SOMETHING IS WRONG. THE\0" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\0" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\0" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\0" \
"\0"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\0" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\0" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\0" \
"OF THE STARBASE AND FIND THE CONTROLLING\0" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\0" \
"HOSTAGE.\0"

// After level 11, put this:

#define C2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\0" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\0"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\0"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\0"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\0"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\0"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\0"\
"THAT YOU HAVE SAVED YOUR SPECIES.\0"\
"\0"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\0"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\0"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\0"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\0"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\0"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\0"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\0"\
"UP AND RETURN TO THE FRAY.\0"


// After level 20, put this:

#define C3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\0"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\0"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\0"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\0"\
"TEETH AND PLUNGE THROUGH IT.\0"\
"\0"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\0"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\0"\
"GOT TO GO THROUGH HELL TO GET TO IT?\0"


// After level 29, put this:

#define C4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\0"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\0"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\0"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\0"\
"UP AND DIES, ITS THRASHING LIMBS\0"\
"DEVASTATING UNTOLD MILES OF HELL'S\0"\
"SURFACE.\0"\
"\0"\
"YOU'VE DONE IT. THE INVASION IS OVER.\0"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\0"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\0"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\0"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\0"\
"HOME. REBUILDING EARTH OUGHT TO BE A\0"\
"LOT MORE FUN THAN RUINING IT WAS.\0"



// Before level 31, put this:

#define C5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\0"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\0"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\0"\
"WHO THE INMATES OF THIS CORNER OF HELL\0"\
"WILL BE.\0"


// Before level 32, put this:

#define C6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\0"\
"SUPER SECRET LEVEL!  YOU'D BETTER\0"\
"BLAZE THROUGH THIS ONE!\0"


#endif

