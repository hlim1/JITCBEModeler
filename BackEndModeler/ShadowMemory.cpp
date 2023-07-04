/**
 * This file contains the Shadow Memory implementation. It defines a data structure that allows us to model
 * every location in memory efficiently. It essentially divides memory into pages, and initializes the pages
 * to a default map. When we write to a memory location, we allocate a new page, copy the default map and
 * modify whatever value we need to. When we read, though, we just need to index the correct page.
 **/

#include "ShadowMemory.h"
#include <cassert>
#include <cstdio>
//#include "VerboseASCIITrace.h"

/**
 * Constructor for ShadowMemory. Sets up the structure of the 4-level map. Essentially there is a primary map(PM), which
 * is statically allocated. The Primary map holds a SM, which in turns holds a TM which finally holds a QM. Note, the PM,SM and TM
 * are all arrays of pointers essentially, while the QM holds the actual information. The map is initialized to the default value
 * which will never change. There is 1 default QM called DQM which has seen set to 0. A pointer to this is populated in all of the
 * entries in the DTM array (default TM), which in turn populates all of the entries in the DSM array and finally the DSM populates
 * all of the entries initially in the PM.
 */
ShadowMemory::ShadowMemory() {
#ifdef TARGET_IA32 //not DTM is not a 2D array...
    DSM = NULL;
    DTM = new QM *[LVL_SIZE];
    DQM = new QM;

    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        PM[i] = new QM **[LVL_SIZE];
        DTM[i] = DQM;
        for(UINT32 j = 0; j < LVL_SIZE; j++) {
            PM[i][j] = DTM;
        }
    }

#else
    DSM = new QM **[LVL_SIZE];
    DTM = new QM *[LVL_SIZE];
    DQM = new QM;

    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        PM[i] = DSM;
        DSM[i] = DTM;
        DTM[i] = DQM;
    }
#endif

    for(UINT32 i = 0; i < SEEN_SIZE; i++) {
        DQM->seen[i] = 0;
    }
}

/*
 * Resets the map to the state it was in when newly initialized
 */
void ShadowMemory::reset() {
    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        SM curSM = PM[i];
        if(DSM == NULL || !is_DSM(curSM)) {
            for(UINT32 j = 0; j < LVL_SIZE; j++) {
                TM curTM = curSM[j];
                if(!is_DTM(curTM)) {
                    for(UINT32 k = 0; k < LVL_SIZE; k++) {
                        QM *curQM = curTM[k];
                        if(!is_DQM(curQM)) {
                            delete curQM;
                            curTM[k] = DQM;
                        }
                    }

                    delete[] curTM;
                    curSM[j] = DTM;
                }
            }
            if(DSM != NULL) {
                delete[] curSM;
                PM[i] = DSM;
            }
        }
    }
}

/*
 * Checks if the parameter is the DTM
 */
inline bool ShadowMemory::is_DTM(TM curTM) {
    return DTM == curTM;
}

/*
 * Checks if the parameter is the DSM
 */
inline bool ShadowMemory::is_DSM(SM curSM) {
    return DSM == curSM;
}

/*
 * Checks if the parameter is the DQM
 */
inline bool ShadowMemory::is_DQM(QM *curQM) {
    return DQM == curQM;
}

/*
 * Retrieves the SM from the primary map that corresponds to the given address
 */
inline SM ShadowMemory::getSM(ADDRINT addr) {
    return PM[addr >> PM_SHAMT];
}

/*
 * Retrieves the TM from the given SM that corresponds to the given address
 *
 * Assumptions: curSM is the correct SM for the given address
 */
inline TM ShadowMemory::getTM(ADDRINT addr, SM curSM) {
    return curSM[(addr >> SM_SHAMT) & ADDR_MASK];
}

/*
 * Retrieves the QM from the given TM that corresponds to the given address
 *
 * Assumptions: curTM is the correct TM for the given address
 */
inline QM *ShadowMemory::getQM(ADDRINT addr, TM curTM) {
    return curTM[(addr >> TM_SHAMT) & ADDR_MASK];
}

/*
 * Returns a pointer to the SM from the PM that corresponds to the given address
 */
inline SM *ShadowMemory::getSMPtr(ADDRINT addr) {
    return &PM[addr >> PM_SHAMT];
}

/*
 * Returns a pointer to the TM from the given SM that corresponds to the given address
 *
 * Assumptions: curSM is the correct SM for the given address
 */
inline TM *ShadowMemory::getTMPtr(ADDRINT addr, SM curSM) {
    return &curSM[(addr >> SM_SHAMT) & ADDR_MASK];
}

/*
 * Returns a pointer to the QM from the given TM that corresponds to the given address
 *
 * Assumptions: curTM is the correct TM for the given address
 */
