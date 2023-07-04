/**
 * This file defines helper functions and instrumentation functions that may be useful for any trace format.
 * This includes functions to get the source/function names, get source/destination operand information for
 * an instruction. Additionally the image instrumentation function which is called whenever an image is 
 * loaded by the executable is defined in here. This is because we typically don't need to do any format
 * specific processing in there, unlike the instruction instrumentation.
 **/

#include <sys/time.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "RegVector.h"
#include "Helpers.h"

extern "C" {
#include "LynxReg.h"
#include "XedLynxReg.h"
#include "xed-interface.h"
#include "xed-flags.h"
#include "xed-decoded-inst-api.h"
}

using std::map;
using std::cerr;

ADDRINT entryPoint;
map<ADDRINT, string> apiMap;
string unknownStr = "Unknown";
string emptyStr = "";

ShadowMemory mem;

PIN_MUTEX traceLock;
PIN_MUTEX dataLock;
PIN_MUTEX errorLock;
TLS_KEY tlsKey;

/**
 * Function: extraceFilename
 * Description: Extracts a file name from a path by looking for the last instance of a '/' or '\'
 * Output: String with paths removed
 **/
std::string extractFilename(const std::string &filename) {
    unsigned long lastBackslash =
#ifdef TARGET_WINDOWS
            filename.rfind("\\");
#else
            filename.rfind("/");
#endif

    return (lastBackslash == std::string::npos) ? filename : filename.substr(lastBackslash + 1);
}

/**
 * Function: getSrcDestInfo
 * Description: Gets information about the sources and destinations of a function. There's a lot of code here
 *  to do this, but it's necessary. I've tried my best to include comments outlining what each part is doing.
 *  Note, we can get the exact registers that will be used, but we won't know memory addresses until runtime,
 *  so we can only count the number of times a particular a memory read/write occurs
 * Side Effects: Populates srcRegs, destRegs, memReadOps, memWriteOps and destFlags
 * Output: None
 **/
