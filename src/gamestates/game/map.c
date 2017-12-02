#include "gamestates/game/map.h"
#include <ace/managers/blit.h>

UBYTE g_pMap[MAP_WIDTH][MAP_HEIGHT];

void mapCreate(const char *szMapName) {
	logBlockBegin("mapCreate(szMapName: %s)", szMapName);
	char szPath[200];
	sprintf(szPath, "data/%s", szMapName);
	FILE *pMapFile = fopen(szPath, "rb");
	if(pMapFile) {
		UWORD uwCharCnt = 0;
		UBYTE x = 0, y = 0;
		while(uwCharCnt < MAP_HEIGHT * MAP_WIDTH) {
			UBYTE c;
			fread(&c, 1, 1, pMapFile);

			// Skip whitespace
			if(c < 32)
				continue;
			if(c == '#')
				g_pMap[x][y] = MAP_TILE_BEB;
			else if(c == '.')
				g_pMap[x][y] = MAP_TILE_FREE;
			else if(c == '+')
				g_pMap[x][y] = MAP_TILE_PICKUP;
			else if(c == 'S') {
				g_pMap[x][y] = MAP_TILE_FREE;
				g_ubStartX = x;
				g_ubStartY = y;
				logWrite("Start: %hhu, %hhu\n", x, y);
			}
			else
				logWrite("ERR: Unknown char: %c (0x%02X) at %hhu,%hhu\n", c, c, x, y);
			++x;
			if(x >= MAP_WIDTH) {
				x = 0;
				++y;
			}
			++uwCharCnt;
		}
		fclose(pMapFile);
	}
	else {
		logWrite("ERR: Map %s not found\n", szPath);
	}

	// Ensure that there is a beb border
	for(UBYTE x = 0; x != MAP_WIDTH; ++x) {
		g_pMap[x][0] = MAP_TILE_BEB;
		g_pMap[x][MAP_HEIGHT-1] = MAP_TILE_BEB;
	}
	for(UBYTE y = 0; y != MAP_HEIGHT; ++y) {
		g_pMap[0][y] = MAP_TILE_BEB;
		g_pMap[MAP_WIDTH-1][y] = MAP_TILE_BEB;
	}
	logBlockEnd("mapCreate()");
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