inline QM **ShadowMemory::getQMPtr(ADDRINT addr, TM curTM) {
    return &curTM[(addr >> TM_SHAMT) & ADDR_MASK];
}

/*
 * Makes a copy of the given srcTM so that it can be written to (for example if srcTM is DTM) and
 * will replace it in the SM. Note, on a 32 bit architecture the new TM is populated entirely with
 * new QMs, while on the 64 bit architecture the new TM only replaces the QM for the given address.
 */
inline QM *ShadowMemory::copyForWriting(ADDRINT addr, TM *srcTM) {
#ifdef TARGET_IA32
    TM newTM = new QM *[LVL_SIZE];

    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        QM *newQM = new QM;
        newTM[i] = newQM;
        PIN_SafeCopy(newQM, (*srcTM)[i], sizeof(QM));
    }
    *srcTM = newTM;

    return getQM(addr, *srcTM);
#else
    TM newTM = new QM *[LVL_SIZE];
    PIN_SafeCopy(newTM, *srcTM, sizeof(QM *[LVL_SIZE]));

    QM *newQM = new QM;
    QM **curQM = getQMPtr(addr, newTM);
    PIN_SafeCopy(newQM, *curQM, sizeof(QM));

    *srcTM = newTM;
    *curQM = newQM;

    return newQM;
#endif
}

/*
 * Makes a copy of the given srcSM so that it can be written to and replaces it in the PM. It will
 * also create a new TM and QM for the given address and place them in their appropriate locations in
 * the map so that they don't have to be copied as well.
 */
inline QM *ShadowMemory::copyForWriting(ADDRINT addr, SM *srcSM) {
    SM newSM = new QM **[LVL_SIZE];
    PIN_SafeCopy(newSM, *srcSM, sizeof(QM **[LVL_SIZE]));

    TM newTM = new QM *[LVL_SIZE];
    TM *curTM = getTMPtr(addr, newSM);
    PIN_SafeCopy(newTM, *curTM, sizeof(QM *[LVL_SIZE]));

    QM *newQM = new QM;
    QM **curQM = getQMPtr(addr, newTM);
    PIN_SafeCopy(newQM, *curQM, sizeof(QM));

    *srcSM = newSM;
    *curTM = newTM;
    *curQM = newQM;

    return newQM;
}

/*
 * Makes a copy of the given QM so that it can be written to and replaces it in the TM.
 */
inline QM *ShadowMemory::copyForWriting(ADDRINT addr, QM **srcQM) {
    QM *newQM = new QM;
    PIN_SafeCopy(newQM, *srcQM, sizeof(QM));

    *srcQM = newQM;

    return newQM;
}

/*
 * Retrieves a QM that can be read from. This is because the default values should not be written to since
 * that would cause changes in all of the locations that use the default values.
 */
inline QM * ShadowMemory::getQMForReading(ADDRINT addr) {
    return PM[addr >> PM_SHAMT][(addr >> SM_SHAMT) & ADDR_MASK][(addr >> TM_SHAMT) & ADDR_MASK];
}

/*
 * Retrieves a QM that can be written to. If one of the default values is found, a copy of the default value will
 * be made and the copy will be placed in the appropriate location.
 */
inline QM *ShadowMemory::getQMForWriting(ADDRINT addr) {
#ifdef TARGET_IA32
    TM *curTM = &PM[addr >> PM_SHAMT][(addr >> SM_SHAMT) & ADDR_MASK];
    if(is_DTM(*curTM)) {
        return copyForWriting(addr, curTM);
    }

    return getQM(addr, *curTM);
#else
    SM *curSM = getSMPtr(addr);
    if(is_DSM(*curSM)) {
        return copyForWriting(addr, curSM);
    }

    TM *curTM = getTMPtr(addr, *curSM);
    if(is_DTM(*curTM)) {
        return copyForWriting(addr, curTM);
    }

    QM **curQM = getQMPtr(addr, *curTM);
    if(is_DQM(*curQM)) {
        return copyForWriting(addr, curQM);
    }

    return *curQM;
#endif
}

/*
 * Checks if the configuration is the default configuration that the system is initialized to
 */
void ShadowMemory::isDefault() {
    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        if(DSM != NULL && !is_DSM(PM[i])) {
            printf("Not DSM at %d\n", i);
        }
        if(DTM != NULL && !is_DTM(DSM[i])) {
            printf("Not DTM at %d\n", i);
        }
        if(DQM != NULL && !is_DQM(DTM[i])) {
            printf("Not DQM at %d\n", i);
        }
    }
}

/*
 * Checks to make sure the the default values are set up correctly
 */
