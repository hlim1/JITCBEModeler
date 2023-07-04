 /* This file is intended to hold everything needed for the analysis functions of a format.
 * In addition, it includes everything that is needed to setup/finish the trace file because
 * historically the Main file contained code for multiple trace formats.
 **/

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include "PinLynxReg.h"
#include "BackEndModelerAPI.h"
#include "DataOpsDefs.h"
#include "Helpers.h"
#include "Tracer.h"

#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::vector;
using std::string;
using std::hex;
using std::dec;
using std::ostringstream;

// This will need to be adjusted if there are more than 16 threads, 
// but this is faster than Pin's TLS
const UINT32 maxThreads = 20;

StringTable strTable;
const RegVector emptyRegVector;
UINT32 sectionOff;
UINT32 traceHeaderOff;
UINT32 traceStart;
UINT32 predTID = 0;
UINT32 numSkipped = 0;
const UINT32 LARGEST_REG_SIZE = 1024;
const UINT32 BUF_SIZE = 16384;
const UINT32 MAX_MEM_OP_SIZE = 1024;
const UINT32 UINT8_SIZE = sizeof(UINT8);
const UINT32 UINT16_SIZE = sizeof(UINT16);
const UINT32 UINT32_SIZE = sizeof(UINT32);
const UINT32 UINT64_SIZE = sizeof(UINT64);
const UINT32 ADDRINT_SIZE = sizeof(ADDRINT);

UINT8 traceBuf[BUF_SIZE];
UINT8 *traceBufPos = traceBuf;
UINT8 dataBuf[BUF_SIZE];
UINT8 *dataBufPos = dataBuf;

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

// Thread local storage structure (for those unfamiliar with C++ structs, they are essentially
// classes with different privacy rules so you can have functions)
struct ThreadData {
    ThreadData() :
            eventId(0), pos(NULL), memWriteAddr(0), memWriteSize(0), destRegs(NULL), 
            destFlags(0), print(false), initialized(false), predSrcId(0), predFuncId(0), 
            predAddr(0), dataPos(NULL), printBinary(NULL), binary(NULL) {}
    UINT64 eventId;
    UINT8 buffer[40];
    UINT8 *pos;
    UINT8 *predRecord;
    ADDRINT memWriteAddr;
    UINT32 memWriteSize;
    const RegVector *destRegs;
    UINT32 destFlags;
    bool print;
    bool initialized;
    uint32_t predSrcId;
    uint32_t predFuncId;
    ADDRINT predAddr;
    UINT8 dataBuffer[4 * MAX_MEM_OP_SIZE];
    UINT8 *dataPos; 
    bool *printBinary;
    uint8_t *binary;
    uint8_t binSize;
};

//Now we don't have to rely on PIN for TLS
ThreadData tls[maxThreads];

// =============================================================

BModel *BackEnd = new BModel();

// Declare consts here.
const string TARGET_REG = "RAX";
const LynxReg TARGET_LREG = LYNX_RAX;
const int MAX_REGS = 10;
const ADDRINT WIPEMEM = 0;

const UINT32 ADD = XED_ICLASS_ADD;
const UINT32 PUSH = XED_ICLASS_PUSH;
const UINT32 MOV = XED_ICLASS_MOV;
const ADDRINT FUNCTION_ENTER = 85;

// Strcut for keeping the instruction id.
struct Instruction {
    Instruction() : id(0) {}
    int id;                   // instruction id.
} instruction;

// Struct for holding the target instruction info.
struct TargetInst {
    UINT32 fnId;               // function id.
    UINT8* binary;             // instruction opcode & operand.
};

// Struct for source register information.
struct RegInfo {
    int instId;                // instruction id.
    UINT32 fnId;               // function id.
    UINT32 instOp;             // instruction opcode.
    LynxReg lReg;              // lynx register.
    ADDRINT value;             // value in the register.
    ADDRINT opcode;            // x86 instruction opcode (in decimal).
};

// Struct for memory write information.
struct MWInst {
    int     instId;            // instruction id.
    UINT32  fnId;              // function id.
    ADDRINT location;          // address location.
    ADDRINT value;             // written value.
    ADDRINT valueSize;         // size of written value in bytes.
    UINT32  opcode;            // x86 instruction opcode (in decimal).
    RegInfo srcRegs[MAX_REGS]; // values in the read registers.
    int regSize = 0;           // number of register values collected.
};

// Struct for memory read information.
struct MRInst {
    UINT32  fnId;               // function id.
    ADDRINT location;           // address location.
    ADDRINT value;              // read value.
    ADDRINT valueSize;          // size of read value in bytes.
    UINT32  opcode;             // x86 instruction opcode (in decimal).
    RegInfo srcRegs[MAX_REGS];  // values in the read registers.
    int regSize = 0;            // number of register values collected.
};

// Struct for rax value information.
struct RAXValue {
    ADDRINT value;              // value in the rax register.
    UINT32 fnId;                // function id.
} raxValue;

// Code buffer.
struct CodeBuffer {
    CodeBuffer() : start(ADDRINT_INVALID), size(DEFAULT_V8_BUFFER_SIZE), end(ADDRINT_INVALID) {}
    ADDRINT start;    // Start address of the buffer that holds the native code.
    ADDRINT size;     // Size of buffer.
    ADDRINT end;      // End address of the buffer.
    ADDRINT pc;       // Program counter.
} code_buffer;

// Structures for holding memory/register reads and writes.
map<ADDRINT,MWInst> writes;      // Write location address to MWInst object.
map<ADDRINT,MRInst> reads;       // Read location address to MRInst object.
map<int,MWInst> targetMWs;       // Write location address to MWInst object (Target Insts. Only).
int targetMWId = 0;              // Target function's MW ID.
map<ADDRINT,MWInst> IRFormerMWs; // IR former function's MW infos.
map<int,RegInfo> targetSrcRegs;  // targetSrcRegsKey to RegInfo object.
map<int,RegInfo> targetDesRegs;  // targetDesRegsKey to RegInfo object.
int targetSrcRegsKey = 0;        // Target source register key.
int targetDesRegsKey = 0;        // Target destination register key.
map<ADDRINT,Node*> irNodeAddrs;  // Addresses of IR nodes.
int nodeId = 0;                  // IR node ID.
RegInfo srcRegsHolder[MAX_REGS]; // Source register holder.
RegInfo desRegsHolder[MAX_REGS]; // Destination register holder.
int srcRegSize = 0;              // Number of tracking source register size.
int desRegSize = 0;              // Number of tracking destination register size.
bool populate_regs = false;      // Flag to indicate either to start tracking register or not.
map<ADDRINT,ADDRINT> opAddr2nodeAddr;      // Node opcode address to node address.
map<ADDRINT,int> elementAddrs;             // Keeping track of generated back-end model addresses.
ADDRINT lastMemReadLoc = ADDRINT_INVALID;  // Tracker of last memory read location address.
bool is_former_range = false;              // Flag to indicate the inst. is within the element former range.
bool is_ir_former_range = false;           // Flag to indicate the inst. is within the IR former range.
vector<UINT32> function_call_stack;        // Function call stack.
ADDRINT malloc_rax_addr = ADDRINT_INVALID; // Value written to RAX in the malloc function.
int current_elem_id = INT_INVALID;          // Current instruction id.
bool is_function_entry = false;            // Flag to indicate the instruction is a function entry.
bool is_jit_code_entry = false;            // Flag to indicate the instruction is a JIT entry.
map<ADDRINT,Code> buffer_offset2value;     // Native code buffer offset to value.

/**
 * Function: constructModel
 * Description: This function analyzes the collected memory/register reads and writes
 *  to construct the JIT compiler back-end element. Then, adds the element to the model.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 * Output: None.
 **/
void constructModel(UINT32 fnId, UINT8* binary, ADDRINT instSize, UINT32 system_id) {

    // Keep a track of system ID in the IR, if not populated already.
    if (BackEnd->systemId == UINT32_INVALID) {
        BackEnd->systemId = system_id;
    }

    // Get address of the target back-end element.
    ADDRINT address = get_element_address(fnId, system_id);
    assert(address != ADDRINT_INVALID);

    // Get (machine) opcode of the target back-end element.
    ADDRINT opcode = get_element_opcode(fnId, system_id, address);
    assert(system_id != SPM && opcode != ADDRINT_INVALID);

    map<ADDRINT,int>::iterator address_it;
    address_it = elementAddrs.find(address);

    // Create a new object and add to the BackEnd only if a valid
    // opcode exists.
    if (opcode != ADDRINT_INVALID && address_it == elementAddrs.end()) {
        // Add the address of newly modeled instruction model's address
        // to the model tracker.
        elementAddrs[address] = BackEnd->lastELMId;

        BElement *element = new BElement();
        element->id = BackEnd->lastELMId;
        element->address = address;
        element->opcode = opcode;

        // Get IR opcodes.
        get_ir_opcodes(fnId, system_id, element);
        // Get architecture opcode.
        element->arch_opcode = get_architecture_opcode(fnId, system_id, opcode);
        // Get the size of element.
        get_size(fnId, system_id, element);
        // Get the operands.
        get_operands(fnId, system_id, element);
        // Get the values written to the element.
        get_written_values(fnId, system_id, element);

        BackEnd->elements[element->id] = element;
        BackEnd->lastELMId++;

        // DEBUG
        //cout << "address: 0x" << hex << address << "; size: " << element->size;
        //cout << "; operand sizes: ";
        //cout << dec << element->output_size << ", " << element->input_size << endl;
       
        //std::vector<UINT32>::iterator it = function_call_stack.end();
        //string current_fn = strTable.get(*it);
        //string prev_fn = strTable.get(*(it-1));
        //cout << "Current function: " << current_fn << "; Previous function: " << prev_fn << endl;

        // Update the element's access log.
        updateLogInfo(element, fnId, binary, instSize, CREATE);
    }

    // Reset temporary value holders.
    targetMWs.clear();
    targetDesRegs.clear();
    targetSrcRegs.clear();
    targetMWId = 0;
    targetDesRegsKey = 0;
    targetSrcRegsKey = 0;
    is_former_range = false;
}

