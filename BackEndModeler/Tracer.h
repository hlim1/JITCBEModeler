#ifndef _TRACER_H_
#define _TRACER_H_

// Tracer initialization functions
bool checkInitializedStatus(THREADID tid);
void initThread(THREADID tid, const CONTEXT *ctx);
void PIN_FAST_ANALYSIS_CALL initIns(THREADID tid);

// Record functions
void recordRegState(THREADID tid, const CONTEXT *ctxt);
void PIN_FAST_ANALYSIS_CALL recordSrcRegs(
        THREADID tid, const CONTEXT *ctx, const RegVector *srcRegs, UINT32 fnId, UINT32 instOpcode);
void PIN_FAST_ANALYSIS_CALL recordSrcId(THREADID tid, UINT32 srcId);
void PIN_FAST_ANALYSIS_CALL recordDestRegs(THREADID tid, const RegVector *destRegs, UINT32 destFlags);
void recordMemWrite(
        THREADID tid, ADDRINT addr, UINT32 size);
void recordMemRead(
        ADDRINT readAddr, UINT32 readSize, UINT32 fnId, UINT8* binary, ADDRINT instSize);
void record2MemRead(
        ADDRINT readAddr1, ADDRINT readAddr2, UINT32 readSize, UINT32 fnId, UINT8* binary, ADDRINT instSize);
void track_function_calls(UINT32 fnId, bool is_ret);

// Analysis functions
bool analyzeRecords(
        THREADID tid, const CONTEXT *ctx, UINT32 fnId, UINT32 opcode, bool is_create, UINT8* binary,
        ADDRINT instSize, UINT32 system_id, ADDRINT execution_addr, UINT32 srcId, bool is_ir_create);
void analyzeRegWrites(
        THREADID tid, const CONTEXT *ctx, UINT32 fnId, UINT32 opcode, bool is_range, UINT8* binary,
        ADDRINT instSize);
void analyzeMemWrites(
        THREADID tid, UINT32 fnId, bool is_range, UINT32 opcode, UINT8* binary, ADDRINT instSize, 
        UINT32 system_id);
void analyzeRegReads();

//handles architectural events
void contextChange(
        THREADID tid, CONTEXT_CHANGE_REASON reason, const CONTEXT *fromCtx, 
        CONTEXT *toCtx, INT32 info, void *v);
void startTrace();
VOID threadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v);

//handling the trace format
void setupFile(UINT16 infoSelect);
void endFile();

#endif
