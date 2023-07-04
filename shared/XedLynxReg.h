#ifndef _XED_LYNX_REG_H_
#define _XED_LYNX_REG_H_
#include <stdio.h>
#include "LynxReg.h"
#include "TraceFileHeader.h"
#include <xed-interface.h>
LynxReg xedReg2LynxReg_all (xed_reg_enum_t reg, ArchType arch);
LynxReg xedReg2LynxReg_nopseudo (xed_reg_enum_t reg);
#endif // _XED_LYNX_REG_H_H
