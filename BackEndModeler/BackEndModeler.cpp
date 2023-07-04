/**
 * This file contains the instrumentation for the DataOpsTrace. Really the only important function in here
 *  is insInstrumentation which fetches instruction information and decides what analysis calls to perform
 *  based on information about the instruction and what the user specifies should be included in the trace.
 **/

#include "pin.H"
#include "PinLynxReg.h"
#include "Helpers.h"
#include <map>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "ShadowMemory.h"
#include "RegVector.h"
#include "DataOpsDefs.h"
#include "StringTable.h"
#include "BackEndModelerAPI.h"
#include "Tracer.h"

#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>

#if defined(TARGET_IA32)
#pragma message("x86")
#define fullLynxReg LynxReg2FullLynxIA32Reg
#define lynxRegSize LynxRegSize32
#elif defined(TARGET_IA32E)
#pragma message("x86-64")
#define fullLynxReg LynxReg2FullLynxIA32EReg
#define lynxRegSize LynxRegSize
#else
#error "Unsupported Architecture"
#endif

using std::map;
using std::cerr;
using std::cout;
using std::endl;
using std::hex;

UINT16 infoSelect = 0xfffc;
bool entryPointFound = false;

bool writeSrcId = true;
bool writeFnId = true;
bool writeAddr = true;
bool writeBin = true;
bool writeSrcRegs = true;
bool writeMemReads = false;
bool writeDestRegs = true;
bool writeMemWrites = true;
bool regsWritten = true;
bool memWritten = true;

KNOB<bool> srcidOff(KNOB_MODE_WRITEONCE, "pintool", "nosrcid", "", "Don't include source IDs in the trace");
KNOB<bool> fnidOff(KNOB_MODE_WRITEONCE, "pintool", "nofnid", "", "Don't include function IDs in the trace");
KNOB<bool> addrOff(KNOB_MODE_WRITEONCE, "pintool", "noaddr", "", "Don't include the instruction's address in the trace");
KNOB<bool> binOff(KNOB_MODE_WRITEONCE, "pintool", "nobin", "", "Don't include the instruction binary in the trace");
KNOB<bool> srcRegOn(KNOB_MODE_WRITEONCE, "pintool", "srcreg", "", "Include an instruction's source registers in the trace");
KNOB<bool> memReadOn(KNOB_MODE_WRITEONCE, "pintool", "memread", "", "Include an instruction's memory reads in the trace");
KNOB<bool> destRegOff(KNOB_MODE_WRITEONCE, "pintool", "nodestreg", "", "Don't include an instruction's destination registers in the trace");
KNOB<bool> memWriteOff(KNOB_MODE_WRITEONCE, "pintool", "nomemwrite", "", "Don't include an instruction's memory writes in the trace");

// JIT IR MODELING ===========================================
UINT32 system_id = UINT32_INVALID;

bool is_jit = false;
// JIT IR MODELING ===========================================

/**
 * Function: pinFinish
 * Description: The function that is called when done executing
 * Output: None
 **/
void pinFinish(int, void *) {
    endFile();
}

/**
 * Function: imgInstrumentation
 * Description: The instrumentation for when an image is loaded. Currently all this does is iterate
 * over the symbols loaded by the image, and puts the addresses and names in a map.
 * If we execute this address, then we say we are executing that function (It seems like PIN is
 * only giving me the symbols for functions anyway? It's a big thing, I'd rather not get into it).
 * Currently there is some commented out code, I'm leaving that in there because it's part of
 * the process of trying to figure out how to get global/dynamic data. 
 * Side Effects: populates apiMap with function names
 * Outputs: None
 **/    
void imgInstrumentation(IMG img, void *v) {
    for(SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym)) {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        ADDRINT symAddr = SYM_Address(sym);
        if(apiMap.find(symAddr) == apiMap.end()) {
            apiMap[symAddr] = undFuncName.c_str();
        }
    }

    if(IMG_IsMainExecutable(img)) {
        entryPoint = IMG_EntryAddress(img);
    }

}

/**
 * Function: insInstrumentation
 * Description: This is the instrumentation portion of the tracer that should only be run once per 
 *  instruction. As such it needs to do a few things:
 *   1) Determine if we are interested in the code. For example, we don't care about code that 
 *       occured before the entry
 *   2) Extract information about the instruction from PIN
 *      - Note, while we generally want to avoid dynamically allocating memory, we do it here
 *        because hopefully this function should be called relatively few times compared to 
 *        the analysis functions, and because PIN only allows us to pass certain things to our 
 *        analysis function, such as a pointer.
 *   3) Decide what needs to be done with the information and schedule the appropriate callbacks.
 *      - This is done based on the operand information and what information the user requested be 
 *        recorded the trace
 * Output: None
 */
