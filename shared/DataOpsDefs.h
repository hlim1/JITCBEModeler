#ifndef _DATA_OPS_DEFS_H_
#define _DATA_OPS_DEFS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*The PredictionStatus simply makes it clear what a correct prediction and an incorrect
  prediction look like.*/
typedef enum PredictionStatus_t {
    PRED_INCORRECT=0,
    PRED_CORRECT=1
} PredictionStatus;

/*The PredictionLoc specifies what bit is used to specify if a particular prediction is correct or
  not. It starts at an odd number because the first 3 bits are used for something else.*/
typedef enum PredictionLoc_t {
    PRED_TID=3,
    PRED_SRCID=4,
    PRED_FUNCID=5,
    PRED_ADDR=6,
    PRED_BIN=7
} PredictionLoc;

/*An EventType specifies if an event is an instruction or an exception*/
typedef enum EventType_t {
    EVENT_INS = 0,
    EVENT_EXCEPT
} EventType;

typedef enum OpType_t {
    OP_LABEL=0,
    OP_REG=1,
    OP_MEM=2,
    OP_DST_MEM=3,
    OP_DST_REG=4,
    OP_TEST_REG=5,
    OP_TEST_MEM=6
} OpType;

#endif
