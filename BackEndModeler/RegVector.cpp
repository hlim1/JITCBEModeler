/**
 * Please see RegVector.h for a description.
 **/

#include "RegVector.h"
#include <cassert>
#include <cstring>

/*
 * Default constructor for a RegVector
 */
RegVector::RegVector() :
        capacity(2), size(0) {
    regs = new UINT8[2];
}

/*
 * Deconstructor for a RegVector
 */
RegVector::~RegVector() {
    delete[] regs;
}

/*
 * Reallocates the array where registers are stored and transfers the contents of the old array
 * into the new one
 */
void RegVector::realloc(int newCapacity) {
    UINT8 *old = regs;
    regs = new UINT8[newCapacity];
    memcpy(regs, old, capacity);
    capacity = newCapacity;
    delete[] old;
}

/*
 * Returns the invalid index
 */
UINT8 RegVector::invalid() const {
    return -1;
}

/*
 * Checks if the RegVector contains the given LynxReg
 */
bool RegVector::contains(LynxReg lReg) const {
    for(int i = 0; i < size; i++) {
        if((UINT8) lReg == regs[i]) {
            return true;
        }
    }
    return false;
}

/*
 * Returns the index of the given LynxReg if it is present in the RegVector. If not present, returns invalid.
 */
UINT8 RegVector::indexOf(LynxReg lReg) const {
    for(int i = 0; i < size; i++) {
        if((UINT8) lReg == regs[i]) {
            return i;
        }
    }
    return -1;
}

/*
 * Inserts the PIN reg into the RegVector. If the register is already present, the method will simply return.
 */
void RegVector::insert(REG r) {
    insert(Reg2LynxReg(r));
}

/*
 * Inserts the LynxReg into the RegVector. If the register is already present, the method will simply return.
 */
void RegVector::insert(LynxReg lReg) {
    if(lReg == LYNX_INVALID) {
        return;
    }
    if(size == capacity) {
        realloc(capacity * 2);
    }
    if(!contains(lReg)) {
        regs[size] = lReg;
        size++;
    }
}

/*
 * Inserts the contents of the given RegVector into this one.
 */
void RegVector::insertAll(RegVector &insRegs) {
    for(int i = 0; i < insRegs.size; i++) {
        insert((LynxReg) insRegs.at(i));
    }
}

/*
 * Removes the LynxReg
 */
void RegVector::remove(LynxReg lReg) {
    UINT8 i = indexOf(lReg);
    if(i != invalid()) {
        size--;
        for(; i < size; i++) {
            regs[i] = regs[i + 1];
        }
    }
}

/*
 * Removes all registers within the given RegVector from this one
 */
void RegVector::removeAll(RegVector &remRegs) {
    for(int i = 0; i < remRegs.size; i++) {
        remove((LynxReg) remRegs.at(i));
    }
}

/*
 * Returns the number of registers in the RegVector
 */
UINT8 RegVector::getSize() const {
    return size;
}

/*
 * Returns the register at the given index
 */
LynxReg RegVector::at(UINT8 i) const {
    if(i >= size) {
        return LYNX_INVALID;
    }
    return (LynxReg) regs[i];
}