void insInstrumentation(INS ins, void *v) {

    const ADDRINT addr = INS_Address(ins);
    
    if(!entryPointFound) {
        if(addr == entryPoint) {
            PIN_RemoveInstrumentation();
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) startTrace, IARG_END);
            IMG_AddInstrumentFunction(imgInstrumentation, 0);
            PIN_AddContextChangeFunction(contextChange, 0);
            entryPointFound = true;
        }
        else {
            return;
        }
    }

    RegVector localSrcRegs;
    RegVector localDestRegs;
    RegVector *srcRegs = &localSrcRegs;
    RegVector *destRegs = &localDestRegs;

    //only dynamically allocate if we need to
    if(writeDestRegs) {
        destRegs = new RegVector();
    }
    if(writeSrcRegs) {
        srcRegs = new RegVector();
    }

    UINT32 memWriteOps = 0, memReadOps = 0;
    UINT32 destFlags = 0;

    if(regsWritten) {
        getSrcDestInfo(ins, srcRegs, destRegs, memReadOps, memWriteOps, destFlags);
    }
    else if(memWritten) {
        getSrcDestInfo(ins, srcRegs, destRegs, memReadOps, memWriteOps, destFlags);
    }

    RTN rtn = INS_Rtn(ins);
    IMG img = IMG_Invalid();

    if(RTN_Valid(rtn)) {
        SEC sec = RTN_Sec(rtn);
        if(SEC_Valid(sec)) {
            img = SEC_Img(sec);
        }
    }

    // Initialize thread(s), if necessary.
    INS_InsertIfCall(
            ins, IPOINT_BEFORE, (AFUNPTR) checkInitializedStatus, IARG_THREAD_ID, IARG_END);
    INS_InsertThenCall(
            ins, IPOINT_BEFORE, (AFUNPTR) initThread, IARG_THREAD_ID, IARG_CONST_CONTEXT, IARG_END);

    // Get the function name of current instruction.
    string fnStr = "";
    getFnName(rtn, img, fnStr);

    // Check and mark is_jit to true if the compiler for the system first appeared in
    // the function name.
    if (
            !is_jit && 
            (fnStr.find(V8_JIT) != std::string::npos || 
             fnStr.find(JSC_JIT) != std::string::npos ||
             fnStr.find(SPM_JIT) != std::string::npos)
    ) {
        is_jit = true;
    }

    // Only anlayze the instructions that execute after the jit compiler executed.
    if (is_jit) {

        UINT32 fnId = strTable.insert(fnStr.c_str());

        // Get the image source name of the current instruction.
        string srcStr;
        getSrcName(img, srcStr);
        UINT32 srcId = strTable.insert(srcStr.c_str());

        // Collect instruction binary (opcode & operands).
        ADDRINT instSize = INS_Size(ins);
        UINT8 *binary = new UINT8[instSize];
        EXCEPTION_INFO *exInfo = NULL;
        PIN_FetchCode(binary, (const void *) addr, instSize, exInfo);
        if(
                INS_IsSyscall(ins) && 
                INS_Opcode(ins) == XED_ICLASS_INT && binary[0] == 0xEA && binary[1] == 0x1e) {
            instSize = 2;
            binary[0] = 0xCD;
            binary[1] = 0x2E;
        }

        // Identify current system.
        if (system_id == UINT32_INVALID) {
            if (fnStr.find(V8_JIT) != std::string::npos) {
                system_id = V8;
            }
            else if (fnStr.find(JSC_JIT) != std::string::npos) {
                system_id = JSC;
            }
            else if (fnStr.find(SPM_JIT) != std::string::npos) {
                system_id = SPM;
            }
        }

        // Flag to keep a track of function return.
        bool is_ret = false;
        if (INS_IsRet(ins)) {
            is_ret = true;
        }
        // Call function call tracker to record function call stack.
        INS_InsertCall(
            ins, IPOINT_BEFORE, (AFUNPTR) track_function_calls, IARG_UINT32, fnId,
            IARG_BOOL, is_ret, IARG_END);

        // Check whether or not the current instruction is an instruction for backend element
        // (e.g., Instruction in V8) creator.
        bool is_BE_creation = fnInCreators(fnStr, BE_CREATORS, BE_CREATORS_SIZE);
        bool is_IR_creation = fnInCreators(fnStr, IR_CREATORS, IR_CREATORS_SIZE);

        // Get IR node addresses.
        if (is_IR_creation && is_ret) {
            INS_InsertCall(
                    ins, IPOINT_BEFORE, (AFUNPTR) populate_node_address, IARG_UINT32,
                    fnId, IARG_UINT32, system_id, IARG_END);
        }

        // If the current instruction is for a node allocation and it's a return, construct modeled
        // IR node.
        if (is_BE_creation && INS_IsRet(ins)) {
            INS_InsertCall(
                    ins, IPOINT_BEFORE, (AFUNPTR) constructModel, IARG_UINT32,
                    fnId, IARG_PTR, binary, IARG_ADDRINT, instSize, IARG_UINT32, 
                    system_id, IARG_END);
        }

        // Record memory reads.
        if(regsWritten || memWritten) {
            if(memReadOps == 1) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) recordMemRead,
                        IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32, fnId, 
                        IARG_PTR, binary, IARG_ADDRINT, instSize, IARG_END);
            }
            else if(memReadOps == 2) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) record2MemRead,
                        IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE,
                        IARG_UINT32, fnId, IARG_PTR, binary, IARG_ADDRINT, instSize, IARG_END);
            }
        }

        // Record source registers.
        if(writeSrcRegs && (srcRegs->getSize() > 0)) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) recordSrcRegs, 
                    IARG_FAST_ANALYSIS_CALL, IARG_THREAD_ID, IARG_CONST_CONTEXT, IARG_PTR, srcRegs, 
                    IARG_UINT32, fnId, IARG_UINT32, INS_Opcode(ins), IARG_END); 
        }

        // Record destination registers.
        if(writeDestRegs && (destRegs->getSize() > 0 || destFlags != 0)) {
            if(destRegs->getSize() == 0) {
                delete destRegs;
                destRegs = NULL;
            }

            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) recordDestRegs,
                    IARG_FAST_ANALYSIS_CALL, IARG_THREAD_ID, IARG_PTR, destRegs,
                    IARG_UINT32, destFlags, IARG_END);
        }

        // Record memory writes.
        if(writeMemWrites && memWriteOps == 1) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) recordMemWrite,
                    IARG_THREAD_ID, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
        }

        // Analyze recorded instruction information, i.e., src. registers, dest. registers,
        // and memory, etc.
        if(!INS_IsSyscall(ins)) {
            bool isAnalyze = false;
            if(INS_HasFallThrough(ins)) {
                INS_InsertCall(
                        ins, IPOINT_AFTER, (AFUNPTR) analyzeRecords, 
                        IARG_THREAD_ID, IARG_CONST_CONTEXT, IARG_UINT32, fnId,
                        IARG_UINT32, INS_Opcode(ins), IARG_BOOL, is_BE_creation,
                        IARG_PTR, binary, IARG_ADDRINT, instSize, IARG_UINT32, system_id,
                        IARG_INST_PTR, IARG_UINT32, srcId, IARG_BOOL, is_IR_creation,
                        IARG_END);
                isAnalyze = true;
            }
            if((INS_IsBranch(ins) || INS_IsCall(ins))
                    && !INS_IsXend(ins) && INS_IsValidForIpointTakenBranch(ins)) {
                INS_InsertCall(
                        ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) analyzeRecords, 
                        IARG_THREAD_ID, IARG_CONST_CONTEXT, IARG_UINT32, fnId,
                        IARG_UINT32, INS_Opcode(ins), IARG_BOOL, is_BE_creation,
                        IARG_PTR, binary, IARG_ADDRINT, instSize, IARG_UINT32, system_id,
                        IARG_INST_PTR, IARG_UINT32, srcId, IARG_BOOL, is_IR_creation,
                        IARG_END);
                isAnalyze = true;
            }
            if(!isAnalyze) {
                fprintf(errorFile, "not analyzing records %s\n", INS_Disassemble(ins).c_str());
            }
        }
    }
}