void ShadowMemory::chkDefaultVals() {
    for(UINT32 i = 0; i < LVL_SIZE; i++) {
        if(DSM != NULL && DSM[i] != DTM) {
            printf("Bad DSM at %d\n", i);
        }
        if(DTM != NULL && DTM[i] != DQM) {
            printf("Bad DTM at %d\n", i);
        }
        if(i < SEEN_SIZE && DQM->seen[i] != 0) {
            printf("Bad DQM at %p, seen: %x\n", DQM, DQM->seen[i]);
        }
    }
}

/*
 * Stores sizeInBytes seen values from val one byte at a time.
 */
void ShadowMemory::storeSeenSlow(ADDRINT addr, UINT32 sizeInBytes, UINT8 val) {
    for(UINT32 i = 0; i < sizeInBytes; i++) {
        storeSeen8(addr + i, val >> i);
    }
}

/*
 * Stores the seen value corresponding to the memory byte located at addr
 *
 * Assumptions: The seen bit to store for addr is the first bit of newSeen
 */
inline void ShadowMemory::storeSeen8(ADDRINT addr, UINT8 newSeen) {
    QM *curQM = getQMForWriting(addr);
    UINT8 shamt = addr & 0x7;
    UINT32 i = (addr & ADDR_MASK) >> 3;
    curQM->seen[i] = (curQM->seen[i] & ~(1 << shamt)) | ((newSeen & 0x1) << shamt);
}

/*
 * Stores the seen value corresponding to 2 memory bytes located at addr
 *
 * Note, we can write it in one chunk if the address is 2 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen array in this case.
 *
 * Assumptions: The seen bits to store for addr are the first 2 bits of newSeen
 */
void ShadowMemory::storeSeen16(ADDRINT addr, UINT8 newSeen) {
    if(isNotAligned(addr, 2)) {
        storeSeenSlow(addr, 2, newSeen);
        return;
    }

    QM *curQM = getQMForWriting(addr);
    UINT8 shamt = addr & 0x7;
    UINT32 i = (addr & ADDR_MASK) >> 3;
    curQM->seen[i] = (curQM->seen[i] & ~(0x3 << shamt)) | ((newSeen & 0x3) << shamt);
}

/*
 * Stores the seen value corresponding to 4 memory bytes located at addr
 *
 * Note, we can write it in one chunk if the address is 4 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen array in this case.
 *
 * Assumptions: The seen bits to store for addr are the first 4 bits of newSeen
 */
void ShadowMemory::storeSeen32(ADDRINT addr, UINT8 newSeen) {
    if(isNotAligned(addr, 4)) {
        storeSeenSlow(addr, 4, newSeen);
        return;
    }

    QM *curQM = getQMForWriting(addr);
    UINT8 shamt = addr & 0x7;
    UINT32 i = (addr & ADDR_MASK) >> 3;
    curQM->seen[i] = (curQM->seen[i] & ~(0xf << shamt)) | ((newSeen & 0xf) << shamt);
}

/*
 * Stores the seen value corresponding to 8 memory bytes located at addr
 *
 * Note, we can write it in one chunk if the address is 2 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen array in this case.
 */
void ShadowMemory::storeSeen64(ADDRINT addr, UINT8 newSeen) {
    if(isNotAligned(addr, 8)) {
        storeSeenSlow(addr, 8, newSeen);
        return;
    }

    QM *curQM = getQMForWriting(addr);
    curQM->seen[(addr & ADDR_MASK) >> 3] = newSeen;
}

/*
 * Loads sizeInBytes seen bits at the address addr one byte at a time
 */
UINT8 ShadowMemory::loadSeenSlow(ADDRINT addr, UINT32 sizeInBytes) {
    UINT8 seenBits = 0;

    for(UINT32 i = 0; i < sizeInBytes; i++) {
        UINT8 seenBit = loadSeen8(addr + i);
        seenBits = seenBits | (seenBit << i);
    }

    return seenBits;
}

/*
 * Checks to see if addr is aligned to the given alignment. This occurs if bits up to
 * and including log_2(alignment) are 0. For example, something is 8 byte aligned if
 * the first 3 bits are 0s.
 */
inline bool ShadowMemory::isNotAligned(ADDRINT addr, UINT32 alignment) {
    return addr & (alignment - 1);
}

/*
 * Loads the seen value for an 8bit value at the given address
 */
inline UINT8 ShadowMemory::loadSeen8(ADDRINT addr) {
    QM *curQM = getQMForReading(addr);
    UINT8 seenBits = curQM->seen[(addr & ADDR_MASK) >> 3];
    return 0x1 & (seenBits >> (addr & 0x7));
}

/*
 * Loads the seen values for a 16bit value at the given address
 *
 * Note, we can read it in one chunk if the address is 2 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen arrayin this case.
 */
