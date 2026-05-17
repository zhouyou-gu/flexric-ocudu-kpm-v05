#ifndef CCC_SM_RIC_H
#define CCC_SM_RIC_H

#include "../sm_ric.h"

typedef struct {
  const char* ctrl_hdr_json;
  const char* ctrl_msg_json;
} ccc_ctrl_req_data_t;

__attribute__((visibility("default"))) sm_ric_t* make_ccc_sm_ric(void);

#endif
