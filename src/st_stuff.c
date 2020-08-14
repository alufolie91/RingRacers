// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2020 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  st_stuff.c
/// \brief Status bar code
///        Does the face/direction indicator animatin.
///        Does palette indicators as well (red pain/berserk, bright pickup)

#include "doomdef.h"
#include "g_game.h"
#include "r_local.h"
#include "p_local.h"
#include "f_finale.h"
#include "st_stuff.h"
#include "i_video.h"
#include "v_video.h"
#include "z_zone.h"
#include "hu_stuff.h"
#include "console.h"
#include "s_sound.h"
#include "i_system.h"
#include "m_menu.h"
#include "m_cheat.h"
#include "m_misc.h" // moviemode
#include "m_anigif.h" // cv_gif_downscale
#include "p_setup.h" // NiGHTS grading
#include "k_hud.h" // SRB2kart

//random index
#include "m_random.h"

// item finder
#include "m_cond.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "lua_hud.h"

UINT16 objectsdrawn = 0;

//
// STATUS BAR DATA
//

patch_t *facerankprefix[MAXSKINS]; // ranking
patch_t *facewantprefix[MAXSKINS]; // wanted
patch_t *facemmapprefix[MAXSKINS]; // minimap

// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
patch_t *sboscore; // Score logo
patch_t *sbotime; // Time logo
patch_t *sbocolon; // Colon for time
patch_t *sboperiod; // Period for time centiseconds
patch_t *livesback; // Lives icon background
patch_t *stlivex;
static patch_t *nrec_timer; // Timer for NiGHTS records
static patch_t *sborings;
static patch_t *slidgame;
static patch_t *slidtime;
static patch_t *slidover;
static patch_t *sboredrings;
static patch_t *sboredtime;
static patch_t *getall; // Special Stage HUD
static patch_t *timeup; // Special Stage HUD
static patch_t *hunthoming[6];
static patch_t *itemhoming[6];
static patch_t *race1;
static patch_t *race2;
static patch_t *race3;
static patch_t *racego;
//static patch_t *ttlnum;
static patch_t *nightslink;
static patch_t *curweapon;
static patch_t *normring;
static patch_t *bouncering;
static patch_t *infinityring;
static patch_t *autoring;
static patch_t *explosionring;
static patch_t *scatterring;
static patch_t *grenadering;
static patch_t *railring;
static patch_t *jumpshield;
static patch_t *forceshield;
static patch_t *ringshield;
static patch_t *watershield;
static patch_t *bombshield;
static patch_t *pityshield;
static patch_t *pinkshield;
static patch_t *flameshield;
static patch_t *bubbleshield;
static patch_t *thundershield;
static patch_t *invincibility;
static patch_t *sneakers;
static patch_t *gravboots;
static patch_t *nonicon;
static patch_t *nonicon2;
static patch_t *bluestat;
static patch_t *byelstat;
static patch_t *orngstat;
static patch_t *redstat;
static patch_t *yelstat;
static patch_t *nbracket;
static patch_t *nring;
static patch_t *nhud[12];
static patch_t *nsshud;
static patch_t *nbon[12];
static patch_t *nssbon;
static patch_t *narrow[9];
static patch_t *nredar[8]; // Red arrow
static patch_t *drillbar;
static patch_t *drillfill[3];
static patch_t *capsulebar;
static patch_t *capsulefill;
patch_t *ngradeletters[7];
static patch_t *minus5sec;
static patch_t *minicaps;
static patch_t *gotrflag;
static patch_t *gotbflag;
static patch_t *fnshico;

// Midnight Channel:
static patch_t *hud_tv1;
static patch_t *hud_tv2;

// SRB2kart

