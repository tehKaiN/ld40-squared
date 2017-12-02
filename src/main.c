#include "main.h"
#include <ace/generic/main.h>
#include <ace/managers/game.h>
#include <ace/managers/joy.h>
#include <ace/managers/mouse.h>
#include "gamestates/game/game.h"

// This is a copypasta from OpenFire - ACE doesn't have this implemented in proper way
void inputProcess() {
	ULONG ulWindowSignal;
	ULONG ulSignals;
	struct IntuiMessage *pMsg;

	ulWindowSignal = 1L << g_sWindowManager.pWindow->UserPort->mp_SigBit;
	ulSignals = SetSignal(0L, 0L);

	joyProcess();

	if (!(ulSignals & ulWindowSignal))
		return;
	while (pMsg = (struct IntuiMessage *) GetMsg(g_sWindowManager.pWindow->UserPort)) {
		switch (pMsg->Class) {
			// Keyboard
			case IDCMP_RAWKEY:
				if (pMsg->Code & IECODE_UP_PREFIX) {
					pMsg->Code -= IECODE_UP_PREFIX;
					keySetState(pMsg->Code, KEY_NACTIVE);
				}
				else if (!keyCheck(pMsg->Code))
					keySetState(pMsg->Code, KEY_ACTIVE);
				break;
			// Mouse
			case IDCMP_MOUSEBUTTONS:
				if (pMsg->Code & IECODE_UP_PREFIX) {
					pMsg->Code -= IECODE_UP_PREFIX;
					mouseSetState(pMsg->Code, MOUSE_NACTIVE);
				}
				else if (!mouseCheck(pMsg->Code))
					mouseSetState(pMsg->Code, MOUSE_ACTIVE);
				break;
		}
		ReplyMsg((struct Message *) pMsg);
	}
}

void genericCreate(void) {
	mouseOpen();
	joyOpen();

	gamePushState(gameGsCreate, gameGsLoop, gameGsDestroy); // gamePushState vs gameChangeState
}

void genericProcess(void) {
	inputProcess();
	gameProcess();
}

void genericDestroy(void) {
	mouseClose();
	joyClose();
}
