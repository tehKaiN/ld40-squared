#include "gamestates/game/game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include <ace/utils/font.h>
#include <ace/generic/screen.h>
#include "gamestates/game/square.h"
#include "gamestates/game/map.h"

static tView *s_pView;
static tVPort *s_pMainVPort, *s_pHudVPort;
tSimpleBufferManager *g_pMainBfrMgr;
static tSimpleBufferManager *s_pHudBfrMgr;
UBYTE g_ubGameOver;
UBYTE g_ubStartX, g_ubStartY; ///<- Start tile coords
UBYTE g_isExit;
UBYTE g_isHard = 0;
tBitMap *s_pGameOverBitmap;
tFont *s_pFont;
UBYTE g_ubCurrMap;
UWORD g_uwScore, s_uwHiScore = 0;

#define GAME_HUD_VPORT_HEIGHT (SCREEN_PAL_HEIGHT - GAME_MAIN_VPORT_HEIGHT)

fix16_t g_pSin[256];

void hudUpdate(void) {
	static UWORD uwPrevScore = 0xFFFF;
	static UBYTE ubPrevSquareCount = 0xFF;
	static UWORD uwPrevHiScore = 0xFFFF;

	if(uwPrevScore != g_uwScore || ubPrevSquareCount != g_ubSquareCount) {
		char szScore[30];
		UBYTE ubScoreFromSquares = g_ubSquareCount << 1;
		sprintf(szScore, "Score %hu (+%hhu)", g_uwScore, ubScoreFromSquares);
		blitRect(s_pHudBfrMgr->pBuffer, 0, 0, 160, 5, 0);
		fontDrawStr(s_pHudBfrMgr->pBuffer, s_pFont, 0, 0, szScore, 1, FONT_COOKIE);
		uwPrevScore = g_uwScore;
		ubPrevSquareCount = g_ubSquareCount;
	}
	if(uwPrevHiScore != s_uwHiScore) {
		char szHiScore[20];
		sprintf(szHiScore, "hi-score: %hu", s_uwHiScore);
		blitRect(s_pHudBfrMgr->pBuffer, 200, 0, 100, 5, 0);
		fontDrawStr(s_pHudBfrMgr->pBuffer, s_pFont, 200, 0, szHiScore, 1, FONT_COOKIE);
		uwPrevHiScore = s_uwHiScore;
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
		g_pMainBfrMgr->pBuffer, s_pFont, SCREEN_PAL_WIDTH/2, uwStartY + 2 + 105 + 5,
		"'R' to retry, 'ESC' to quit",
		1, FONT_HCENTER | FONT_TOP | FONT_COOKIE
	);
	g_uwScore = 0;
	g_ubCurrMap = 0;
}

void gameGsCreate(void) {
	logBlockBegin("gameGsCreate()");
	const UWORD pPalette[8] = {0x000, 0xFFF, 0x333, 0xF00, 0x999, 0x0F0};
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

	// Read sine table
	// Generated using libfixmath with floats enabled, dumped to file
	FILE *pSinFile = fopen("data/sin.dat", "rb");
	for(UWORD i = 0; i < 256; ++i) {
		fread(&g_pSin[i], sizeof(fix16_t), 1, pSinFile);
	}
	fclose(pSinFile);

	randInit(2184);
	squaresManagerCreate();

	// Hud
	blitRect(
		s_pHudBfrMgr->pBuffer, 0, s_pHudBfrMgr->pBuffer->Rows-1,
		SCREEN_PAL_WIDTH, 1, 2
	);

	// Game over stuff
	s_pFont = fontCreate("data/silkscreen5.fnt");
	s_pGameOverBitmap = bitmapCreateFromFile("data/gameover.bm");

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
		gameClose();
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
		gameClose();
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
		gameChangeLoop(gameGsGameOverLoop);
		return;
	}
	else if(g_isExit) {
		++g_ubCurrMap;
		if(g_ubCurrMap >= MAP_COUNT)
			g_ubCurrMap = 0;
		g_uwScore += g_ubSquareCount << 1;
		loadLevel();
		return;
	}

	squaresOrderForDraw();
	squaresDraw();
}

void gameGsDestroy(void) {
	viewDestroy(s_pView);

	bitmapDestroy(s_pGameOverBitmap);
	fontDestroy(s_pFont);

	squaresManagerDestroy();
}
