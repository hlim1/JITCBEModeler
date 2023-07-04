#ifndef TRACE_H_
#define TRACE_H_

#include "pin.H"
#include "ShadowMemory.h"

/*#ifdef TARGET_WINDOWS
#define ftell _ftelli64
#define fseek _fseeki64
#else
#define ftell ftello64
#define fseek fseeko64
#define fopen fopen64
#endif*/

class MemRegion {
public:
    MemRegion(ADDRINT baseAddr, UINT32 byteSize) :
            addr(baseAddr), size(byteSize) {
    }
    bool includes(ADDRINT testAddr, UINT32 byteSize) {
        return (testAddr >= addr && testAddr < (addr + size)) || ((testAddr + byteSize) >= addr && (testAddr + byteSize) < (addr + size));
    }
private:
    ADDRINT addr;
    UINT32 size;
};

extern UINT64 eventId;
extern const UINT32 numRegions;
extern MemRegion specialRegions[];
extern FILE *traceFile;
extern FILE *errorFile;
extern FILE *dataFile;
//extern PIN_MUTEX traceLock;
//extern PIN_MUTEX dataLock;
//extern TLS_KEY tlsKey;
//extern ShadowMemory mem;
extern bool printTrace;
extern UINT32 numThreads;
#endif
