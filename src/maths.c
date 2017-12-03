#include "maths.h"
#include <stdio.h>
#include <ace/types.h>

fix16_t g_pSin[256];

void mathsInit(void) {
	// Read sine table
	// Generated using libfixmath with floats enabled, dumped to file
	FILE *pSinFile = fopen("data/sin.dat", "rb");
	for(UWORD i = 0; i < 256; ++i)
		fread(&g_pSin[i], sizeof(fix16_t), 1, pSinFile);
	fclose(pSinFile);
}