/**
 * Function: populate_node_address
 * Description: This function populates optimizer IR node addresses
 * to the irNodeAddrs, i.e., IR node generator tracker.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 * Output: None.
 **/
void populate_node_address(UINT32 fnId, UINT32 system_id) {

    ADDRINT address = get_node_address(fnId, system_id);
    assert (address != ADDRINT_INVALID);

    Node *node = new Node();
    node->id = nodeId;
    node->intAddress = address;

    ADDRINT *opcode;
    opcode = get_opcode(node, system_id, fnId);
    node->opcode = opcode[0];
    node->opcodeAddress = opcode[1];

    irNodeAddrs[address] = node;
    nodeId++;

    opAddr2nodeAddr[opcode[1]] = address;

    IRFormerMWs.clear();
}

/**
 * Function: get_opcode
 * Description: This function calls appropriate function for each JIT compiler system
 *  to retrieve the opcode of currently forming IR node model.
 * Input:
 *  - node (Node*): Currently forming IR node model.
 *  - system_id (UINT32): JIT compiler system ID.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output: Pointer to opcode array.
 **/
ADDRINT *get_opcode(Node *node, UINT32 system_id, UINT32 fnId) {

    static ADDRINT *opcode;

    if (system_id == V8) {
        opcode = get_opcode_v8(node);    
    }

    return opcode;
}

/**
 * Function: get_opcode_v8
 * Description: This function analyzes collected memory write information to
 *  retrieve the V8 IR node opcode and address where opcode is stored.
 * Input:
 *  - node (Node*): Currently forming IR node model.
 * Output: Statically allocated opcode array, where index 0 holds opcode and
 * index 1 holds address where opcode is stored within the node block range.
 **/
ADDRINT *get_opcode_v8(Node *node) {

    ADDRINT value = ADDRINT_INVALID;
    map<ADDRINT,MWInst>::iterator it;
    for (it = IRFormerMWs.begin(); it != IRFormerMWs.end(); ++it) {
        if (it->first == node->intAddress) {
            value = (it->second).value;
            break;
        }
    }
    assert(value != ADDRINT_INVALID);

    static ADDRINT opcode[2];
    opcode[0] = ADDRINT_INVALID;
    opcode[1] = ADDRINT_INVALID;

    map<ADDRINT,MWInst>::iterator it2;
    for (it2 = writes.begin(); it2 != writes.end(); ++it2) {
        MWInst write = it2->second;
        if (write.valueSize == V8_OPCODE_SIZE) {
            for (int i = 0; i < write.regSize; i++) {
                if (value == write.srcRegs[i].value) {
                    opcode[0] = write.value;
                    opcode[1] = write.location;
                    break;
                }
            }
            if (opcode[0] != ADDRINT_INVALID) {
                break;
            }
        }
    }

    return opcode;
}

/* API functions Begin */

/**
 * Function: get_element_address
 * Description: Calls appropriate system-specific API function to get
 * the address of modeled back-end element.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  Output:
 *  - address (ADDRINT): address of modeled back-end element.
 **/
ADDRINT get_element_address(UINT32 fnId, UINT32 system_id) {

    ADDRINT address = ADDRINT_INVALID;

    if (system_id == V8) {
        address = get_element_address_v8(fnId);
    }
    else if (system_id == SPM) {
        address = get_element_address_spm(fnId);
    } 
    
    return address;
}

/**
 * Function: get_element_opcode
 * Description: Calls appropirate system-specific API function to get
 * the opcode of modeled back-end element.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  Output:
 *  - opcode (ADDRINT): opcode of modeled back-end element.
 **/
ADDRINT get_element_opcode(UINT32 fnId, UINT32 system_id, ADDRINT address) {

    ADDRINT opcode = ADDRINT_INVALID;

    if (system_id == V8) {
        opcode = get_element_opcode_v8(fnId, address);
    }
    else if (system_id == SPM) {
        opcode = get_element_opcode_spm(fnId, address);
    }

    return opcode;
}

/**
 * Function: get_architecture_opcode
 * Description: This function computes the architecture opcode from the back-end
 * representation opcode.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  - opcode (ADDRINT): back-end representation opcode.
 * Output:
 * - arch_opcode (ADDRINT): architecture opcode.
 **/
ADDRINT get_architecture_opcode(UINT32 fnId, UINT32 system_id, ADDRINT opcode) {
    
    ADDRINT arch_opcode = ADDRINT_INVALID;

    if (system_id == V8) {
        arch_opcode = get_architecture_opcode_v8(fnId, opcode);
    }

    return arch_opcode;
}

/**
 * Function: get_ir_opcodes
 * Description: This function gets the IR opcode that the back-end element
 * is derived from.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  - element (BElement): Back-end representation model element.
 * Output: None.
 **/
void get_ir_opcodes(UINT32 fnId, UINT32 system_id, BElement* element) {

    if (system_id == V8) {
        get_ir_opcodes_v8(fnId, element);
    }
}

/**
 * Function: get_node_address
 * Description: This function identifies the optimizer IR node address.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 * Output:
 *  - address (ADDRINT): address of identified optimizer IR node.
 **/
ADDRINT get_node_address(UINT32 fnId, UINT32 system_id) {

    ADDRINT address = ADDRINT_INVALID;

    if (system_id == V8) {
        address = get_node_address_v8(fnId);
    }

    return address;
}

/**
 * Function: get_size
 * Description: This function identifies and gets the size of back-end model.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_size(UINT32 fnId, UINT32 system_id, BElement *element) {
    
    if (system_id == V8) {
        get_size_v8(fnId, element);
    }
}

/**
 * Function: get_operands
 * Description: This function identifies the operands of the back-end model.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_operands(UINT32 fnId, UINT32 system_id, BElement *element) {

    if (system_id == V8) {
        get_operand_sizes_v8(fnId, element);
    }
}

/**
 * Function: get_written_values
 * Description: This function extracts the values written to the element
 * at the time of generation.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - system_id (UIN32): JIT compiler system ID.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_written_values(UINT32 fnId, UINT32 system_id, BElement *element) {

    ADDRINT start = element->address;
    ADDRINT last = (element->address + element->size);

    map<int,MWInst>::iterator it;
    for (it = targetMWs.begin(); it != targetMWs.end(); ++it) {
        MWInst write = it->second;
        string fn = strTable.get(write.fnId);
        if (write.location > start && write.location < last) {
            element->addr2val[write.location] = write.value;
        }
    }
}

/**
 * Function: get_element_address_v8
 * Description: Returns address of modeled V8 back-end element.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output:
 *  - address (ADDRINT): address of modeled V8 back-end element.
 */
ADDRINT get_element_address_v8(UINT32 fnId) {

    assert (fnId == raxValue.fnId);
    return raxValue.value;
}

/**
 * Function: get_element_address_spm
 * Description: Returns address of modeled SpiderMonkey back-end element.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output:
 *  - address (ADDRINT): address of modeled SpiderMonkey back-end element.
 */
ADDRINT get_element_address_spm(UINT32 fnId) {

    assert (fnId == raxValue.fnId);
    return raxValue.value;
}

/**
 * Function: get_element_opcode_v8
 * Description: Returns opcode of modeled V8 back-end element.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - address (ADDRINT): Address of element.
 *  Output:
 *  - opcode (ADDRINT): opcode of modeled V8 back-end element.
 */
ADDRINT get_element_opcode_v8(UINT32 fnId, ADDRINT address) {

    ADDRINT opcode = ADDRINT_INVALID;

    map<ADDRINT,MWInst>::iterator it;
    for (it = writes.begin(); it != writes.end(); ++it) {
        MWInst write = it->second;
        string fn = strTable.get(write.fnId);
        if (fnInFormers(fn) && write.location == address) {
            opcode = write.value;
            break;
        }
    }

    return opcode;
}

/**
 * Function: get_architecture_opcode_v8
 * Description: This function computes the architecture opcode from the V8's back-end
 * representation opcode.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - opcode (ADDRINT): back-end representation opcode.
 * Output:
 * - arch_opcode (ADDRINT): architecture opcode.
 **/
ADDRINT get_architecture_opcode_v8(UINT32 fnId, ADDRINT opcode) {
    
    const int DECODE_VAL = 511;

    ADDRINT arch_opcode = ADDRINT_INVALID;

    arch_opcode = opcode & DECODE_VAL;

    return arch_opcode;
}

/**
 * Function: get_ir_opcodes_v8
 * Description: This function gets the IR opcode that the back-end model is derived from.
 * This function is V8-specific API function.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_ir_opcodes_v8(UINT32 fnId, BElement* element) {

    assert (function_call_stack.size() > 2);

    vector<UINT32>::iterator call_it = function_call_stack.end();
    UINT32 prev_fnId = *(call_it-1);

    bool is_prev = false;
    map<ADDRINT,MWInst>::reverse_iterator writes_it;
    for (writes_it = writes.rbegin(); writes_it != writes.rend(); ++writes_it) {
        MWInst write = writes_it->second;

        map<ADDRINT,Node*>::iterator nodes_it;
        nodes_it = irNodeAddrs.find(write.value);
        if (nodes_it != irNodeAddrs.end()) {
            string fn = strTable.get(write.fnId);
            string prev_fn = strTable.get(prev_fnId);
            if (prev_fnId == write.fnId) {
                is_prev = true;
                element->ir_opcodes.push((nodes_it->second)->opcode);
            }
            else if (write.fnId != prev_fnId && is_prev){
                break;
            }
        }
    }
}

/**
 * Function: get_element_opcode_spm
 * Description: This function analyzes collected memory write information to
 *  retrieve the SPM IR node opcode and address where opcode is stored.
 * Input:
 *  - node (Node*): Currently forming IR node model.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output: Statically allocated opcode array, where index 0 holds opcode and
 * index 1 holds address where opcode is stored within the node block range.
 **/
