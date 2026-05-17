#include "ccc_sm_agent.h"
#include "ccc_sm_id.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  sm_agent_t base;
} sm_ccc_agent_t;

static subscribe_timer_t on_subscription_ccc_sm_ag(sm_agent_t const* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  return (subscribe_timer_t){.ms = 0, .type = NONE_SUB_DATA_ENUM};
}

static sm_ind_data_t on_indication_ccc_sm_ag(sm_agent_t const* sm_agent, void* act_def)
{
  assert(sm_agent != NULL);
  (void)act_def;
  return (sm_ind_data_t){0};
}

static sm_ctrl_out_data_t on_control_ccc_sm_ag(sm_agent_t const* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  return (sm_ctrl_out_data_t){0};
}

static sm_e2_setup_data_t on_e2_setup_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);

  sm_e2_setup_data_t setup = {0};
  const size_t len = strnlen(SM_CCC_SHORT_NAME, 256);
  assert(len > 0 && len < 256);
  setup.ran_fun_def = calloc(len, sizeof(uint8_t));
  assert(setup.ran_fun_def != NULL && "Memory exhausted");
  memcpy(setup.ran_fun_def, SM_CCC_SHORT_NAME, len);
  setup.len_rfd = len;
  return setup;
}

static sm_ric_service_update_data_t on_ric_service_update_ccc_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);
  return (sm_ric_service_update_data_t){0};
}

#ifdef E2AP_V3
static sm_ric_query_out_data_t on_ric_query_ccc_sm_ag(sm_agent_t const* sm_agent, sm_ric_query_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  return (sm_ric_query_out_data_t){0};
}

static void on_subscription_mod_ccc_sm_ag(sm_agent_t const* sm_agent, sm_sub_mod_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
}
#endif

static void free_ccc_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  free((sm_ccc_agent_t*)sm_agent);
}

static char const* def_ccc_sm_ag(void)
{
  return SM_CCC_DESCRIPTION;
}

static uint16_t id_ccc_sm_ag(void)
{
  return SM_CCC_ID;
}

static uint16_t rev_ccc_sm_ag(void)
{
  return SM_CCC_REV;
}

static char const* oid_ccc_sm_ag(void)
{
  return SM_CCC_OID;
}

__attribute__((visibility("default"))) sm_agent_t* make_ccc_sm_agent(sm_io_ag_ran_t io)
{
  sm_ccc_agent_t* sm = calloc(1, sizeof(sm_ccc_agent_t));
  assert(sm != NULL && "Memory exhausted");
  (void)io;

  sm->base.free_sm = free_ccc_sm_ag;
  sm->base.free_act_def = NULL;
  sm->base.proc.on_subscription = on_subscription_ccc_sm_ag;
  sm->base.proc.on_indication = on_indication_ccc_sm_ag;
  sm->base.proc.on_control = on_control_ccc_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_ccc_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_ccc_sm_ag;
#ifdef E2AP_V3
  sm->base.proc.on_ric_query = on_ric_query_ccc_sm_ag;
  sm->base.proc.on_subscription_mod = on_subscription_mod_ccc_sm_ag;
#endif
  sm->base.handle = NULL;
  sm->base.info.def = def_ccc_sm_ag;
  sm->base.info.id = id_ccc_sm_ag;
  sm->base.info.rev = rev_ccc_sm_ag;
  sm->base.info.oid = oid_ccc_sm_ag;
  return &sm->base;
}
