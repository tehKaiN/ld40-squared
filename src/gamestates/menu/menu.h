#ifndef _LD40_GAMESTATES_MENU_MENU_H
#define _LD40_GAMESTATES_MENU_MENU_H

#include <ace/managers/viewport/simplebuffer.h>

void menuGsCreate(void);

void menuGsLoop(void);

void menuGsDestroy(void);

void menuDraw(void);

extern tSimpleBufferManager *g_pMenuBfrMgr;

#endif // _LD40_GAMESTATES_MENU_MENU_H
