#ifndef _TRACE_FILE_HEADER_H_
#define _TRACE_FILE_HEADER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//9 as per the trace header spec (chosen to make spacing nicer)
#define IDENT_LEN 9

//15 as per the section table spec (chosen to make spacing nicer)
#define NAME_SIZE 15

typedef enum {
    LINUX_SIGNAL,
    WINDOWS_EXCEPTION
} ExceptionType;

typedef struct {
    uint8_t ident[IDENT_LEN];
    uint8_t traceType;
    uint8_t archType;
    uint8_t machineFlags;
    uint32_t sectionTableOff;
    uint16_t sectionEntrySize;
    uint16_t sectionNumEntry;
} FileHeader;

typedef struct {
    uint8_t name[NAME_SIZE];
    uint8_t type;
    uint64_t offset;
    uint64_t size;
} SectionEntry;

typedef struct {
    uint64_t addr;
    uint64_t size;
    uint64_t alignment;
    uint32_t flags;
} SegmentLoad;

typedef struct {
    uint32_t numThreads;
} TraceHeader;

typedef struct {
    uint16_t selections;
} InfoSelHeader;

typedef enum {
    SEG_EXEC = 0x1,
    SEG_WRITE = 0x2,
    SEG_READ = 0x4
} SegmentFlags;

/*The DataSelect enum specifies the bit position that marks if a particular field is present in an
  InfoSel trace.*/
typedef enum {
    SEL_SRCREG = 0,
    SEL_MEMREAD,
    SEL_DESTREG,
    SEL_MEMWRITE,
    SEL_SRCID,
    SEL_FNID,
    SEL_ADDR,
    SEL_BIN,
    SEL_TID,
    SEL_LAST
} DataSelect;

typedef enum TraceType_t {
    INVALID_TRACE = 0,
    ASCII_TRACE,
    VERBOSE_BINARY_TRACE,
    PRED_BINARY_TRACE,
    INFO_SEL_TRACE,
    DATA_OPS_TRACE
} TraceType;

typedef enum ArchType_t {
    X86_ARCH = 0,
    X86_64_ARCH
} ArchType;

typedef enum SectionEntryType_t {
    INVALID_SECTION = 0,
    TRACE_SECTION,
    DATA_SECTION,
    TRACE_HEADER_SECTION,
    STR_TABLE,
    INFO_SEL_HEADER,
    SEGMENT_LOADS
} SectionEntryType;

#endif
