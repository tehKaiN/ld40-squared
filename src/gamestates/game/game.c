#include <gamestates/game/game.h>
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h>

static tView *s_pView;
static tVPort *s_pVport;
static tSimpleBufferManager *s_pBfrMngr;

void gameGsCreate(void) {
	logBlockBegin("gameGsCreate()");
	const UWORD pPalette[16] = {0x000, 0xFFF};
	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);
	s_pVport = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 4,
		TAG_VPORT_PALETTE_PTR, pPalette,
		TAG_VPORT_PALETTE_SIZE, 16,
	TAG_DONE);
	s_pBfrMngr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pVport,
	TAG_DONE);

	viewLoad(s_pView);

	blitRect(s_pBfrMngr->pBuffer, 0, 0, 8, 8, 1);
	logBlockEnd("gameGsCreate()");
}

void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}
}

void gameGsDestroy(void) {
	viewDestroy(s_pView);
}
