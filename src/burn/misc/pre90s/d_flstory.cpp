// FB Alpha Fairyland Story driver module
// Based on MAME driver by Nicola Salmoria, Gareth Hall, and Takahiro Nog

// Missing MSM5232 sound

#include "tiles_generic.h"
#include "../../taito/taito_m68705.h"
#include "driver.h"
#include "dac.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvZ80ROM0;
static unsigned char *DrvZ80ROM1;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvMcuROM;
static unsigned char *DrvZ80RAM0;
static unsigned char *DrvZ80RAM1;
static unsigned char *DrvVidRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvMcuRAM;

static unsigned int  *DrvPalette;
static unsigned char DrvRecalc;

static short *pAY8910Buffer[3];

static unsigned char  DrvJoy1[8];
static unsigned char  DrvJoy2[8];
static unsigned char  DrvJoy3[8];
static unsigned char  DrvJoy4[8];
static unsigned char  DrvJoy5[8];
static unsigned char  DrvDips[3];
static unsigned char  DrvInputs[5];
static unsigned char  DrvReset;

static unsigned char *flipscreen;
static unsigned char *soundlatch;

static unsigned char snd_data;
static unsigned char snd_flag;
static int nmi_enable;
static int pending_nmi;
static int char_bank;
static int mcu_select;

static int select_game;

static struct BurnInputInfo FlstoryInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 3,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Flstory)

static struct BurnInputInfo Onna34roInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 3,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Onna34ro)

