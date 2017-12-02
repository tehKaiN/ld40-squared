#ifndef _LD40_GAMESTATES_GAME_GAME_H
#define _LD40_GAMESTATES_GAME_GAME_H

#include <ace/managers/viewport/simplebuffer.h>

void gameGsCreate(void);

void gameGsLoop(void);

void gameGsDestroy(void);

extern tSimpleBufferManager *g_pMainBfrMgr;

#endif // _LD40_GAMESTATES_GAME_GAME_H
