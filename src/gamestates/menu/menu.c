#include "gamestates/menu/menu.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/utils/font.h>
#include <ace/generic/screen.h>
#include <fixmath/fixmath.h>
#include "maths.h"
#include "gamestates/game/game.h"

static tView *s_pView;
static tVPort *s_pVPort;
static tSimpleBufferManager *s_pBfrMgr;

static tBitMap *s_pLetters[7];

static tFont *s_pFont;
static UBYTE s_ubMenuPos, s_ubPrevMenuPos;

#define MENU_ITEM_COUNT 4

void vPortWaitForPos(tVPort *pVPort, UWORD uwLine) {
#ifdef AMIGA
	UWORD uwEndPos;
	UWORD uwCurrFrame;

	// Determine VPort end position
	uwEndPos = pVPort->uwOffsY + uwLine + 0x2C; // Addition from DiWStrt
	if(vhPosRegs->uwPosY < uwEndPos) {
		// If current beam is before pos, wait for pos @ current frame
		while(vhPosRegs->uwPosY < uwEndPos);
	}
	else {
		uwCurrFrame = g_sTimerManager.uwFrameCounter;
		while(
			vhPosRegs->uwPosY < uwEndPos ||
			g_sTimerManager.uwFrameCounter == uwCurrFrame
		);
	}

	// Otherwise wait for pos @ next frame
#endif // AMIGA
}

char *s_pEntryTxts[MENU_ITEM_COUNT] = {"Easy", "Nightmare", "Scores", "Enough"};

void drawMenu(void) {
	for(UBYTE i = 0; i < MENU_ITEM_COUNT; ++i) {
		fontDrawStr(
			s_pBfrMgr->pBuffer, s_pFont,
			SCREEN_PAL_WIDTH>>1, (SCREEN_PAL_HEIGHT>>1) + 10*i,
			s_pEntryTxts[i], 2, FONT_CENTER
		);
	}
}

void updateMenuPos(void) {
	const UBYTE pEntryColors[MENU_ITEM_COUNT] = {1, 3, 1, 1};

	fontDrawStr(
		s_pBfrMgr->pBuffer, s_pFont,
		SCREEN_PAL_WIDTH>>1, (SCREEN_PAL_HEIGHT>>1) + 10*s_ubPrevMenuPos,
		s_pEntryTxts[s_ubPrevMenuPos], 2, FONT_CENTER
	);

	fontDrawStr(
		s_pBfrMgr->pBuffer, s_pFont,
		SCREEN_PAL_WIDTH>>1, (SCREEN_PAL_HEIGHT>>1) + 10*s_ubMenuPos,
		s_pEntryTxts[s_ubMenuPos], pEntryColors[s_ubMenuPos], FONT_CENTER
	);

	s_ubPrevMenuPos = s_ubMenuPos;
}

void menuGsCreate(void) {
	viewLoad(0);
	const UWORD pPalette[8] = {0x000, 0xFFF, 0x333, 0xF00, 0x999, 0x0F0};
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);
	s_pVPort = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 3,
		TAG_VPORT_PALETTE_PTR, pPalette,
		TAG_VPORT_PALETTE_SIZE, 8,
	TAG_DONE);
	s_pBfrMgr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pVPort,
	TAG_DONE);
	copBlockDisableSprites(s_pView->pCopList, 0xFF);

	s_pLetters[0] = bitmapCreateFromFile("data/logo/s.bm");
	s_pLetters[1] = bitmapCreateFromFile("data/logo/q.bm");
	s_pLetters[2] = bitmapCreateFromFile("data/logo/u.bm");
	s_pLetters[3] = bitmapCreateFromFile("data/logo/a.bm");
	s_pLetters[4] = bitmapCreateFromFile("data/logo/r.bm");
	s_pLetters[5] = bitmapCreateFromFile("data/logo/e.bm");
	s_pLetters[6] = bitmapCreateFromFile("data/logo/d.bm");

	s_pFont = fontCreate("data/silkscreen5.fnt");

	s_ubMenuPos = 0;
	s_ubPrevMenuPos = 0;
	drawMenu();
	updateMenuPos();

	viewLoad(s_pView);
}

void menuGsLoop(void) {
	static UBYTE ubTime = 0;
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}

	vPortWaitForPos(s_pVPort, 100);
	const fix16_t fFifteen = fix16_from_int(15);
	for(UBYTE i = 0; i != 7; ++i) {
		fix16_t fY = fFifteen + fix16_mul(fFifteen,cSin((2*ubTime + i*16) & 0xFF));
		blitCopy(
			s_pLetters[i], 0, 0,
			s_pBfrMgr->pBuffer, 20 + 40*i, fix16_to_int(fY),
			40, 42, MINTERM_COOKIE, 0xFF
		);
	}

	if(keyUse(KEY_W) || joyUse(JOY1_UP)) {
		if(s_ubMenuPos) {
			--s_ubMenuPos;
			updateMenuPos();
		}
	}
	else if(keyUse(KEY_S) || joyUse(JOY1_DOWN)) {
		if(s_ubMenuPos < MENU_ITEM_COUNT-1) {
			++s_ubMenuPos;
			updateMenuPos();
		}
	}
	else if(keyUse(KEY_RETURN) || keyUse(KEY_NUMENTER) || keyUse(KEY_SPACE) || joyUse(JOY1_FIRE)) {
		switch(s_ubMenuPos) {
			case 0:
				g_isHard = 0;
				gameChangeState(gameGsCreate, gameGsLoop, gameGsDestroy);
				return;
			case 1:
				g_isHard = 1;
				gameChangeState(gameGsCreate, gameGsLoop, gameGsDestroy);
				return;
			case 2:
				// gameChangeState(scoreGsCreate, scoreGsLoop, scoreGsDestroy);
			case 3:
				gameClose();
				return;
		}
	}

	++ubTime;
}

void menuGsDestroy(void) {
	viewDestroy(s_pView);

	fontDestroy(s_pFont);

	for(UBYTE i = 0; i != 7; ++i)
		bitmapDestroy(s_pLetters[i]);
}
