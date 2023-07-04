#ifndef _BACKEND_
#define _BACKEND_

#include "pin.H"
#include <map>
#include <stack>

const int MAX_ELEMENTS = 2000;  // Max number of elements.
const int MAX_INPUTS = 10;
const int MAX_OUTPUTS = 10;
const int MAX_TEMPS = 10;

enum ACCESS {
    ACCESS_INVALID  = -1,
    CREATE          = 0,
    NEW_VALUE       = 1,
    UPDATE_VALUE    = 2,
    ELEM_WRITE      = 3,
    MEM_READ        = 4,
    JIT_CODE_WRITE  = 5
};

struct InstInfo {
    int     instId;     // Instruction id.
    UINT32  fnId;       // Function id.
    UINT8*  binary;     // Instruction in binary (opcode & operands).
    ADDRINT instSize;   // Instruction size.
    ACCESS  aType;      // function access type.
};

struct Update {
    int     instId;     // Instruction id.
    UINT32  fnId;       // Function id.
    ADDRINT location;   // Memory location address.
    ADDRINT from_value; // Value in already in the location.
    ADDRINT to_value;   // New value written to the location.
};

struct Code {
    int     fnId;       // Function id.
    int     instId;     // Instruction id.
    ADDRINT value;      // Opcode/operand.
    ADDRINT size;       // Size of opcode/operand.
};

// BElement: B(ack-end)Element.
struct BElement {
    BElement() : id(-1), BModel_id(-1), opcode(-1), arch_opcode(-1),
    input_size(-1), output_size(-1) {}

    int     id;         // id of BElement.
    int     BModel_id;  // id of BModel.
    ADDRINT address;    // address of BElement.
    ADDRINT size;       // size of element.
    // JIT compiler opcodes : Opt.IR opcode -> Back-End opcode -> Machine opcode.
    ADDRINT opcode;                // opcode of BElement, i.e., opcode of back-end model.
    std::stack<int> ir_opcodes;    // opcode of IR node.
    ADDRINT arch_opcode;           // opcode of architecture-specific opcode, e.g., x86 opcode, etc.
    // There are three types of operands, i.e., input and output operands.
    std::map<ADDRINT,ADDRINT> inputs;   // Inputs of the instruction.
    ADDRINT input_size;                 // Number of input operands.
    std::map<ADDRINT,ADDRINT> outputs;  // Outputs of the instruction.
    ADDRINT output_size;                // Number of output operands.
    // Written values.
    std::map<ADDRINT,ADDRINT> addr2val; // address, i.e., start < addr < end, to value.
    ADDRINT writtenLoc;                 // address of memory location where the element is written to.
    // Executed instruction access log.
    std::map<int, InstInfo> instInfo;   // Execution instruction id to information object.
    int lastInfoId;                     // Tracking the last instruction id added to instInfo.
    std::map<int, Update> updateInfo;   // Tracks the instructions made update to the element.
    std::map<int, Update> readInfo;     // Tracks the instructions made memory read to the element.
    // Generated Native Code.
    std::map<ADDRINT,Code> native_code; // Buffer loation offset-to-value (opcode/operand).
};

// BModel: B(ack-end)Model. 
struct BModel {
    BModel() : id(-1), lastELMId(0), systemId(-1) {}

    int         id;                        // id of BModel.
    BElement    *elements[MAX_ELEMENTS];   // array holding ptrs to BElement object.
    int         lastELMId;                 // id of last element added to the elements array.
    UINT32      systemId;                  // JIT Compiler system ID.
};

#endif
