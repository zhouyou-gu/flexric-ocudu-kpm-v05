#include "ccc_sm_ric.h"
#include "ccc_sm_id.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  sm_ric_t base;
} sm_ccc_ric_t;

static uint8_t* copy_json_octets(const char* text, size_t* len)
{
  assert(text != NULL);
  assert(len != NULL);
  *len = strlen(text);
  uint8_t* buf = calloc(*len, sizeof(uint8_t));
  assert(buf != NULL && "Memory exhausted");
  memcpy(buf, text, *len);
  return buf;
}

static sm_subs_data_t on_subscription_ccc_sm_ric(sm_ric_t const* sm_ric, void* cmd)
{
  assert(sm_ric != NULL);
  assert(cmd != NULL);
  assert(0 != 0 && "CCC subscription is not implemented");
  return (sm_subs_data_t){0};
}

static sm_ag_if_rd_ind_t on_indication_ccc_sm_ric(sm_ric_t const* sm_ric, sm_ind_data_t const* data)
{
  assert(sm_ric != NULL);
  assert(data != NULL);
  assert(0 != 0 && "CCC indication is not implemented");
  return (sm_ag_if_rd_ind_t){0};
}

static sm_ctrl_req_data_t on_control_req_ccc_sm_ric(sm_ric_t const* sm_ric, void* ctrl)
{
  assert(sm_ric != NULL);
  assert(ctrl != NULL);
  ccc_ctrl_req_data_t const* data = (ccc_ctrl_req_data_t const*)ctrl;
  assert(data->ctrl_hdr_json != NULL);
  assert(data->ctrl_msg_json != NULL);

  sm_ctrl_req_data_t dst = {0};
  dst.ctrl_hdr = copy_json_octets(data->ctrl_hdr_json, &dst.len_hdr);
  dst.ctrl_msg = copy_json_octets(data->ctrl_msg_json, &dst.len_msg);
  return dst;
}

static sm_ag_if_ans_ctrl_t on_control_out_ccc_sm_ric(sm_ric_t const* sm_ric, const sm_ctrl_out_data_t* out)
{
  assert(sm_ric != NULL);
  assert(out != NULL);
  return (sm_ag_if_ans_ctrl_t){.type = RAN_CTRL_V1_3_AGENT_IF_CTRL_ANS_V0};
}

static sm_ag_if_rd_e2setup_t on_e2_setup_ccc_sm_ric(sm_ric_t const* sm_ric, sm_e2_setup_data_t const* setup)
{
  assert(sm_ric != NULL);
  assert(setup != NULL);
  return (sm_ag_if_rd_e2setup_t){.type = RAN_CTRL_V1_3_AGENT_IF_E2_SETUP_ANS_V0};
}

static sm_ag_if_rd_rsu_t on_ric_service_update_ccc_sm_ric(sm_ric_t const* sm_ric, sm_ric_service_update_data_t const* data)
{
  assert(sm_ric != NULL);
  assert(data != NULL);
  return (sm_ag_if_rd_rsu_t){.type = RAN_CTRL_V1_3_AGENT_IF_RIC_SERV_UPDATE_CTRL_ANS_V0};
}

static void free_ccc_sm_ric(sm_ric_t* sm_ric)
{
  assert(sm_ric != NULL);
  free((sm_ccc_ric_t*)sm_ric);
}

static void free_noop(void* msg)
{
  (void)msg;
}

__attribute__((visibility("default"))) sm_ric_t* make_ccc_sm_ric(void)
{
  sm_ccc_ric_t* sm = calloc(1, sizeof(sm_ccc_ric_t));
  assert(sm != NULL && "Memory exhausted");

  *((uint16_t*)&sm->base.ran_func_id) = SM_CCC_ID;
  sm->base.free_sm = free_ccc_sm_ric;
  sm->base.alloc.free_subs_data_msg = free_noop;
  sm->base.alloc.free_ind_data = free_noop;
  sm->base.alloc.free_ctrl_req_data = free_noop;
  sm->base.alloc.free_ctrl_out_data = free_noop;
  sm->base.alloc.free_e2_setup = free_noop;
  sm->base.alloc.free_ric_service_update = free_noop;
  sm->base.proc.on_subscription = on_subscription_ccc_sm_ric;
  sm->base.proc.on_indication = on_indication_ccc_sm_ric;
  sm->base.proc.on_control_req = on_control_req_ccc_sm_ric;
  sm->base.proc.on_control_out = on_control_out_ccc_sm_ric;
  sm->base.proc.on_e2_setup = on_e2_setup_ccc_sm_ric;
  sm->base.proc.on_ric_service_update = on_ric_service_update_ccc_sm_ric;
  sm->base.handle = NULL;
  assert(strlen(SM_CCC_SHORT_NAME) < sizeof(sm->base.ran_func_name));
  memcpy(sm->base.ran_func_name, SM_CCC_SHORT_NAME, strlen(SM_CCC_SHORT_NAME));
  return &sm->base;
}
