/*
 * ShadowMemory.h
 *
 *  Created on: Jun 9, 2016
 *      Author: Anon
 */

#ifndef SHADOWMEMORY_H_
#define SHADOWMEMORY_H_

#include "pin.H"

#ifdef TARGET_IA32
const UINT32 LVL_WIDTH = 8;
const UINT32 LVL_SIZE = 1 << LVL_WIDTH;
#else
const UINT32 LVL_WIDTH = 16;
const UINT32 LVL_SIZE = 1 << LVL_WIDTH;
#endif

const UINT8 PM_SHAMT = LVL_WIDTH * 3;
const UINT8 SM_SHAMT = LVL_WIDTH * 2;
const UINT8 TM_SHAMT = LVL_WIDTH;
const UINT32 ADDR_MASK = LVL_SIZE - 1;

const UINT32 SEEN_SIZE = LVL_SIZE / 8;

#define SM QM***
#define TM QM**

typedef struct QM_t {
    UINT8 seen[SEEN_SIZE];
} QM;

class ShadowMemory {
public:
    ShadowMemory();
    ~ShadowMemory();

    void loadSeen(ADDRINT addr, UINT32 size, UINT8 *val);
    void setSeen(ADDRINT addr, UINT32 size);
    void unsetSeen(ADDRINT addr, UINT32 size);

    void chkDefaultVals();

    void isDefault();

    /*currently this is being called whenever a syscall is called, that needs to be changed when taint
     * analysis is implemented because it will reset the taint as well, which we don't want. Instead for
     * the implemented syscalls use setSeenAndTaintVals and for the unimplemented ones have a function that
     * will iterate over all non-DQM pages of the shadow memory and set the seen bits back to 0*/
    void reset();

    //make inline
//	bool inSeparateQM(ADDRINT addr, UINT32 sizeInBytes);

private:
    void setSeenVal(ADDRINT addr, UINT32 size, UINT8 val);
    UINT8 loadSeenSlow(ADDRINT addr, UINT32 sizeInBytes);
    inline UINT8 loadSeen8(ADDRINT addr);
    UINT8 loadSeen16(ADDRINT addr);
    UINT8 loadSeen32(ADDRINT addr);
    UINT8 loadSeen64(ADDRINT addr);

    void storeSeenSlow(ADDRINT addr, UINT32 sizeInBytes, UINT8 val);
    inline void storeSeen8(ADDRINT addr, UINT8 newSeen);
    void storeSeen16(ADDRINT addr, UINT8 newSeen);
    void storeSeen32(ADDRINT addr, UINT8 newSeen);
    void storeSeen64(ADDRINT addr, UINT8 newSeen);

    inline SM getSM(ADDRINT addr);
    inline TM getTM(ADDRINT addr, SM curSM);
    inline QM *getQM(ADDRINT addr, TM curTM);

    inline SM *getSMPtr(ADDRINT addr);
    inline TM *getTMPtr(ADDRINT addr, SM curSM);
    inline QM **getQMPtr(ADDRINT addr, TM curTM);

    inline bool is_DSM(SM curSM);
    inline bool is_DTM(TM curTM);
    inline bool is_DQM(QM *curQM);

    inline QM *copyForWriting(ADDRINT addr, TM *srcTM);
    inline QM *copyForWriting(ADDRINT addr, SM *srcSM);
    inline QM *copyForWriting(ADDRINT addr, QM **srcQM);

    inline QM *getQMForReading(ADDRINT addr);
    inline QM *getQMForWriting(ADDRINT addr);

    inline bool isNotAligned(ADDRINT addr, UINT32 alignment);

    QM ***PM[LVL_SIZE];

    //defualt secondary map
    QM ***DSM;
    QM **DTM;
    QM *DQM;//256
};

#endif /* SHADOWMEMORY_H_ */
