#include "main.h"
#include <ace/generic/main.h>
#include <ace/managers/game.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include "gamestates/game/game.h"
#include "gamestates/menu/menu.h"
#include "maths.h"

tFont *g_pFont;

void inputProcess() {
	joyProcess();
	keyProcess();
}

void genericCreate(void) {
	keyCreate();
	joyOpen();
	mathsInit();

	g_pFont = fontCreate("data/silkscreen5.fnt");

	gamePushState(menuGsCreate, menuGsLoop, menuGsDestroy); // gamePushState vs gameChangeState
}

void genericProcess(void) {
	inputProcess();
	gameProcess();
}

void genericDestroy(void) {
	fontDestroy(g_pFont);
	keyDestroy();
	joyClose();
}
