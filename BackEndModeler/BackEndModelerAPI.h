#ifndef _IRMODELERAPI_
#define _IRMODELERAPI_

#include "pin.H"
#include "TraceFileHeader.h"
#include "RegVector.h"
#include "Trace.h"
#include "StringTable.h"
#include "BackEnd.h"
#include "IR.h"

const UINT32  UINT32_INVALID  = -1;
const ADDRINT ADDRINT_INVALID = -1;
const int     INT_INVALID     = -1;

const UINT32 V8 = 1;
const UINT32 JSC = 2;
const UINT32 SPM = 3;

const UINT32 MEMSIZE    = 8;

const std::string V8_JIT = "v8::internal::compiler";
const std::string JSC_JIT = "JSC::DFG";
const std::string SPM_JIT = "js::jit";

const int V8_OPCODE_SIZE = 2;
const int JSC_OPCODE_SIZE = 4;
const int SPM_OPCODE_SIZE = 2;

const std::string FORMERS[1] = {
    "v8::internal::compiler::Instruction::Instruction"
};

const std::string BE_CREATORS[45] {
    "v8::internal::compiler::InstructionSelector::Emit",
    "js::jit::MBasicBlock::New",
    "js::jit::MGoto::New",
    "js::jit::MParameter::New<int const&>",
    "js::jit::MConstant::New",
    "js::jit::MConstant::MConstant",
    "js::jit::MToDouble::New<js::jit::MDefinition*&>",
    "js::jit::MAdd::New<js::jit::MDefinition*&, js::jit::MConstant*&, js::jit::MIRType>",
    "js::jit::MSub::New<js::jit::MDefinition*&, js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MDiv::New",
    "js::jit::MMul::New",
    "js::jit::MMod::New",
    "js::jit::MOsrEntry::New<>",
    "js::jit::MReturn::New<js::jit::MDefinition*&>",
    "js::jit::MEnclosingEnvironment::New",
    "js::jit::MGuardShape::New<js::jit::MDefinition*&, js::Shape*&>",
    "js::jit::MSlots::New<js::jit::MDefinition*&>",
    "js::jit::MLoadDynamicSlot::New<js::jit::MSlots*&, unsigned long&>",
    "js::jit::MCompare::New<js::jit::MDefinition*&, js::jit::MDefinition*&, JSOp&, js::jit::MCompare::CompareType&>",
    "js::jit::MCall::New",
    "js::jit::MPostWriteBarrier::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MBox::New<js::jit::MDefinition*&>",
    "js::jit::MLimitedTruncate::New<js::jit::MDefinition*&, js::jit::TruncateKind>",
    "js::jit::MUnbox::New",
    "js::jit::MUnreachableResult::New<js::jit::MIRType&>",
    "js::jit::MCheckReturn::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MCheckThis::New<js::jit::MDefinition*&>",
    "js::jit::MCheckThisReinit::New<js::jit::MDefinition*&>",
    "js::jit::MSuperFunction::New<js::jit::MDefinition*&>",
    "js::jit::MBigIntPow::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MAbs::New<js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MMathFunction::New<js::jit::MDefinition*&, js::UnaryMathFunction&>",
    "js::jit::MSqrt::New<js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MCeil::New<js::jit::MDefinition*&>",
    "js::jit::MFloor::New<js::jit::MDefinition*&>",
    "js::jit::MRound::New<js::jit::MDefinition*&>",
    "js::jit::MNewArray::MNewArray",
    "js::jit::MToNumberInt32::New<js::jit::MDefinition*&, js::jit::IntConversionInputKind>",
    "js::jit::MStoreElement::New<js::jit::MElements*&, js::jit::MConstant*&, js::jit::MDefinition*&, bool>",
    "js::jit::MToFloat32::New<js::jit::MDefinition*&>",
    "js::jit::MPhi::New",
    "js::jit::MSlots::New<js::jit::MDefinition*&>",
    "js::jit::MNearbyInt::New<js::jit::MDefinition*&, js::jit::MIRType, js::jit::RoundingMode>",
    "js::jit::MNot::New<js::jit::MDefinition*&>",
    "js::jit::MTest::New<js::jit::MDefinition*, js::jit::MBasicBlock*, js::jit::MBasicBlock*>"
};
const std::string IR_CREATORS[45] {
    "v8::internal::compiler::Node::New",
    // "v8::internal::compiler::Node::NewImpl<v8::internal::compiler::Node*>",
    "js::jit::MBasicBlock::New",
    "js::jit::MGoto::New",
    "js::jit::MParameter::New<int const&>",
    "js::jit::MConstant::New",
    "js::jit::MConstant::MConstant",
    "js::jit::MToDouble::New<js::jit::MDefinition*&>",
    "js::jit::MAdd::New<js::jit::MDefinition*&, js::jit::MConstant*&, js::jit::MIRType>",
    "js::jit::MSub::New<js::jit::MDefinition*&, js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MDiv::New",
    "js::jit::MMul::New",
    "js::jit::MMod::New",
    "js::jit::MOsrEntry::New<>",
    "js::jit::MReturn::New<js::jit::MDefinition*&>",
    "js::jit::MEnclosingEnvironment::New",
    "js::jit::MGuardShape::New<js::jit::MDefinition*&, js::Shape*&>",
    "js::jit::MSlots::New<js::jit::MDefinition*&>",
    "js::jit::MLoadDynamicSlot::New<js::jit::MSlots*&, unsigned long&>",
    "js::jit::MCompare::New<js::jit::MDefinition*&, js::jit::MDefinition*&, JSOp&, js::jit::MCompare::CompareType&>",
    "js::jit::MCall::New",
    "js::jit::MPostWriteBarrier::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MBox::New<js::jit::MDefinition*&>",
    "js::jit::MLimitedTruncate::New<js::jit::MDefinition*&, js::jit::TruncateKind>",
    "js::jit::MUnbox::New",
    "js::jit::MUnreachableResult::New<js::jit::MIRType&>",
    "js::jit::MCheckReturn::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MCheckThis::New<js::jit::MDefinition*&>",
    "js::jit::MCheckThisReinit::New<js::jit::MDefinition*&>",
    "js::jit::MSuperFunction::New<js::jit::MDefinition*&>",
    "js::jit::MBigIntPow::New<js::jit::MDefinition*&, js::jit::MDefinition*&>",
    "js::jit::MAbs::New<js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MMathFunction::New<js::jit::MDefinition*&, js::UnaryMathFunction&>",
    "js::jit::MSqrt::New<js::jit::MDefinition*&, js::jit::MIRType>",
    "js::jit::MCeil::New<js::jit::MDefinition*&>",
    "js::jit::MFloor::New<js::jit::MDefinition*&>",
    "js::jit::MRound::New<js::jit::MDefinition*&>",
    "js::jit::MNewArray::MNewArray",
    "js::jit::MToNumberInt32::New<js::jit::MDefinition*&, js::jit::IntConversionInputKind>",
    "js::jit::MStoreElement::New<js::jit::MElements*&, js::jit::MConstant*&, js::jit::MDefinition*&, bool>",
    "js::jit::MToFloat32::New<js::jit::MDefinition*&>",
    "js::jit::MPhi::New",
    "js::jit::MSlots::New<js::jit::MDefinition*&>",
    "js::jit::MNearbyInt::New<js::jit::MDefinition*&, js::jit::MIRType, js::jit::RoundingMode>",
    "js::jit::MNot::New<js::jit::MDefinition*&>",
    "js::jit::MTest::New<js::jit::MDefinition*, js::jit::MBasicBlock*, js::jit::MBasicBlock*>"
};

