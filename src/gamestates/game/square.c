#include "gamestates/game/square.h"
#include <ace/managers/blit.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/utils/chunky.h>
#include "gamestates/game/game.h"
#include "gamestates/game/map.h"

UBYTE s_ubSquareCount;
tSquare *g_pSquareFirst, *g_pSquareDisplayFirst;

tBitMap *s_pSquareBitmap, *s_pSquareBg, *s_pDirBitmap;

// Copied from ACE, forced sine table use
void chunkyRotateTabled(
	UBYTE *pSource, UBYTE *pDest,
	UBYTE ubAngle, UBYTE ubBgColor,
	WORD wWidth, WORD wHeight
) {
	fix16_t fSin, fCos, fCx, fCy;
	WORD x,y;
	WORD u,v;

	fCx = fix16_div(fix16_from_int(wWidth-1), fix16_from_int(2));
	fCy = fix16_div(fix16_from_int(wHeight-1), fix16_from_int(2));

	// For each of new bitmap's pixel sample color from rotated source x,y
	fSin = cSin(ubAngle);
	fCos = cCos(ubAngle);
	fix16_t dx, dy;
	for(y = 0; y != wHeight; ++y) {
		dy = fix16_sub(fix16_from_int(y), fCy);
		for(x = 0; x != wWidth; ++x) {
			dx = fix16_sub(fix16_from_int(x), fCx);
			// u = round(fCos*(x-fCx) - fSin*(y-fCy) +(fCx));
			u = fix16_to_int(
				fix16_add(
					fix16_sub(
						fix16_mul(fCos, dx),
						fix16_mul(fSin, dy)),
					fCx
				)
			);
			// v = fix16_to_int(fSin*(x-fCx) + fCos*(y-fCy) +(fCy));
			v = fix16_to_int(
				fix16_add(
					fix16_add(
						fix16_mul(fSin, dx),
						fix16_mul(fCos, dy)
					),
					fCy
				)
			);

			if(u < 0 || v < 0 || u >= wWidth || v >= wHeight)
				pDest[y*wWidth + x] = ubBgColor;
			else
				pDest[y*wWidth + x] = pSource[v*wWidth + u];
		}
	}
}

// Shameless copy from OpenFire's gamemath.h
UBYTE getAngleBetweenPoints(
	tUwCoordYX *pSrc, tUwCoordYX *pDst
) {
	UWORD uwDx = pDst->sUwCoord.uwX - pSrc->sUwCoord.uwX;
	UWORD uwDy = pDst->sUwCoord.uwY - pSrc->sUwCoord.uwY;
	// calc: ubAngle = ((pi + atan2(uwDy, uwDx)) * 128)/pi
	const fix16_t fPiHalf = fix16_div(fix16_pi, fix16_from_int(2));
	UBYTE ubAngle = fix16_to_int(
		fix16_div(
			fix16_mul(
				fix16_atan2(fix16_from_int(-uwDy), fix16_from_int(uwDx)),
				fix16_from_int(64)
			),
			fPiHalf
		)
	);
	return ubAngle;
}

WORD getDeltaAngleDirection(UBYTE ubPrevAngle, UBYTE ubNewAngle, UBYTE ubStep) {
	WORD wDelta = ubNewAngle - ubPrevAngle;
	if(wDelta < 0) {
		if(wDelta + 256 < ubStep)
			ubStep = wDelta + 256;
	}
	else if(wDelta < ubStep)
		ubStep = wDelta;
	if(!wDelta)
		return 0;
	if((wDelta > 0 && wDelta < 128) || wDelta + 256 < 128)
		return ubStep;
	return -ubStep;
}

