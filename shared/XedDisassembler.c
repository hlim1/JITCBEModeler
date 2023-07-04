/**
 * This file contains all of the functions that interface directly with the XED disassembler. This is
 *  intended to reduce how much the reader is dependent on a single disassembler, but it unfortunately
 *  does bleed into other files. Xed's footprint in the reader could be further reduced by pulling 
 *  fetchInsInfo into this file and instead have the public API expose a pass through function. Also
 *  we could create our own instruction categorization and use that instead of XED's.
 **/

#include "XedDisassembler.h"
#include "XedLynxReg.h"
#include "ShadowRegisters.h"
#include "ReaderUtils.h"

struct XedState_t {
    xed_state_t xedState;
    ArchType arch;
};

typedef struct {
    uint32_t tid;
    RegState *regState;
    ArchType arch;
} Context;

/**
 * Function: getMemReg
 * Description: This function is intended to allow xed to interface with our shadow architecture. It will
 *  fetch a register value from the shadow registers and return it as a uint64_t. If the register was not
 *  found, error will be 1.
 * Assumptions: context must be a uint32_t * that points to the thread id of the requested register.
 * Output: The value of the requested register.
 **/
xed_uint64_t getMemReg(xed_reg_enum_t reg, void *c, xed_bool_t *error) {
    Context *context = (Context *) c;

    if(context->regState != NULL) {
        LynxReg lReg = xedReg2LynxReg_all(reg, context->arch);

        const uint8_t *val = getThreadVal(context->regState, lReg, context->tid);
        uint8_t size = LynxRegSize(lReg);
        if (size == 1) {
            return *((uint8_t *) val);
        }
        else if (size == 2) {
            return *((uint16_t *) val);
        }
        else if (size == 4) {
            return *((uint32_t *) val);
        }
        else if (size == 8) {
            return *((uint64_t *) val);
        }
    }

    *error = 1;
    return 0;
}

/**
 * Function: createReaderOp
 * Description: Creates a new ReaderOp that has been initialized to NONE
 * Output: A new ReaderOp
 **/
ReaderOp *createReaderOp() {
    ReaderOp *op = malloc(sizeof(ReaderOp));
    if (op == NULL) {
        throwError("Unable to allocate new ReaderOp");
    }

    op->type = NONE_OP;
    op->next = NULL;
    return op;
}

/**
 * Function: setupDisassembler_x86
 * Description: Sets up the xed disassembler so that it can process x86 instructions.
 * Outputs: none
 **/
DisassemblerState *setupXed_x86() {
    DisassemblerState *state = malloc(sizeof(DisassemblerState));
    if (state == NULL) {
        throwError("Couldn't allocate disassembler state");
    }
    xed_tables_init();
    xed_state_zero(&state->xedState);
    xed_state_init2(&state->xedState, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b);
    xed_agen_register_callback(getMemReg, getMemReg);
    state->arch = X86_ARCH;
    return state;
}

/**
 * Function: setupDisassembler_x64
 * Description: Sets up the xed disassembler so that it can process x86-64 intructions
 * Outputs: none
 **/