ADDRINT get_element_opcode_spm(UINT32 fnId, ADDRINT address) {

    ADDRINT opcode = ADDRINT_INVALID;

    string fn = strTable.get(fnId);

    // CFG Block (MBasicBlock) does not hold an opcode, so we analyze the recorded instruction
    // data only if the current node is not a block node. Otherwise, we set is_nonIR to true.
    if (!fnInNonIRAllocs(fn)) {
        map<int,MWInst>::iterator it;
        for (it = targetMWs.begin(); it != targetMWs.end(); ++it) {
            MWInst write = it->second;
            if (write.valueSize == SPM_OPCODE_SIZE) {
                for (int i = 0; i < write.regSize; i++) {
                    if ((write.srcRegs[i]).value == address) {
                        opcode = write.value;
                        break;
                    }
                }
            }
            if (opcode != ADDRINT_INVALID) {
                break;
            }
        }
    }

    return opcode;
}

/**
 * Function: get_node_address_v8
 * Description: This function specifically targets to find the Google V8's optimizer 
 * IR node address.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output:
 *  - address (ADDRINT): address of identified optimizer IR node.
 **/
ADDRINT get_node_address_v8(UINT32 fnId) {
    
    if (fnId != raxValue.fnId) {
        string fn1 = strTable.get(fnId);
        cerr << "fnId: " << fnId << "(" << fn1 << ")" << endl;
        string fn2 = strTable.get(raxValue.fnId);
        cerr << "raxValue.fnId: " << raxValue.fnId << "(" << fn2 << ")" << endl; 
    }
    assert (fnId == raxValue.fnId);
    return raxValue.value;
}

/**
 * Function: get_size_v8
 * Description: This function identifies and gets the size of V8 back-end model.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_size_v8(UINT32 fnId, BElement *element) {

    ADDRINT size = ADDRINT_INVALID;

    map<int,RegInfo>::iterator it;
    for (it = targetSrcRegs.begin(); it != targetSrcRegs.end(); ++it) {
        RegInfo srcReg = it->second;
        string fn = strTable.get(srcReg.fnId);
        if (fnInCreators(fn, BE_CREATORS, BE_CREATORS_SIZE)
                && srcReg.instOp == ADD
                && srcReg.value == element->address) {
            map<int,RegInfo>::iterator it2;
            for (it2 = targetDesRegs.begin(); it2 != targetDesRegs.end(); ++it2) {
                RegInfo desReg = it2->second;
                if (desReg.instId == srcReg.instId) {
                    size = desReg.value - srcReg.value;
                    break;
                }
            }
            if (size != ADDRINT_INVALID) {
                break;
            }
        }
    }

    //assert (size != ADDRINT_INVALID);
    if (size != ADDRINT_INVALID) {
        element->size = size;
    }
    else {
        // TODO: Temporarily default to set element's size to 48 bytes if no
        // size was found. 48 bytes is the smallest element size of V8.
        // We can also get the size of elements by analysing the instructions
        // that belong to "v8::internal::compiler::InstructionSelector::VisitBlock"
        // function. Fix it later.
        element->size = 48;
    }
}


/**
 * Function: get_inputs_v8
 * Description: This function extracts the sizes of oprands by analyzing
 * function argument values.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - element (BElement): Model of back-end element.
 * Output: None.
 **/
void get_operand_sizes_v8(UINT32 fnId, BElement *element) {

    int arg_num = 1;

    map<int,MWInst>::iterator it;
    for (it = targetMWs.begin(); it != targetMWs.end(); ++it) {
        MWInst write = it->second;
        string fn = strTable.get(write.fnId);
        if (fnInFormers(fn) && write.opcode == PUSH) {
            if (arg_num == 2) {
                // TODO: Fails with V8 version < 8.0. Temporary Comment Out.
                // assert (write.value == element->opcode);
                //if (write.value != element->opcode) {
                //    cout << "write.value != element->opcode: " << hex << write.value << ", " << element->opcode << endl;
                //}
            }
            else if (arg_num == 3) {
                element->output_size = write.value;
            }
            else if (arg_num == 5) {
                element->input_size = write.value;
            }
            else if (arg_num == 6) {
                // TODO: Fails with V8 version < 8.0. Temporary Comment Out.
                // assert (write.value == element->address);
                //if (write.value != element->opcode) {
                //    cout << "write.value != element->opcode: " << hex << write.value << ", " << element->opcode << endl;
                //}
            }
            arg_num++;
        }
    }
}

/* API functions End */

/* Tracer(.h) functions Begin */

/**
 * Function: recordSrcRegs
 * Description: Records information on source registers of an instruction.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - ctx (CONTEXT*): A structure that keeps architectural state of the processor.
 *  - srcRegs (RegVector*): Source register information holder.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - instOpcode (UINT32): Instruction opcode.
 * Output: None
 **/
void PIN_FAST_ANALYSIS_CALL recordSrcRegs(
        THREADID tid, const CONTEXT *ctx, const RegVector *srcRegs, UINT32 fnId,
        UINT32 instOpcode) {

    // In case srcRegSize was not reset, reset it to zero.
    srcRegSize = 0;

    UINT8 buf[LARGEST_REG_SIZE];
    for(UINT8 i = 0; i < srcRegs->getSize(); i++) {
        LynxReg lReg = srcRegs->at(i);
        LynxReg fullLReg = fullLynxReg(lReg);
        UINT32 fullSize = lynxRegSize(fullLReg);
        REG reg = LynxReg2Reg(fullLReg);
        PIN_GetContextRegval(ctx, reg, buf);

        UINT8 RegValue[fullSize];
        PIN_SafeCopy(RegValue, buf, fullSize);

        ADDRINT regValueInt = uint8Toaddrint(RegValue, fullSize);

        // Create a new srcRegs object.
        RegInfo srcReg;
        srcReg.instId = instruction.id;
        srcReg.fnId = fnId;
        srcReg.instOp = instOpcode;
        srcReg.lReg = fullLReg;
        srcReg.value = regValueInt;

        assert (srcRegSize < MAX_REGS);
        srcRegsHolder[srcRegSize] = srcReg;
        srcRegSize++;
    }
}

/**
 * Function: recordDestRegs
 * Description: Records the instruction's destination registers and flags in thread local 
 * storage so that they can be printed after the instruction executes.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - destRegs (RegVector*): Destination register information vector.
 *  - destFalgs (UINT32): Destination register flag.
 * Output: None
 **/
void PIN_FAST_ANALYSIS_CALL recordDestRegs(
        THREADID tid, const RegVector *destRegs, UINT32 destFlags) {
    ThreadData &data = tls[tid];
    data.destFlags = destFlags;
    data.destRegs = destRegs;
}

/**
 * Function: recordMemRead
 * Description: This function needs to be executed regardless of whether we are writing memory reads 
 *  into the trace. It checks to make sure we've already seen a memory value at this address. If not,
 *  it needs to be recorded in the trace (since it won't be in the reader's shadow architecture). 
 *  It also checks to see if we are currently looking at a special memory region that gets updated by
 *  the kernel. In this case, we won't know if we saw the current value before so we also must always
 *  print out those values.
 * Input:
 *  - readAddr (ADDRINT): Memory read location address.
 *  - readSize (UINT32): Size of readAddr.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output: None
 **/
void recordMemRead(ADDRINT readAddr, UINT32 readSize, UINT32 fnId, UINT8* binary, ADDRINT instSize) {

    string fn = strTable.get(fnId);

    UINT8 value[MAX_MEM_OP_SIZE];
    PIN_SafeCopy(value, (UINT8 *) (readAddr), readSize);

    ADDRINT valueInt = uint8Toaddrint(value, readSize);

    // Create new object to hold memory read information.
    MRInst read;
    read.fnId = fnId;
    read.location = readAddr;
    read.value = valueInt;
    read.valueSize = readSize;
    // Store it in the reads map.
    assert(reads.size()+1 < reads.max_size());
    reads[readAddr] = read;
    // Track the last added key.
    lastMemReadLoc = readAddr;
    // Mark that the tool needs to update the register.
    populate_regs = true;

    PIN_MutexUnlock(&dataLock);

    int element_id = get_accessing(read.location, read.value);
    if (element_id != INT_INVALID) {
        ADDRINT value = get_value(read.value);
        ACCESS aType = updateElementRead(BackEnd->elements[element_id], fnId, read.location, value);
        if (aType > ACCESS_INVALID) {
            updateLogInfo(BackEnd->elements[element_id], fnId, binary, instSize, aType);
        }
    }
}

/**
 * Function: record2MemRead
 * Description: This function simply calls recordMemRead twice.
 * Input:
 *  - readAddr1 (ADDRINT): Address of first memory read location address.
 *  - readAddr2 (ADDRINT): Address of second memory read location address.
 *  - readSize (UINT32): Size of memory reads.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 * Output: None.
 **/
void record2MemRead(
        ADDRINT readAddr1, ADDRINT readAddr2, UINT32 readSize, UINT32 fnId, 
        UINT8* binary, ADDRINT instSize)
{
    recordMemRead(readAddr1, readSize, fnId, binary, instSize);
    recordMemRead(readAddr2, readSize, fnId, binary, instSize);
}
/**
 * Function: recordMemWrite
 * Description: We can only get the address of a memory write from PIN before an instruction
 * executes. Since we need this information after the instruction executes (so we can get the new
 * values), we need to record the address and size of the memory write in thread local storage so
 * we can get the information later.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - addr (ADDRINT): Memory write location address.
 *  - size (UINT32): Size of memory write value.
 * Output: None
 **/
void recordMemWrite(THREADID tid, ADDRINT addr, UINT32 size) {
    // For a memory write all we need to do is record information about it
    // so after the instruction we can print it
    ThreadData &data = tls[tid];
    data.memWriteAddr = addr;
    data.memWriteSize = size;

    PIN_MutexLock(&dataLock);
    mem.setSeen(addr, size);
    PIN_MutexUnlock(&dataLock);
}

