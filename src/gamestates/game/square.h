#ifndef _LD40_GAMESTATES_GAME_SQUARE_H
#define _LD40_GAMESTATES_GAME_SQUARE_H

#include <ace/types.h>
#include <fixmath/fixmath.h>

#define SQUARES_MAX 30

typedef struct _tSquare {
	struct _tSquare *pDisplayNext;
	struct _tSquare *pNext;
	struct _tSquare *pPrev;
	UBYTE ubAngle;
	fix16_t fX;
	fix16_t fY;
	fix16_t fSpeed;
	tUwCoordYX sCoord;
	tUwCoordYX sPrevCoord;
} tSquare;

void squaresManagerCreate(void);

void squaresManagerDestroy(void);

tSquare *squareAdd(UWORD uwX, UWORD uwY);

void squareRemove(tSquare *pSquare);

void squaresDraw(void);

void squaresUndraw(void);

void squaresOrderForDraw(void);

void squareProcessPlayer(void);
void squareProcessAi(void);

#endif // _LD40_GAMESTATES_GAME_SQUARE_H
