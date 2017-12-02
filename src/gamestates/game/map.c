#include "gamestates/game/map.h"
#include <ace/managers/blit.h>

UBYTE g_pMap[MAP_WIDTH][MAP_HEIGHT];

void mapCreate(void) {
	for(UBYTE x = 0; x != MAP_WIDTH; ++x) {
		for(UBYTE y = 0; y != MAP_HEIGHT; ++y) {
			g_pMap[x][y] = 0;
		}
	}
	for(UBYTE x = 0; x != MAP_WIDTH; ++x) {
		g_pMap[x][0] = MAP_TILE_BEB;
		g_pMap[x][MAP_HEIGHT-1] = MAP_TILE_BEB;
	}
	for(UBYTE y = 0; y != MAP_HEIGHT; ++y) {
		g_pMap[0][y] = MAP_TILE_BEB;
		g_pMap[MAP_WIDTH-1][y] = MAP_TILE_BEB;
	}
}

void mapDraw(void) {
	for(UBYTE x = 0; x != MAP_WIDTH; ++x) {
		for(UBYTE y = 0; y != MAP_HEIGHT; ++y) {
			switch(g_pMap[x][y]) {
				case MAP_TILE_BEB:
					blitRect(g_pMainBfrMgr->pBuffer, x<<3, y<<3, 8, 8, 3);
					break;
				case MAP_TILE_PICKUP:
					blitRect(g_pMainBfrMgr->pBuffer, x<<3, y<<3, 8, 8, 4);
					break;
			}
		}
	}
}
