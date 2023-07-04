/**
 * Defines data structures that will be needed by all trace format's analysis functions
 **/
#include "Trace.h"

#ifdef TARGET_WINDOWS
const UINT32 numRegions = 1;
MemRegion specialRegions[numRegions] = {MemRegion(0x7ffe0000, 0x330)};
#else
const UINT32 numRegions = 0;
MemRegion specialRegions[numRegions] = {};
#endif

FILE *traceFile;
FILE *dataFile;
FILE *errorFile;
UINT64 eventId = 0;
bool printTrace = false;
UINT32 numThreads = 0;