const std::string NONIR_NODE_ALLOCATORS[2] = {
    "js::jit::MBasicBlock::New",
    "js::jit::MResumePoint::New"
};

const std::string CODE_BUFFER_ALLOCATORS[1] {
    "__libc_malloc"
};

const std::string ASSEMBLERS[1] = {
    "v8::internal::AssemblerBase::AssemblerBase"
};

const std::string CODE_ASSEMBLERS[1] = {
    "v8::internal::compiler::CodeGenerator::AssembleArchInstruction"
};

const int FORMERS_SIZE = 1;
const int BE_CREATORS_SIZE = 45;
const int IR_CREATORS_SIZE = 45;
const int NONIR_NODE_ALLOC_SIZE = 2;
const int CODE_BUFFER_ALLOCATORS_SIZE = 1;
const int ASSEMBLERS_SIZE = 1;
const int CODE_ASSEMBLERS_SIZE = 1;

const ADDRINT DEFAULT_V8_BUFFER_SIZE = 4096;

// Model constructor function(s).
void constructModel(UINT32 fnId, UINT8* binary, ADDRINT instSize, UINT32 system_id);
void populate_node_address(UINT32 fnId, UINT32 system_id);

// API functions.
ADDRINT get_element_address(UINT32 fnId, UINT32 system_id);
ADDRINT get_element_opcode(UINT32 fnId, UINT32 system_id, ADDRINT address);
ADDRINT get_architecture_opcode(UINT32 fnId, UINT32 system_id, ADDRINT opcode);
ADDRINT get_node_address(UINT32 fnId, UINT32 system_id);
void    get_size(UINT32 fnId, UINT32 system_id, BElement *element);
void    get_operands(UINT32 fnId, UINT32 system_id, BElement *element);
void    get_written_values(UINT32 fnId, UINT32 system_id, BElement *element);
void    get_ir_opcodes(UINT32 fnId, UINT32 system_id, BElement* element);
ADDRINT *get_opcode(Node *node, UINT32 system_id, UINT32 fnId);