/**
 * Function: analyzeMemWrites
 * Description: This function analyzes the memory writes of current instruction.
 *  In addition, it calls trackOptimization function to further analyze the memory writes
 *  to seek for potential optimization event happeneing to the IR.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - is_range (bool): Boolean flag to indicate whether the currently analyzing instruction
 *    is within the range of node formation or not.
 *  - system_id (UINT32): JIT compiler system ID. 
 * Output: None.
 **/
void analyzeMemWrites(
        THREADID tid, UINT32 fnId, bool is_range, UINT32 opcode, UINT8* binary,
        ADDRINT instSize, UINT32 system_id) 
{

    ThreadData &data = tls[tid];

    string fn = strTable.get(fnId);

    UINT8 value[data.memWriteSize];
    PIN_SafeCopy(value, (UINT8 *) (data.memWriteAddr), data.memWriteSize);

    // Below code is to fix the bug (likely) caused by the Pin tool.
    // In some cases, bits of the value are off, e.g., 00007f is returned in 007f00.
    // Thus, we are currently fixing the bits manually.
    ADDRINT valueInt = ADDRINT_INVALID;
    if (data.memWriteSize == MEMSIZE) {
        bool is_correct = checkCopiedValue(value, data.memWriteSize);
        if (!is_correct) {
            UINT8 fixed[data.memWriteSize];
            fixCopyValue(value, fixed, data.memWriteSize);
            valueInt = uint8Toaddrint(fixed, data.memWriteSize);
        }
        else {
            valueInt = uint8Toaddrint(value, data.memWriteSize);
        }
    }
    else {
        // Convert value (UINT8) to ADDRINT type value.
        valueInt = uint8Toaddrint(value, data.memWriteSize);
    }

    // Create a new object to hold the memory write and register information of the instruction.
    MWInst write;
    write.instId = instruction.id;
    write.fnId = fnId;
    write.location = data.memWriteAddr;
    write.value = valueInt;
    write.valueSize = data.memWriteSize;
    write.opcode = opcode;
    for (int i = 0; i < srcRegSize; i++) {
        assert (i < MAX_REGS);
        write.srcRegs[i] = srcRegsHolder[i];
    }
    write.regSize = srcRegSize;

    // Collect location and value in MWs only in the node former function instructions.
    if (is_range) {
        assert(targetMWs.size()+1 < targetMWs.max_size());
        targetMWs[targetMWId] = write;
        targetMWId++;
    }

    // Collect write if the current instruction is within the range of IR former function.
    if (is_ir_former_range) {
        IRFormerMWs[write.location] = write;
    }

    // Check if the current write is IR node's opcode update optimization.
    map<ADDRINT,ADDRINT>::iterator it_1;
    it_1 = opAddr2nodeAddr.find(write.location);
    if (it_1 != opAddr2nodeAddr.end() && write.valueSize == V8_OPCODE_SIZE) {
        map<ADDRINT,Node*>::iterator it_2;
        it_2 = irNodeAddrs.find(it_1->second);
        assert (it_2 != irNodeAddrs.end());
        (irNodeAddrs[it_1->second])->opcode = write.value;
    }

    // Check if the current instruction is making an update to one of the back-end models.
    int element_id = get_accessing(write.location, write.value);

    // If it is a back-end model update, update the appropirate element and the log information.
    if (element_id != INT_INVALID) {
        ADDRINT value = get_value(write.value);
        ACCESS aType = updateElementWrite(
                            BackEnd->elements[element_id], fnId, write.location, value);
        if (aType > ACCESS_INVALID) {
            updateLogInfo(BackEnd->elements[element_id], fnId, binary, instSize, aType);
        }

        if (is_function_entry && BackEnd->elements[element_id]->address == valueInt) {
            is_function_entry = false;
            current_elem_id = element_id;
        }
    }

    // Check if the current instruction is writing a native (jitted) code to the buffer.
    if (current_elem_id != INT_INVALID && is_writing_to_buffer(write.location)) {
        ADDRINT offset = write.location - code_buffer.start;
        Code code;
        code.value = valueInt;
        code.size = data.memWriteSize;
        code.fnId = fnId;
        code.instId = instruction.id;
        BackEnd->elements[current_elem_id]->native_code[offset] = code;
        // We need to record the function that writes to the buffer. The record
        // needs to be kept in the element's history.
        updateLogInfo(BackEnd->elements[current_elem_id], fnId, binary, instSize, JIT_CODE_WRITE);
    }

    // Add the 'write' object to the 'writes' map.
    assert(writes.size()+1 < writes.max_size());
    writes[data.memWriteAddr] = write;
}

/**
 * Function: analyzeRegWrites
 * Description: This function analyzes register writes of the current instruction.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - ctx (CONTEXT*): A structure that keeps architectural state of the processor.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - opcode (UINT32): Instruction opcode.
 * Output: None.
 **/
void analyzeRegWrites(
        THREADID tid, const CONTEXT *ctx, UINT32 fnId, UINT32 opcode, bool is_range, 
        UINT8* binary, ADDRINT instSize) {

    ThreadData &data = tls[tid];

    desRegSize = 0;

    REG reg;
    UINT8 buf[LARGEST_REG_SIZE];
    const RegVector *writeRegs = data.destRegs;
    for(UINT8 i = 0; i < writeRegs->getSize(); i++) {
        LynxReg lReg = writeRegs->at(i);
        LynxReg fullLReg = fullLynxReg(lReg);
        UINT32 fullSize = lynxRegSize(fullLReg);
        reg = LynxReg2Reg(fullLReg);
        PIN_GetContextRegval(ctx, reg, buf);

        UINT8 RegValue[fullSize];
        PIN_SafeCopy(RegValue, buf, fullSize);

        ADDRINT regValueInt = uint8Toaddrint(RegValue, fullSize);

        string LynRegStr = LynxReg2Str(fullLReg);
        // Update currentRaxVal.
        if (LynRegStr == TARGET_REG) {
            if (checkRAXValue(RegValue)) {
                assert (fullSize > 0);
                raxValue.value = regValueInt;
                raxValue.fnId = fnId;

                string fn = strTable.get(fnId);
                if (fnInCodeBufferAllocs(fn)) {
                    malloc_rax_addr = regValueInt;
                }
            }
        }

        // Create a new desRegs object.
        RegInfo desReg;
        desReg.instId = instruction.id;
        desReg.fnId = fnId;
        desReg.instOp = opcode;
        desReg.opcode = get_inst_opcode(binary, instSize);
        desReg.lReg = fullLReg;
        desReg.value = regValueInt;

        assert (desRegSize < MAX_REGS);
        desRegsHolder[desRegSize] = desReg;
        desRegSize++;

        if (is_range) {
            targetDesRegs[targetDesRegsKey] = desReg;
            targetDesRegsKey++;
        }
    }
}

/**
 * Function: analyzeRecords
 * Description: This function analyzes all the recorded information for the instruction.
 * Input:
 *  - tid (THREADID): Thread ID struct object.
 *  - ctx (CONTEXT*): A structure that keeps architectural state of the processor.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - opcode (UINT32): Instruction opcode.
 *  - is_create(bool): Flag indicating whether the current instruction belongs node creator
 *  function or not.
 *  - system_id (UINT32): JIT compiler system ID.
 * Output: true if labeled, false otherwise.
 **/
bool analyzeRecords(
        THREADID tid, const CONTEXT *ctx, UINT32 fnId, UINT32 opcode,
        bool is_create, UINT8* binary, ADDRINT instSize, UINT32 system_id,
        ADDRINT execution_addr, UINT32 srcId, bool is_ir_create) {

    ThreadData &data = tls[tid];

    // Check if current instruction is within the node creation range.
    // If true, set the is_former_range, which is the variable to keep track of the range, to true.
    if (!is_former_range && is_create) {
        is_former_range = true;
    }

    if (!is_ir_former_range && is_ir_create) {
        is_ir_former_range = true;
    }

    // Retrieve the function name.
    string fn = strTable.get(fnId);

    // If the instruction has register write, then analyze register write, e.g., W:RAX=...
    // Note: We only care about the last RAX register value of node allocator
    // function instructions.
    if(data.destRegs != NULL) {
        analyzeRegWrites(tid, ctx, fnId, opcode, is_former_range, binary, instSize);
        data.destRegs = NULL;
    }

    // If the instruction has memory write, analyze memory write, e.g., MW[..]=...
    if(data.memWriteSize != 0) {
        analyzeMemWrites(tid, fnId, is_former_range, opcode, binary, instSize, system_id);
        data.memWriteSize = 0; 
    }

    // If the instruction has memory read, populate the source register
    // information to the MR tracker object.
    if (populate_regs) {
        for (int i = 0; i < srcRegSize; i++) {
            assert(i < MAX_REGS);
            reads[lastMemReadLoc].srcRegs[i] = srcRegsHolder[i];
        }
        reads[lastMemReadLoc].regSize = srcRegSize;
        populate_regs = false;
    }

    // If the instruction is within the target function(s) range, separately
    // hold the source registers info.
    if (is_former_range) {
        for (int i = 0; i < srcRegSize; i++) {
            targetSrcRegs[targetSrcRegsKey] = srcRegsHolder[i];
            targetSrcRegsKey++;
        }
    }

    // Check and extract the start & end addresses of the code buffer.
    // if (entered_assembler[0] && !entered_assembler[1]) {
    if (fnInAssemblers(fn) && malloc_rax_addr != ADDRINT_INVALID) {

        assert (malloc_rax_addr != ADDRINT_INVALID);

        code_buffer.start = malloc_rax_addr;
        code_buffer.end = code_buffer.start + code_buffer.size;
        code_buffer.pc = code_buffer.start;

        malloc_rax_addr = ADDRINT_INVALID;
    }

    // Check if the current instruction is an entry to the code assembler function.
    if (fnInCodeAssemblers(fn)) {
        ADDRINT op = get_inst_opcode(binary, instSize);
        if (op == FUNCTION_ENTER) {
            is_function_entry = true;
        }
    }

    memset(srcRegsHolder, 0, srcRegSize);
    srcRegSize = 0;
    memset(desRegsHolder, 0, desRegSize);
    desRegSize = 0;

    PIN_MutexLock(&traceLock);
    data.eventId = eventId;
    eventId++;

    // Update instruction id
    instruction.id++;

    PIN_MutexUnlock(&traceLock);

    //mark that we printed the instruction
    bool labeled = false;
    data.print = false;
    return labeled;
}

