#include "ocudu/asn1/e2sm/e2sm_rc_ies.h"
#include "ocudu/adt/byte_buffer.h"

extern "C" {
#include "ocudu_prb_control_common.h"

#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/util/e2ap_ngran_types.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/xApp/e42_xapp_api.h"

#include <getopt.h>
#include <unistd.h>
}

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace asn1::e2sm;

namespace {

constexpr int RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION = 2;
constexpr int RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA    = 6;

constexpr uint64_t RRM_POLICY_RATIO_LIST       = 1;
constexpr uint64_t RRM_POLICY_RATIO_GROUP      = 2;
constexpr uint64_t RRM_POLICY                  = 3;
constexpr uint64_t RRM_POLICY_MEMBER_LIST      = 5;
constexpr uint64_t RRM_POLICY_MEMBER           = 6;
constexpr uint64_t PLMN_IDENTITY               = 7;
constexpr uint64_t S_NSSAI                     = 8;
constexpr uint64_t SST                         = 9;
constexpr uint64_t SD                          = 10;
constexpr uint64_t MIN_PRB_POLICY_RATIO        = 11;
constexpr uint64_t MAX_PRB_POLICY_RATIO        = 12;
constexpr uint64_t DEDICATED_PRB_POLICY_RATIO  = 13;

void set_octet_value(ran_param_value_type_c& value_type, const std::vector<uint8_t>& value)
{
  auto& elem_false = value_type.set_ran_p_choice_elem_false();
  elem_false.ran_param_value_present = true;
  elem_false.ran_param_value.set_value_oct_s().from_bytes(ocudu::span<const uint8_t>(value.data(), value.size()));
}

void set_integer_value(ran_param_value_type_c& value_type, int value)
{
  auto& elem_false = value_type.set_ran_p_choice_elem_false();
  elem_false.ran_param_value_present = true;
  elem_false.ran_param_value.set_value_int() = value;
}

std::vector<uint8_t> byte_array_to_vector(byte_array_t value)
{
  std::vector<uint8_t> out;
  if (value.buf != nullptr && value.len > 0) {
    out.assign(value.buf, value.buf + value.len);
  }
  free_byte_array(value);
  return out;
}

byte_array_t vector_to_byte_array(const std::vector<uint8_t>& value)
{
  byte_array_t out = {};
  out.len          = value.size();
  out.buf          = static_cast<uint8_t*>(std::malloc(value.size()));
  assert(out.buf != nullptr && "Memory exhausted");
  std::memcpy(out.buf, value.data(), value.size());
  return out;
}

bool pack_ctrl_header(uint32_t du_ue_id, std::vector<uint8_t>& out)
{
  asn1::e2sm::e2sm_rc_ctrl_hdr_s ctrl_hdr;
  ctrl_hdr.ric_ctrl_hdr_formats.set_ctrl_hdr_format1();
  auto& hdr_f1 = ctrl_hdr.ric_ctrl_hdr_formats.ctrl_hdr_format1();
  hdr_f1.ric_ctrl_decision_present  = false;
  hdr_f1.ric_style_type             = RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION;
  hdr_f1.ric_ctrl_action_id         = RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA;
  hdr_f1.ue_id.set_gnb_du_ue_id();
  hdr_f1.ue_id.gnb_du_ue_id().gnb_cu_ue_f1ap_id = du_ue_id;

  ocudu::byte_buffer buffer;
  asn1::bit_ref      bref(buffer);
  if (ctrl_hdr.pack(bref) != asn1::OCUDUASN_SUCCESS) {
    return false;
  }
  out.assign(buffer.begin(), buffer.end());
  return !out.empty();
}

bool pack_ctrl_message(const ocudu_prb_args_t& args, std::vector<uint8_t>& out)
{
  asn1::e2sm::e2sm_rc_ctrl_msg_s ctrl_msg;
  ctrl_msg.ric_ctrl_msg_formats.set_ctrl_msg_format1();
  auto& msg_f1 = ctrl_msg.ric_ctrl_msg_formats.ctrl_msg_format1();

  msg_f1.ran_p_list.resize(1);
  auto& rrm_policy_ratio_list  = msg_f1.ran_p_list[0];
  rrm_policy_ratio_list.ran_param_id = RRM_POLICY_RATIO_LIST;
  rrm_policy_ratio_list.ran_param_value_type.set_ran_p_choice_list().ran_param_list.list_of_ran_param.resize(1);
  rrm_policy_ratio_list.ran_param_value_type.ran_p_choice_list()
      .ran_param_list.list_of_ran_param[0]
      .seq_of_ran_params.resize(1);

  auto& ratio_group = rrm_policy_ratio_list.ran_param_value_type.ran_p_choice_list()
                          .ran_param_list.list_of_ran_param[0]
                          .seq_of_ran_params[0];
  ratio_group.ran_param_id = RRM_POLICY_RATIO_GROUP;
  ratio_group.ran_param_value_type.set_ran_p_choice_structure().ran_param_structure.seq_of_ran_params.resize(4);

  auto& rrm_policy = ratio_group.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[0];
  rrm_policy.ran_param_id = RRM_POLICY;
  rrm_policy.ran_param_value_type.set_ran_p_choice_structure().ran_param_structure.seq_of_ran_params.resize(1);

  auto& member_list = rrm_policy.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[0];
  member_list.ran_param_id = RRM_POLICY_MEMBER_LIST;
  member_list.ran_param_value_type.set_ran_p_choice_list().ran_param_list.list_of_ran_param.resize(1);
  member_list.ran_param_value_type.ran_p_choice_list()
      .ran_param_list.list_of_ran_param[0]
      .seq_of_ran_params.resize(1);

  auto& member = member_list.ran_param_value_type.ran_p_choice_list()
                     .ran_param_list.list_of_ran_param[0]
                     .seq_of_ran_params[0];
  member.ran_param_id = RRM_POLICY_MEMBER;
  member.ran_param_value_type.set_ran_p_choice_structure().ran_param_structure.seq_of_ran_params.resize(2);

  auto& plmn = member.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[0];
  plmn.ran_param_id = PLMN_IDENTITY;
  set_octet_value(plmn.ran_param_value_type, byte_array_to_vector(ocudu_plmn_to_bcd(args.plmn)));

  auto& snssai = member.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[1];
  snssai.ran_param_id = S_NSSAI;
  snssai.ran_param_value_type.set_ran_p_choice_structure().ran_param_structure.seq_of_ran_params.resize(2);

  auto& sst = snssai.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[0];
  sst.ran_param_id = SST;
  set_octet_value(sst.ran_param_value_type, byte_array_to_vector(ocudu_uint_to_octets(static_cast<unsigned>(args.sst), 1)));

  auto& sd = snssai.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[1];
  sd.ran_param_id = SD;
  set_octet_value(sd.ran_param_value_type, byte_array_to_vector(ocudu_uint_to_octets(args.sd, 3)));

  auto& min_prb = ratio_group.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[1];
  min_prb.ran_param_id = MIN_PRB_POLICY_RATIO;
  set_integer_value(min_prb.ran_param_value_type, args.min_prb);

  auto& max_prb = ratio_group.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[2];
  max_prb.ran_param_id = MAX_PRB_POLICY_RATIO;
  set_integer_value(max_prb.ran_param_value_type, args.max_prb);

  auto& dedicated_prb = ratio_group.ran_param_value_type.ran_p_choice_structure().ran_param_structure.seq_of_ran_params[3];
  dedicated_prb.ran_param_id = DEDICATED_PRB_POLICY_RATIO;
  set_integer_value(dedicated_prb.ran_param_value_type, args.dedicated_prb);

  ocudu::byte_buffer buffer;
  asn1::bit_ref      bref(buffer);
  if (ctrl_msg.pack(bref) != asn1::OCUDUASN_SUCCESS) {
    return false;
  }
  out.assign(buffer.begin(), buffer.end());
  return !out.empty();
}

void stop_xapp()
{
  for (int i = 0; i < 1000; ++i) {
    if (try_stop_xapp_api()) {
      return;
    }
    usleep(1000);
  }
}

bool node_supports_ran_function(const e2_node_connected_t* node, int ran_function_id)
{
  for (size_t i = 0; i < node->len_rf; ++i) {
    if (node->ack_rf[i].id == ran_function_id) {
      return true;
    }
  }
  return false;
}

} // namespace

