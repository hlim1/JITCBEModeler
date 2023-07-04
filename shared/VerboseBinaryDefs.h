#ifndef _VERBOSE_BINARY_DEFS_H_
#define _VERBOSE_BINARY_DEFS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*This enum describes what the next bytes in the data section represent. OP_REG and OP_MEM are self
  explanitory, but OP_LABEL specifies a lable will follow next which will tells the programmer when
  to apply the updates between this label and the next label*/
typedef enum OpType_t {
	OP_REG=0x0,
	OP_MEM=0x1,
	OP_LABEL=0x2
} OpType;

/*This enum specifies whether an operand was read or written by an instruction*/
typedef enum OpAccessType_t {
	OP_READ=0x0,
	OP_WRITE=0x2
} OpAccessType;

/*An EventType specifies if an event is an instruction or an exception*/
typedef enum EventType_t {
	EVENT_INS = 0,
	EVENT_EXCEPT
} EventType;

#endif
