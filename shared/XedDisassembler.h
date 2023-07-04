
#ifndef __XED_DISASSEMBLER_H_
#define __XED_DISASSEMBLER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <xed-interface.h>
#include <LynxReg.h>
#include "ReaderUtils.h"
#include "Reader.h"
#include "ShadowRegisters.h"
#include "typedefs.h"

//returns bytelength of binary
DisassemblerState *setupXed_x86();
DisassemblerState *setupXed_x64();
void closeXed(DisassemblerState *s);
int decodeIns(DisassemblerState *s, xed_decoded_inst_t *xedIns, uint8_t *binary, int bufSize);
xed_iclass_enum_t getInsClass(xed_decoded_inst_t *xedIns);
uint8_t getInsMnemonic(xed_decoded_inst_t *xedIns, char *mnemonicBuf, int bufSize);
void getInsOps(DisassemblerState *s, RegState *regState, xed_decoded_inst_t *xedIns, uint32_t tid, ReaderOp *srcOpsHead, uint8_t *srcOpCnt, ReaderOp *dstOpsHead, uint8_t *dstOpCnt, ReaderOp *readWriteOpsHead, uint8_t *readWriteOpCnt);
void getInsFlags(xed_decoded_inst_t *xedIns, uint32_t *srcFlags, uint32_t *dstFlags);
ArchType getXedArchType(DisassemblerState *state);

#endif