hudinfo_t hudinfo[NUMHUDITEMS] =
{
	{  16, 176, V_SNAPTOLEFT|V_SNAPTOBOTTOM}, // HUD_LIVES

	{  16,  42, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_RINGS
	{  96,  42, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_RINGSNUM
	{ 120,  42, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_RINGSNUMTICS

	{  16,  10, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_SCORE
	{ 120,  10, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_SCORENUM

	{  16,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_TIME
	{  72,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_MINUTES
	{  72,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_TIMECOLON
	{  96,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_SECONDS
	{  96,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_TIMETICCOLON
	{ 120,  26, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_TICS

	{   0,  56, V_SNAPTOLEFT|V_SNAPTOTOP}, // HUD_SS_TOTALRINGS

	{ 110,  93, 0}, // HUD_GETRINGS
	{ 160,  93, 0}, // HUD_GETRINGSNUM
	{ 124, 160, 0}, // HUD_TIMELEFT
	{ 168, 176, 0}, // HUD_TIMELEFTNUM
	{ 130,  93, 0}, // HUD_TIMEUP
	{ 152, 168, 0}, // HUD_HUNTPICS

	{ 288, 176, V_SNAPTORIGHT|V_SNAPTOBOTTOM}, // HUD_POWERUPS
};

//
// STATUS BAR CODE
//

boolean ST_SameTeam(player_t *a, player_t *b)
{
	// Spectator chat.
	if (a->spectator && b->spectator)
	{
		return true;
	}

	// Team chat.
	if (G_GametypeHasTeams() == true)
	{
		// You get team messages if you're on the same team.
		return (a->ctfteam == b->ctfteam);
	}
	else
	{
		// Not that everyone's not on the same team, but team messages go to normal chat if everyone's not in the same team.
		return true;
	}
}

static boolean st_stopped = true;

void ST_Ticker(boolean run)
{
	if (st_stopped)
		return;

	if (run)
		ST_runTitleCard();
}

// 0 is default, any others are special palettes.
INT32 st_palette = 0;
INT32 st_translucency = 10;

void ST_doPaletteStuff(void)
{
	INT32 palette;

	if (paused || P_AutoPause())
		palette = 0;
	else if (stplyr && stplyr->flashcount)
		palette = stplyr->flashpal;
	else
		palette = 0;

#ifdef HWRENDER
	if (rendermode == render_opengl)
		palette = 0; // No flashpals here in OpenGL
#endif

	palette = min(max(palette, 0), 13);

	if (palette != st_palette)
	{
		st_palette = palette;

		if (rendermode != render_none)
		{
			//V_SetPaletteLump(GetPalette()); // Reset the palette -- is this needed?
			if (!r_splitscreen)
				V_SetPalette(palette);
		}
	}
}

void ST_UnloadGraphics(void)
{
	Z_FreeTag(PU_HUDGFX);
}

void ST_LoadGraphics(void)
{
	int i;

	// SRB2 border patch
	// st_borderpatchnum = W_GetNumForName("GFZFLR01");
	// scr_borderpatch = W_CacheLumpNum(st_borderpatchnum, PU_HUDGFX);

	// the original Doom uses 'STF' as base name for all face graphics
	// Graue 04-08-2004: face/name graphics are now indexed by skins
	//                   but load them in R_AddSkins, that gets called
	//                   first anyway
	// cache the status bar overlay icons (fullscreen mode)

	// Prefix "STT" is whitelisted (doesn't trigger ISGAMEMODIFIED), btw
	sborings = W_CachePatchName("STTRINGS", PU_HUDGFX);
	sboredrings = W_CachePatchName("STTRRING", PU_HUDGFX);
	sboscore = W_CachePatchName("STTSCORE", PU_HUDGFX);
	sbotime = W_CachePatchName("STTTIME", PU_HUDGFX); // Time logo
	sboredtime = W_CachePatchName("STTRTIME", PU_HUDGFX);
	sbocolon = W_CachePatchName("STTCOLON", PU_HUDGFX); // Colon for time
	sboperiod = W_CachePatchName("STTPERIO", PU_HUDGFX); // Period for time centiseconds

	slidgame = W_CachePatchName("SLIDGAME", PU_HUDGFX);
	slidtime = W_CachePatchName("SLIDTIME", PU_HUDGFX);
	slidover = W_CachePatchName("SLIDOVER", PU_HUDGFX);

	stlivex = W_CachePatchName("STLIVEX", PU_HUDGFX);
	livesback = W_CachePatchName("STLIVEBK", PU_HUDGFX);
	nrec_timer = W_CachePatchName("NGRTIMER", PU_HUDGFX); // Timer for NiGHTS
	getall = W_CachePatchName("GETALL", PU_HUDGFX); // Special Stage HUD
	timeup = W_CachePatchName("TIMEUP", PU_HUDGFX); // Special Stage HUD
	race1 = W_CachePatchName("RACE1", PU_HUDGFX);
	race2 = W_CachePatchName("RACE2", PU_HUDGFX);
	race3 = W_CachePatchName("RACE3", PU_HUDGFX);
	racego = W_CachePatchName("RACEGO", PU_HUDGFX);
	nightslink = W_CachePatchName("NGHTLINK", PU_HUDGFX);

	for (i = 0; i < 6; ++i)
	{
		hunthoming[i] = W_CachePatchName(va("HOMING%d", i+1), PU_HUDGFX);
		itemhoming[i] = W_CachePatchName(va("HOMITM%d", i+1), PU_HUDGFX);
	}

	curweapon = W_CachePatchName("CURWEAP", PU_HUDGFX);
	normring = W_CachePatchName("RINGIND", PU_HUDGFX);
	bouncering = W_CachePatchName("BNCEIND", PU_HUDGFX);
	infinityring = W_CachePatchName("INFNIND", PU_HUDGFX);
	autoring = W_CachePatchName("AUTOIND", PU_HUDGFX);
	explosionring = W_CachePatchName("BOMBIND", PU_HUDGFX);
	scatterring = W_CachePatchName("SCATIND", PU_HUDGFX);
	grenadering = W_CachePatchName("GRENIND", PU_HUDGFX);
	railring = W_CachePatchName("RAILIND", PU_HUDGFX);
	jumpshield = W_CachePatchName("TVWWICON", PU_HUDGFX);
	forceshield = W_CachePatchName("TVFOICON", PU_HUDGFX);
	ringshield = W_CachePatchName("TVATICON", PU_HUDGFX);
	watershield = W_CachePatchName("TVELICON", PU_HUDGFX);
	bombshield = W_CachePatchName("TVARICON", PU_HUDGFX);
	pityshield = W_CachePatchName("TVPIICON", PU_HUDGFX);
	pinkshield = W_CachePatchName("TVPPICON", PU_HUDGFX);
	flameshield = W_CachePatchName("TVFLICON", PU_HUDGFX);
	bubbleshield = W_CachePatchName("TVBBICON", PU_HUDGFX);
	thundershield = W_CachePatchName("TVZPICON", PU_HUDGFX);
	invincibility = W_CachePatchName("TVIVICON", PU_HUDGFX);
	sneakers = W_CachePatchName("TVSSICON", PU_HUDGFX);
	gravboots = W_CachePatchName("TVGVICON", PU_HUDGFX);

	tagico = W_CachePatchName("TAGICO", PU_HUDGFX);
	rflagico = W_CachePatchName("RFLAGICO", PU_HUDGFX);
	bflagico = W_CachePatchName("BFLAGICO", PU_HUDGFX);
	rmatcico = W_CachePatchName("RMATCICO", PU_HUDGFX);
	bmatcico = W_CachePatchName("BMATCICO", PU_HUDGFX);
	gotrflag = W_CachePatchName("GOTRFLAG", PU_HUDGFX);
	gotbflag = W_CachePatchName("GOTBFLAG", PU_HUDGFX);
	fnshico = W_CachePatchName("FNSHICO", PU_HUDGFX);
	nonicon = W_CachePatchName("NONICON", PU_HUDGFX);
	nonicon2 = W_CachePatchName("NONICON2", PU_HUDGFX);

	// NiGHTS HUD things
	bluestat = W_CachePatchName("BLUESTAT", PU_HUDGFX);
	byelstat = W_CachePatchName("BYELSTAT", PU_HUDGFX);
	orngstat = W_CachePatchName("ORNGSTAT", PU_HUDGFX);
	redstat = W_CachePatchName("REDSTAT", PU_HUDGFX);
	yelstat = W_CachePatchName("YELSTAT", PU_HUDGFX);
	nbracket = W_CachePatchName("NBRACKET", PU_HUDGFX);
	nring = W_CachePatchName("NRNG1", PU_HUDGFX);
	for (i = 0; i < 12; ++i)
	{
		nhud[i] = W_CachePatchName(va("NHUD%d", i+1), PU_HUDGFX);
		nbon[i] = W_CachePatchName(va("NBON%d", i+1), PU_HUDGFX);
	}
	nsshud = W_CachePatchName("NSSHUD", PU_HUDGFX);
	nssbon = W_CachePatchName("NSSBON", PU_HUDGFX);
	minicaps = W_CachePatchName("MINICAPS", PU_HUDGFX);

	for (i = 0; i < 8; ++i)
	{
		narrow[i] = W_CachePatchName(va("NARROW%d", i+1), PU_HUDGFX);
		nredar[i] = W_CachePatchName(va("NREDAR%d", i+1), PU_HUDGFX);
	}

	// non-animated version
	narrow[8] = W_CachePatchName("NARROW9", PU_HUDGFX);

	drillbar = W_CachePatchName("DRILLBAR", PU_HUDGFX);
	for (i = 0; i < 3; ++i)
		drillfill[i] = W_CachePatchName(va("DRILLFI%d", i+1), PU_HUDGFX);
	capsulebar = W_CachePatchName("CAPSBAR", PU_HUDGFX);
	capsulefill = W_CachePatchName("CAPSFILL", PU_HUDGFX);
	minus5sec = W_CachePatchName("MINUS5", PU_HUDGFX);

	for (i = 0; i < 7; ++i)
		ngradeletters[i] = W_CachePatchName(va("GRADE%d", i), PU_HUDGFX);

	K_LoadKartHUDGraphics();

	// Midnight Channel:
	hud_tv1 = W_CachePatchName("HUD_TV1", PU_HUDGFX);
	hud_tv2 = W_CachePatchName("HUD_TV2", PU_HUDGFX);
}

// made separate so that skins code can reload custom face graphics
void ST_LoadFaceGraphics(char *rankstr, char *wantstr, char *mmapstr, INT32 skinnum)
{
	facerankprefix[skinnum] = W_CachePatchName(rankstr, PU_HUDGFX);
	facewantprefix[skinnum] = W_CachePatchName(wantstr, PU_HUDGFX);
	facemmapprefix[skinnum] = W_CachePatchName(mmapstr, PU_HUDGFX);
}

void ST_ReloadSkinFaceGraphics(void)
{
	INT32 i;

	for (i = 0; i < numskins; i++)
		ST_LoadFaceGraphics(skins[i].facerank, skins[i].facewant, skins[i].facemmap, i);
}

static inline void ST_InitData(void)
{
	// 'link' the statusbar display to a player, which could be
	// another player than consoleplayer, for example, when you
	// change the view in a multiplayer demo with F12.
	stplyr = &players[displayplayers[0]];

	st_palette = -1;
}

static inline void ST_Stop(void)
{
	if (st_stopped)
		return;

	V_SetPalette(0);

	st_stopped = true;
}

void ST_Start(void)
{
	if (!st_stopped)
		ST_Stop();

	ST_InitData();
	st_stopped = false;
}

//
// Initializes the status bar, sets the defaults border patch for the window borders.
//

// used by OpenGL mode, holds lumpnum of flat used to fill space around the viewwindow
lumpnum_t st_borderpatchnum;

void ST_Init(void)
{
	if (dedicated)
		return;

	ST_LoadGraphics();
}

// change the status bar too, when pressing F12 while viewing a demo.
void ST_changeDemoView(void)
{
	// the same routine is called at multiplayer deathmatch spawn
	// so it can be called multiple times
	ST_Start();
}

// =========================================================================
//                         STATUS BAR OVERLAY
// =========================================================================

boolean st_overlay;

/*
static INT32 SCZ(INT32 z)
{
	return FixedInt(FixedMul(z<<FRACBITS, vid.fdupy));
}
*/

/*
static INT32 SCY(INT32 y)
{
	//31/10/99: fixed by Hurdler so it _works_ also in hardware mode
	// do not scale to resolution for hardware accelerated
	// because these modes always scale by default
	y = SCZ(y); // scale to resolution
	if (splitscreen)
	{
		y >>= 1;
		if (stplyr != &players[displayplayers[0]])
			y += vid.height / 2;
	}
	return y;
}
*/

/*
static INT32 STRINGY(INT32 y)
{
	//31/10/99: fixed by Hurdler so it _works_ also in hardware mode
	// do not scale to resolution for hardware accelerated
	// because these modes always scale by default
	if (splitscreen)
	{
		y >>= 1;
		if (stplyr != &players[displayplayers[0]])
			y += BASEVIDHEIGHT / 2;
	}
	return y;
}
*/

/*
static INT32 SPLITFLAGS(INT32 f)
{
	// Pass this V_SNAPTO(TOP|BOTTOM) and it'll trim them to account for splitscreen! -Red
	if (splitscreen)
	{
		if (stplyr != &players[displayplayers[0]])
			f &= ~V_SNAPTOTOP;
		else
			f &= ~V_SNAPTOBOTTOM;
	}
	return f;
}
*/

/*
static INT32 SCX(INT32 x)
{
	return FixedInt(FixedMul(x<<FRACBITS, vid.fdupx));
}
*/

#if 0
static INT32 SCR(INT32 r)
{
	fixed_t y;
		//31/10/99: fixed by Hurdler so it _works_ also in hardware mode
	// do not scale to resolution for hardware accelerated
	// because these modes always scale by default
	y = FixedMul(r*FRACUNIT, vid.fdupy); // scale to resolution
	if (splitscreen)
	{
		y >>= 1;
		if (stplyr != &players[displayplayers[0]])
			y += vid.height / 2;
	}
	return FixedInt(FixedDiv(y, vid.fdupy));
}
#endif

// =========================================================================
//                          INTERNAL DRAWING
// =========================================================================
#define ST_DrawTopLeftOverlayPatch(x,y,p)         V_DrawScaledPatch(SCX(hudinfo[h+!!r_splitscreen].x), SCY(hudinfo[h+!!r_splitscreen].y), V_SNAPTOTOP|V_SNAPTOLEFT|V_HUDTRANS, p)
#define ST_DrawOverlayNum(x,y,n)           V_DrawTallNum(x, y, V_NOSCALESTART|V_HUDTRANS, n)
#define ST_DrawPaddedOverlayNum(x,y,n,d)   V_DrawPaddedTallNum(x, y, V_NOSCALESTART|V_HUDTRANS, n, d)
#define ST_DrawOverlayPatch(x,y,p)         V_DrawScaledPatch(x, y, V_NOSCALESTART|V_HUDTRANS, p)
#define ST_DrawMappedOverlayPatch(x,y,p,c) V_DrawMappedScaledPatch(x, y, V_NOSCALESTART|V_HUDTRANS, p, c)
#define ST_DrawNumFromHud(h,n)        V_DrawTallNum(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART|V_HUDTRANS, n)
#define ST_DrawPadNumFromHud(h,n,q)   V_DrawPaddedTallNum(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART|V_HUDTRANS, n, q)
#define ST_DrawPatchFromHud(h,p)      V_DrawScaledPatch(SCX(hudinfo[h].x), SCY(hudinfo[h].y), V_NOSCALESTART|V_HUDTRANS, p)
#define ST_DrawNumFromHudWS(h,n)      V_DrawTallNum(SCX(hudinfo[h+!!r_splitscreen].x), SCY(hudinfo[h+!!r_splitscreen].y), V_NOSCALESTART|V_HUDTRANS, n)
#define ST_DrawPadNumFromHudWS(h,n,q) V_DrawPaddedTallNum(SCX(hudinfo[h+!!r_splitscreen].x), SCY(hudinfo[h+!!r_splitscreen].y), V_NOSCALESTART|V_HUDTRANS, n, q)
#define ST_DrawPatchFromHudWS(h,p)    V_DrawScaledPatch(SCX(hudinfo[h+!!r_splitscreen].x), SCY(hudinfo[h+!!r_splitscreen].y), V_NOSCALESTART|V_HUDTRANS, p)

// Devmode information
static void ST_drawDebugInfo(void)
{
	INT32 height = 192;

	if (!stplyr->mo)
		return;

	if (cv_debug & DBG_BASIC)
	{
		const fixed_t d = AngleFixed(stplyr->mo->angle);
		V_DrawRightAlignedString(320, 168, V_MONOSPACE, va("X: %6d", stplyr->mo->x>>FRACBITS));
		V_DrawRightAlignedString(320, 176, V_MONOSPACE, va("Y: %6d", stplyr->mo->y>>FRACBITS));
		V_DrawRightAlignedString(320, 184, V_MONOSPACE, va("Z: %6d", stplyr->mo->z>>FRACBITS));
		V_DrawRightAlignedString(320, 192, V_MONOSPACE, va("A: %6d", FixedInt(d)));

		height = 152;
	}

	if (cv_debug & DBG_DETAILED)
	{
		//V_DrawRightAlignedString(320, height - 104, V_MONOSPACE, va("SHIELD: %5x", stplyr->powers[pw_shield]));
		V_DrawRightAlignedString(320, height - 96,  V_MONOSPACE, va("SCALE: %5d%%", (stplyr->mo->scale*100)/FRACUNIT));
		//V_DrawRightAlignedString(320, height - 88,  V_MONOSPACE, va("DASH: %3d/%3d", stplyr->dashspeed>>FRACBITS, FixedMul(stplyr->maxdash,stplyr->mo->scale)>>FRACBITS));
		//V_DrawRightAlignedString(320, height - 80,  V_MONOSPACE, va("AIR: %4d, %3d", stplyr->powers[pw_underwater], stplyr->powers[pw_spacetime]));

		// Flags
		//V_DrawRightAlignedString(304-64, height - 72, V_MONOSPACE, "Flags:");
		//V_DrawString(304-60,             height - 72, (stplyr->jumping) ? V_GREENMAP : V_REDMAP, "JM");
		//V_DrawString(304-40,             height - 72, (stplyr->pflags & PF_JUMPED) ? V_GREENMAP : V_REDMAP, "JD");
		//V_DrawString(304-20,             height - 72, (stplyr->pflags & PF_SPINNING) ? V_GREENMAP : V_REDMAP, "SP");
		//V_DrawString(304,                height - 72, (stplyr->pflags & PF_STARTDASH) ? V_GREENMAP : V_REDMAP, "ST");

		V_DrawRightAlignedString(320, height - 64, V_MONOSPACE, va("CEILZ: %6d", stplyr->mo->ceilingz>>FRACBITS));
		V_DrawRightAlignedString(320, height - 56, V_MONOSPACE, va("FLOORZ: %6d", stplyr->mo->floorz>>FRACBITS));

		V_DrawRightAlignedString(320, height - 48, V_MONOSPACE, va("CNVX: %6d", stplyr->cmomx>>FRACBITS));
		V_DrawRightAlignedString(320, height - 40, V_MONOSPACE, va("CNVY: %6d", stplyr->cmomy>>FRACBITS));
		V_DrawRightAlignedString(320, height - 32, V_MONOSPACE, va("PLTZ: %6d", stplyr->mo->pmomz>>FRACBITS));

		V_DrawRightAlignedString(320, height - 24, V_MONOSPACE, va("MOMX: %6d", stplyr->rmomx>>FRACBITS));
		V_DrawRightAlignedString(320, height - 16, V_MONOSPACE, va("MOMY: %6d", stplyr->rmomy>>FRACBITS));
		V_DrawRightAlignedString(320, height - 8,  V_MONOSPACE, va("MOMZ: %6d", stplyr->mo->momz>>FRACBITS));
		V_DrawRightAlignedString(320, height,      V_MONOSPACE, va("SPEED: %6d", stplyr->speed>>FRACBITS));

		height -= 120;
	}

	if (cv_debug & DBG_RANDOMIZER) // randomizer testing
	{
		fixed_t peekres = P_RandomPeek();
		peekres *= 10000;     // Change from fixed point
		peekres >>= FRACBITS; // to displayable decimal

		V_DrawRightAlignedString(320, height - 16, V_MONOSPACE, va("Init: %08x", P_GetInitSeed()));
		V_DrawRightAlignedString(320, height - 8,  V_MONOSPACE, va("Seed: %08x", P_GetRandSeed()));
		V_DrawRightAlignedString(320, height,      V_MONOSPACE, va("==  :    .%04d", peekres));

		height -= 32;
	}

	if (cv_debug & DBG_MEMORY)
		V_DrawRightAlignedString(320, height,     V_MONOSPACE, va("Heap used: %7sKB", sizeu1(Z_TagsUsage(0, INT32_MAX)>>10)));
}

static patch_t *lt_patches[3];
static INT32 lt_scroll = 0;
static INT32 lt_mom = 0;
static INT32 lt_zigzag = 0;

tic_t lt_ticker = 0, lt_lasttic = 0;
tic_t lt_exitticker = 0, lt_endtime = 0;

//
// Load the graphics for the title card.
// Don't let LJ see this
//
static void ST_cacheLevelTitle(void)
{
#define SETPATCH(default, warning, custom, idx) \
{ \
	lumpnum_t patlumpnum = LUMPERROR; \
	if (mapheaderinfo[gamemap-1]->custom[0] != '\0') \
	{ \
		patlumpnum = W_CheckNumForName(mapheaderinfo[gamemap-1]->custom); \
		if (patlumpnum != LUMPERROR) \
			lt_patches[idx] = (patch_t *)W_CachePatchNum(patlumpnum, PU_HUDGFX); \
	} \
	if (patlumpnum == LUMPERROR) \
	{ \
		if (!(mapheaderinfo[gamemap-1]->levelflags & LF_WARNINGTITLE)) \
			lt_patches[idx] = (patch_t *)W_CachePatchName(default, PU_HUDGFX); \
		else \
			lt_patches[idx] = (patch_t *)W_CachePatchName(warning, PU_HUDGFX); \
	} \
}

	SETPATCH("LTACTBLU", "LTACTRED", ltactdiamond, 0)
	SETPATCH("LTZIGZAG", "LTZIGRED", ltzzpatch, 1)
	SETPATCH("LTZZTEXT", "LTZZWARN", ltzztext, 2)

#undef SETPATCH
}

//
// Start the title card.
//
void ST_startTitleCard(void)
{
	// cache every HUD patch used
	ST_cacheLevelTitle();

	// initialize HUD variables
	lt_ticker = lt_exitticker = lt_lasttic = 0;
	lt_endtime = 2*TICRATE + (10*NEWTICRATERATIO);
	lt_scroll = BASEVIDWIDTH * FRACUNIT;
	lt_zigzag = -((lt_patches[1])->width * FRACUNIT);
	lt_mom = 0;
}

//
// What happens before drawing the title card.
// Which is just setting the HUD translucency.
//
void ST_preDrawTitleCard(void)
{
	if (!G_IsTitleCardAvailable())
		return;

	if (lt_ticker >= (lt_endtime + TICRATE))
		return;

	if (!lt_exitticker)
		st_translucency = 0;
	else
		st_translucency = max(0, min((INT32)lt_exitticker-4, cv_translucenthud.value));
}

//
// Run the title card.
// Called from ST_Ticker.
//
void ST_runTitleCard(void)
{
	boolean run = !(paused || P_AutoPause());

	if (!G_IsTitleCardAvailable())
		return;

	if (lt_ticker >= (lt_endtime + TICRATE))
		return;

	if (run || (lt_ticker < PRELEVELTIME))
	{
		// tick
		lt_ticker++;
		if (lt_ticker >= lt_endtime)
			lt_exitticker++;

		// scroll to screen (level title)
		if (!lt_exitticker)
		{
			if (abs(lt_scroll) > FRACUNIT)
				lt_scroll -= (lt_scroll>>2);
			else
				lt_scroll = 0;
		}
		// scroll away from screen (level title)
		else
		{
			lt_mom -= FRACUNIT*6;
			lt_scroll += lt_mom;
		}

		// scroll to screen (zigzag)
		if (!lt_exitticker)
		{
			if (abs(lt_zigzag) > FRACUNIT)
				lt_zigzag -= (lt_zigzag>>2);
			else
				lt_zigzag = 0;
		}
		// scroll away from screen (zigzag)
		else
			lt_zigzag += lt_mom;
	}
}

//
// Draw the title card itself.
//
void ST_drawTitleCard(void)
{
	char *lvlttl = mapheaderinfo[gamemap-1]->lvlttl;
	char *subttl = mapheaderinfo[gamemap-1]->subttl;
	UINT8 actnum = mapheaderinfo[gamemap-1]->actnum;
	INT32 lvlttlxpos, ttlnumxpos, zonexpos;
	INT32 subttlxpos = BASEVIDWIDTH/2;
	INT32 ttlscroll = FixedInt(lt_scroll);
	INT32 zzticker;
	patch_t *actpat, *zigzag, *zztext;
	UINT8 colornum;
	const UINT8 *colormap;

	if (players[g_localplayers[0]].skincolor)
		colornum = players[g_localplayers[0]].skincolor;
	else
		colornum = cv_playercolor[0].value;

	colormap = R_GetTranslationColormap(TC_DEFAULT, colornum, GTC_CACHE);

	if (!G_IsTitleCardAvailable())
		return;

	if (!LUA_HudEnabled(hud_stagetitle))
		goto luahook;

	if (lt_ticker >= (lt_endtime + TICRATE))
		goto luahook;

	if ((lt_ticker-lt_lasttic) > 1)
		lt_ticker = lt_lasttic+1;

	ST_cacheLevelTitle();
	actpat = lt_patches[0];
	zigzag = lt_patches[1];
	zztext = lt_patches[2];

	lvlttlxpos = ((BASEVIDWIDTH/2) - (V_LevelNameWidth(lvlttl)/2));

	if (actnum > 0)
		lvlttlxpos -= V_LevelActNumWidth(actnum);

	ttlnumxpos = lvlttlxpos + V_LevelNameWidth(lvlttl);
	zonexpos = ttlnumxpos - V_LevelNameWidth(M_GetText("Zone"));
	ttlnumxpos++;

	if (lvlttlxpos < 0)
		lvlttlxpos = 0;

	if (!splitscreen || (splitscreen && stplyr == &players[displayplayers[0]]))
	{
		zzticker = lt_ticker;
		V_DrawMappedPatch(FixedInt(lt_zigzag), (-zzticker) % zigzag->height, V_SNAPTOTOP|V_SNAPTOLEFT, zigzag, colormap);
		V_DrawMappedPatch(FixedInt(lt_zigzag), (zigzag->height-zzticker) % zigzag->height, V_SNAPTOTOP|V_SNAPTOLEFT, zigzag, colormap);
		V_DrawMappedPatch(FixedInt(lt_zigzag), (-zigzag->height+zzticker) % zztext->height, V_SNAPTOTOP|V_SNAPTOLEFT, zztext, colormap);
		V_DrawMappedPatch(FixedInt(lt_zigzag), (zzticker) % zztext->height, V_SNAPTOTOP|V_SNAPTOLEFT, zztext, colormap);
	}

	if (actnum)
	{
		if (!splitscreen)
		{
			if (actnum > 9) // slightly offset the act diamond for two-digit act numbers
				V_DrawMappedPatch(ttlnumxpos + (V_LevelActNumWidth(actnum)/4) + ttlscroll, 104 - ttlscroll, 0, actpat, colormap);
			else
				V_DrawMappedPatch(ttlnumxpos + ttlscroll, 104 - ttlscroll, 0, actpat, colormap);
		}
		V_DrawLevelActNum(ttlnumxpos + ttlscroll, 104, V_SPLITSCREEN, actnum);
	}

	V_DrawLevelTitle(lvlttlxpos - ttlscroll, 80, V_SPLITSCREEN, lvlttl);
	if (!(mapheaderinfo[gamemap-1]->levelflags & LF_NOZONE))
		V_DrawLevelTitle(zonexpos + ttlscroll, 104, V_SPLITSCREEN, M_GetText("Zone"));
	V_DrawCenteredString(subttlxpos - ttlscroll, 135, V_SPLITSCREEN|V_ALLOWLOWERCASE, subttl);

	lt_lasttic = lt_ticker;

luahook:
	LUAh_TitleCardHUD(stplyr);
}

//
// Drawer for G_PreLevelTitleCard.
//
void ST_preLevelTitleCardDrawer(void)
{
	V_DrawFill(0, 0, BASEVIDWIDTH, BASEVIDHEIGHT, levelfadecol);
	ST_drawWipeTitleCard();
	I_OsPolling();
	I_UpdateNoBlit();
}

//
// Draw the title card while on a wipe.
// Also used in G_PreLevelTitleCard.
//
void ST_drawWipeTitleCard(void)
{
	UINT8 i;

	for (i = 0; i <= r_splitscreen; i++)
	{
		stplyr = &players[displayplayers[i]];
		ST_preDrawTitleCard();
		ST_drawTitleCard();
	}
}

//
// Draw the status bar overlay, customisable: the user chooses which
// kind of information to overlay
//
static void ST_overlayDrawer(void)
{
	// Decide whether to draw the stage title or not
	boolean stagetitle = false;

	// Check for a valid level title
	// If the HUD is enabled
	// And, if Lua is running, if the HUD library has the stage title enabled
	if (G_IsTitleCardAvailable() && *mapheaderinfo[gamemap-1]->lvlttl != '\0' && !(hu_showscores && (netgame || multiplayer)))
	{
		stagetitle = true;
		ST_preDrawTitleCard();
	}

	// hu_showscores = auto hide score/time/rings when tab rankings are shown
	if (!(hu_showscores && (netgame || multiplayer)))
		K_drawKartHUD();

	if (!hu_showscores) // hide the following if TAB is held
	{
		if (cv_showviewpointtext.value)
		{
			if (!(multiplayer && demo.playback))
			{
				if(!P_IsLocalPlayer(stplyr))
				{
					/*char name[MAXPLAYERNAME+1];
					// shorten the name if its more than twelve characters.
					strlcpy(name, player_names[stplyr-players], 13);*/

					// Show name of player being displayed
					V_DrawCenteredString((BASEVIDWIDTH/2), BASEVIDHEIGHT-40, 0, M_GetText("VIEWPOINT:"));
					V_DrawCenteredString((BASEVIDWIDTH/2), BASEVIDHEIGHT-32, V_ALLOWLOWERCASE, player_names[stplyr-players]);
				}
			}
			else if (!demo.title)
			{
				if (!r_splitscreen)
				{
					V_DrawCenteredString((BASEVIDWIDTH/2), BASEVIDHEIGHT-40, V_HUDTRANSHALF, M_GetText("VIEWPOINT:"));
					V_DrawCenteredString((BASEVIDWIDTH/2), BASEVIDHEIGHT-32, V_HUDTRANSHALF|V_ALLOWLOWERCASE, player_names[stplyr-players]);
				}
				else if (r_splitscreen == 1)
				{
					char name[MAXPLAYERNAME+12];

					INT32 y = (stplyr == &players[displayplayers[0]]) ? 4 : BASEVIDHEIGHT/2-12;
					sprintf(name, "VIEWPOINT: %s", player_names[stplyr-players]);
					V_DrawRightAlignedThinString(BASEVIDWIDTH-40, y, V_HUDTRANSHALF|V_ALLOWLOWERCASE|V_SNAPTOTOP|V_SNAPTOBOTTOM|V_SNAPTORIGHT|V_SPLITSCREEN, name);
				}
				else if (r_splitscreen)
				{
					V_DrawCenteredThinString((vid.width/vid.dupx)/4, BASEVIDHEIGHT/2 - 12, V_HUDTRANSHALF|V_ALLOWLOWERCASE|V_SNAPTOBOTTOM|V_SNAPTOLEFT|V_SPLITSCREEN, player_names[stplyr-players]);
				}
			}
		}
	}
	else if (!(netgame || multiplayer) && cv_powerupdisplay.value == 2)
		ST_drawPowerupHUD(); // same as it ever was...

	if (!(netgame || multiplayer) || !hu_showscores)
		LUAh_GameHUD(stplyr);

	// draw level title Tails
	if (*mapheaderinfo[gamemap-1]->lvlttl != '\0' && !(hu_showscores && (netgame || multiplayer) && !mapreset)
		&& LUA_HudEnabled(hud_stagetitle)
	)
		ST_drawLevelTitle();

	if (!hu_showscores && netgame && !mapreset)
	{
		if (stplyr->spectator && LUA_HudEnabled(hud_textspectator))
		{
			const char *itemtxt = M_GetText("Item - Join Game");

			if (stplyr->powers[pw_flashing])
				itemtxt = M_GetText("Item - . . .");
			else if (stplyr->pflags & PF_WANTSTOJOIN)
				itemtxt = M_GetText("Item - Cancel Join");
			else if (G_GametypeHasTeams())
				itemtxt = M_GetText("Item - Join Team");

			if (cv_ingamecap.value)
			{
				UINT8 numingame = 0;
				UINT8 i;

				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i] && !players[i].spectator)
						numingame++;

				itemtxt = va("%s (%s: %d)", itemtxt, M_GetText("Slots left"), max(0, cv_ingamecap.value - numingame));
			}

			// SRB2kart: changed positions & text
			if (r_splitscreen)
			{
				V_DrawThinString(2, (BASEVIDHEIGHT/2)-20, V_YELLOWMAP|V_HUDTRANSHALF|V_SPLITSCREEN, M_GetText("- SPECTATING -"));
				V_DrawThinString(2, (BASEVIDHEIGHT/2)-10, V_HUDTRANSHALF|V_SPLITSCREEN, itemtxt);
			}
			else
			{
				V_DrawString(2, BASEVIDHEIGHT-40, V_HUDTRANSHALF|V_SPLITSCREEN|V_YELLOWMAP, M_GetText("- SPECTATING -"));
				V_DrawString(2, BASEVIDHEIGHT-30, V_HUDTRANSHALF|V_SPLITSCREEN, itemtxt);
				V_DrawString(2, BASEVIDHEIGHT-20, V_HUDTRANSHALF|V_SPLITSCREEN, M_GetText("Accelerate - Float"));
				V_DrawString(2, BASEVIDHEIGHT-10, V_HUDTRANSHALF|V_SPLITSCREEN, M_GetText("Brake - Sink"));
			}
		}
	}

	// Replay manual-save stuff
	if (demo.recording && multiplayer && demo.savebutton && demo.savebutton + 3*TICRATE < leveltime)
	{
		switch (demo.savemode)
		{
		case DSM_NOTSAVING:
			V_DrawRightAlignedThinString(BASEVIDWIDTH - 2, 2, V_HUDTRANS|V_SNAPTOTOP|V_SNAPTORIGHT|V_ALLOWLOWERCASE|((gametyperules & GTR_BUMPERS) ? V_REDMAP : V_SKYMAP), "Look Backward: Save replay");
			break;

		case DSM_WILLAUTOSAVE:
			V_DrawRightAlignedThinString(BASEVIDWIDTH - 2, 2, V_HUDTRANS|V_SNAPTOTOP|V_SNAPTORIGHT|V_ALLOWLOWERCASE|((gametyperules & GTR_BUMPERS) ? V_REDMAP : V_SKYMAP), "Replay will be saved. (Look Backward: Change title)");
			break;

		case DSM_WILLSAVE:
			V_DrawRightAlignedThinString(BASEVIDWIDTH - 2, 2, V_HUDTRANS|V_SNAPTOTOP|V_SNAPTORIGHT|V_ALLOWLOWERCASE|((gametyperules & GTR_BUMPERS) ? V_REDMAP : V_SKYMAP), "Replay will be saved.");
			break;

		case DSM_TITLEENTRY:
			ST_DrawDemoTitleEntry();
			break;

		default: // Don't render anything
			break;
		}
	}
}

void ST_DrawDemoTitleEntry(void)
{
	static UINT8 skullAnimCounter = 0;
	char *nametodraw;

	skullAnimCounter++;
	skullAnimCounter %= 8;

	nametodraw = demo.titlename;
	while (V_StringWidth(nametodraw, 0) > MAXSTRINGLENGTH*8 - 8)
		nametodraw++;

#define x (BASEVIDWIDTH/2 - 139)
#define y (BASEVIDHEIGHT/2)
	M_DrawTextBox(x, y + 4, MAXSTRINGLENGTH, 1);
	V_DrawString(x + 8, y + 12, V_ALLOWLOWERCASE, nametodraw);
	if (skullAnimCounter < 4)
		V_DrawCharacter(x + 8 + V_StringWidth(nametodraw, 0), y + 12,
			'_' | 0x80, false);

	M_DrawTextBox(x + 30, y - 24, 26, 1);
	V_DrawString(x + 38, y - 16, V_ALLOWLOWERCASE, "Enter the name of the replay.");

	M_DrawTextBox(x + 50, y + 20, 20, 1);
	V_DrawThinString(x + 58, y + 28, V_ALLOWLOWERCASE, "Escape - Cancel");
	V_DrawRightAlignedThinString(x + 220, y + 28, V_ALLOWLOWERCASE, "Enter - Confirm");
#undef x
#undef y
}

// MayonakaStatic: draw Midnight Channel's TV-like borders
static void ST_MayonakaStatic(void)
{
	INT32 flag = (leveltime%2) ? V_90TRANS : V_70TRANS;

	V_DrawFixedPatch(0, 0, FRACUNIT, V_SNAPTOTOP|V_SNAPTOLEFT|flag, hud_tv1, NULL);
	V_DrawFixedPatch(320<<FRACBITS, 0, FRACUNIT, V_SNAPTOTOP|V_SNAPTORIGHT|V_FLIP|flag, hud_tv1, NULL);
	V_DrawFixedPatch(0, 142<<FRACBITS, FRACUNIT, V_SNAPTOBOTTOM|V_SNAPTOLEFT|flag, hud_tv2, NULL);
	V_DrawFixedPatch(320<<FRACBITS, 142<<FRACBITS, FRACUNIT, V_SNAPTOBOTTOM|V_SNAPTORIGHT|V_FLIP|flag, hud_tv2, NULL);
}

void ST_Drawer(void)
{
	if (needpatchrecache)
		R_ReloadHUDGraphics();

#ifdef SEENAMES
	if (cv_seenames.value && cv_allowseenames.value && displayplayers[0] == consoleplayer && seenplayer && seenplayer->mo)
	{
		INT32 c = 0;
		switch (cv_seenames.value)
		{
			case 1: // Colorless
				break;
			case 2: // Team
				if (G_GametypeHasTeams())
					c = (seenplayer->ctfteam == 1) ? V_REDMAP : V_BLUEMAP;
				break;
			case 3: // Ally/Foe
			default:
				// Green = Ally, Red = Foe
				if (G_GametypeHasTeams())
					c = (players[consoleplayer].ctfteam == seenplayer->ctfteam) ? V_GREENMAP : V_REDMAP;
				else // Everyone is an ally, or everyone is a foe!
					c = (G_RingSlingerGametype()) ? V_REDMAP : V_GREENMAP;
				break;
		}

		V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2 + 15, V_HUDTRANSHALF|c, player_names[seenplayer-players]);
	}
#endif

	// Doom's status bar only updated if necessary.
	// However, ours updates every frame regardless, so the "refresh" param was removed
	//(void)refresh;

	// force a set of the palette by using doPaletteStuff()
	if (vid.recalc)
		st_palette = -1;

	// Do red-/gold-shifts from damage/items
#ifdef HWRENDER
	//25/08/99: Hurdler: palette changes is done for all players,
	//                   not only player1! That's why this part
	//                   of code is moved somewhere else.
	if (rendermode == render_soft)
#endif
		if (rendermode != render_none) ST_doPaletteStuff();

	st_translucency = cv_translucenthud.value;

	if (st_overlay)
	{
		// No deadview!
		for (i = 0; i <= r_splitscreen; i++)
		{
			stplyr = &players[displayplayers[i]];
			ST_overlayDrawer();
		}

		// draw Midnight Channel's overlay ontop
		if (mapheaderinfo[gamemap-1]->typeoflevel & TOL_TV)	// Very specific Midnight Channel stuff.
			ST_MayonakaStatic();
	}

	// Draw a fade on level opening
	if (timeinmap < 16)
		V_DrawCustomFadeScreen(((levelfadecol == 0) ? "FADEMAP1" : "FADEMAP0"), 32-(timeinmap*2)); // Then gradually fade out from there

	ST_drawDebugInfo();
}
