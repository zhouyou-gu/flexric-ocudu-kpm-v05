#include "ocudu_prb_control_common.h"

#include "../../../../src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/sm/rc_sm/ie/rc_data_ie.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/xApp/e42_xapp_api.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
  RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION = 2,
  RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA = 6,
} ocudu_rc_ids_e;

typedef enum {
  RRM_POLICY_RATIO_LIST = 1,
  RRM_POLICY_RATIO_GROUP = 2,
  RRM_POLICY = 3,
  RRM_POLICY_MEMBER_LIST = 5,
  RRM_POLICY_MEMBER = 6,
  PLMN_IDENTITY = 7,
  S_NSSAI = 8,
  SST = 9,
  SD = 10,
  MIN_PRB_POLICY_RATIO = 11,
  MAX_PRB_POLICY_RATIO = 12,
  DEDICATED_PRB_POLICY_RATIO = 13,
} ocudu_rc_prb_quota_param_id_e;

static void set_element_integer(seq_ran_param_t* param, uint32_t id, int value)
{
  param->ran_param_id = id;
  param->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  param->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(param->ran_param_val.flag_false != NULL && "Memory exhausted");
  param->ran_param_val.flag_false->type = INTEGER_RAN_PARAMETER_VALUE;
  param->ran_param_val.flag_false->int_ran = value;
}

static void set_element_octets(seq_ran_param_t* param, uint32_t id, byte_array_t value)
{
  param->ran_param_id = id;
  param->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  param->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(param->ran_param_val.flag_false != NULL && "Memory exhausted");
  param->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  param->ran_param_val.flag_false->octet_str_ran = value;
}

