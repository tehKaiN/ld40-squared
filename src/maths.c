#include "maths.h"
#include <stdio.h>
#include <ace/types.h>
#include <ace/utils/file.h>

fix16_t g_pSin[256];

void mathsInit(void) {
	// Read sine table
	// Generated using libfixmath with floats enabled, dumped to file
	tFile *pSinFile = fileOpen("data/sin.dat", "rb");
	fileRead(pSinFile, &g_pSin, 256*sizeof(fix16_t));
	fileClose(pSinFile);
}