void squaresManagerCreate(void) {
	logBlockBegin("squaresManagerCreate()");
	s_ubSquareCount = 0;
	g_pSquareFirst = 0;
	g_pSquareDisplayFirst = 0;

	s_pSquareBitmap = bitmapCreate(8, 8, 3, BMF_INTERLEAVED);
	blitRect(s_pSquareBitmap, 0, 0, 8, 8, 1);

	s_pSquareBg = bitmapCreate(8, 8, 3, BMF_INTERLEAVED | BMF_CLEAR);

	// Generate direction bitmap
	tBitMap *pSource = bitmapCreate(16,16, 3, BMF_INTERLEAVED);
	tBitMap *pRotated = bitmapCreate(16,16, 3, BMF_INTERLEAVED);
	s_pDirBitmap = bitmapCreate(8, 8*256, 3, BMF_INTERLEAVED | BMF_CLEAR);
	blitRect(pSource, 0, 0, 16, 16, 1);
	blitRect(pSource, 8-2, 16/2 - 3, 1, 5, 0);
	blitRect(pSource, 8-1, 16/2 - 2, 1, 3, 0);
	blitRect(pSource, 8-0, 16/2 - 1, 1, 1, 0);

	UBYTE pChunkySource[16*16], pChunkyRotated[16*16];
	for(UBYTE y = 0; y != 16; ++y)
		chunkyFromPlanar16(pSource, 0, y, &pChunkySource[16*y]);
	for(UWORD i = 0; i != 256; ++i) {
		chunkyRotateTabled(pChunkySource, pChunkyRotated,	i, 0, 16, 16);
		for(UBYTE y = 0; y != 16; ++y)
			chunkyToPlanar16(&pChunkyRotated[16*y], 0, y, pRotated);
		blitCopy(pRotated, 4, 4, s_pDirBitmap, 0, i*8, 8, 8, MINTERM_COOKIE, 0xFF);
	}
	bitmapDestroy(pSource);
	bitmapDestroy(pRotated);

	logBlockEnd("squaresManagerCreate()");
}

void squaresManagerClear(void) {
	while(g_pSquareFirst)
		squareRemove(g_pSquareFirst);
}

void squaresManagerDestroy(void) {
	logBlockBegin("squaresManagerDestroy()");
	squaresManagerClear();

	bitmapDestroy(s_pSquareBitmap);
	bitmapDestroy(s_pSquareBg);
	bitmapDestroy(s_pDirBitmap);

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
	pSquare->sPrevCoord.ulYX = pSquare->sCoord.ulYX;
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

	//  Set angle to target
	pSquare->ubAngle = getAngleBetweenPoints(
		&pSquare->sCoord, &pSquare->pPrev->sCoord
	);

	++s_ubSquareCount;
	return pSquare;
}

void squareRemove(tSquare *pSquare) {
	// Detach from list
	pSquare->pNext->pPrev = pSquare->pPrev;
	if(pSquare->pPrev)
		pSquare->pPrev->pNext = pSquare->pNext;
	else
		g_pSquareFirst = pSquare->pNext;
	// TODO remove all next squares instead?

	// Free mem
	memFree(pSquare, sizeof(tSquare));
	--s_ubSquareCount;
}

UBYTE squareMove(tSquare *pSquare) {
	pSquare->fX = fix16_add(pSquare->fX, cCos(pSquare->ubAngle));
	pSquare->fY = fix16_sub(pSquare->fY, cSin(pSquare->ubAngle));
	pSquare->sCoord.sUwCoord.uwX = fix16_to_int(pSquare->fX);
	pSquare->sCoord.sUwCoord.uwY = fix16_to_int(pSquare->fY);

	const tUwCoordYX *pPos = &pSquare->sCoord;
	if(
		g_pMap[pPos->sUwCoord.uwX >> 3][pPos->sUwCoord.uwY >> 3] == MAP_TILE_BEB ||
		g_pMap[pPos->sUwCoord.uwX >> 3][(pPos->sUwCoord.uwY+7) >> 3] == MAP_TILE_BEB ||
		g_pMap[(pPos->sUwCoord.uwX+7) >> 3][pPos->sUwCoord.uwY >> 3] == MAP_TILE_BEB ||
		g_pMap[(pPos->sUwCoord.uwX+7) >> 3][(pPos->sUwCoord.uwY+7) >> 3] == MAP_TILE_BEB
	) {
		squareRemove(pSquare);
		return 0;
	}

	UWORD uwTileX, uwTileY;
	uwTileX = pPos->sUwCoord.uwX >> 3;
	uwTileY = pPos->sUwCoord.uwY >> 3;
	if(g_pMap[uwTileX][uwTileY] == MAP_TILE_PICKUP) {
		squareAdd(uwTileX << 3, uwTileY << 3);
		g_pMap[uwTileX][uwTileY] = MAP_TILE_FREE;
	}

	uwTileY = (pPos->sUwCoord.uwY+7) >> 3;
	if(g_pMap[uwTileX][uwTileY] == MAP_TILE_PICKUP) {
		squareAdd(uwTileX << 3, uwTileY << 3);
		g_pMap[uwTileX][uwTileY] = MAP_TILE_FREE;
	}

	uwTileX = (pPos->sUwCoord.uwX+7) >> 3;
	if(g_pMap[uwTileX][uwTileY] == MAP_TILE_PICKUP) {
		squareAdd(uwTileX << 3, uwTileY << 3);
		g_pMap[uwTileX][uwTileY] = MAP_TILE_FREE;
	}

	uwTileY = pPos->sUwCoord.uwY >> 3;
	if(g_pMap[uwTileX][uwTileY] == MAP_TILE_PICKUP) {
		squareAdd(uwTileX << 3, uwTileY << 3);
		g_pMap[uwTileX][uwTileY] = MAP_TILE_FREE;
	}
	return 1;
}