UINT8 ShadowMemory::loadSeen16(ADDRINT addr) {
    if(isNotAligned(addr, 2)) {
        return loadSeenSlow(addr, 2);
    }

    QM *curQM = getQMForReading(addr);
    UINT8 seenBits = curQM->seen[(addr & ADDR_MASK) >> 3];
    return 0x3 & (seenBits >> (addr & 0x7));
}

/*
 * Loads the seen values for a 32bit value at the given address
 *
 * Note, we can read it in one chunk if the address is 4 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen array in this case.
 */
UINT8 ShadowMemory::loadSeen32(ADDRINT addr) {
    if(isNotAligned(addr, 4)) {
        return loadSeenSlow(addr, 4);
    }

    QM *curQM = getQMForReading(addr);
    UINT8 seenBits = curQM->seen[(addr & ADDR_MASK) >> 3];
    return 0xf & (seenBits >> (addr & 0x7));
}

/*
 * Loads the seen values for a 64bit value at the given address
 *
 * Note, we can read it in one chunk if the address is 8 byte aligned since the seen bits
 * can never be in 2 different bytes in the QM's seen array in this case.
 */
UINT8 ShadowMemory::loadSeen64(ADDRINT addr) {
    if(isNotAligned(addr, 8)) {
        return loadSeenSlow(addr, 8);
    }

    QM *curQM = getQMForReading(addr);
    return curQM->seen[(addr & ADDR_MASK) >> 3];
}

/*
 * A call to set all of the seen bits for size bytes starting at addr to 1.
 */
void ShadowMemory::setSeen(ADDRINT addr, UINT32 size) {
    setSeenVal(addr, size, 0xff);
}

/*
 * A call to unset all of the seen bits for size bytes starting at addr to 0
 */
void ShadowMemory::unsetSeen(ADDRINT addr, UINT32 size) {
    setSeenVal(addr, size, 0);
}

/*
 * Sets all of the seen bits for size bytes starting at addr to val.
 */
void ShadowMemory::setSeenVal(ADDRINT addr, UINT32 sizeInBytes, UINT8 val) {
    if(sizeInBytes <= 8) {
        if(sizeInBytes == 4) {
            storeSeen32(addr, val);
            return;
        }
        else if(sizeInBytes == 8) {
            storeSeen64(addr, val);
            return;
        }
        else if(sizeInBytes == 1) {
            storeSeen8(addr, val);
            return;
        }
        else if(sizeInBytes == 2) {
            storeSeen16(addr, val);
            return;
        }
    }

    while(isNotAligned(addr, 8)) {
        storeSeen8(addr++, val);
        sizeInBytes--;
    }

    while(sizeInBytes >= 8) {
        storeSeen64(addr, val);
        addr += 8;
        sizeInBytes -= 8;
    }

    while(sizeInBytes > 0) {
        storeSeen8(addr++, val);
        sizeInBytes--;
    }
}

/*
 * Loads all of the seen bits for sizeInBytes bytes starting at addr into the
 * buffer val
 *
 * Assumption: val is big enough to hold all of the seen bits
 */
void ShadowMemory::loadSeen(ADDRINT addr, UINT32 sizeInBytes, UINT8 *val) {
    if(sizeInBytes <= 8) {
        if(sizeInBytes == 4) {
            val[0] = loadSeen32(addr);
            return;
        }
        else if(sizeInBytes == 8) {
            val[0] = loadSeen64(addr);
            return;
        }
        else if(sizeInBytes == 1) {
            val[0] = loadSeen8(addr);
            return;
        }
        else if(sizeInBytes == 2) {
            val[0] = loadSeen16(addr);
            return;
        }
    }

    UINT32 i = 0;
    val[0] = 0;
    while(isNotAligned(addr, 8)) {
        val[0] |= loadSeen8(addr++) << i++;
        sizeInBytes--;
    }

    UINT8 leftShamt = i;
    UINT8 rightShamt = 8 - i;
    i = 0;

    if(leftShamt == 0) {
        while(sizeInBytes >= 8) {
            UINT8 seenBits = loadSeen64(addr);
            val[i] = seenBits << leftShamt;
            i++;
            addr += 8;
            sizeInBytes -= 8;
        }
    }
    else {
        while(sizeInBytes >= 8) {
            UINT8 seenBits = loadSeen64(addr);
            val[i] |= seenBits << leftShamt;
            val[i + 1] = seenBits >> rightShamt;
            i++;
            addr += 8;
            sizeInBytes -= 8;
        }
    }

    while(sizeInBytes > 0) {
        val[i] |= loadSeen8(addr++) << leftShamt++;
        sizeInBytes--;
        if(sizeInBytes != 0 && leftShamt == 0) {
            val[++i] = 0;
        }

    }
}

ShadowMemory::~ShadowMemory() {
    // TODO Auto-generated destructor stub
}

