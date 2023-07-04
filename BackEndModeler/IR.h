#ifndef _IR_
#define _IR_

#include "pin.H"

struct Node {
    int id;
    ADDRINT intAddress;
    ADDRINT opcode;
    ADDRINT opcodeAddress;
};

#endif
