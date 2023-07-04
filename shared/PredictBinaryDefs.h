#ifndef _PREDICT_BINARY_DEFS_H_
#define _PREDICT_BINARY_DEFS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VerboseBinaryDefs.h"

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

#endif
