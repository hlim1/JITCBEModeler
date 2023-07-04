#ifndef _HELPERS_H_
#define _HELPERS_H_

using namespace std;

#include <map>
#include <string>
#include "pin.H"
#include "ShadowMemory.h"
#include "RegVector.h"

//void pinFinish(int, void *);
std::string extractFilename(const std::string &filename);
void getSrcDestInfo(INS &ins, RegVector *srcRegs, RegVector *destRegs, UINT32 &memReadOps, UINT32 &memWriteOps, UINT32 &destFlags);
void getSrcName(IMG img, string &srcStr);
void getFnName(RTN rtn, IMG img, string &fnStr);
void imgInstrumentation(IMG img, void *v);
//void setupTls();
void setupLocks();

extern ADDRINT entryPoint;
extern map<ADDRINT, string> apiMap;
extern ShadowMemory mem;
extern PIN_MUTEX traceLock;
extern PIN_MUTEX dataLock;
extern PIN_MUTEX errorLock;
extern TLS_KEY tlsKey;

#endif