static struct BurnInputInfo VictnineInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy4 + 2,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy4 + 3,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 fire 5"	},
	{"P1 Button 6",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 6"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy5 + 2,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy5 + 3,	"p2 fire 4"	},
	{"P2 Button 5",		BIT_DIGITAL,	DrvJoy5 + 4,	"p2 fire 5"	},
	{"P2 Button 6",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 3,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Victnine)

static struct BurnDIPInfo FlstoryDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xec, NULL				},
	{0x10, 0xff, 0xff, 0x00, NULL				},
	{0x11, 0xff, 0xff, 0xf8, NULL				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x0f, 0x01, 0x03, 0x00, "30000 100000"			},
	{0x0f, 0x01, 0x03, 0x01, "30000 150000"			},
	{0x0f, 0x01, 0x03, 0x02, "50000 150000"			},
	{0x0f, 0x01, 0x03, 0x03, "70000 150000"			},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x0f, 0x01, 0x04, 0x04, "Off"				},
	{0x0f, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x0f, 0x01, 0x18, 0x08, "3"				},
	{0x0f, 0x01, 0x18, 0x10, "4"				},
	{0x0f, 0x01, 0x18, 0x18, "5"				},
	{0x0f, 0x01, 0x18, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    2, "Debug Mode"			},
	{0x0f, 0x01, 0x20, 0x20, "Off"				},
	{0x0f, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x0f, 0x01, 0x40, 0x40, "Off"				},
	{0x0f, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x0f, 0x01, 0x80, 0x80, "Upright"			},
	{0x0f, 0x01, 0x80, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,   16, "Coin A"			},
	{0x10, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"		},
	{0x10, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"		},
	{0x10, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"		},
	{0x10, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"		},
	{0x10, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"		},
	{0x10, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"		},
	{0x10, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"		},
	{0x10, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"		},
	{0x10, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,   16, "Coin B"			},
	{0x10, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x10, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"		},
	{0x10, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"		},
	{0x10, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"		},
	{0x10, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"		},
	{0x10, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"		},
	{0x10, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"		},
	{0x10, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x11, 0x01, 0x08, 0x00, "No"				},
	{0x11, 0x01, 0x08, 0x08, "Yes"				},

	{0   , 0xfe, 0   ,    2, "Attract Animation"		},
	{0x11, 0x01, 0x10, 0x00, "Off"				},
	{0x11, 0x01, 0x10, 0x10, "On"				},

	{0   , 0xfe, 0   ,    2, "Leave Off"			},
	{0x11, 0x01, 0x20, 0x20, "Off"				},
	{0x11, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x11, 0x01, 0x40, 0x40, "Off"				},
	{0x11, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Slots"			},
	{0x11, 0x01, 0x80, 0x00, "1"				},
	{0x11, 0x01, 0x80, 0x80, "2"				},
};

STDDIPINFO(Flstory)


static struct BurnDIPInfo Onna34roDIPList[]=
{
	{0x13, 0xff, 0xff, 0xe0, NULL				},
	{0x14, 0xff, 0xff, 0x00, NULL				},
	{0x15, 0xff, 0xff, 0x80, NULL				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x13, 0x01, 0x03, 0x00, "200000 200000"		},
	{0x13, 0x01, 0x03, 0x01, "200000 300000"		},
	{0x13, 0x01, 0x03, 0x02, "100000 200000"		},
	{0x13, 0x01, 0x03, 0x03, "200000 100000"		},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x13, 0x01, 0x04, 0x00, "Off"				},
	{0x13, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x13, 0x01, 0x18, 0x10, "1"				},
	{0x13, 0x01, 0x18, 0x08, "2"				},
	{0x13, 0x01, 0x18, 0x00, "3"				},
	{0x13, 0x01, 0x18, 0x18, "Endless (Cheat)"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x13, 0x01, 0x40, 0x40, "Off"				},
	{0x13, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x13, 0x01, 0x80, 0x80, "Upright"			},
	{0x13, 0x01, 0x80, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,   16, "Coin A"			},
	{0x14, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,   16, "Coin B"			},
	{0x14, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"},
	{0x15, 0x01, 0x01, 0x00, "Off"				},
	{0x15, 0x01, 0x01, 0x01, "On"				},

	{0   , 0xfe, 0   ,    2, "Rack Test"			},
	{0x15, 0x01, 0x02, 0x00, "Off"				},
	{0x15, 0x01, 0x02, 0x02, "On"				},

	{0   , 0xfe, 0   ,    2, "Freeze"			},
	{0x15, 0x01, 0x08, 0x00, "Off"				},
	{0x15, 0x01, 0x08, 0x08, "On"				},

	{0   , 0xfe, 0   ,    2, "Coinage Display"		},
	{0x15, 0x01, 0x10, 0x10, "Off"				},
	{0x15, 0x01, 0x10, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x15, 0x01, 0x60, 0x20, "Easy"				},
	{0x15, 0x01, 0x60, 0x00, "Normal"			},
	{0x15, 0x01, 0x60, 0x40, "Difficult"			},
	{0x15, 0x01, 0x60, 0x60, "Very Difficult"		},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x15, 0x01, 0x80, 0x80, "A and B"			},
	{0x15, 0x01, 0x80, 0x00, "A only"			},
};

STDDIPINFO(Onna34ro)

static struct BurnDIPInfo VictnineDIPList[]=
{
	{0x1b, 0xff, 0xff, 0x67, NULL				},
	{0x1c, 0xff, 0xff, 0x00, NULL				},
	{0x1d, 0xff, 0xff, 0x7f, NULL				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x1b, 0x01, 0x04, 0x04, "Off"				},
	{0x1b, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x1b, 0x01, 0x40, 0x40, "Off"				},
	{0x1b, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    3, "Cabinet"			},
	{0x1b, 0x01, 0xa0, 0x20, "Upright"			},
	{0x1b, 0x01, 0xa0, 0xa0, "Cocktail"			},
	{0x1b, 0x01, 0xa0, 0x00, "MA / MB"			},

	{0   , 0xfe, 0   ,   16, "Coin A"			},
	{0x1c, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"		},
	{0x1c, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"		},
	{0x1c, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"		},
	{0x1c, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"		},
	{0x1c, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"		},
	{0x1c, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"		},
	{0x1c, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"		},
	{0x1c, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,   16, "Coin B"			},
	{0x1c, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"		},
	{0x1c, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x1c, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"		},
	{0x1c, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"		},
	{0x1c, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"		},
	{0x1c, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"		},
	{0x1c, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"		},
	{0x1c, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"		},
	{0x1c, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    2, "Coinage Display"		},
	{0x1d, 0x01, 0x10, 0x00, "Off"				},
	{0x1d, 0x01, 0x10, 0x10, "On"				},

	{0   , 0xfe, 0   ,    2, "Show Year"			},
	{0x1d, 0x01, 0x20, 0x00, "Off"				},
	{0x1d, 0x01, 0x20, 0x20, "On"				},

	{0   , 0xfe, 0   ,    2, "No hit"			},
	{0x1d, 0x01, 0x40, 0x40, "Off"				},
	{0x1d, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x1d, 0x01, 0x80, 0x80, "A and B"			},
	{0x1d, 0x01, 0x80, 0x00, "A only"			},
};

STDDIPINFO(Victnine)

static void gfxctrl_write(int data)
{
	char_bank = (data & 0x10) >> 4;

	int bank = (data & 0x20) << 3;

	ZetMapArea(0xdd00, 0xddff, 0, DrvPalRAM + 0x000 + bank);
	ZetMapArea(0xdd00, 0xddff, 1, DrvPalRAM + 0x000 + bank);
	ZetMapArea(0xde00, 0xdeff, 0, DrvPalRAM + 0x200 + bank);
	ZetMapArea(0xde00, 0xdeff, 1, DrvPalRAM + 0x200 + bank);

	if (data & 4) *flipscreen = (~data & 0x01);
}

static void onna34ro_mcu_write(int data)
{
	int score_adr = (ZetReadByte(0xe29e) << 8) | ZetReadByte(0xe29d);

	mcu_sent = 1;

	switch (data)
	{
		case 0x0e:
			from_mcu = 0xff;
			break;

		case 0x01:
			from_mcu = 0x6a;
			break;

		case 0x40:
			from_mcu = ZetReadByte(score_adr);
			break;

		case 0x41:
			from_mcu = ZetReadByte(score_adr+1);
			break;

		case 0x42:
			from_mcu = ZetReadByte(score_adr+2) & 0x0f;
			break;

		default:
			from_mcu = 0x80;
	}
}

static const unsigned char victnine_mcu_data[0x100] =
{
	0x3e, 0x08, 0xdd, 0x29, 0xcb, 0x14, 0xfd, 0x29,
	0xcb, 0x15, 0xd9, 0x29, 0xd9, 0x30, 0x0d, 0xd9,
	0x19, 0xd9, 0xdd, 0x09, 0x30, 0x01, 0x24, 0xfd,
	0x19, 0x30, 0x01, 0x2c, 0x3d, 0x20, 0xe3, 0xc9,
	0x11, 0x14, 0x00, 0x19, 0x7e, 0x32, 0xed, 0xe4,
	0x2a, 0x52, 0xe5, 0x22, 0xe9, 0xe4, 0x2a, 0x54,
	0xe5, 0x22, 0xeb, 0xe4, 0x21, 0x2a, 0xe6, 0xfe,
	0x06, 0x38, 0x02, 0xcb, 0xc6, 0xcb, 0xce, 0xc9,
	0x06, 0x00, 0x3a, 0xaa, 0xe4, 0x07, 0x07, 0x07,
	0xb0, 0x47, 0x3a, 0xab, 0xe4, 0x07, 0x07, 0xb0,
	0x47, 0x3a, 0xac, 0xe4, 0xe6, 0x03, 0xb0, 0x21,
	0xe3, 0xe6, 0xc9, 0x38, 0xe1, 0x29, 0x07, 0xc9,
	0x23, 0x7e, 0x47, 0xe6, 0x1f, 0x32, 0x0c, 0xe6,
	0x78, 0xe6, 0xe0, 0x07, 0x07, 0x47, 0xe6, 0x03,
	0x28, 0x06, 0xcb, 0x7a, 0x28, 0x02, 0xc6, 0x02,
	0x32, 0x0a, 0xe6, 0x78, 0xe6, 0x80, 0xc9, 0x3a,
	0x21, 0x29, 0xe5, 0x7e, 0xe6, 0xf8, 0xf6, 0x01,
	0x77, 0x23, 0x3a, 0x0a, 0xe6, 0x77, 0x21, 0x08,
	0xe6, 0xcb, 0xc6, 0xcb, 0x8e, 0x3a, 0x2b, 0xe5,
	0x21, 0xff, 0xe5, 0xfe, 0x02, 0xc9, 0x1f, 0xc6,
	0x47, 0x3a, 0xaa, 0xe4, 0xa7, 0x21, 0x00, 0xe5,
	0x28, 0x03, 0x21, 0x1b, 0xe5, 0x70, 0x3a, 0xaa,
	0xe4, 0xee, 0x01, 0x32, 0xaa, 0xe4, 0x21, 0xb0,
	0xe4, 0x34, 0x23, 0x36, 0x00, 0xc9, 0x2b, 0xb2,
	0xaf, 0x77, 0x12, 0x23, 0x13, 0x3c, 0xfe, 0x09,
	0x20, 0xf7, 0x3e, 0x01, 0x32, 0xad, 0xe4, 0x21,
	0x48, 0xe5, 0xcb, 0xfe, 0xc9, 0x32, 0xe5, 0xaa,
	0x21, 0x00, 0x13, 0xe4, 0x47, 0x1b, 0xa1, 0xc9,
	0x00, 0x08, 0x04, 0x0c, 0x05, 0x0d, 0x06, 0x0e,
	0x22, 0x66, 0xaa, 0x22, 0x33, 0x01, 0x11, 0x88,
	0x06, 0x05, 0x03, 0x04, 0x08, 0x01, 0x03, 0x02,
	0x06, 0x07, 0x02, 0x03, 0x15, 0x17, 0x11, 0x13
};

static void victnine_mcu_write(int data)
{
	int seed = ZetReadByte(0xe685);

	if (!seed && (data & 0x37) == 0x37)
	{
		from_mcu = 0xa6;
	}
	else
	{
		data += seed;

		if ((data & ~0x1f) == 0xa0)
		{
			mcu_select = data & 0x1f;
		}
		else if (data < 0x20)
		{
			int offset = mcu_select * 8 + data;
			from_mcu = victnine_mcu_data[offset];
		}
		else if (data >= 0x38 && data <= 0x3a)
		{
			from_mcu = ZetReadByte(0xe691 - 0x38 + data);
		}
	}
}

void __fastcall flstory_main_write(unsigned short address, unsigned char data)
{
	if ((address & 0xff00) == 0xdc00) {
		DrvSprRAM[address & 0xff] = data;

		if (select_game == 2 && address == 0xdce0) {
			gfxctrl_write((data ^ 1) & ~0x10);
		}

		return;
	}

	switch (address)
	{
		case 0xd000:
			if (select_game == 2) {
				victnine_mcu_write(data);
			} if (select_game == 1) {
				onna34ro_mcu_write(data);
			} else {
				standard_taito_mcu_write(data);
			}
		return;

		case 0xd400:
		{
			*soundlatch = data;

			if (nmi_enable) {
				ZetClose();
				ZetOpen(1);
				ZetNmi();
				ZetClose();
				ZetOpen(0);
			} else {
				pending_nmi = 1;
			}
		}
		return;

		case 0xdf03:
			if (select_game != 2) gfxctrl_write(data | 0x04);
		return;
	}
}

unsigned char __fastcall flstory_main_read(unsigned short address)
{
	switch (address)
	{
		case 0xd000:
			if (select_game == 2) {
				return from_mcu - ZetReadByte(0xe685);
			} else {
				return standard_taito_mcu_read();
			}

		case 0xd400:
			snd_flag = 0;
			return snd_data;

		case 0xd401:
			return snd_flag | 0xfd;

		case 0xd800:
		case 0xd801:
		case 0xd802:
			return DrvDips[address & 3];

		case 0xd803:
			return DrvInputs[0] & 0x3f;

		case 0xd804:
			return DrvInputs[1];

		case 0xd805:
		{
			int res = 0;
			if (!main_sent) res |= 0x01;
			if (mcu_sent) res |= 0x02;

			if (select_game == 2) res |= DrvInputs[3];

			return res;
		}

		case 0xd806:
			return DrvInputs[2];

		case 0xd807:
			return DrvInputs[4];
	}

	return 0;
}

void __fastcall flstory_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xc800:
		case 0xc801:
			AY8910Write(0, address & 1, data);
		return;

		case 0xca00:
		case 0xca01:
		case 0xca02:
		case 0xca03:
		case 0xca04:
		case 0xca05:
		case 0xca06:
		case 0xca07:
		case 0xca08:
		case 0xca09:
		case 0xca0a:
		case 0xca0b:
		case 0xca0c:
		case 0xca0d:
			// msm5232
		return;

		case 0xd800:
			snd_data = data;
			snd_flag = 2;
		return;

		case 0xda00:
			nmi_enable = 1;
			if (pending_nmi)
			{
				ZetNmi();
				pending_nmi = 0;
			}
		return;

		case 0xdc00:
			nmi_enable = 0;
		return;

		case 0xde00:
			DACSignedWrite(data);
		return;
	}
}

unsigned char __fastcall flstory_sound_read(unsigned short address)
{
	if (address == 0xd800) {
		return *soundlatch;
	}

	return 0;
}

static int DrvDoReset()
{
	DrvReset = 0;
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	m67805_taito_reset();

	AY8910Reset(0);
	DACReset();

	snd_data = 0;
	snd_flag = 0;
	nmi_enable = 0;
	pending_nmi = 0;
	char_bank = 0;
	mcu_select = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM0		= Next; Next += 0x010000;
	DrvZ80ROM1		= Next; Next += 0x010000;

	DrvMcuROM		= Next; Next += 0x000800;

	DrvGfxROM0		= Next; Next += 0x040000;
	DrvGfxROM1		= Next; Next += 0x040000;

	DrvPalette		= (unsigned int*)Next; Next += 0x0200 * sizeof(int);

	AllRam			= Next;

	DrvVidRAM		= Next; Next += 0x001000;
	DrvZ80RAM0		= Next; Next += 0x000800;
	DrvZ80RAM1		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000400;
	DrvSprRAM		= Next; Next += 0x000100;

	DrvMcuRAM		= Next; Next += 0x000080;

	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;

	pAY8910Buffer[0]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[1]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[2]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);

	MemEnd			= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4]  = { 0x80000, 0x80004, 0x00000, 0x00004 };
	int XOffs[16] = { 0x003, 0x002, 0x001, 0x000, 0x00b, 0x00a, 0x009, 0x008,
			  0x083, 0x082, 0x081, 0x080, 0x08b, 0x08a, 0x089, 0x088 };
	int YOffs[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
			  0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170 };

	unsigned char *tmp = (unsigned char*)malloc(0x20000);
	if (tmp == NULL) {
		return 1;
	}

	for (int i = 0; i < 0x20000; i++) tmp[i] = DrvGfxROM0[i] ^ 0xff;

	GfxDecode(0x1000, 4,  8,  8, Plane, XOffs, YOffs, 0x080, tmp, DrvGfxROM0);
	GfxDecode(0x0400, 4, 16, 16, Plane, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	free (tmp);

	return 0;
}

static int DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (select_game == 0) {
			if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  2, 1)) return 1;

			if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  3, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x02000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x00000,  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x04000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x08000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0c000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x10000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x14000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x18000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x1c000, 12, 1)) return 1;

			if (BurnLoadRom(DrvMcuROM  + 0x00000, 13, 1)) return 1;
		} else if (select_game == 1) {
			if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  2, 1)) return 1;

			if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  3, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x02000,  4, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x04000,  5, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x06000,  6, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x08000,  7, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x00000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x04000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x08000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0c000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x10000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x14000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x18000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x1c000, 15, 1)) return 1;
		} else if (select_game == 2) {
			if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x02000,  1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  2, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x06000,  3, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  4, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x0a000,  5, 1)) return 1;

			if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  6, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x02000,  7, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x04000,  8, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x06000,  9, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x08000, 10, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM1 + 0x0a000, 11, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x00000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x02000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x04000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x06000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x10000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x12000, 17, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x14000, 18, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x16000, 19, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(2);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvVidRAM);
	ZetMapArea(0xc000, 0xcfff, 1, DrvVidRAM);
	ZetMapArea(0xc000, 0xcfff, 2, DrvVidRAM);
	ZetMapArea(0xdc00, 0xdcff, 0, DrvSprRAM);
//	ZetMapArea(0xdc00, 0xdcff, 1, DrvSprRAM);
	ZetMapArea(0xdc00, 0xdcff, 2, DrvSprRAM);
	ZetMapArea(0xdd00, 0xddff, 0, DrvPalRAM);
	ZetMapArea(0xdd00, 0xddff, 1, DrvPalRAM);
	ZetMapArea(0xdd00, 0xddff, 2, DrvPalRAM);
	ZetMapArea(0xde00, 0xdeff, 0, DrvPalRAM + 0x200);
	ZetMapArea(0xde00, 0xdeff, 1, DrvPalRAM + 0x200);
	ZetMapArea(0xde00, 0xdeff, 2, DrvPalRAM + 0x200);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(flstory_main_write);
	ZetSetReadHandler(flstory_main_read);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM1);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80ROM1 + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80ROM1 + 0xe000);
	ZetSetWriteHandler(flstory_sound_write);
	ZetSetReadHandler(flstory_sound_read);
	ZetMemEnd();
	ZetClose();

	m67805_taito_init(DrvMcuROM, DrvMcuRAM, &standard_m68705_interface);

	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	DACInit(0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	ZetExit();
	m67805_taito_exit();

	AY8910Exit(0);
	DACExit();

	free (AllMem);
	AllMem = NULL;

	select_game = 0;

	return 0;
}

static void draw_background_layer(int type, int priority)
{
	int masks[4] = { 0x3fff, 0xc000, 0x8000, 0x7fff };
	int mask = masks[type];

	for (int offs = 0; offs < 32 * 32; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		int sy = (offs >> 5) << 3;

		sy -= DrvSprRAM[0xa0 + (offs & 0x1f)] + 16;
		if (sy < -7) sy += 256;
		if (sy >= nScreenHeight) continue;

		int attr  = DrvVidRAM[offs * 2 + 1];
		int code  = DrvVidRAM[offs * 2 + 0] | ((attr & 0xc0) << 2) | 0x400 | (char_bank * 0x800);

		int flipy =  attr & 0x10;
		int flipx =  attr & 0x08;
		int color = (attr & 0x0f) << 4;
		int prio  = (attr & 0x20) >> 5;
		if (priority && !prio) continue;

		{
			if (flipy) flipy  = 0x38;
			if (flipx) flipy |= 0x07;
			unsigned char *src = DrvGfxROM0 + (code * 8 * 8);
			unsigned short *dst;

			for (int y = 0; y < 8; y++, sy++) {
				if (sy < 0 || sy >= nScreenHeight) continue;
				dst = pTransDraw + sy * nScreenWidth;

				for (int x = 0; x < 8; x++, sx++) {
					if (sx < 0 || sx >= nScreenWidth) continue;

					int pxl = src[((y << 3) | x) ^ flipy];
					if (mask & (1 << pxl)) continue;

					dst[sx] = pxl | color;
				}

				sx -= 8;
			}
		}
	}
}

static void victnine_draw_background_layer()
{
	for (int offs = 0; offs < 32 * 32; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		int sy = (offs >> 5) << 3;

		sy -= DrvSprRAM[0xa0 + (offs & 0x1f)] + 16;
		if (sy < -7) sy += 256;
		if (sy >= nScreenHeight) continue;

		int attr  = DrvVidRAM[offs * 2 + 1];
		int code  = DrvVidRAM[offs * 2 + 0] | ((attr & 0x38) << 5);

		int flipy = attr & 0x80;
		int flipx = attr & 0x40;
		int color = attr & 0x07;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			}
		}

	}
}

static void draw_sprite_tiles(int code, int color, int sx, int sy, int flipx, int flipy)
{
	if (flipy) {
		if (flipx) {
			Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x100, DrvGfxROM1);
		} else {
			Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x100, DrvGfxROM1);
		}
	} else {
		if (flipx) {
			Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x100, DrvGfxROM1);
		} else {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x100, DrvGfxROM1);
		}
	}
}

static void draw_sprites(int pri, int type)
{
	for (int i = 0; i < 0x20; i++)
	{
		int pr = DrvSprRAM[0xa0-1 -i];
		int offs = (pr & 0x1f) * 4;

		if ((pr & 0x80) == pri || type)
		{
			int sy    = DrvSprRAM[offs + 0];
			int attr  = DrvSprRAM[offs + 1];
			int code  = DrvSprRAM[offs + 2];
			if (type) {
				code += (attr & 0x20) << 3;
			} else {
				code += ((attr & 0x30) << 4);
			}

			int sx    = DrvSprRAM[offs + 3];
			int flipx = attr & 0x40;
			int flipy = attr & 0x80;

			if (*flipscreen)
			{
				sx = (240 - sx) & 0xff;
				sy = sy - 1;

				flipx ^= 0x40;
				flipx ^= 0x80;
			}
			else
				sy = 240 - sy - 1;

			draw_sprite_tiles(code, attr & 0x0f, sx, sy - 16, flipx, flipy);

			if (sx > 240) {
				draw_sprite_tiles(code, attr & 0x0f, sx - 256, sy - 16, flipx, flipy);;
			}
		}
	}
}

static inline void DrvRecalcPalette()
{
	unsigned char r,g,b;
	for (int i = 0; i < 0x200; i++) {
		int d = DrvPalRAM[i] | (DrvPalRAM[i + 0x200] << 8);
		b = (d >> 8) & 0x0f;
		g = (d >> 4) & 0x0f;
		r = d & 0x0f;
		DrvPalette[i] = BurnHighCol((r << 4) | r, (g << 4) | g, (b << 4) | b, 0);
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	BurnTransferClear();

	if (nBurnLayer & 1) if (nBurnLayer & 1) draw_background_layer(1, 0);
	if (nBurnLayer & 2) if (nBurnLayer & 4) draw_background_layer(3, 1);
	if (nSpriteEnable & 1) draw_sprites(0x00, 0);
	if (nBurnLayer & 4) if (nBurnLayer & 2) draw_background_layer(0, 0);
	if (nSpriteEnable & 2) draw_sprites(0x80, 0);
	if (nBurnLayer & 8) if (nBurnLayer & 8) draw_background_layer(2, 1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int victnineDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	victnine_draw_background_layer();
	draw_sprites(0, 1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 5);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	int nSoundBufferPos = 0;
	int nInterleave = nBurnSoundLen ? nBurnSoundLen : 100;
	int nCyclesTotal[3] = { 5366500 / 60, 4000000 / 60, 3072000 / 60 };
	int nCyclesDone[3]  = { 0, 0, 0 };
	if (select_game == 2) nCyclesTotal[0] = nCyclesTotal[1];

	for (int i = 0; i < nInterleave; i++)
	{
		int nSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		ZetOpen(1);
		nSegment = nCyclesTotal[1] / nInterleave;
		nCyclesDone[1] += ZetRun(nSegment);
		if (i == (nInterleave / 2) - 1) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		if (i == (nInterleave / 1) - 1) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		if (select_game == 0) {
			m6805Open(0);
			nSegment = nCyclesTotal[2] / nInterleave;
			nCyclesDone[2] += m6805Run(nSegment);
			m6805Close();
		}

		if (pBurnSoundOut) {
			int nSample;
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				if (nSample < -32768) {
					nSample = -32768;
				} else {
					if (nSample > 32767) {
						nSample = 32767;
					}
				}

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}

			DACUpdate(pSoundBuf, nSegmentLength);

			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		int nSample;
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				if (nSample < -32768) {
					nSample = -32768;
				} else {
					if (nSample > 32767) {
						nSample = 32767;
					}
				}

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
		}

		DACUpdate(pSoundBuf, nSegmentLength);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);

		ZetScan(nAction);
		m68705_taito_scan(nAction);

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(nmi_enable);
		SCAN_VAR(pending_nmi);
		SCAN_VAR(char_bank);
		SCAN_VAR(mcu_select);
		SCAN_VAR(snd_flag);
		SCAN_VAR(snd_data);
	}

	return 0;
}


// The FairyLand Story

static struct BurnRomInfo flstoryRomDesc[] = {
	{ "cpu-a45.15",		0x4000, 0xf03fc969, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code
	{ "cpu-a45.16",		0x4000, 0x311aa82e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpu-a45.17",		0x4000, 0xa2b5d17d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "snd.22",		0x2000, 0xd58b201d, 2 | BRF_PRG | BRF_ESS }, //  3 Sound Z80 Code
	{ "snd.23",		0x2000, 0x25e7fd9d, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "vid-a45.18",		0x4000, 0x6f08f69e, 3 | BRF_GRA },           //  5 Tiles and Sprites
	{ "vid-a45.06",		0x4000, 0xdc856a75, 3 | BRF_GRA },           //  6
	{ "vid-a45.08",		0x4000, 0xd0b028ca, 3 | BRF_GRA },           //  7
	{ "vid-a45.20",		0x4000, 0x1b0edf34, 3 | BRF_GRA },           //  8
	{ "vid-a45.19",		0x4000, 0x2b572dc9, 3 | BRF_GRA },           //  9
	{ "vid-a45.07",		0x4000, 0xaa4b0762, 3 | BRF_GRA },           // 10
	{ "vid-a45.09",		0x4000, 0x8336be58, 3 | BRF_GRA },           // 11
	{ "vid-a45.21",		0x4000, 0xfc382bd1, 3 | BRF_GRA },           // 12

	{ "a45.mcu",		0x0800, 0x5378253c, 4 | BRF_PRG | BRF_ESS }, // 13 M68705 MCU Code
};

STD_ROM_PICK(flstory)
STD_ROM_FN(flstory)

static int flstoryInit()
{
	select_game = 0;

	return DrvInit();
}

struct BurnDriver BurnDrvFlstory = {
	"flstory", NULL, NULL, "1985",
	"The FairyLand Story\0", "Imperfect sound", "Taito", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_MISC,
	NULL, flstoryRomInfo, flstoryRomName, FlstoryInputInfo, FlstoryDIPInfo,
	flstoryInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc,
	256, 224, 4, 3
};


// The FairyLand Story (Japan)

static struct BurnRomInfo flstoryjRomDesc[] = {
	{ "cpu-a45.15",		0x4000, 0xf03fc969, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code
	{ "cpu-a45.16",		0x4000, 0x311aa82e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpu-a45.17",		0x4000, 0xa2b5d17d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "a45_12.8",		0x2000, 0xd6f593fb, 2 | BRF_PRG | BRF_ESS }, //  3 Sound Z80 Code
	{ "a45_13.9",		0x2000, 0x451f92f9, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "vid-a45.18",		0x4000, 0x6f08f69e, 3 | BRF_GRA },           //  5 Tiles and Sprites
	{ "vid-a45.06",		0x4000, 0xdc856a75, 3 | BRF_GRA },           //  6
	{ "vid-a45.08",		0x4000, 0xd0b028ca, 3 | BRF_GRA },           //  7
	{ "vid-a45.20",		0x4000, 0x1b0edf34, 3 | BRF_GRA },           //  8
	{ "vid-a45.19",		0x4000, 0x2b572dc9, 3 | BRF_GRA },           //  9
	{ "vid-a45.07",		0x4000, 0xaa4b0762, 3 | BRF_GRA },           // 10
	{ "vid-a45.09",		0x4000, 0x8336be58, 3 | BRF_GRA },           // 11
	{ "vid-a45.21",		0x4000, 0xfc382bd1, 3 | BRF_GRA },           // 12

	{ "a45.mcu",		0x0800, 0x5378253c, 4 | BRF_PRG | BRF_ESS }, // 13 M68705 MCU Code
};

STD_ROM_PICK(flstoryj)
STD_ROM_FN(flstoryj)

struct BurnDriver BurnDrvFlstoryj = {
	"flstoryj", "flstory", NULL, "1985",
	"The FairyLand Story (Japan)\0", "Imperfect sound", "Taito", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_MISC,
	NULL, flstoryjRomInfo, flstoryjRomName, FlstoryInputInfo, FlstoryDIPInfo,
	flstoryInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc,
	256, 224, 4, 3
};


// Onna Sansirou - Typhoon Gal (set 1)

static struct BurnRomInfo onna34roRomDesc[] = {
	{ "a52-01-1.40c",	0x4000, 0xffddcb02, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code
	{ "a52-02-1.41c",	0x4000, 0xda97150d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a52-03-1.42c",	0x4000, 0xb9749a53, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "a52-12.08s",		0x2000, 0x28f48096, 2 | BRF_PRG | BRF_ESS }, //  3 Sound Z80 Code
	{ "a52-13.09s",		0x2000, 0x4d3b16f3, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "a52-14.10s",		0x2000, 0x90a6f4e8, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "a52-15.37s",		0x2000, 0x5afc21d0, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "a52-16.38s",		0x2000, 0xccf42aee, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "a52-04.11v",		0x4000, 0x5b126294, 3 | BRF_GRA },           //  8 Tiles and Sprites
	{ "a52-06.10v",		0x4000, 0x78114721, 3 | BRF_GRA },           //  9
	{ "a52-08.09v",		0x4000, 0x4a293745, 3 | BRF_GRA },           // 10
	{ "a52-10.08v",		0x4000, 0x8be7b4db, 3 | BRF_GRA },           // 11
	{ "a52-05.35v",		0x4000, 0xa1a99588, 3 | BRF_GRA },           // 12
	{ "a52-07.34v",		0x4000, 0x0bf420f2, 3 | BRF_GRA },           // 13
	{ "a52-09.33v",		0x4000, 0x39c543b5, 3 | BRF_GRA },           // 14
	{ "a52-11.32v",		0x4000, 0xd1dda6b3, 3 | BRF_GRA },           // 15

	{ "a52-17.54c",		0x0800, 0x00000000, 4 | BRF_NODUMP },        // 16 M68705 MCU Code
};

STD_ROM_PICK(onna34ro)
STD_ROM_FN(onna34ro)

static int onna34roInit()
{
	select_game = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvOnna34ro = {
	"onna34ro", NULL, NULL, "1985",
	"Onna Sansirou - Typhoon Gal (set 1)\0", "Imperfect sound", "Taito", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, onna34roRomInfo, onna34roRomName, Onna34roInputInfo, Onna34roDIPInfo,
	onna34roInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc,
	256, 224, 4, 3
};


// Onna Sansirou - Typhoon Gal (set 2)

static struct BurnRomInfo onna34raRomDesc[] = {
	{ "ry-08.rom",		0x4000, 0xe4587b85, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code
	{ "ry-07.rom",		0x4000, 0x6ffda515, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ry-06.rom",		0x4000, 0x6fefcda8, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "a52-12.08s",		0x2000, 0x28f48096, 2 | BRF_PRG | BRF_ESS }, //  3 Sound Z80 Code
	{ "a52-13.09s",		0x2000, 0x4d3b16f3, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "a52-14.10s",		0x2000, 0x90a6f4e8, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "a52-15.37s",		0x2000, 0x5afc21d0, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "a52-16.38s",		0x2000, 0xccf42aee, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "a52-04.11v",		0x4000, 0x5b126294, 3 | BRF_GRA },           //  8 Tiles and Sprites
	{ "a52-06.10v",		0x4000, 0x78114721, 3 | BRF_GRA },           //  9
	{ "a52-08.09v",		0x4000, 0x4a293745, 3 | BRF_GRA },           // 10
	{ "a52-10.08v",		0x4000, 0x8be7b4db, 3 | BRF_GRA },           // 11
	{ "a52-05.35v",		0x4000, 0xa1a99588, 3 | BRF_GRA },           // 12
	{ "a52-07.34v",		0x4000, 0x0bf420f2, 3 | BRF_GRA },           // 13
	{ "a52-09.33v",		0x4000, 0x39c543b5, 3 | BRF_GRA },           // 14
	{ "a52-11.32v",		0x4000, 0xd1dda6b3, 3 | BRF_GRA },           // 15

	{ "a52-17.54c",		0x0800, 0x00000000, 4 | BRF_NODUMP },        // 16 M68705 MCU Code
};

STD_ROM_PICK(onna34ra)
STD_ROM_FN(onna34ra)

struct BurnDriver BurnDrvOnna34ra = {
	"onna34roa", "onna34ro", NULL, "1985",
	"Onna Sansirou - Typhoon Gal (set 2)\0", "Imperfect sound", "Taito", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_MISC,
	NULL, onna34raRomInfo, onna34raRomName, Onna34roInputInfo, Onna34roDIPInfo,
	onna34roInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc,
	256, 224, 4, 3
};


// Victorious Nine

static struct BurnRomInfo victnineRomDesc[] = {
	{ "a16-19.1",		0x2000, 0xdeb7c439, 1 | BRF_PRG | BRF_ESS }, //  0 Main Z80 Code
	{ "a16-20.2",		0x2000, 0x60cdb6ae, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a16-21.3",		0x2000, 0x121bea03, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "a16-22.4",		0x2000, 0xb20e3027, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "a16-23.5",		0x2000, 0x95fe9cb7, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "a16-24.6",		0x2000, 0x32b5c155, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "a16-12.8",		0x2000, 0x4b9bff43, 2 | BRF_PRG | BRF_ESS }, //  6 Sound Z80 Code
	{ "a16-13.9",		0x2000, 0x355121b9, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "a16-14.10",		0x2000, 0x0f33ef4d, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "a16-15.37",		0x2000, 0xf91d63dc, 2 | BRF_PRG | BRF_ESS }, //  9
	{ "a16-16.38",		0x2000, 0x9395351b, 2 | BRF_PRG | BRF_ESS }, // 10
	{ "a16-17.39",		0x2000, 0x872270b3, 2 | BRF_PRG | BRF_ESS }, // 11

	{ "a16-06-1.7",		0x2000, 0xb708134d, 3 | BRF_GRA },           // 12 Tiles and Sprites
	{ "a16-07-2.8",		0x2000, 0xcdaf7f83, 3 | BRF_GRA },           // 13
	{ "a16-10.90",		0x2000, 0xe8e42454, 3 | BRF_GRA },           // 14
	{ "a16-11-1.91",	0x2000, 0x1f766661, 3 | BRF_GRA },           // 15
	{ "a16-04.5",		0x2000, 0xb2fae99f, 3 | BRF_GRA },           // 16
	{ "a16-05-1.6",		0x2000, 0x85dfbb6e, 3 | BRF_GRA },           // 17
	{ "a16-08.88",		0x2000, 0x1ddb6466, 3 | BRF_GRA },           // 18
	{ "a16-09-1.89",	0x2000, 0x23d4c43c, 3 | BRF_GRA },           // 19

	{ "a16-18.mcu",		0x0800, 0x00000000, 4 | BRF_NODUMP },        // 20 M68705 MCU Code
};

STD_ROM_PICK(victnine)
STD_ROM_FN(victnine)

static int victnineInit()
{
	select_game = 2;

	return DrvInit();
}

struct BurnDriver BurnDrvVictnine = {
	"victnine", NULL, NULL, "1984",
	"Victorious Nine\0", "Imperfect sound", "Taito", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, victnineRomInfo, victnineRomName, VictnineInputInfo, VictnineDIPInfo,
	victnineInit, DrvExit, DrvFrame, victnineDraw, DrvScan, &DrvRecalc,
	256, 224, 4, 3
};
