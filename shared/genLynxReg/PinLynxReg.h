#ifndef _PIN_LYNX_REG_H_
#define _PIN_LYNX_REG_H_
#include "pin.H"
#include <cstdio>
extern "C" {
#include "../shared/LynxReg.h"
}
extern PIN_MUTEX errorLock;
extern FILE *errorFile;

//Gets the PIN REG that corresponds to the given LynxReg
REG LynxReg2Reg(LynxReg lReg);

//Gets the LynxReg that corresponds to the given PIN reg
LynxReg Reg2LynxReg(REG reg);
#endif