DisassemblerState *setupXed_x64() {
    DisassemblerState *state = malloc(sizeof(DisassemblerState));
    if (state == NULL) {
        throwError("Couldn't allocate disassembler state");
    }
    xed_tables_init();
    xed_state_zero(&state->xedState);
    xed_state_init2(&state->xedState, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
    xed_agen_register_callback(getMemReg, getMemReg);
    state->arch = X86_64_ARCH;
    return state;
}

void closeXed(DisassemblerState *s) {
    free(s);
}

/**
 * Function: decodeIns
 * Description: Disassembles the instruction stored in binary and populates xed's xed_decoded_inst_t
 *  so that additional queries can be performed on the instruction without requiring xed to disassemble
 *  the instruction again.
 * Output: The size of the instruction in bytes (since it may not be equal to bufSize)
 **/
int decodeIns(DisassemblerState *state, xed_decoded_inst_t *xedIns, uint8_t *binary, int bufSize) {
    xed_decoded_inst_zero_set_mode(xedIns, &state->xedState);
    xed_error_enum_t err = xed_decode(xedIns, binary, bufSize);
    if (err != XED_ERROR_NONE) {
        return 0;
    }

    return xed_decoded_inst_get_length(xedIns);
}

/**
 * Function: getInsClass
 * Description: Retrieves the instruction class from xed's xed_decoded_inst_t
 * Assumptions: xed_decoded_inst_t was already initialized by decodeIns
 * Output: The instruction class
 **/
xed_iclass_enum_t getInsClass(xed_decoded_inst_t *xedIns) {
    return xed_decoded_inst_get_iclass(xedIns);
}

/**
 * Function: getInsMnemonic
 * Description: Retrieves the instruction's textual representation from xed's xed_decoded_inst_t in the
 *  intel format.
 * Assumptions: xed_decoded_inst_t was already initialized by decodeIns and buffer is at least 25 bytes long
 * Output: 1 if disassembly fails, 0 otherwise
 **/
uint8_t getInsMnemonic(xed_decoded_inst_t *xedIns, char *mnemonicBuf, int bufSize) {
    xed_print_info_t info;
    xed_init_print_info(&info);
    info.blen = bufSize;
    info.buf = mnemonicBuf;
    info.p = xedIns;
    return !xed_format_generic(&info);
}

/**
 * Function: getMemOpInfo
 * Description: Populates op with information about a memory access
 * Assumptions: xedIns was initialized by decodeIns and memOpInd is valid
 * Output: 1 if failure, 0 otherwise
 **/
uint8_t getMemOpInfo(DisassemblerState *state, RegState *regState, xed_decoded_inst_t *xedIns, uint32_t tid, ReaderOp *op, int memOpInd) {
    op->mem.seg = xedReg2LynxReg_all(xed_decoded_inst_get_seg_reg(xedIns, memOpInd), state->arch);
    op->mem.base = xedReg2LynxReg_all(xed_decoded_inst_get_base_reg(xedIns, memOpInd), state->arch);
    op->mem.index = xedReg2LynxReg_all(xed_decoded_inst_get_index_reg(xedIns, memOpInd), state->arch);
    op->mem.scale = xed_decoded_inst_get_scale(xedIns, memOpInd);
    op->mem.disp = xed_decoded_inst_get_memory_displacement(xedIns, memOpInd);
    op->mem.size = xed_decoded_inst_get_memory_operand_length(xedIns, memOpInd);

    Context context;
    context.tid = tid;
    context.regState = regState;
    context.arch = state->arch;
    
    xed_error_enum_t err = xed_agen(xedIns, memOpInd, &context, &op->mem.addr);

    return err != XED_ERROR_NONE;
}

/**
 * Function: getInsMemOps
 * Description: Iterates through the instruction's memory operations and populates srcOps, dstOps and
 *  readWriteOps accordingly.
 * Assumptions: xedIns was initialized by decodeIns
 * Side effects: the values pointed to by srcOps, dstOps and readWriteOps will be updated so that they point
 *  to the position a new op should be placed at. srcOpCnt, dstOpCnt and readWriteOpCnt will be updated as
 *  expected.
 * Output: None
 **/
void getInsMemOps(DisassemblerState *state, RegState *regState, xed_decoded_inst_t *xedIns, uint32_t tid, ReaderOp ***srcOps, uint8_t *srcOpCnt, ReaderOp ***dstOps, uint8_t *dstOpCnt, ReaderOp ***readWriteOps, uint8_t *readWriteOpCnt) {
    uint32_t i;
    uint32_t numOps = xed_decoded_inst_number_of_memory_operands(xedIns);
    
    for(i = 0; i < numOps; i++) {
        if (xed_decoded_inst_mem_written_only(xedIns, i)) {
            if (**dstOps == NULL) {
                **dstOps = createReaderOp();
            }

            (**dstOps)->type = MEM_OP;
            getMemOpInfo(state, regState, xedIns, tid, **dstOps, i);
            (**dstOps)->mem.addrGen = 0;
            *dstOps = &((**dstOps)->next);
            (*dstOpCnt)++;
        }
        else if (xed_decoded_inst_mem_written(xedIns, i)) {
            if (**readWriteOps == NULL) {
                **readWriteOps = createReaderOp();
            }

            (**readWriteOps)->type = MEM_OP;
            getMemOpInfo(state, regState, xedIns, tid, **readWriteOps, i);
            (**readWriteOps)->mem.addrGen = 0;
            *readWriteOps = &((**readWriteOps)->next);
            (*readWriteOpCnt)++;
        }
        //handle memory reads or address generators
        else {
            if (**srcOps == NULL) {
                **srcOps = createReaderOp();
            }

            getMemOpInfo(state, regState, xedIns, tid, **srcOps, i);

            if (xed_decoded_inst_mem_read(xedIns, i)) {
                (**srcOps)->mem.addrGen = 0;
            }
            else {
                (**srcOps)->mem.addrGen = 1;
                (**srcOps)->mem.size = 0;
            }

            (**srcOps)->type = MEM_OP;
            *srcOps = &((**srcOps)->next);
            (*srcOpCnt)++;
        }
    }
}

/**
 * Function: getInsReg
 * Description: Populates the ReaderOp with information about the register specified by opType.
 * Assumptions: xedIns was initialized by decodeIns
 * Side effects: cnt gets incremented
 * Output: The location of the next available readerOp
 **/
ReaderOp **getInsReg(DisassemblerState *state, xed_decoded_inst_t *xedIns, xed_operand_enum_t opType, ReaderOp **op, uint8_t *cnt) {
    if (*op == NULL) {
        *op = createReaderOp();
    }

    (*op)->type = REG_OP;
    (*op)->reg = xedReg2LynxReg_all(xed_decoded_inst_get_reg(xedIns, opType), state->arch);

    op = &((*op)->next);
    (*cnt)++;

    return op;
}

/**
 * Function: getInsOps
 * Description: Populates srcOps, dstOps and readWriteOps with all of the instruction's operands and updates
 *  the srcOpCnt, dstOpCnt and readWriteOpCnt so that they reflect the number of operands added to these linked
 *  lists.
 * Assumptions: xedIns has been initialized by decodeIns. Nothing is NULL.
 * Side Effects: The integers pointed to by srcOpCnt, dstOpCnt and readWriteOpCnt will be incremented by the 
 *  number of operands added to each list.
 * Output: None
 **/
void getInsOps(DisassemblerState *state, RegState *regState, xed_decoded_inst_t *xedIns, uint32_t tid, ReaderOp *srcOpsHead, uint8_t *srcOpCnt, ReaderOp *dstOpsHead, uint8_t *dstOpCnt, ReaderOp *readWriteOpsHead, uint8_t *readWriteOpCnt) {
    //initialize pointers to the current position in the list
    ReaderOp **srcOps = &srcOpsHead;
    ReaderOp **dstOps = &dstOpsHead;
    ReaderOp **readWriteOps = &readWriteOpsHead;

    //get memory operands
    getInsMemOps(state, regState, xedIns, tid, &srcOps, srcOpCnt, &dstOps, 
        dstOpCnt, &readWriteOps, readWriteOpCnt);

    //get integer operands
    const xed_inst_t *xedConstIns = xed_decoded_inst_inst(xedIns);
    uint32_t numOps = xed_inst_noperands(xedConstIns);

    uint32_t i;
    for(i = 0; i < numOps; i++) {
        const xed_operand_t *xedOp = xed_inst_operand(xedConstIns, i);
        xed_operand_enum_t opType = xed_operand_name(xedOp);

        switch(opType) {
            //taken care of already
            case XED_OPERAND_AGEN:
            case XED_OPERAND_MEM0:
            case XED_OPERAND_MEM1:
                continue;
                break;
            case XED_OPERAND_PTR:
                break;
            case XED_OPERAND_RELBR:
                if (*srcOps == NULL) {
                    *srcOps = createReaderOp();
                }
               
                (*srcOps)->type = SIGNED_IMM_OP;
                (*srcOps)->signedImm = xed_decoded_inst_get_branch_displacement(xedIns);

                srcOps = &((*srcOps)->next);
                (*srcOpCnt)++;
                break;
            case XED_OPERAND_IMM0:
                if (*srcOps == NULL) {
                    *srcOps = createReaderOp();
                }

                if (xed_decoded_inst_get_immediate_is_signed(xedIns)) {
                    (*srcOps)->type = SIGNED_IMM_OP;
                    (*srcOps)->signedImm = xed_decoded_inst_get_signed_immediate(xedIns);
                }
                else {
                    (*srcOps)->type = UNSIGNED_IMM_OP;
                    (*srcOps)->unsignedImm = xed_decoded_inst_get_unsigned_immediate(xedIns);
                }

                srcOps = &((*srcOps)->next);
                (*srcOpCnt)++;
                break;
            case XED_OPERAND_IMM1:
                if (*srcOps == NULL) {
                    *srcOps = createReaderOp();
                }

                (*srcOps)->type = UNSIGNED_IMM_OP;
                (*srcOps)->unsignedImm = xed_decoded_inst_get_second_immediate(xedIns);

                srcOps = &((*srcOps)->next);
                (*srcOpCnt)++;
                break;
            case XED_OPERAND_REG0:
            case XED_OPERAND_REG1:
            case XED_OPERAND_REG2:
            case XED_OPERAND_REG3:
            case XED_OPERAND_REG4:
            case XED_OPERAND_REG5:
            case XED_OPERAND_REG6:
            case XED_OPERAND_REG7:
            case XED_OPERAND_REG8:
                if (xed_operand_read_and_written(xedOp)) {
                    readWriteOps = getInsReg(state, xedIns, opType, readWriteOps, readWriteOpCnt);
                }
                else if (xed_operand_read(xedOp)) {
                    srcOps = getInsReg(state, xedIns, opType, srcOps, srcOpCnt);
                }
                else if (xed_operand_written(xedOp)) {
                    dstOps = getInsReg(state, xedIns, opType, dstOps, dstOpCnt);
                }
                break;
            case XED_OPERAND_BASE0:
            case XED_OPERAND_BASE1:
                //the read we took care of already in mem
                if (xed_operand_written(xedOp)) {
                    dstOps = getInsReg(state, xedIns, opType, dstOps, dstOpCnt);
                }
                break;
            default:
                throwWarning("Unknown xed operand type found");
                break;
        }
    }
}

/**
 * Function: getInsFlags
 * Descriptions: Retrieves the set of flags used by the instruction and the set of flags set by the
 *  instruction and populates the integers pointed to by srcFlags and dstFlags
 * Assumptions: xedIns has been initialized by decodeIns. Nothing is NULL
 * Side Effects: the integers pointed to by srcFlags and dstFlags are populated
 * Output: None
 **/
void getInsFlags(xed_decoded_inst_t *xedIns, uint32_t *srcFlags, uint32_t *dstFlags) {
    const xed_simple_flag_t *flags = xed_decoded_inst_get_rflags_info(xedIns);
    if (flags == NULL) {
        *srcFlags = *dstFlags = 0;
    }
    else {
        *srcFlags = (uint32_t) (flags->read.flat);
        *dstFlags = (uint32_t) (flags->written.flat);
    }
}

ArchType getXedArchType(DisassemblerState *state) {
    return state->arch;
}

