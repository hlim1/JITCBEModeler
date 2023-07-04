#ifndef _INFO_SEL_DEFS_H_
#define _INFO_SEL_DEFS_H_

#include "PredictBinaryDefs.h"

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

#endif