/* Tracer(.h) functions End */

/* Update functions Begin */

/**
 * Function: UpdateLogInfo
 * Description: This function updates the element's instInfo map.
 * Input:
 *  - element (BElement*): model of back-end representation.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - binary (UINT8*): execution instruction's opcode & operand information.
 *  - instSize (ADDRINT): binary size.
 *  - aType (Access): Access type enum value.
 * Output: None.
 **/
void updateLogInfo(BElement *element, UINT32 fnId, UINT8* binary, ADDRINT instSize, ACCESS aType) {

    // Create a new instInfo object and update it.
    InstInfo instInfo;
    instInfo.instId = instruction.id;
    if (aType == CREATE) {
        std::vector<UINT32>::iterator it = function_call_stack.end();
        instInfo.fnId = *(it-1);
    }
    else {
        instInfo.fnId = fnId;
    }
    instInfo.binary = new UINT8[instSize];
    memcpy(instInfo.binary, binary, instSize);
    instInfo.instSize = instSize;
    instInfo.aType = aType;
    
    element->instInfo[element->lastInfoId] = instInfo;
    element->lastInfoId++;
}

/**
 * Function: updateElementWrite
 * Description: This function updates the element's structure (or value) based on
 * on the modeled JIT compiler's behavior.
 * Input:
 *  - element (BElement*): model of back-end representation.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - location (ADDRINT): memory location that the memory write is happening.
 *  - value (ADDRINT): value written to the lcoation. 
 * Output:
 *  - aType (int): access type that the JIT compiler made to the element.
 **/
ACCESS updateElementWrite(BElement *element, UINT32 fnId, ADDRINT location, ADDRINT value) {

    ACCESS aType = ACCESS_INVALID;

    Update update;
    update.fnId = fnId;
    update.instId = instruction.id;

    bool is_element = false;
    bool is_exists = false;
    bool is_elem_write = false;

    map<ADDRINT,ADDRINT>::iterator it;
    for (it = element->addr2val.begin(); it != element->addr2val.end(); ++it) {
        ADDRINT written_loc = it->first;
        ADDRINT written_val = it->second;

        if (location == written_loc) {
            update.location = location;
            update.from_value = written_val;
            update.to_value = value;
            element->addr2val[location] = value;
            element->updateInfo[element->lastInfoId] = update;

            aType = UPDATE_VALUE;
            is_exists = true;
        }
        else if (value == element->address) {
            element->writtenLoc = location;

            aType = ELEM_WRITE;
            is_elem_write = true;
        }
    }

    if (location > element->address && location < (element->address)+(element->size)) {
        is_element = true;
    }

    if (!is_exists && !is_elem_write && is_element) {
        update.location = location;
        update.from_value = ADDRINT_INVALID;
        update.to_value = value;
        element->addr2val[location] = value;
        element->updateInfo[element->lastInfoId] = update;

        aType = NEW_VALUE;
    }

    return aType;
}

/**
 * Function: updateElementRead
 * Description: This function updates element read information.
 * Input:
 *  - element (BElement*): model of back-end representation.
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - location (ADDRINT): memory location that the memory write is happening.
 *  - value (ADDRINT): value written to the lcoation. 
 * Output:
 *  - aType (int): access type that the JIT compiler made to the element.
 **/
ACCESS updateElementRead (BElement *element, UINT32 fnId, ADDRINT location, ADDRINT value) {

    ACCESS aType = MEM_READ;

    if (location >= element->address && location < (element->address)+(element->size)) {
        Update update;
        update.fnId = fnId;
        update.instId = instruction.id;

        update.location = location;
        update.from_value = value;

        element->readInfo[element->lastInfoId] = update; 
    } 

    return aType;
}

/* Update functions End */

/**
 * Function: fnInCreators
 * Description: This function checks whether the passed function name is an element
 *  creator function or not.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a node creator function, false otherwise.
 **/
