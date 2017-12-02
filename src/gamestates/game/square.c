#include "gamestates/game/square.h"
#include <ace/managers/blit.h>
#include <ace/managers/key.h>
#include "gamestates/game/game.h"

UBYTE s_ubSquareCount;
tSquare *g_pSquareFirst;
tSquare *g_pSquareDisplayFirst;

tBitMap *s_pSquareBitmap, *s_pSquareBg;

void squaresManagerCreate(void) {
	logBlockBegin("squaresManagerCreate()");
	s_ubSquareCount = 0;
	g_pSquareFirst = 0;
	g_pSquareDisplayFirst = 0;

	s_pSquareBitmap = bitmapCreate(8, 8, 3, BMF_INTERLEAVED);
	blitRect(s_pSquareBitmap, 0, 0, 8, 8, 1);

	s_pSquareBg = bitmapCreate(8, 8, 3, BMF_INTERLEAVED | BMF_CLEAR);

	logBlockEnd("squaresManagerCreate()");
}

void squaresManagerDestroy(void) {
	logBlockBegin("squaresManagerDestroy()");
	while(g_pSquareFirst)
		squareRemove(g_pSquareFirst);

	bitmapDestroy(s_pSquareBitmap);
	bitmapDestroy(s_pSquareBg);

	logBlockBegin("squaresManagerDestroy()");
}

tSquare *squareAdd(UWORD uwX, UWORD uwY) {
	const fix16_t fSpeedMul = fix16_div(fix16_from_int(4), fix16_from_int(5));
	// Create a square
	tSquare *pSquare = memAllocFast(sizeof(tSquare));
	pSquare->fX = fix16_from_int(uwX);
	pSquare->fY = fix16_from_int(uwY);
	pSquare->sCoord.sUwCoord.uwX = uwX;
	pSquare->sCoord.sUwCoord.uwY = uwY;
	pSquare->ubAngle = 0;
	pSquare->pDisplayNext = 0;
	pSquare->pNext = 0;

	// Attach to list
	if(!g_pSquareFirst) {
		g_pSquareFirst = pSquare;
		pSquare->pPrev = 0;
		pSquare->fSpeed = fix16_one;
	}
	else {
		tSquare *pPrev = g_pSquareFirst;
		while(pPrev->pNext)
			pPrev = pPrev->pNext;
		pPrev->pNext = pSquare;
		pSquare->pPrev = pPrev;
		pSquare->fSpeed = fSpeedMul;
	}
	++s_ubSquareCount;
	return pSquare;
}

void squareRemove(tSquare *pSquare) {
	// Detach from list
	pSquare->pNext->pPrev = 0;
	if(pSquare->pPrev)
		pSquare->pPrev->pNext = pSquare->pNext;
	else
		g_pSquareFirst = pSquare->pNext;
	// TODO remove all next squares instead?

	// Free mem
	memFree(pSquare, sizeof(tSquare));
	--s_ubSquareCount;
}

void squaresUndraw(void) {
	tSquare *pSquare = g_pSquareFirst;
	do {
		blitCopy(
			s_pSquareBg, 0, 0,
			g_pMainBfrMgr->pBuffer,
			pSquare->sCoord.sUwCoord.uwX, pSquare->sCoord.sUwCoord.uwY,
			8, 8, MINTERM_COOKIE, 0xFF
		);
		pSquare->pDisplayNext = 0;
		pSquare = pSquare->pNext;
	} while(pSquare);
	g_pSquareDisplayFirst = 0;
}

void squaresOrderDraw(void) {
	tSquare *pSquare = g_pSquareFirst;
	while(pSquare) {
		// First one?
		if(!g_pSquareDisplayFirst) {
			g_pSquareDisplayFirst = pSquare;
			pSquare = pSquare->pNext;
			continue;
		}

		tSquare *pPrevSquare = 0;
		tSquare *pOtherSquare = g_pSquareDisplayFirst;

		// Find Last or one that's lower on screen
		while(pOtherSquare && pOtherSquare->sCoord.ulYX < pSquare->sCoord.ulYX) {
			pPrevSquare = pOtherSquare;
			pOtherSquare = pOtherSquare->pDisplayNext;
		}

		if(!pPrevSquare) {
			// No prev, so other was first to display
			// swap and place old first as next
			pSquare->pDisplayNext = g_pSquareDisplayFirst;
			g_pSquareDisplayFirst = pSquare;
		}
		else if(!pOtherSquare->pDisplayNext) {
			// No next one to display- attach as next
			pOtherSquare->pDisplayNext = pSquare;
		}
		else {
			// Attach as prev's next, other is current next
			pSquare->pDisplayNext = pPrevSquare->pDisplayNext;
			pPrevSquare->pDisplayNext = pSquare;
		}

		pSquare = pSquare->pNext;
	}
}

void squareConstrainPos(tSquare *pSquare) {
	pSquare->sCoord.sUwCoord.uwX = fix16_to_int(pSquare->fX);
	pSquare->sCoord.sUwCoord.uwY = fix16_to_int(pSquare->fY);
}

void squareProcessPlayer(void) {
	tSquare *pSquare = g_pSquareFirst;

	if(keyCheck(KEY_W))
		pSquare->fY -= fix16_one;
	else if(keyCheck(KEY_S))
		pSquare->fY += fix16_one;
	if(keyCheck(KEY_A))
		pSquare->fX -= fix16_one;
	else if(keyCheck(KEY_D))
		pSquare->fX += fix16_one;

	squareConstrainPos(pSquare);
}

void squareProcessAi(void) {
	if(!g_pSquareFirst)
		return;
	tSquare *pSquare = g_pSquareFirst->pNext;
	while(pSquare) {
		tSquare *pTarget = pSquare->pPrev;
		if(!pTarget)
			continue;
		WORD wDx = pTarget->sCoord.sUwCoord.uwX - pSquare->sCoord.sUwCoord.uwX;
		WORD wDy = pTarget->sCoord.sUwCoord.uwY - pSquare->sCoord.sUwCoord.uwY;
		if(ABS(wDx) > 8)
			pSquare->fX += fix16_mul(fix16_from_int(SGN(wDx)), pSquare->fSpeed);
		if(ABS(wDy) > 8)
			pSquare->fY += fix16_mul(fix16_from_int(SGN(wDy)), pSquare->fSpeed);

		squareConstrainPos(pSquare);
		pSquare = pSquare->pNext;
	}
}

void squaresDraw(void) {
	tSquare *pSquare = g_pSquareDisplayFirst;
	while(pSquare) {
		blitCopy(
			s_pSquareBitmap, 0, 0,
			g_pMainBfrMgr->pBuffer,
			pSquare->sCoord.sUwCoord.uwX, pSquare->sCoord.sUwCoord.uwY,
			8, 8, MINTERM_COOKIE, 0xFF
		);
		pSquare = pSquare->pDisplayNext;
	}
}
