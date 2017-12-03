#include "gamestates/game/game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include <ace/utils/font.h>
#include <ace/generic/screen.h>
#include "main.h"
#include "gamestates/game/square.h"
#include "gamestates/game/map.h"
#include "gamestates/menu/menu.h"

static tView *s_pView;
static tVPort *s_pMainVPort, *s_pHudVPort;
tSimpleBufferManager *g_pMainBfrMgr;
static tSimpleBufferManager *s_pHudBfrMgr;
UBYTE g_ubGameOver;
UBYTE g_ubStartX, g_ubStartY; ///<- Start tile coords
UBYTE g_isExit;
UBYTE g_isHard = 0;
static tBitMap *s_pGameOverBitmap;
UBYTE g_ubCurrMap;
UWORD g_uwScore = 0;
UWORD g_uwHiScore, g_uwLoScore;

static UWORD s_uwPrevScore;
static UBYTE s_ubPrevSquareCount;
static UWORD s_uwPrevHiScore;

void gameGsGameOverLoop(void);

#define GAME_HUD_VPORT_HEIGHT (SCREEN_PAL_HEIGHT - GAME_MAIN_VPORT_HEIGHT)

void hudUpdate(void) {
	if(s_uwPrevScore != g_uwScore || s_ubPrevSquareCount != g_ubSquareCount) {
		char szScore[30];
		UBYTE ubScoreFromSquares = g_ubSquareCount << 1;
		sprintf(szScore, "Score %hu (+%hhu)", g_uwScore, ubScoreFromSquares);
		blitRect(s_pHudBfrMgr->pBuffer, 0, 0, 160, 5, 0);
		fontDrawStr(s_pHudBfrMgr->pBuffer, g_pFont, 0, 0, szScore, 1, FONT_COOKIE);
		s_uwPrevScore = g_uwScore;
		s_ubPrevSquareCount = g_ubSquareCount;
		if(g_uwScore > g_uwHiScore)
			g_uwHiScore = g_uwScore;
	}
	if(s_uwPrevHiScore != g_uwHiScore) {
		char szHiScore[20];
		sprintf(szHiScore, "hi-score: %hu", g_uwHiScore);
		blitRect(s_pHudBfrMgr->pBuffer, 200, 0, 100, 5, 0);
		UBYTE ubColor;
		if(g_uwScore == g_uwHiScore)
			ubColor = 5;
		else
			ubColor = 1;
		fontDrawStr(s_pHudBfrMgr->pBuffer, g_pFont, 200, 0, szHiScore, ubColor, FONT_COOKIE);
		s_uwPrevHiScore = g_uwHiScore;
	}
}

void loadLevel() {
	viewLoad(0);
	blitRect(
		g_pMainBfrMgr->pBuffer, 0, 0,
		bitmapGetByteWidth(g_pMainBfrMgr->pBuffer) << 3,
		g_pMainBfrMgr->pBuffer->Rows,
		0
	);

	squaresManagerClear();

	char szName[10];
	sprintf(szName, "map%hhu.txt", g_ubCurrMap);
	mapCreate(szName);
	mapDraw();
	g_ubGameOver = 0;
	g_isExit = 0;
	squareAdd(g_ubStartX << 3, g_ubStartY << 3);

	gameChangeLoop(gameGsLoop);

	viewLoad(s_pView);
}

void displayGameOver(void) {
	// if(g_uwScore > g_uwLoScore) {
	// 	gameChangeState(menuGsCreate, menuGsLoop, menuGsDestroy);
	// 	return;
	// }
	const UWORD uwWidth = 220;
	const UWORD uwHeight = 130;
	const UWORD uwStartX = (SCREEN_PAL_WIDTH - uwWidth)/2;
	const UWORD uwStartY = (SCREEN_PAL_HEIGHT - uwHeight)/2 - GAME_HUD_VPORT_HEIGHT;
	blitRect(g_pMainBfrMgr->pBuffer, uwStartX,  uwStartY, uwWidth, uwHeight, 1);
	blitRect(g_pMainBfrMgr->pBuffer, uwStartX +1,  uwStartY +1, uwWidth -2, uwHeight -2, 0);
	blitCopy(
		s_pGameOverBitmap, 0, 0, g_pMainBfrMgr->pBuffer,
		uwStartX + (uwWidth-208)/2, uwStartY + 2, 208, 105,
		MINTERM_COOKIE, 0xFF
	);
	fontDrawStr(
		g_pMainBfrMgr->pBuffer, g_pFont, SCREEN_PAL_WIDTH/2, uwStartY + 2 + 105 + 5,
		"'R' to retry, 'ESC' to quit",
		1, FONT_HCENTER | FONT_TOP | FONT_COOKIE
	);
	g_uwScore = 0;
	g_ubCurrMap = 0;
	gameChangeLoop(gameGsGameOverLoop);
}

void gameGsCreate(void) {
	logBlockBegin("gameGsCreate()");
	const UWORD pPalette[8] = {0x000, 0xFFF, 0x333, 0x900, 0x999, 0x0F0};
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
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pMainVPort,
	TAG_DONE);
	copBlockDisableSprites(s_pView->pCopList, 0xFF);

	randInit(2184);
	squaresManagerCreate();

	// Game over stuff
	s_pGameOverBitmap = bitmapCreateFromFile("data/gameover.bm");

	// HUD stuff
	s_uwPrevScore = 0xFFFF;
	s_ubPrevSquareCount = 0xFF;
	s_uwPrevHiScore = 0xFFFF;

	g_ubCurrMap = 0;
	g_uwScore = 0;
	loadLevel();

	viewLoad(s_pView);
	logBlockEnd("gameGsCreate()");
}

void gameGsGameOverLoop(void) {
	if(keyUse(KEY_R)) {
		loadLevel();
		return;
	}
	else if(keyUse(KEY_ESCAPE)) {
		gameChangeState(menuGsCreate, menuGsLoop, menuGsDestroy);
		return;
	}
	vPortWaitForEnd(s_pMainVPort);
}

void gameGsLoop(void) {
	if(keyUse(KEY_R)) {
		loadLevel();
		return;
	}
	else if(keyUse(KEY_ESCAPE)) {
		gameChangeState(menuGsCreate, menuGsLoop, menuGsDestroy);
		return;
	}
	else if(keyUse(KEY_N)) {
		// DEBUG
		squareAdd(
			g_pSquareFirst->sCoord.sUwCoord.uwX - 16 + uwRandMinMax(0, 32),
			g_pSquareFirst->sCoord.sUwCoord.uwY - 16 + uwRandMinMax(0, 32)
		);
		logWrite("new\n");
	}
	hudUpdate();

	vPortWaitForEnd(s_pMainVPort);
	squareProcessPlayer();
	squaresUndraw();
	squareProcessAi();

	if(g_ubGameOver) {
		displayGameOver();
		return;
	}
	else if(g_isExit) {
		g_uwScore += g_ubSquareCount << 1;
		++g_ubCurrMap;
		if(g_ubCurrMap >= MAP_COUNT) {
			gameChangeState(menuGsCreate, menuGsLoop, menuGsDestroy);
			return;
		}
		loadLevel();
		return;
	}

	squaresOrderForDraw();
	squaresDraw();
}

void gameGsDestroy(void) {
	viewDestroy(s_pView);

	bitmapDestroy(s_pGameOverBitmap);

	squaresManagerDestroy();
}
