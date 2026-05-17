#include "ocudu_prb_control_common.h"

#include "../../../../src/sm/ccc_sm/ccc_sm_id.h"
#include "../../../../src/sm/ccc_sm/ccc_sm_ric.h"
#include "../../../../src/util/e2ap_ngran_types.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/xApp/e42_xapp_api.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
  CCC_CTRL_STYLE_CELL_CONFIG = 2,
  CCC_CTRL_ACTION_O_RRM_POLICY_RATIO = 6,
} ocudu_ccc_ids_e;

static bool build_rrm_policy_json(const ocudu_prb_args_t* args, char* out, size_t out_len)
{
  ocudu_plmn_parts_t plmn = {0};
  if (!ocudu_split_plmn(args->plmn, &plmn))
    return false;
  char sd_hex[7] = {0};
  ocudu_sd_to_hex(args->sd, sd_hex);
  int rc = snprintf(
      out,
      out_len,
      "{\"resourceType\":\"PRB_DL\","
      "\"rRMPolicyMemberList\":[{\"plmnId\":{\"mcc\":\"%s\",\"mnc\":\"%s\"},"
      "\"snssai\":{\"sst\":%d,\"sd\":\"%s\"}}],"
      "\"rRMPolicyMaxRatio\":%d,"
      "\"rRMPolicyMinRatio\":%d,"
      "\"rRMPolicyDedicatedRatio\":%d}",
      plmn.mcc,
      plmn.mnc,
      args->sst,
      sd_hex,
      args->max_prb,
      args->min_prb,
      args->dedicated_prb);
  return rc > 0 && (size_t)rc < out_len;
}

static bool build_ccc_json(const ocudu_prb_args_t* args, char* hdr, size_t hdr_len, char* msg, size_t msg_len)
{
  ocudu_plmn_parts_t plmn = {0};
  if (!ocudu_split_plmn(args->plmn, &plmn))
    return false;

  char policy_json[768] = {0};
  if (!build_rrm_policy_json(args, policy_json, sizeof(policy_json)))
    return false;

  int hdr_rc = snprintf(hdr, hdr_len, "{\"controlHeaderFormat\":{\"ricStyleType\":%d}}", CCC_CTRL_STYLE_CELL_CONFIG);
  if (hdr_rc <= 0 || (size_t)hdr_rc >= hdr_len)
    return false;

  int msg_rc = snprintf(
      msg,
      msg_len,
      "{\"controlMessageFormat\":{\"listOfCellsControlled\":[{"
      "\"cellGlobalId\":{\"plmnIdentity\":{\"mcc\":\"%s\",\"mnc\":\"%s\"},\"nRCellIdentity\":\"%s\"},"
      "\"listOfConfigurationStructures\":[{"
      "\"ranConfigurationStructureName\":\"O-RRMPolicyRatio\","
      "\"oldValuesOfAttributes\":{\"ranConfigurationStructure\":%s},"
      "\"newValuesOfAttributes\":{\"ranConfigurationStructure\":%s}"
      "}]}]}}",
      plmn.mcc,
      plmn.mnc,
      args->nci_hex,
      policy_json,
      policy_json);
  return msg_rc > 0 && (size_t)msg_rc < msg_len;
}

static void stop_xapp(void)
{
  for (int i = 0; i < 1000; ++i) {
    if (try_stop_xapp_api())
      return;
    usleep(1000);
  }
}

static bool node_supports_ran_function(const e2_node_connected_t* node, int ran_function_id)
{
  for (size_t i = 0; i < node->len_rf; ++i) {
    if (node->ack_rf[i].id == ran_function_id)
      return true;
  }
  return false;
}

int main(int argc, char* argv[])
{
  ocudu_prb_args_t bench_args = {0};
  char error[256] = {0};
  if (!ocudu_prb_parse_args(argc, argv, false, &bench_args, error, sizeof(error))) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_CCC", error);
    return 2;
  }

  char hdr_json[256] = {0};
  char msg_json[1792] = {0};
  if (!build_ccc_json(&bench_args, hdr_json, sizeof(hdr_json), msg_json, sizeof(msg_json))) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_CCC", "failed to build CCC control JSON");
    return 2;
  }

  char* fr_argv[] = {(char*)"ocudu-ccc-prb-control", (char*)"-c", bench_args.conf_path};
  optind = 1;
  fr_args_t fr_args = init_fr_args(3, fr_argv);
  init_xapp_api(&fr_args);
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  if (nodes.len == 0) {
    free_e2_node_arr(&nodes);
    stop_xapp();
    free_fr_args(&fr_args);
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_CCC", "no E2 nodes connected");
    return 3;
  }

  ccc_ctrl_req_data_t ccc_ctrl = {.ctrl_hdr_json = hdr_json, .ctrl_msg_json = msg_json};
  int64_t start = time_now_us();
  size_t target_count = 0;
  bool control_failed = false;
  char control_error[256] = {0};
  for (size_t i = 0; i < nodes.len; ++i) {
    if (!E2AP_NODE_IS_DU(nodes.n[i].id.type) || !node_supports_ran_function(&nodes.n[i], SM_CCC_ID))
      continue;
    sm_ans_xapp_t ans = control_sm_xapp_api(&nodes.n[i].id, SM_CCC_ID, &ccc_ctrl);
    if (!ans.success) {
      snprintf(control_error, sizeof(control_error), "%s", ans.u.reason != NULL ? ans.u.reason : "E2SM-CCC control failed");
      control_failed = true;
      break;
    }
    ++target_count;
  }
  int64_t elapsed_us = time_now_us() - start;
  (void)elapsed_us;

  free_e2_node_arr(&nodes);
  stop_xapp();
  free_fr_args(&fr_args);

  if (target_count == 0) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_CCC", "no DU E2 node advertises E2SM-CCC");
    return 3;
  }
  if (control_failed) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_CCC", control_error);
    return 4;
  }

  ocudu_print_success_json(
      "SET_PRB_POLICY_RATIO_CCC",
      SM_CCC_ID,
      "O-RRMPolicyRatio",
      CCC_CTRL_STYLE_CELL_CONFIG,
      CCC_CTRL_ACTION_O_RRM_POLICY_RATIO,
      &bench_args,
      "FlexRIC E2SM-CCC control acknowledged");
  return 0;
}
