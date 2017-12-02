#include <gamestates/game/game.h>
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include <ace/generic/screen.h>

static tView *s_pView;
static tVPort *s_pMainVPort, *s_pHudVPort;
static tSimpleBufferManager *s_pMainBfrMgr, *s_pHudBfrMgr;

#define GAME_MAIN_VPORT_HEIGHT 224
#define GAME_HUD_VPORT_HEIGHT (SCREEN_PAL_HEIGHT - GAME_MAIN_VPORT_HEIGHT)

#define ENEMY_MAX_X (320-8)
#define ENEMY_MAX_Y (GAME_MAIN_VPORT_HEIGHT-8)
#define ENEMY_COUNT 30

typedef struct _tEnemy {
	struct _tEnemy *pNext;
	tUwCoordYX sCoord;
	BYTE bDirX;
	BYTE bDirY;
	UBYTE ubSpeed;
} tEnemy;

tEnemy *s_pEnemies[ENEMY_COUNT];
tEnemy *s_pFirstEnemy = 0;

tEnemy *enemyCreate(void) {
	tEnemy *pEnemy = memAllocFast(sizeof(tEnemy));
	pEnemy->sCoord.sUwCoord.uwX = uwRandMax(ENEMY_MAX_X);
	pEnemy->sCoord.sUwCoord.uwY = uwRandMax(ENEMY_MAX_Y);
	pEnemy->bDirX = (uwRand() > (0xFFFF>>1) ? 1 : -1);
	pEnemy->bDirY = (uwRand() > (0xFFFF>>1) ? 1 : -1);
	pEnemy->ubSpeed = 1;
	return pEnemy;
}

void enemyDestroy(tEnemy *pEnemy) {
	memFree(pEnemy, sizeof(tEnemy));
}

void enemyProcess(tEnemy *pEnemy) {
	if(pEnemy->bDirX > 0) {
		if(pEnemy->sCoord.sUwCoord.uwX + pEnemy->ubSpeed >= ENEMY_MAX_X) {
			pEnemy->sCoord.sUwCoord.uwX = ENEMY_MAX_X;
			pEnemy->bDirX = -1;
		}
		else
			pEnemy->sCoord.sUwCoord.uwX += pEnemy->ubSpeed;
	}
	else {
		if(pEnemy->sCoord.sUwCoord.uwX - pEnemy->ubSpeed <= 0) {
			pEnemy->sCoord.sUwCoord.uwX = 0;
			pEnemy->bDirX = 1;
		}
		else
			pEnemy->sCoord.sUwCoord.uwX -= pEnemy->ubSpeed;
	}

	if(pEnemy->bDirY > 0) {
		if(pEnemy->sCoord.sUwCoord.uwY + pEnemy->ubSpeed >= ENEMY_MAX_Y) {
			pEnemy->sCoord.sUwCoord.uwY = ENEMY_MAX_Y;
			pEnemy->bDirY = -1;
		}
		else
			pEnemy->sCoord.sUwCoord.uwY += pEnemy->ubSpeed;
	}
	else {
		if(pEnemy->sCoord.sUwCoord.uwY - pEnemy->ubSpeed <= 0) {
			pEnemy->sCoord.sUwCoord.uwY = 0;
			pEnemy->bDirY = 1;
		}
		else
			pEnemy->sCoord.sUwCoord.uwY -= pEnemy->ubSpeed;
	}

	if(!s_pFirstEnemy) {
		s_pFirstEnemy = pEnemy;
		return;
	}

	tEnemy *pPrevEnemy = 0;
	tEnemy *pOtherEnemy = s_pFirstEnemy;
	while(pOtherEnemy->sCoord.ulYX < pEnemy->sCoord.ulYX) {
		if(!pOtherEnemy->pNext) {
			pOtherEnemy->pNext = pEnemy;
			return;
		}
		else {
			pPrevEnemy = pOtherEnemy;
			pOtherEnemy = pOtherEnemy->pNext;
		}
	}
	if(!pPrevEnemy) {
		pEnemy->pNext = s_pFirstEnemy;
		s_pFirstEnemy = pEnemy;
	}
	else {
		pEnemy->pNext = pPrevEnemy->pNext;
		pPrevEnemy->pNext = pEnemy;
	}
}

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
	s_pMainBfrMgr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pMainVPort,
	TAG_DONE);
	copBlockDisableSprites(s_pView->pCopList, 0xFF);


	randInit(2184);
	for(UBYTE i = 0; i != ENEMY_COUNT; ++i) {
		s_pEnemies[i] = enemyCreate();
	}
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

	// Clear all
	for(UBYTE i = 0; i != ENEMY_COUNT; ++i) {
		blitRect(s_pMainBfrMgr->pBuffer, s_pEnemies[i]->sCoord.sUwCoord.uwX, s_pEnemies[i]->sCoord.sUwCoord.uwY, 8, 8, 0);
		s_pEnemies[i]->pNext = 0;
	}
	s_pFirstEnemy = 0;

	for(UBYTE i = 0; i != ENEMY_COUNT; ++i) {
		enemyProcess(s_pEnemies[i]);
	}

	// Draw all
	tEnemy *pEnemy = s_pFirstEnemy;
	do {
		blitRect(
			s_pMainBfrMgr->pBuffer,
			pEnemy->sCoord.sUwCoord.uwX, pEnemy->sCoord.sUwCoord.uwY,
			8, 8, 1
		);
		pEnemy = pEnemy->pNext;
	} while(pEnemy);

	vPortWaitForEnd(s_pMainVPort);
}

void gameGsDestroy(void) {
	viewDestroy(s_pView);

	for(UBYTE i = 0; i != ENEMY_COUNT; ++i) {
		enemyDestroy(s_pEnemies[i]);
	}
}