/**
 * Function: checkSelections
 * Description: Checks the command line arguments and sets flags indicating what information to 
 * include in the trace.
 * Output: None
 **/
void checkSelections() {
    if(srcidOff.Value()) {
        infoSelect &= ~(1 << SEL_SRCID);
        writeSrcId = false;
    }
    if(fnidOff.Value()) {
        infoSelect &= ~(1 << SEL_FNID);
        writeFnId = false;
    }
    if(addrOff.Value()) {
        infoSelect &= ~(1 << SEL_ADDR);
        writeAddr = false;
    }
    if(binOff.Value()) {
        infoSelect &= ~(1 << SEL_BIN);
        writeBin = false;
    }
    if(srcRegOn.Value()) {
        infoSelect |= (1 << SEL_SRCREG);
        writeSrcRegs = true;
    }
    if(memReadOn.Value()) {
        infoSelect |= (1 << SEL_MEMREAD);
        writeMemReads = true;
    }
    if(destRegOff.Value()) {
        infoSelect &= ~(1 << SEL_DESTREG);
        writeDestRegs = false;
    }
    if(memWriteOff.Value()) {
        infoSelect &= ~(1 << SEL_MEMWRITE);
        writeMemWrites = false;
    }

    regsWritten = writeSrcRegs || writeDestRegs;
    memWritten = writeMemReads || writeMemWrites;
}

/**
 * Function: main
 * Description: Sets up PIN
 * Output: None actually, since you never return from this
 **/
int main(int argc, char *argv[]) {
    LEVEL_PINCLIENT::PIN_InitSymbols();

    if(PIN_Init(argc, argv)) {
        cerr << KNOB_BASE::StringKnobSummary() << endl;
        return -1;
    }

    checkSelections();

    setupFile(infoSelect);
    setupLocks();

    IMG_AddInstrumentFunction(imgInstrumentation, 0);
    INS_AddInstrumentFunction(insInstrumentation, 0);
    PIN_AddFiniFunction(pinFinish, 0);
    PIN_AddThreadStartFunction(threadStart, 0);


    // Never returns
    PIN_StartProgram();

    return 0;
}