static void gen_rrm_policy_member(lst_ran_param_t* member, const ocudu_prb_args_t* args)
{
  member->ran_param_id = RRM_POLICY_MEMBER;
  member->ran_param_struct.sz_ran_param_struct = 2;
  member->ran_param_struct.ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(member->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  set_element_octets(&member->ran_param_struct.ran_param_struct[0], PLMN_IDENTITY, ocudu_plmn_to_bcd(args->plmn));

  seq_ran_param_t* snssai = &member->ran_param_struct.ran_param_struct[1];
  snssai->ran_param_id = S_NSSAI;
  snssai->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  snssai->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(snssai->ran_param_val.strct != NULL && "Memory exhausted");
  snssai->ran_param_val.strct->sz_ran_param_struct = 2;
  snssai->ran_param_val.strct->ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(snssai->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");
  set_element_octets(&snssai->ran_param_val.strct->ran_param_struct[0], SST, ocudu_uint_to_octets((unsigned)args->sst, 1));
  set_element_octets(&snssai->ran_param_val.strct->ran_param_struct[1], SD, ocudu_uint_to_octets(args->sd, 3));
}

static void gen_rrm_policy_ratio_group(lst_ran_param_t* group, const ocudu_prb_args_t* args)
{
  group->ran_param_id = RRM_POLICY_RATIO_GROUP;
  group->ran_param_struct.sz_ran_param_struct = 4;
  group->ran_param_struct.ran_param_struct = calloc(4, sizeof(seq_ran_param_t));
  assert(group->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  seq_ran_param_t* policy = &group->ran_param_struct.ran_param_struct[0];
  policy->ran_param_id = RRM_POLICY;
  policy->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  policy->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(policy->ran_param_val.strct != NULL && "Memory exhausted");
  policy->ran_param_val.strct->sz_ran_param_struct = 1;
  policy->ran_param_val.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(policy->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");

  seq_ran_param_t* member_list = &policy->ran_param_val.strct->ran_param_struct[0];
  member_list->ran_param_id = RRM_POLICY_MEMBER_LIST;
  member_list->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  member_list->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(member_list->ran_param_val.lst != NULL && "Memory exhausted");
  member_list->ran_param_val.lst->sz_lst_ran_param = 1;
  member_list->ran_param_val.lst->lst_ran_param = calloc(1, sizeof(lst_ran_param_t));
  assert(member_list->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");
  gen_rrm_policy_member(&member_list->ran_param_val.lst->lst_ran_param[0], args);

  set_element_integer(&group->ran_param_struct.ran_param_struct[1], MIN_PRB_POLICY_RATIO, args->min_prb);
  set_element_integer(&group->ran_param_struct.ran_param_struct[2], MAX_PRB_POLICY_RATIO, args->max_prb);
  set_element_integer(&group->ran_param_struct.ran_param_struct[3], DEDICATED_PRB_POLICY_RATIO, args->dedicated_prb);
}

static e2sm_rc_ctrl_hdr_t gen_ctrl_hdr(const ocudu_prb_args_t* args)
{
  e2sm_rc_ctrl_hdr_t hdr = {0};
  hdr.format = FORMAT_1_E2SM_RC_CTRL_HDR;
  hdr.frmt_1.ue_id.type = GNB_DU_UE_ID_E2SM;
  hdr.frmt_1.ue_id.gnb_du.gnb_cu_ue_f1ap = args->du_ue_id;
  hdr.frmt_1.ric_style_type = RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION;
  hdr.frmt_1.ctrl_act_id = RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA;
  return hdr;
}

static e2sm_rc_ctrl_msg_t gen_ctrl_msg(const ocudu_prb_args_t* args)
{
  e2sm_rc_ctrl_msg_t msg = {0};
  msg.format = FORMAT_1_E2SM_RC_CTRL_MSG;
  msg.frmt_1.sz_ran_param = 1;
  msg.frmt_1.ran_param = calloc(1, sizeof(seq_ran_param_t));
  assert(msg.frmt_1.ran_param != NULL && "Memory exhausted");
  seq_ran_param_t* list = &msg.frmt_1.ran_param[0];
  list->ran_param_id = RRM_POLICY_RATIO_LIST;
  list->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  list->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(list->ran_param_val.lst != NULL && "Memory exhausted");
  list->ran_param_val.lst->sz_lst_ran_param = 1;
  list->ran_param_val.lst->lst_ran_param = calloc(1, sizeof(lst_ran_param_t));
  assert(list->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");
  gen_rrm_policy_ratio_group(&list->ran_param_val.lst->lst_ran_param[0], args);
  return msg;
}

static void stop_xapp(void)
{
  for (int i = 0; i < 1000; ++i) {
    if (try_stop_xapp_api())
      return;
    usleep(1000);
  }
}

int main(int argc, char* argv[])
{
  ocudu_prb_args_t bench_args = {0};
  char error[256] = {0};
  if (!ocudu_prb_parse_args(argc, argv, true, &bench_args, error, sizeof(error))) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_RC_DU", error);
    return 2;
  }

  char* fr_argv[] = {(char*)"ocudu-rc-du-prb-control", (char*)"-c", bench_args.conf_path};
  optind = 1;
  fr_args_t fr_args = init_fr_args(3, fr_argv);
  init_xapp_api(&fr_args);
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  if (nodes.len == 0) {
    free_e2_node_arr(&nodes);
    stop_xapp();
    free_fr_args(&fr_args);
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_RC_DU", "no E2 nodes connected");
    return 3;
  }

  rc_ctrl_req_data_t rc_ctrl = {0};
  rc_ctrl.hdr = gen_ctrl_hdr(&bench_args);
  rc_ctrl.msg = gen_ctrl_msg(&bench_args);

  int64_t start = time_now_us();
  for (size_t i = 0; i < nodes.len; ++i) {
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
  }
  int64_t elapsed_us = time_now_us() - start;
  (void)elapsed_us;

  free_rc_ctrl_req_data(&rc_ctrl);
  free_e2_node_arr(&nodes);
  stop_xapp();
  free_fr_args(&fr_args);

  ocudu_print_success_json(
      "SET_PRB_POLICY_RATIO_RC_DU",
      SM_RC_ID,
      "slice-level PRB quota",
      RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION,
      RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA,
      &bench_args,
      "FlexRIC E2SM-RC control acknowledged");
  return 0;
}