void squareProcessPlayer(void) {
	tSquare *pSquare = g_pSquareFirst;
	UBYTE ubDestAngle = 0xFF;

	// NOTE: This is nasty but I guess it's quickest, think of something better?
	if(keyCheck(KEY_W) || joyPeek(JOY1_UP)) {
		if(keyCheck(KEY_A) || joyPeek(JOY1_LEFT))
			ubDestAngle = 96;
		else if(keyCheck(KEY_D) || joyPeek(JOY1_RIGHT))
			ubDestAngle = 32;
		else
			ubDestAngle = 64;
	}
	else if(keyCheck(KEY_S) || joyPeek(JOY1_DOWN)) {
		if(keyCheck(KEY_A) || joyPeek(JOY1_LEFT))
			ubDestAngle = 160;
		else if(keyCheck(KEY_D) || joyPeek(JOY1_RIGHT))
			ubDestAngle = 224;
		else
			ubDestAngle = 192;
	}
	else if(keyCheck(KEY_A) || joyPeek(JOY1_LEFT))
		ubDestAngle = 128;
	else if(keyCheck(KEY_D) || joyPeek(JOY1_RIGHT))
		ubDestAngle = 0;

	if(ubDestAngle != 0xFF) {
		if(pSquare->ubAngle != ubDestAngle)
			pSquare->ubAngle += getDeltaAngleDirection(pSquare->ubAngle, ubDestAngle, 5);
		if(!squareMove(pSquare)) {
			g_ubGameOver = 1;
			logWrite("BEB\n");
		}
	}
}

void squareProcessAi(void) {
	if(!g_pSquareFirst)
		return;
	tSquare *pSquare = g_pSquareFirst->pNext;
	while(pSquare) {
		tSquare *pTarget = pSquare->pPrev;
		if(!pTarget)
			continue;

		// Update angle
		WORD wDx = pTarget->sCoord.sUwCoord.uwX - pSquare->sCoord.sUwCoord.uwX;
		WORD wDy = pTarget->sCoord.sUwCoord.uwY - pSquare->sCoord.sUwCoord.uwY;
		UBYTE ubDestAngle = getAngleBetweenPoints(&pSquare->sCoord, &pTarget->sCoord);
		pSquare->ubAngle += getDeltaAngleDirection(pSquare->ubAngle, ubDestAngle, 4);

		// Current may cease to exist during movement - save 'next' for continuing
		tSquare *pNext = pSquare->pNext;

		// Move if too far
		if(ABS(wDx) > 9 || ABS(wDy) > 9)
			squareMove(pSquare);

		// Process next
		pSquare = pNext;
	}
}

void squaresOrderForDraw(void) {
	// Clear display order
	g_pSquareDisplayFirst = 0;
	tSquare *pSquare = g_pSquareFirst;
	while(pSquare) {
		pSquare->pDisplayNext = 0;
		pSquare = pSquare->pNext;
	}

	// Create new order
	pSquare = g_pSquareFirst;
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

void squaresUndraw(void) {
	// Direction arrow
	tSquare *pSquare = g_pSquareFirst;
	while(pSquare) {
		blitCopy(
			s_pSquareBg, 0, 0,
			g_pMainBfrMgr->pBuffer,
			pSquare->sPrevCoord.sUwCoord.uwX, pSquare->sPrevCoord.sUwCoord.uwY,
			8, 8, MINTERM_COOKIE, 0xFF
		);
		pSquare = pSquare->pNext;
	}
}

void squaresDraw(void) {
	// Draw with direction arrows
	tSquare *pSquare = g_pSquareDisplayFirst;
	while(pSquare) {
		blitCopy(
			s_pDirBitmap, 0, pSquare->ubAngle << 3,
			g_pMainBfrMgr->pBuffer,
			pSquare->sCoord.sUwCoord.uwX, pSquare->sCoord.sUwCoord.uwY,
			8, 8, MINTERM_COOKIE, 0xFF
		);
		pSquare->sPrevCoord.ulYX = pSquare->sCoord.ulYX;
		pSquare = pSquare->pDisplayNext;
	}
}