int main(int argc, char* argv[])
{
  ocudu_prb_args_t bench_args = {};
  char             error[256] = {};
  if (!ocudu_prb_parse_args(argc, argv, true, &bench_args, error, sizeof(error))) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_RC_DU", error);
    return 2;
  }

  std::vector<uint8_t> ctrl_hdr;
  std::vector<uint8_t> ctrl_msg;
  if (!pack_ctrl_header(bench_args.du_ue_id, ctrl_hdr) || !pack_ctrl_message(bench_args, ctrl_msg)) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_RC_DU", "failed to encode OCUDU E2SM-RC control payload");
    return 2;
  }

  char*     fr_argv[] = {(char*)"ocudu-rc-du-prb-control", (char*)"-c", bench_args.conf_path};
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

  int64_t start = time_now_us();
  size_t  target_count = 0;
  for (size_t i = 0; i < nodes.len; ++i) {
    if (!E2AP_NODE_IS_DU(nodes.n[i].id.type) || !node_supports_ran_function(&nodes.n[i], SM_RC_ID)) {
      continue;
    }
    byte_array_t hdr_ba = vector_to_byte_array(ctrl_hdr);
    byte_array_t msg_ba = vector_to_byte_array(ctrl_msg);
    control_sm_raw_xapp_api(&nodes.n[i].id, SM_RC_ID, hdr_ba, msg_ba);
    free_byte_array(hdr_ba);
    free_byte_array(msg_ba);
    ++target_count;
  }
  int64_t elapsed_us = time_now_us() - start;
  (void)elapsed_us;

  free_e2_node_arr(&nodes);
  stop_xapp();
  free_fr_args(&fr_args);

  if (target_count == 0) {
    ocudu_print_error_json("SET_PRB_POLICY_RATIO_RC_DU", "no DU E2 node advertises E2SM-RC");
    return 3;
  }

  ocudu_print_success_json("SET_PRB_POLICY_RATIO_RC_DU",
                           SM_RC_ID,
                           "slice-level PRB quota",
                           RC_CTRL_STYLE_RADIO_RESOURCE_ALLOCATION,
                           RC_CTRL_ACTION_SLICE_LEVEL_PRB_QUOTA,
                           &bench_args,
                           "OCUDU E2SM-RC control acknowledged");
  return 0;
}
