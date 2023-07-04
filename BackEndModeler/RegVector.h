#ifndef _REG_VECTOR_H_
#define _REG_VECTOR_H_

#include "pin.H"
#include "LynxReg.h"
#include "PinLynxReg.h"

/**
 * So typically we would represent something like this as a bit set, where each register was designated by a single bit in an
 * int or something like that. However, that assumed a small number of registers (for integer x86 we only used 32 bits). But for
 * a larger amount of registers like we have, a bitset really didn't make sense. Usually we only need to store 2 or 3 registers in
 * a single regVector and don't need to do too many operations on them, just get the registers. With a bitset that is 16 bytes (which
 * is the size I calculated the x86-64 bit set to be), it would take longer to return a register in the set, which is what we usually
 * want to do. So this is a very small implementation of a dynamic array. It assumes that a LynxReg can fit in a UINT8 (so there can only
 * be 256 registers) and the number of registers in the regVector will be small (capacity is 255 at max). These should be valid assumptions
 * because as far as I know, only 16 registers will ever be modified at once.
 *
 * Note: Current sizes of this implementation of RegVector are 8 bytes for x86 and 16 bytes for x86-64 due to alignment stuff. Thus, if
 * it is determined that a bitset should be used, they are of a comparable size (ish). However the C++ bitset implementation might not be
 * the best to use, because it seems to be missing some stuff we need like anding/oring two bitsets. So it will have to be implemented from
 * scratch.
 */
class RegVector {
public:
    RegVector();
    ~RegVector();
    void insert(LynxReg lReg);
    void insert(REG r);
    void insertAll(RegVector &insRegs);
    void remove(LynxReg lReg);
    void removeAll(RegVector &remRegs);
    bool contains(LynxReg lReg) const;
    UINT8 indexOf(LynxReg lReg) const;
    UINT8 getSize() const;
    LynxReg at(UINT8 i) const;
    UINT8 invalid() const;
private:
    void realloc(int newCapacity);
    UINT8 *regs;
    UINT8 capacity;
    UINT8 size;
};

#endif
