#ifndef _LD40_GAMESTATES_GAME_GAME_H
#define _LD40_GAMESTATES_GAME_GAME_H

#include <ace/managers/viewport/simplebuffer.h>
#include <fixmath/fixmath.h>

#define GAME_MAIN_VPORT_HEIGHT 224

extern fix16_t g_pSin[256];

#define cSin(x) g_pSin[x]
#define cCos(x) g_pSin[((x)+64) & 0xFF]

void gameGsCreate(void);

void gameGsLoop(void);

void gameGsDestroy(void);

extern tSimpleBufferManager *g_pMainBfrMgr;

extern UBYTE g_ubGameOver;
extern UBYTE g_ubStartX, g_ubStartY;
extern UBYTE g_isExit;

#endif // _LD40_GAMESTATES_GAME_GAME_H
