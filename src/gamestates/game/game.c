#include "gamestates/game/game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include <ace/generic/screen.h>
#include "gamestates/game/square.h"

static tView *s_pView;
static tVPort *s_pMainVPort, *s_pHudVPort;
tSimpleBufferManager *g_pMainBfrMgr;
static tSimpleBufferManager *s_pHudBfrMgr;

#define GAME_MAIN_VPORT_HEIGHT 224
#define GAME_HUD_VPORT_HEIGHT (SCREEN_PAL_HEIGHT - GAME_MAIN_VPORT_HEIGHT)

fix16_t g_pSin[256];

void gameGsCreate(void) {
	logBlockBegin("gameGsCreate()");
	const UWORD pPalette[8] = {0x000, 0xFFF, 0x333};
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);
	s_pHudVPort = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 3,
		TAG_VPORT_PALETTE_PTR, pPalette,
		TAG_VPORT_PALETTE_SIZE, 8,
		TAG_VPORT_HEIGHT, GAME_HUD_VPORT_HEIGHT,
	TAG_DONE);
	s_pMainVPort = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 3,
		TAG_VPORT_PALETTE_PTR, pPalette,
		TAG_VPORT_PALETTE_SIZE, 8,
		TAG_VPORT_HEIGHT, GAME_MAIN_VPORT_HEIGHT,
	TAG_DONE);
	s_pHudBfrMgr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pHudVPort,
	TAG_DONE);
	g_pMainBfrMgr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pMainVPort,
	TAG_DONE);
	copBlockDisableSprites(s_pView->pCopList, 0xFF);

	// Generate sine table
	// Precision is too small to multiply pi by 255, later to be divided by 128
	// so here's multiplying pi/2 by 255 and divided by 64
	fix16_t fHalfPi = fix16_div(fix16_pi, fix16_from_int(2));
	for(UWORD i = 0; i < 256; ++i)
		g_pSin[i] = fix16_sin(fix16_div(fHalfPi * i, fix16_from_int(64)));

	randInit(2184);
	squaresManagerCreate();
	squareAdd(100, 100);

	// Hud
	blitRect(
		s_pHudBfrMgr->pBuffer, 0, s_pHudBfrMgr->pBuffer->Rows-1,
		SCREEN_PAL_WIDTH, 1, 2
	);

	viewLoad(s_pView);
	logBlockEnd("gameGsCreate()");
}

void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}
	if(keyUse(KEY_N)) {
		squareAdd(160, 160);
		logWrite("new\n");
	}

	squareProcessPlayer();
	squareProcessAi();
	squaresOrderForDraw();
	vPortWaitForEnd(s_pMainVPort);
	squaresUndraw();
	squaresDraw();
}

void gameGsDestroy(void) {
	viewDestroy(s_pView);

	squaresManagerDestroy();
}