void getSrcDestInfo(INS &ins, RegVector *srcRegs, RegVector *destRegs, UINT32 &memReadOps, UINT32 &memWriteOps, UINT32 &destFlags) {
    xed_decoded_inst_t* xedIns = INS_XedDec(ins);

    const xed_simple_flag_t *flags = xed_decoded_inst_get_rflags_info(xedIns);

    if(flags != NULL) {
        //srcFlags = (unsigned int)(flags->read.flat) & 0x3fffff;
        destFlags = (unsigned int) (flags->written.flat) & 0x3fffff;
    }


    if(INS_Opcode(ins) != XED_ICLASS_NOP) {
        int numOps = INS_OperandCount(ins);

        for(int i = 0; i < numOps; ++i) {
            //If memory, increment memory counters and add base, index and segment registers to srcRegs
            if(INS_OperandIsMemory(ins, i)) {
                if(INS_OperandReadAndWritten(ins, i)) {
                    ++memWriteOps;
                    ++memReadOps;
                }
                else if(INS_OperandWritten(ins, i)) {
                    ++memWriteOps;
                }
                else {
                    ++memReadOps;
                }

                REG r = INS_OperandMemoryBaseReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
                r = INS_OperandMemoryIndexReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
                r = INS_OperandMemorySegmentReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
            }
            //If register, just add reg to approprate list
            else if(INS_OperandIsReg(ins, i)) {
                REG r = INS_OperandReg(ins, i);

                if(r != REG_INST_PTR && r != REG_GFLAGS) {
                    if(INS_OperandReadAndWritten(ins, i)) {
                        srcRegs->insert(r);
                        destRegs->insert(r);
                    }
                    else if(INS_OperandWritten(ins, i)) {
                        destRegs->insert(r);
                    }
                    else {
                        srcRegs->insert(r);
                    }
                }
            }
            //We don't need immediate values
            else if(INS_OperandIsImmediate(ins, i)) {
                //	immediateVals.push_back(INS_OperandImmediate(ins, i));
            }
            //Stuff gets complicated here. Some instructions have side effects and implicitly read/write
            // particular values. These values are fetched here. Unfortunately we cannot interface with
            // pin to get these values, they come from XED. So we first try to transform easy registers.
            // If we can't then there are some architecture specific registers that we deal with 
            // separately (some need to be added to xedReg2LynxReg). If we still can't find the register
            // we print out an error so it can be added
            else if(INS_OperandIsImplicit(ins, i)) {
                const xed_operand_t *op = xed_inst_operand(xedIns->_inst, i);
                if(op->_type == XED_OPERAND_TYPE_REG) {
                    LynxReg r = xedReg2LynxReg_nopseudo(op->_u._reg);

                    if(r == LYNX_INVALID) {
                        if(op->_u._reg == XED_REG_STACKPUSH || op->_u._reg == XED_REG_STACKPOP) {
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            srcRegs->insert(LYNX_RSP);
                            destRegs->insert(LYNX_RSP);
#else
                            srcRegs->insert(LYNX_ESP);
                            destRegs->insert(LYNX_ESP);
#endif
                        }
                        else if(op->_u._reg == XED_REG_X87STATUS) {
                            if(op->_rw == XED_OPERAND_ACTION_R || op->_rw == XED_OPERAND_ACTION_CR) {
                                srcRegs->insert(LYNX_FPSW);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_W || op->_rw == XED_OPERAND_ACTION_CW) {
                                destRegs->insert(LYNX_FPSW);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_RW || op->_rw == XED_OPERAND_ACTION_CRW || op->_rw == XED_OPERAND_ACTION_RCW) {
                                srcRegs->insert(LYNX_FPSW);
                                destRegs->insert(LYNX_FPSW);
                            }
                            else {
                                fprintf(errorFile, "invalid operand action for %s %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                            }
                        }
                        else if(op->_u._reg == XED_REG_X87CONTROL) {
                            if(op->_rw == XED_OPERAND_ACTION_R || op->_rw == XED_OPERAND_ACTION_CR) {
                                srcRegs->insert(LYNX_FPCW);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_W || op->_rw == XED_OPERAND_ACTION_CW) {
                                destRegs->insert(LYNX_FPCW);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_RW || op->_rw == XED_OPERAND_ACTION_CRW || op->_rw == XED_OPERAND_ACTION_RCW) {
                                srcRegs->insert(LYNX_FPCW);
                                destRegs->insert(LYNX_FPCW);
                            }
                            else {
                                fprintf(errorFile, "invalid operand action for %s %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                            }
                        }
                        else if(op->_u._reg == XED_REG_X87TAG) {
                            if(op->_rw == XED_OPERAND_ACTION_R || op->_rw == XED_OPERAND_ACTION_CR) {
                                srcRegs->insert(LYNX_FPTAG_FULL);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_W || op->_rw == XED_OPERAND_ACTION_CW) {
                                destRegs->insert(LYNX_FPTAG_FULL);
                            }
                            else if(op->_rw == XED_OPERAND_ACTION_RW || op->_rw == XED_OPERAND_ACTION_CRW || op->_rw == XED_OPERAND_ACTION_RCW) {
                                srcRegs->insert(LYNX_FPTAG_FULL);
                                destRegs->insert(LYNX_FPTAG_FULL);
                            }
                            else {
                                fprintf(errorFile, "invalid operand action for %s %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                            }
                        }
                        else if(op->_u._reg == XED_REG_X87PUSH) {
                            destRegs->insert(LYNX_ST0);
                            destRegs->insert(LYNX_FPSW);
                        }
                        else if(op->_u._reg == XED_REG_X87POP) {
                            srcRegs->insert(LYNX_ST0);
                            destRegs->insert(LYNX_FPSW);
                        }
                        else if(op->_u._reg == XED_REG_TSC || op->_u._reg == XED_REG_TMP3) {
                            //skip
                        }
                        else {
                            fprintf(errorFile, "Unknown Register in %s: %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                        }
                    }
                    else {
                        fprintf(errorFile, "%s: %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                        if(op->_rw == XED_OPERAND_ACTION_R || op->_rw == XED_OPERAND_ACTION_CR) {
                            srcRegs->insert(r);
                        }
                        else if(op->_rw == XED_OPERAND_ACTION_W || op->_rw == XED_OPERAND_ACTION_CW) {
                            destRegs->insert(r);
                        }
                        else if(op->_rw == XED_OPERAND_ACTION_RW || op->_rw == XED_OPERAND_ACTION_CRW || op->_rw == XED_OPERAND_ACTION_RCW) {
                            srcRegs->insert(r);
                            destRegs->insert(r);
                        }
                        else {
                            fprintf(errorFile, "invalid operand action for %s %s\n", INS_Disassemble(ins).c_str(), xed_reg_enum_t2str(op->_u._reg));
                        }
                    }
                }
                //I don't know why, but sometimes XED doesn't treat these like registers, and we have to
                // interpret the non-terminal
                else if(op->_type == XED_OPERAND_TYPE_NT_LOOKUP_FN) {
                    LynxReg r = LYNX_INVALID;
                    switch(op->_u._nt) {
                        case XED_NONTERMINAL_AR10:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R10;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR11:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R11;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR12:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R12;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR13:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R13;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR14:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R14;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR15:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R15;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR8:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R8;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_AR9:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_R9;
#else
                            assert(0);
#endif
                            break;
                        case XED_NONTERMINAL_ARAX:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RAX;
#else
                            r = LYNX_EAX;
#endif
                            break;
                        case XED_NONTERMINAL_ARBP:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RBP;
#else
                            r = LYNX_EBP;
#endif
                            break;
                        case XED_NONTERMINAL_ARBX:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RBX;
#else
                            r = LYNX_EBX;
#endif
                            break;
                        case XED_NONTERMINAL_ARCX:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RCX;
#else
                            r = LYNX_ECX;
#endif
                            break;
                        case XED_NONTERMINAL_ARDI:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RDI;
#else
                            r = LYNX_EDI;
#endif
                            break;
                        case XED_NONTERMINAL_ARDX:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RDX;
#else
                            r = LYNX_EDX;
#endif
                            break;
                        case XED_NONTERMINAL_ARSI:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RSI;
#else
                            r = LYNX_ESI;
#endif
                            break;
                        case XED_NONTERMINAL_ARSP:
                        case XED_NONTERMINAL_SRSP:
#if defined(TARGET_IA32E) || defined(TARGET_MIC)
                            r = LYNX_RSP;
#else
                            r = LYNX_ESP;
#endif
                            break;
                        default:
                            break;
                    }
                    // if we can't find the register, print an error
                    if(r == LYNX_INVALID) {
                        fprintf(errorFile, "implicit NT operand %s %s %d %d\n", INS_Disassemble(ins).c_str(), xed_operand_enum_t2str((xed_operand_enum_t)op->_name), op->_rw, op->_u._nt);

                    }
                    else {
                        if(op->_rw == XED_OPERAND_ACTION_R || op->_rw == XED_OPERAND_ACTION_CR) {
                            srcRegs->insert(r);
                        }
                        else if(op->_rw == XED_OPERAND_ACTION_W || op->_rw == XED_OPERAND_ACTION_CW) {
                            destRegs->insert(r);
                        }
                        else if(op->_rw == XED_OPERAND_ACTION_RW || op->_rw == XED_OPERAND_ACTION_CRW || op->_rw == XED_OPERAND_ACTION_RCW) {
                            srcRegs->insert(r);
                            destRegs->insert(r);
                        }
                        else {
                            fprintf(errorFile, "invalid operand action for %s %s\n", INS_Disassemble(ins).c_str(), xed_nonterminal_enum_t2str(op->_u._nt));
                        }
                    }
                }
                //if not a register or a non-terminal, print an error
                else {
                    fprintf(errorFile, "implicit operand %s %s %d\n", INS_Disassemble(ins).c_str(), xed_operand_enum_t2str((xed_operand_enum_t)op->_name), op->_type);
                }
            }
            //This doesn't need to be in the trace
            else if(INS_OperandIsBranchDisplacement(ins, i)) {
                //fprintf(errorFile, "Branch Displacement\n");
            }
            //Have never seen this pop up, I think it's handled by the memory case but I'm not sure
            else if(INS_OperandIsFixedMemop(ins, i)) {
                fprintf(errorFile, "Fixed Memory Op\n");
                //skip
            }
            //This is used for the lea instruction. We just need to get the base, index and segment register
            else if(INS_OperandIsAddressGenerator(ins, i)) {
                REG r = INS_OperandMemoryBaseReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
                r = INS_OperandMemoryIndexReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
                r = INS_OperandMemorySegmentReg(ins, i);
                if(REG_valid(r) && r != REG_INST_PTR) {
                    srcRegs->insert(r);
                }
            }
            //anything else is caught here
            else {
                fprintf(errorFile, "UNKNOWN OPERAND: %s\n", INS_Disassemble(ins).c_str());
            }
        }

        /**Set XED corrections here**/
        if(INS_Opcode(ins) == XED_ICLASS_FNSTENV) {
            srcRegs->insert(LYNX_FPCW);
            srcRegs->insert(LYNX_FPSW);
            srcRegs->insert(LYNX_FPTAG_FULL);
            srcRegs->insert(LYNX_FPIP_SEL);
            srcRegs->insert(LYNX_FPIP_OFF);
            srcRegs->insert(LYNX_FPDP_SEL);
            srcRegs->insert(LYNX_FPDP_OFF);
            srcRegs->insert(LYNX_FPOPCODE);
            destRegs->insert(LYNX_FPCW);
        }
        if(INS_Opcode(ins) == XED_ICLASS_FLDENV) {
            destRegs->insert(LYNX_FPCW);
            destRegs->insert(LYNX_FPSW);
            destRegs->insert(LYNX_FPTAG_FULL);
            destRegs->insert(LYNX_FPIP_SEL);
            destRegs->insert(LYNX_FPIP_OFF);
            destRegs->insert(LYNX_FPDP_SEL);
            destRegs->insert(LYNX_FPDP_OFF);
            destRegs->insert(LYNX_FPOPCODE);
        }
    }
}

/**
 * Function: getSrcName
 * Description: Extracts the name of the executable/library that was the source of the instruction. If
 *  PIN can't determine the source, uses Unknown (dynamically modified code).
 * Side Effects: Fills in srcStr (that way srcStr can be dynamically allocated if required)
 * Outputs: none
 **/
void getSrcName(IMG img, string &srcStr) {
    if(IMG_Valid(img)) { 
        srcStr = extractFilename(IMG_Name(img));
    }
    else {
        srcStr = unknownStr;
    }
}

/**
 * Function getFnName
 * Description: Extracts the name of the function that the instruction belongs to. If the function can't be
 *  found, uses the empty string
 * Side Effects: Fills in fnStr (that way it can be dynamically allocated if necessary)
 * Outputs: None
 **/
void getFnName(RTN rtn, IMG img, string &fnStr) {
    if(IMG_Valid(img)) { 
        ADDRINT rtnAddr = RTN_Address(rtn);
        if(apiMap.find(rtnAddr) != apiMap.end()) {
            fnStr = apiMap[rtnAddr];
        }
        else {
            fnStr = emptyStr;
        }
    }
    else {
        fnStr = emptyStr;
    }
}

/**
 * Function: imgInstrumentation
 * Description: The instrumentation for when an image is loaded. Currently all this does is iterate over the
 *  symbols loaded by the image, and puts the addresses and names in a map. If we execute this address, then
 *  we say we are executing that function (It seems like PIN is only giving me the symbols for functions 
 *  anyway? It's a big thing, I'd rather not get into it). Currently there is some commented out code, I'm
 *  leaving that in there because it's part of the process of trying to figure out how to get global/dynamic
 *  data. 
 * Side Effects: populates apiMap with function names
 * Outputs: None
 *
void imgInstrumentation(IMG img, void *v) {
    printf("img %s %llx %lx\n", IMG_Name(img).c_str(), (unsigned long long) IMG_LoadOffset(img), (unsigned long) IMG_SizeMapped(img));
    if(IMG_SecHead(img) == IMG_SecTail(img)) {
        printf("No sections\n");
    }
    for(SEC sec = IMG_SecHead(img); sec != IMG_SecTail(img); sec = SEC_Next(sec)) {
        printf("%llx %lx %d ", (unsigned long long) SEC_Address(sec), (unsigned long) SEC_Size(sec), SEC_Type(sec));
        if(SEC_IsReadable(sec)) {
            printf("r");
        }
        if(SEC_IsWriteable(sec)) {
            printf("w");
        }
        if(SEC_IsExecutable(sec)) {
            printf("x");
        }
        printf("\n");
    }
    for(SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)) {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        ADDRINT symAddr = SYM_Address(sym);
        if(apiMap.find(symAddr) == apiMap.end()) {
            apiMap[symAddr] = undFuncName.c_str();
        }
    }

    if(IMG_IsMainExecutable(img)) {
        int fd = fopen(IMG_NAME(img).c_str(), "rb");
        Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
        if(elf == NULL) {
            fprintf(stderr, "panic\n");
        }

        GElf_Ehdr elfHeader;
        if(gelf_getehdr(elf, &elfHeader) == NULL) {
            fprintf(stderr, "panic\n");
        }

        uint64_t elfEntry = elfHeader.e_entry();
        entryPoint = IMG_EntryAddress(img);

        uint64_t reloc = entryPoint - elfEntry;

        int numSegments;
        if(elf_getphdrnum(elf, &n)) {
            fprintf(stderr, "panic\n");
        }

        segmentInfo = ;

        for(int i = 0; i < n; i++) {
            GElf_Phdr segment; 
            if(gelf_getphdr (elf, i, &segment) != &segment) {
                if(segment.p_type == PT_LOAD) {
                    segment.p_vaddr + reloc;
                    segment.p_memsz;
                    segment.p_align;
                    segment.p_flags;
                }
            }
        }
    }
}*/

/**
 * Function: setupLocks
 * Description: Sets up file locks. 
 * Output: None
 **/
void setupLocks() {
    PIN_MutexInit(&traceLock);
    PIN_MutexInit(&dataLock);
    PIN_MutexInit(&errorLock);
}