// Above functions call functions below to handle system-specific information extraction.
// To handle other system's opcode extraction, add function below and call from get_opcode(..).

// Google V8
ADDRINT get_element_address_v8(UINT32 fnId);
ADDRINT get_element_opcode_v8(UINT32 fnId, ADDRINT address);
ADDRINT get_architecture_opcode_v8(UINT32 fnId, ADDRINT opcode);
void get_ir_opcodes_v8(UINT32 fnId, BElement* element);
ADDRINT get_node_address_v8(UINT32 fnId);
void get_size_v8(UINT32 fnId, BElement *element);
void get_operand_sizes_v8(UINT32 fnId, BElement *element);
ADDRINT *get_opcode_v8(Node *node);
// Mozilla SpiderMonkey
ADDRINT get_element_address_spm(UINT32 fnId);
ADDRINT get_element_opcode_spm(UINT32 fnId, ADDRINT address);
ADDRINT get_node_address_spm(UINT32 fnId);
void get_inputs_spm(UINT32 fnId, BElement *element);
void get_outputs_spm(UINT32 fnId, BElement *element);
// WebKit JavaScriptCore

// Checker functions.
bool fnInCreators(std::string fn, const std::string *creators, const int size);
bool fnInFormers(std::string fn);
bool fnInNonIRAllocs(std::string fn);
bool fnInCodeBufferAllocs(std::string fn);
bool fnInAssemblers(std::string fn);
bool fnInCodeAssemblers(std::string fn);

// Update functions.
ACCESS  updateElementWrite(BElement *element, UINT32 fnId, ADDRINT location, ADDRINT value);
ACCESS  updateElementRead (BElement *element, UINT32 fnId, ADDRINT location, ADDRINT value);
void updateLogInfo(BElement *element, UINT32 fnId, UINT8* binary, ADDRINT instSize, ACCESS aType);

// Helper functions.
ADDRINT uint8Toaddrint(UINT8* target, UINT32 size);
std::string uint8Tostring(UINT8* target, ADDRINT size);
UINT8* addrintTouint8(ADDRINT target, UINT32 size);
ADDRINT get_inst_opcode(UINT8* binary, ADDRINT instSize);
bool checkCopiedValue(UINT8 *value, UINT32 size);
void fixCopyValue(UINT8 *buggy, UINT8 *fixed, UINT32 size);
bool checkRAXValue(UINT8 *value);
int  get_accessing(ADDRINT location, ADDRINT value);
ADDRINT get_value(ADDRINT value);
bool is_writing_to_buffer(ADDRINT location);
void checkAllCodes();
bool check_jit_code_entry(UINT32 srcId);
UINT8* reverseUint8(UINT8* target, ADDRINT size);

// Prints for debugging.
void printUINT8(UINT8 *currentRaxVal, UINT32 currentRaxValSize);

extern StringTable strTable;

#endif