bool fnInCreators(string fn, const string *creators, const int size) {

    bool is_exists = false;
    for (int i = 0; i < size; i++) {
        if (creators[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: fnInFormers
 * Description: This function checks whether the passed function name is a node
 *  former function or not.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a node block former function, false otherwise.
**/
bool fnInFormers(string fn) {

    bool is_exists = false;
    for (int i = 0; i < FORMERS_SIZE; i++) {
        if (FORMERS[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: fnInNonIRAllocs
 * Description: This function checks whether the passed function name is a non-IR
 *  object allocator or not.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a non-IR object allocator, flase otherwise.
 **/
bool fnInNonIRAllocs(string fn) {

    bool is_exists = false;
    for (int i = 0; i < NONIR_NODE_ALLOC_SIZE; i++) {
        if (NONIR_NODE_ALLOCATORS[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: fnInCodeBufferAllocs
 * Description: This function checks whether the passed function name is a buffer
 * allocation, which the buffer is to hold the jitted code.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a buffer allocator, flase otherwise.
 **/
bool fnInCodeBufferAllocs(string fn) {

    bool is_exists = false;
    for (int i = 0; i < CODE_BUFFER_ALLOCATORS_SIZE; i++) {
        if (CODE_BUFFER_ALLOCATORS[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: fnInAssemblers
 * Description: This function checks whether the passed function name is in
 * the assembler function.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a assembler, flase otherwise.
 **/
bool fnInAssemblers(string fn) {

    bool is_exists = false;
    for (int i = 0; i < ASSEMBLERS_SIZE; i++) {
        if (ASSEMBLERS[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: fnInAssemblers
 * Description: This function checks whether the passed function name is in
 * the code assembler function.
 * Input:
 *  - fn (string): Function name string.
 * Output: true if the function is a code assembler, flase otherwise.
 **/
bool fnInCodeAssemblers(string fn) {

    bool is_exists = false;
    for (int i = 0; i < CODE_ASSEMBLERS_SIZE; i++) {
        if (CODE_ASSEMBLERS[i] == fn) {
            is_exists = true;
            break;
        }
    }

    return is_exists;
}

/**
 * Function: track_function_calls
 * Description: This function keesp a track of function calls and updates
 * the function call stack appropriately.
 * Input:
 *  - fnId (UINT32): ID of a function that currently analyzing instruction belongs to.
 *  - is_ret (bool): Flag indicating whether the current instruction is a function return or not.
 * Output: true if the function is a code assembler, flase otherwise.
 **/
void track_function_calls(UINT32 fnId, bool is_ret) {

    string fn = strTable.get(fnId);

    // If the call stack is empty or the current function (id) differs to the
    // last element, i.e., latest called function's id, of the stack, add the
    // fnId to the stack as it is function enter.
    if (
            (fn.find("v8") != std::string::npos)
            && (function_call_stack.empty() || fnId != function_call_stack.back())) {
        function_call_stack.push_back(fnId);
    }
    // Else if the current instruction is a function return and the current
    // fnId is equal to the last element in the stack, it represents the
    // function return. Thus, pop (remove) the last element of the stack.
    else if (
            (fn.find("v8") != std::string::npos) 
            && (is_ret && fnId == function_call_stack.back())) {
        function_call_stack.pop_back();
    }
}

/* Helper Functions Begin */

/**
 * Function: uint8Toaddrint
 * Description: This function converts value that is in UINT32 type to ADDRINT type.
 * Input:
 *  - target (UINT8*): Target value to convert to ADDRINT type.
 *  - size (UINT32): Size of the target value.
 * Output: ADDRINT type converted value.
 **/
ADDRINT uint8Toaddrint(UINT8* target, UINT32 size) {
    
    assert (size > 0);

    ADDRINT to = 0;
    for (int i = size-1; i >= 0; i--) {
        to = (to << 8) | target[i];
    }
    
    return to;
}

/**
 * Function: uint8Tostring
 * Description: This function converts array of UINT8 type values to string.
 * Input:
 *  - target (ADDRINT): Target value to convert to string type.
 *  - size (UINT32): Size of the target value.
 * Output: String type converted value.
 **/
string uint8Tostring(UINT8* target, ADDRINT size) {
    ostringstream strStream;
    for (int i = size-1; i > 0; i--) {
        strStream << hex << (int)target[i] << " ";
    }
    strStream << hex << (int)target[0];

    return strStream.str();
}

/**
 * Function: addrintTouint8
 * Description: This function converts value that is in ADDRINT type to UINT32 type.
 * Input:
 *  - target (ADDRINT): Target value to convert to UINT8 type.
 *  - size (UINT32): Size of the target value.
 * Output: UINT32 type converted value.
 **/
UINT8* addrintTouint8(ADDRINT target, UINT32 size) {
    UINT8 *to = new UINT8();
    memcpy(to, &target, size);

    return to;
}

/**
 * Function: reverseUint8
 * Description:
 **/
UINT8* reverseUint8(UINT8* target, ADDRINT size) {
    UINT8 reversed[size];
    UINT8 *ptr = reversed;

    int j = 0;
    for (int i = size-1; i >= 0; i--) {
        ptr[j] = target[i];
        j++;
    }

    return ptr;
}

/**
 * Function: checkCopiedValue
 * Description: Check a value copied by PIN_SafeCopy. If the copied value has
 *  an incorrect format, e.g., 0040032c3bef7f00, where the first byte of '00' is
 *  moved to the end, return false. Otherwise, true.
 * Input:
 *  - value (UINT8*): Value to check the format.
 *  - size (UINT32): Size of value.
 * Output: true if the format is correct, false otherwise.
 **/
bool checkCopiedValue(UINT8 *value, UINT32 size) {

    bool is_correct = true;

    if (size == MEMSIZE) {
        if (value[7] == 0x00 && value[6] != 0x00 && value[0] == 0x00) {
            is_correct = false;
        }
    }

    return is_correct;
}

/**
 * Function: fixCopyValue
 * Description: Fix the value to the correct format if checkCopiedValue returned false
 *  on the input value, e.g., fix 0040032c3bef7f00 to 40032c3bef7f0000.
 * Input:
 *  - buggy (UINT8*): Buggy format value to fix.
 *  - fixed (UINT8*): Fixed value.
 *  - size *UINT32): Size of value.
 * Output: None.
*/
void fixCopyValue(UINT8 *buggy, UINT8 *fixed, UINT32 size) {

    assert (sizeof(buggy) == size);

    // Copy only the 6 bytes between the first and last 00s.
    UINT32 fix_idx = 0;
    for (UINT32 buggy_idx = 1; buggy_idx < size-1; buggy_idx++) {
        assert (buggy_idx < sizeof(buggy));
        fixed[fix_idx] = buggy[buggy_idx];
        fix_idx++;
    }
    // Manually add two 00s at the end of the fix array.
    fixed[fix_idx+1] = 0x00;
    fixed[fix_idx+2] = 0x00;
}

/**
 * Function: checkRAXValue
 * Description: This function checks the format of RAX value.
 *  This function is needed because the Pin Tool (for some unknown reason)
 *  messes up the byte location,
 *  e.g., Correct format: 00007f25103b2528; Incorrect format: 007f25103b252800.
 * Input:
 *  - value (UINT32*): Value to check the format.
 * Output: true if the value format is correct, false otherwise.
 **/
bool checkRAXValue(UINT8 *value) {

    bool is_valid = true;

    if (value[2] == 0x00) {
        is_valid = false;
    }

    return is_valid;
}

/**
 * Function: get_inst_opcode
 * Description:
 * Input:
 * Output:
 **/
ADDRINT get_inst_opcode(UINT8* binary, ADDRINT instSize) {

    assert (instSize > 0);

    ADDRINT op = (0 << 8) | binary[0];

    return op;
}


/**
 * Function: get_accessing
 *
 **/
int get_accessing(ADDRINT location, ADDRINT value) {

    int id = INT_INVALID;

    for (int i = 0; i < BackEnd->lastELMId; i++) {
        BElement *element = BackEnd->elements[i];
        if (
                (location >= element->address
                 && location < (element->address + element->size)) ||
                (value >= element->address
                 && value < (element->address + element->size))
        ) {
            id = i;
            return id;
        }

    }

    return id;
}

/**
 * Function: get_value
 * Description:
 * Input:
 * Output:
 **/
ADDRINT get_value(ADDRINT value) {

    ADDRINT _value = ADDRINT_INVALID;

    map<ADDRINT,MWInst>::iterator it;
    it = writes.find(value);
    if (it != writes.end()) {
        _value = get_value(it->second.value);
    }
    else {
        _value = value;
    }

    return _value;
}

bool is_writing_to_buffer(ADDRINT location) {

    return location > code_buffer.start && location < code_buffer.end;
}

void checkAllCodes() {

    // Populate the buffer_offset2value by analyzing the writes.
    map<ADDRINT,MWInst>::iterator it;
    for (it = writes.begin(); it != writes.end(); ++it) {
        if (is_writing_to_buffer(it->first)) {
            ADDRINT offset = it->first - code_buffer.start;
            MWInst write = it->second;
            Code code;
            code.value = write.value;
            code.size = write.valueSize;
            code.fnId = write.fnId;
            code.instId = write.instId;
            if (write.valueSize != 8) {
                buffer_offset2value[offset] = code;
            }
        }
    }
    
    for (int i = 0; i < BackEnd->lastELMId; i++) {
        BElement *element = BackEnd->elements[i];
        map<ADDRINT,Code>::iterator code_it;
        ADDRINT prev_offset = ADDRINT_INVALID;
        map<ADDRINT,Code> _temp;
        for (code_it = element->native_code.begin(); code_it != element->native_code.end(); ++code_it) {
            if (prev_offset != ADDRINT_INVALID) {
                if (code_it->first-1 == prev_offset) {
                    _temp[code_it->first] = code_it->second;
                    prev_offset++;
                }
            }
            else {
                _temp[code_it->first] = code_it->second;
                prev_offset = code_it->first;
            }

        }
        element->native_code.clear();
        map<ADDRINT,Code>::iterator _temp_it;
        for (_temp_it = _temp.begin(); _temp_it != _temp.end(); ++_temp_it) {
            if ((_temp_it->second).size == 1) {
                element->native_code[_temp_it->first] = _temp_it->second;
            }
            else if ((_temp_it->second).size != 8) {
                UINT8 *val = addrintTouint8((_temp_it->second).value, (_temp_it->second).size);
                UINT8 *reversed = reverseUint8(val, (_temp_it->second).size);

                ADDRINT offset = _temp_it->first+1;
                for (ADDRINT i = 0; i < (_temp_it->second).size; i++) {
                    Code code;
                    code.value = uint8Toaddrint(reversed+i, 1);
                    code.size = 1;
                    code.fnId = (_temp_it->second).fnId;
                    code.instId = (_temp_it->second).instId;
                    element->native_code[offset] = code;
                    offset++;
                }
            }
            /*else {
                Code code;
                code.value = get_value((_temp_it->second).value);
                code.size = 0;
                element->native_code[_temp_it->first] = code;
            }*/
        }
    }
}

bool check_jit_code_entry(UINT32 srcId) {

    string src = strTable.get(srcId);

    if (src == "Unknown") {
        is_jit_code_entry = true;
        return true;
    }

    return false;
}

/* Helper Functions End */

/**
 * Function: printUINT8
 * Description: This function prints UINT8 type value in format.
 * Input:
 *  - arr (UINT8*): UINT8 type value array.
 *  - size (UINT32): Size of value.
 * Output: None.
 **/
void printUINT8(UINT8* arr, UINT32 size) {

    assert (size > 0);

    for (int i = size-1; i > 0; i--) {
        printf("%02x ", arr[i]);
    }
    printf("%02x\n", arr[0]);
}

/**
 * Function: getFileBuf
 * Description: Get a buffer for the file that is guaranteed to fit the given size. Must provide the file's
 *  buffer and the current position in that buffer. 
 * Side Effects: Writes to file if there is not enough space in buffer
 * Output: the position in the buffer that it is safe to write to
 **/
UINT8 *getFileBuf(UINT32 size, UINT8 *fileBuf, UINT8 *curFilePos, FILE *file) {
    UINT16 bufSize = curFilePos - fileBuf;
    if((bufSize + size) > BUF_SIZE) {
        fwrite(fileBuf, UINT8_SIZE, bufSize, file);
        curFilePos = fileBuf;
    }

    return curFilePos;
}

/**
 * Function: startTrace
 * Description: Function used to mark when to start tracing.
 * Output: None
 **/
void startTrace() {
    printTrace = true;
}

/**
 * Function: printDataLabel
 * Description: Writes a data label with the given eventId to the buffer specified by pos
 * Assumptions: There is enough space in the buffer
 * Output: New current position in buffer
 **/
UINT8 *printDataLabel(UINT8 *pos, UINT64 eventId) {
    *pos = (UINT8) OP_LABEL;
    pos += UINT8_SIZE;

    *((UINT64 *) pos) = eventId;
    pos += UINT64_SIZE;

    return pos;
}

/**
 * Function: printMemData
 * Description: Writes a MemData entry into the location specified by pos given the memory's 
 *  address, value and size. Additionally, if sizePos is provided, it set the sizePos's value to
 *  the location of size in the buffer.
 * Assumptions: There is enough space in the buffer
 * Output: New current position in buffer
 **/
UINT8 *printMemData(UINT8 *pos, UINT16 size, ADDRINT addr, UINT8 *val, UINT8 **sizePos) {
    *pos = (UINT8) OP_MEM;
    pos += UINT8_SIZE;

    if(sizePos != NULL) {
        *sizePos = pos;
    }

    *((UINT16 *) pos) = size;
    pos += UINT16_SIZE;

    *((ADDRINT *) pos) = addr;
    pos += ADDRINT_SIZE;

    if(size != 0 && val != NULL) {
        memcpy(pos, val, size);
        pos += size;
    }

    return pos;
}

/**
 * Function: printExceptionEvent
 * Description: Writes an exception event into the location specified by pos
 * Assumptions: There is enough space in the buffer
 * Output: New current position in buffer
 **/
UINT8 *printExceptionEvent(UINT8 *pos, ExceptionType eType, INT32 info, THREADID tid, ADDRINT addr) {
    UINT8 *buf = pos;

    *pos = EVENT_EXCEPT; //event_type
    pos += UINT8_SIZE;

    *pos = eType; //type
    pos += UINT8_SIZE;

    *((INT32 *) pos) = info;
    pos += sizeof(INT32);

    *((UINT32 *) pos) = tid;
    pos += UINT32_SIZE;

    *((ADDRINT *) pos) = addr;
    pos += ADDRINT_SIZE;

    pos += UINT16_SIZE;

    *((UINT16 *)(pos - UINT16_SIZE)) = (UINT16)(pos - buf);

    return pos;
}

/**
 * Function: printDataReg
 * Description: Writes a data register entry for lReg into the buffer location specified by pos.
 *  It also fills in valBuf with the value of the register
 * Assumptions: There is enough space in the buffer, valBuf is at least 64 bytes
 * Side Effects: Fills in valBuf with register value
 * Output: New current position in buffer
 **/
UINT8 *printDataReg(THREADID tid, UINT8 *pos, LynxReg lReg, const CONTEXT *ctxt, UINT8 *valBuf) {
    REG reg = LynxReg2Reg(lReg);

    PIN_GetContextRegval(ctxt, reg, valBuf);

    *pos = (UINT8) OP_REG;
    pos += UINT8_SIZE;

    *((UINT32 *) pos) = tid;
    pos += UINT32_SIZE;

    *pos = (UINT8) lReg;
    pos += UINT8_SIZE;

    UINT32 size = lynxRegSize(lReg);
    memcpy(pos, valBuf, size);
    pos += size;

    return pos;
} 

/**
 * Function: checkInitializedStatus
 * Description: Checks to see if we have printed out the initial status of a thread.
 * Output: True if initialized, false otherwise
 **/
bool checkInitializedStatus(THREADID tid) {
    return !tls[tid].initialized;
}

/**
 * Function: initThread
 * Description: Records the initial state of a thread in the trace
 * Output: None
 **/
void initThread(THREADID tid, const CONTEXT *ctx) {
    if(!tls[tid].initialized) {
        PIN_MutexLock(&dataLock);
        recordRegState(tid, ctx);
        PIN_MutexUnlock(&dataLock);
        tls[tid].initialized = true;
    }
}

/**
 * Function: initIns
 * Description: Initializes the thread local storage for a new instruction. 
 * Output: None
 **/
void PIN_FAST_ANALYSIS_CALL initIns(THREADID tid) {
    ThreadData &data = tls[tid];
    data.pos = data.buffer + UINT8_SIZE;
    data.dataPos = data.dataBuffer;

    //save instruction info in TLS
    *((UINT8 *) data.pos) = EVENT_INS;
    data.predRecord = data.pos;
    data.pos += UINT8_SIZE;

    //be predict everythign is right
    *data.predRecord |= 0xf8;

    //mark that this instruction needs to be printed
    data.print = true;
}

/**
 * Function: recordSrcId
 * Description: Records the source ID of the instruction in the thread local storage's trace buffer. 
 *  After the instruction executes, this buffer will be written to the trace.
 * Output: None
 **/
void PIN_FAST_ANALYSIS_CALL recordSrcId(THREADID tid, UINT32 srcId) {
    ThreadData &data = tls[tid];
    bool incorrect = (srcId != data.predSrcId);
    data.predSrcId = *((UINT32 *) data.pos) = srcId;
    *data.predRecord &= ~((incorrect) << PRED_SRCID);
    data.pos += (incorrect) ? UINT32_SIZE : 0;
}

/**
 * Function: contextChange
 * Description: Records information in a trace if an exception occurred, resulting in a context change. 
 *  Note, this is the only place that event ids are adjusted due to an exception event.
 * Output: None
 **/
void contextChange(THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *fromCtx, CONTEXT *toCtx, INT32 info, void *v) {
    if(printTrace) {
        //check to see if an exception occurred. If so, print out info about it
        if(reason == CONTEXT_CHANGE_REASON_FATALSIGNAL || reason == CONTEXT_CHANGE_REASON_SIGNAL) {
            PIN_MutexLock(&traceLock);
            traceBufPos = getFileBuf(32, traceBuf, traceBufPos, traceFile);
            *traceBufPos = numSkipped;
            traceBufPos += 1;
            traceBufPos = printExceptionEvent(traceBufPos, LINUX_SIGNAL, info, tid, PIN_GetContextReg(fromCtx, REG_INST_PTR));
            eventId++;
            PIN_MutexUnlock(&traceLock);
        }
        else if(reason == CONTEXT_CHANGE_REASON_EXCEPTION) {
            PIN_MutexLock(&traceLock);
            traceBufPos = getFileBuf(32, traceBuf, traceBufPos, traceFile);
            *traceBufPos = numSkipped;
            traceBufPos += 1;
            traceBufPos = printExceptionEvent(traceBufPos, WINDOWS_EXCEPTION, info, tid, PIN_GetContextReg(fromCtx, REG_INST_PTR));
            eventId++;
            PIN_MutexUnlock(&traceLock);
        }
        /*Causing issues now because destRegs and destFlags are available, and apparently are not in the context
         else {
         printIns(tid, fromCtx);
         }*/
    }
}

/**
 * Function: recordRegState
 * Description: Records the register state for the current architecture in data file.
 * Output: None
 **/
void recordRegState(THREADID tid, const CONTEXT *ctxt) {
    UINT8 val[LARGEST_REG_SIZE];
    UINT8 *pos = getFileBuf(4096, dataBuf, dataBufPos, dataFile);

#if defined(TARGET_MIC) || defined(TARGET_IA32E)
    for(UINT32 lReg = LYNX_GR64_FIRST; lReg <= LYNX_GR64_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
#endif
#if defined(TARGET_IA32)
    for(UINT32 lReg = LYNX_X86_GR32_FIRST; lReg <= LYNX_X86_GR32_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
#endif
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    for(UINT32 lReg = LYNX_YMM_X86_FIRST; lReg <= LYNX_YMM_X86_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
#endif
#if defined(TARGET_IA32E)
    for(UINT32 lReg = LYNX_YMM_X64_FIRST; lReg <= LYNX_YMM_X64_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
#endif
#if defined(TARGET_MIC)
    for(UINT32 lReg = LYNX_ZMM_FIRST; lReg <= LYNX_ZMM_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
    for(UINT32 lReg = LYNX_K_MASK_FIRST; lReg <= LYNX_K_MASK_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
#endif

    for(UINT32 lReg = LYNX_SEG_FIRST; lReg <= LYNX_SEG_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
    for(UINT32 lReg = LYNX_SSE_FLG_FIRST; lReg <= LYNX_SSE_FLG_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
    for(UINT32 lReg = LYNX_FPU_FIRST; lReg <= LYNX_FPU_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }
    for(UINT32 lReg = LYNX_FPU_STAT_FIRST; lReg <= LYNX_FPU_STAT_LAST; lReg++) {
	    pos = printDataReg(tid, pos, (LynxReg)lReg, ctxt, val);
    }

    //for some reason if I try to print these out, PIN will crash
#if 0
    for(UINT32 lReg = LYNX_DBG_FIRST; lReg <= LYNX_DBG_LAST; lReg++) {
        printReg(tid, (LynxReg)lReg, ctxt, val, file);
    }
    for(UINT32 lReg = LYNX_CTRL_FIRST; lReg <= LYNX_CTRL_LAST; lReg++) {
        printReg(tid, (LynxReg)lReg, ctxt, val, file);
    }
#endif

    dataBufPos = pos;

    //fwrite(buf, UINT8_SIZE, pos - buf, dataFile);

}

/**
 * Function: threadStart
 * Description: Call this when another thread starts so we can accurately track the number of threads 
 *  the program has. We also need to initialize the thread local storage for the new thread. Note, we 
 *  check here to make sure we are still within maxThreads
 * Output: None
 **/
VOID threadStart(THREADID tid, CONTEXT *ctxt, INT32 flags, void *v) {
    if(tid >= maxThreads) {
        fprintf(stderr, "More than %u threads are being used", maxThreads);
        exit(1);
    }
    //we've got another thread
    numThreads++;

    ThreadData &data = tls[tid];
    data.initialized = false;
    data.pos = data.buffer;
    data.dataPos = data.dataBuffer;
    data.predRecord = NULL;
    data.print = false;
    data.destRegs = NULL;
    data.destFlags = 0;
    data.memWriteSize = 0;
}

/**
 * Function: setupFile
 * Description: Opens and sets up any output files. Note, since this is an ASCII trace, it will setup 
 * trace.out according to the Data Ops trace file format.
 * Output: None
 **/
void setupFile(UINT16 infoSelect) {
    traceFile = fopen("trace.out", "wb");
    dataFile = fopen("data.out", "w+b");
    errorFile = fopen("errors.out", "w");

    FileHeader h;
    h.ident[0] = 'U';
    h.ident[1] = 'A';
    h.ident[2] = 'T';
    h.ident[3] = 'R';
    h.ident[4] = 'C';
    h.ident[5] = 2;
    h.ident[6] = 0;
    h.ident[7] = 0;
    h.ident[8] = 0;

    h.traceType = DATA_OPS_TRACE;

#if defined(TARGET_IA32)
    h.archType = X86_ARCH;
#else
    h.archType = X86_64_ARCH;
#endif

    int x = 1;
    char *c = (char *) &x;

    //we only support little-endian right now.
    assert(c);

    h.machineFlags = *c;
    h.sectionNumEntry = 5;

    sectionOff = sizeof(FileHeader);
    UINT32 sectionSize = sizeof(SectionEntry);
    traceHeaderOff = sectionOff + h.sectionNumEntry * sectionSize;
    UINT64 infoSelHeaderOff = traceHeaderOff + sizeof(TraceHeader);
    traceStart = infoSelHeaderOff + sizeof(InfoSelHeader);

    h.sectionTableOff = sectionOff;

    h.sectionEntrySize = sectionSize;


    fwrite(&h, sizeof(FileHeader), 1, traceFile);

    SectionEntry entries[h.sectionNumEntry];

    strncpy((char *) entries[0].name, "TRACE", 15);
    entries[0].offset = traceStart;
    entries[0].type = TRACE_SECTION;

    strncpy((char *) entries[1].name, "DATA", 15);
    entries[1].type = DATA_SECTION;

    strncpy((char *) entries[2].name, "TRACE_HEADER", 15);
    entries[2].type = TRACE_HEADER_SECTION;
    entries[2].offset = traceHeaderOff;
    entries[2].size = sizeof(TraceHeader);

    strncpy((char *) entries[3].name, "STR_TABLE", 15);
    entries[3].type = STR_TABLE;

    /*strncpy((char *) entries[4].name, "SEGMENT_LOADS", 15);
    entries[4].type = SEGMENT_LOADS;*/

    strncpy((char *) entries[4].name, "INFO_SEL_HEAD", 15);
    entries[4].type = INFO_SEL_HEADER;
    entries[4].offset = infoSelHeaderOff;
    entries[4].size = sizeof(InfoSelHeader);

    //entries[5].offest = 

    fwrite(entries, sectionSize, h.sectionNumEntry, traceFile);

    TraceHeader traceHeader;
    fwrite(&traceHeader, sizeof(TraceHeader), 1, traceFile);

    InfoSelHeader infoSelHeader;
    infoSelHeader.selections = infoSelect;
    fwrite(&infoSelHeader, sizeof(InfoSelHeader), 1, traceFile);
}

// ===========================================================

/**
 * Function: write2Json
 * Description: This function writes modeled IR to a JSON file in formatted structure.
 * Input: None.
 * Output: None.
 **/
void write2Json() {

    ofstream jsonFile;
    jsonFile.open("backend.json");

    map<UINT32,string> fnId2Str;

    jsonFile << "{" << endl;
    jsonFile << "   \"is_ir\":false," << endl;
    jsonFile << "   \"elements\": [" << endl;
    for (int i = 0; i < BackEnd->lastELMId; i++) {
        BElement *element = BackEnd->elements[i];
        jsonFile << "       {" << endl;
        // Basic element information.
        jsonFile << "           \"id\":" << dec << element->id << "," << endl; 
        jsonFile << "           \"address\":\"" << hex << element->address << "\"," << endl; 
        jsonFile << "           \"size\":" << dec << element->size << "," << endl; 
        jsonFile << "           \"opcode\":\"" << hex << element->opcode << "\"," << endl; 
        jsonFile << "           \"ir_opcodes\":[" << endl;
        size_t j = 0;
        while (!(element->ir_opcodes).empty()) {
            jsonFile << "               \"" << hex << (element->ir_opcodes).top() << "\"";
            (element->ir_opcodes).pop();
            if (j < (element->ir_opcodes).size()) {
                jsonFile << "," << endl;
            }
            else {
                jsonFile << endl;
            }
        }
        jsonFile << "           ]," << endl;
        jsonFile << "           \"arch_opcode\":\"" << hex << element->arch_opcode << "\"," << endl; 
        // Values written to the element.
        jsonFile << "           \"address-to-value\":{" << endl;
        map<ADDRINT, ADDRINT>::iterator val_it;
        for(val_it = element->addr2val.begin(); val_it != element->addr2val.end();) {
            jsonFile << "               \"" << hex << val_it->first << "\":\"";
            jsonFile << hex << val_it->second << "\"";
            if (++val_it != element->addr2val.end()) {
                jsonFile << "," << endl;
            }
            else {
                jsonFile << endl;
            }
        }
        jsonFile << "           }," << endl;
        // Native Code.
        jsonFile << "           \"generated-code\":{" << endl;
        map<ADDRINT, Code>::iterator code_it;
        for (code_it = element->native_code.begin(); code_it != element->native_code.end();) {
            jsonFile << "               \"" << dec << code_it->first <<"\":[";
            jsonFile << "\"" << hex << (code_it->second).value << "\",";
            jsonFile << "\"" << dec << (code_it->second).fnId << "\",";
            jsonFile << "\"" << dec << (code_it->second).instId << "\"";
            jsonFile << "]";
            if (++code_it != element->native_code.end()) {
                jsonFile << "," << endl;
            }
            else {
                jsonFile << endl;
            }
        }
        jsonFile << "           }," << endl;
        // Element Update (MW) Information.
        jsonFile << "           \"updates\":{" << endl;
        map<int, Update>::iterator update_it;
        for (update_it = element->updateInfo.begin(); update_it != element->updateInfo.end();) {
            Update udt = update_it->second;
            jsonFile << "                \"" << dec << udt.instId << "\":{" << endl;
            jsonFile << "                   \"location\":\"" << hex << udt.location << "\"," << endl;
            jsonFile << "                   \"from\":\"" << hex << udt.from_value << "\"," << endl;
            jsonFile << "                   \"to\":\"" << hex << udt.to_value << "\"" << endl;
            if (++update_it != element->updateInfo.end()) {
                jsonFile << "                }," << endl;
            }
            else {
                jsonFile << "                }" << endl;
            }
        }
        jsonFile << "           }," << endl;
        // Element Read (MR) Information.
        jsonFile << "           \"reads\":{" << endl;
        map<int, Update>::iterator reads_it;
        for (reads_it = element->readInfo.begin(); reads_it != element->readInfo.end();) {
            Update udt = reads_it->second;
            jsonFile << "                \"" << dec << udt.instId << "\":{" << endl;
            jsonFile << "                   \"location\":\"" << hex << udt.location << "\"," << endl;
            jsonFile << "                   \"value\":\"" << hex << udt.from_value << "\"" << endl;
            if (++reads_it != element->readInfo.end()) {
                jsonFile << "                }," << endl;
            }
            else {
                jsonFile << "                }" << endl;
            }
        }
        jsonFile << "           }," << endl;
        // All Element Access History.
        jsonFile << "           \"history\":{" << endl;
        map<int,InstInfo>::iterator hist_it;
        for(hist_it = element->instInfo.begin(); hist_it != element->instInfo.end();) {
            int instId = hist_it->second.instId;
            UINT32 fnId = hist_it->second.fnId;

            fnId2Str[fnId] = strTable.get(fnId);

            UINT8* binary = hist_it->second.binary;
            ADDRINT instSize = hist_it->second.instSize;
            string binString = uint8Tostring(binary, instSize);
            ACCESS aType = hist_it->second.aType;

            jsonFile << "                \"" << dec << instId << "\":{" << endl;
            jsonFile << "                   \"fnId\":\"" << dec << fnId << "\"," << endl;
            jsonFile << "                   \"binary\":\"" << binString << "\"," << endl;
            jsonFile << "                   \"type\":\"" << dec << aType << "\"" << endl;

            if (++hist_it != element->instInfo.end()) {
                jsonFile << "                }," << endl;
            }
            else {
                jsonFile << "                }" << endl;
            }
        }
        jsonFile << "           }" << endl;

        if (i < BackEnd->lastELMId-1) {
            jsonFile << "       }," << endl;
        }
        else {
            jsonFile << "       }" << endl;
        }
    }
    jsonFile << "   ]," << endl;
    jsonFile << "   \"jitBuffer\":{" << endl;
    map<ADDRINT,Code>::iterator buf_it;
    for (buf_it = buffer_offset2value.begin(); buf_it != buffer_offset2value.end();) {
        Code code = buf_it->second;
        jsonFile << "       \"" << dec << buf_it->first << "\":[";
        jsonFile << "\"" << hex << code.value << "\",";
        jsonFile << "\"" << dec << code.fnId << "\"";
        jsonFile << "]";
        if (++buf_it != buffer_offset2value.end()) {
            jsonFile << "," << endl;
        }
        else {
            jsonFile << endl;
        }
    }
    jsonFile << "   }," << endl;
    jsonFile << "   \"fnId2Name\":{" << endl;
    map<UINT32,string>::iterator fn_it;
    for (fn_it = fnId2Str.begin(); fn_it != fnId2Str.end();) {
        jsonFile << "       \"" << dec << fn_it->first << "\":";
        jsonFile << "\"" << fn_it->second << "\"";

        if (++fn_it != fnId2Str.end()) {
            jsonFile << "," << endl;
        }
        else {
            jsonFile << endl;
        }
    }
    jsonFile << "   }" << endl;
    jsonFile << "}" << endl;
}

void std_print() {

    cout << "Instruction: __id__; __representation_opcode__; __architecture_opcode__; ";
    cout << "__jit_code_opcode_and_operands;" << endl;
    for (int i = 0; i < BackEnd->lastELMId; i++) {
        BElement *element = BackEnd->elements[i];
        cout << "Instruction: ";
        cout << dec << element->id << "; " << hex << element->address << "; ";
        cout << hex << element->opcode << "; " << element->arch_opcode << "; ";
        map<ADDRINT,Code>::iterator code_it;
        for (code_it = element->native_code.begin(); code_it != element->native_code.end(); ++code_it) {
            cout << hex << (code_it->second).value << " ";
        }
        cout << endl;

        /*cout << "Address-to-Value:" << endl;
        map<ADDRINT, ADDRINT>::iterator val_it;
        for(val_it = element->addr2val.begin(); val_it != element->addr2val.end(); ++val_it) {
            cout << hex << "   - " << val_it->first << " : " << val_it->second << endl;
        }
        cout << "Update Information:" << endl;
        map<int, Update>::iterator update_it;
        for (update_it = element->updateInfo.begin(); update_it != element->updateInfo.end(); ++update_it) {
            string fn = strTable.get((update_it->second).fnId);

            cout << dec << "   - " << fn << endl;
            cout << hex << "     - 0x" << (update_it->second).location << ": { from: 0x";
            cout << (update_it->second).from_value << ", to: 0x" << (update_it->second).to_value;
            cout << "}" << endl;
        }
        cout << "Read Information:" << endl;
        for (update_it = element->readInfo.begin(); update_it != element->readInfo.end(); ++update_it) {
            string fn = strTable.get((update_it->second).fnId);

            cout << dec << "   - " << fn << endl;
            cout << hex << "     - 0x" << (update_it->second).location << ":";
            cout << (update_it->second).from_value << endl;
        }
        cout << "Instructions:" << endl;
        map<int,InstInfo>::iterator it;
        for(it = element->instInfo.begin(); it != element->instInfo.end(); ++it) {
            string fn = strTable.get(it->second.fnId);
            int instId = it->second.instId;
            UINT8* binary = it->second.binary;
            ADDRINT instSize = it->second.instSize;
            ACCESS aType = it->second.aType;
            cout << "   - " << dec << aType << "; " << instId << "; " << fn << "; ";
            printUINT8(binary, instSize);
        }*/
    }

    cout << "IR Nodes:" << endl;
    map<ADDRINT,Node*>::iterator it;
    for (it = irNodeAddrs.begin(); it != irNodeAddrs.end(); ++it) {
        Node *node = it->second;
        cout << dec << node->id << "; 0x" << hex << node->intAddress << "; 0x" << node->opcode << endl;
    }


    /*cout << "All MWs:" << endl;
    map<ADDRINT,MWInst>::iterator it;
    for (it = writes.begin(); it != writes.end(); ++it) {
        cout << "   - " << hex << it->second.location << ":" << it->second.value << endl;
    }*/
}

/**
 * Function: endFile
 * Description: This function is being called at the end of tracing.
 *  Currently, this function simply calls write2Json to write the modeled IR to a JSON file.
 * Input: None.
 * Output: None.
 **/
void endFile() {
    //
    checkAllCodes();
    //
    // std_print();
    // Write model to a file in JSON.
    write2Json();
}
