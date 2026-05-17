#ifndef OCUDU_PRB_CONTROL_COMMON_H
#define OCUDU_PRB_CONTROL_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../../../src/util/byte_array.h"

#define OCUDU_PRB_DEFAULT_CONF "/usr/local/etc/flexric/xapp_oran_sm.conf"
#define OCUDU_PRB_DEFAULT_RIC "127.0.0.1"
#define OCUDU_PRB_DEFAULT_PORT 36421
#define OCUDU_PRB_DEFAULT_PLMN "00101"
#define OCUDU_PRB_DEFAULT_SST 1
#define OCUDU_PRB_DEFAULT_SD 0xFFFFFFu
#define OCUDU_PRB_DEFAULT_NCI_HEX "00066C000"

typedef struct {
  char ric_addr[64];
  int port;
  char plmn[8];
  int sst;
  bool sd_present;
  unsigned sd;
  bool min_present;
  bool max_present;
  int min_prb;
  int max_prb;
  bool dedicated_present;
  int dedicated_prb;
  bool du_ue_id_present;
  uint32_t du_ue_id;
  bool json;
  char conf_path[256];
  char nci_hex[16];
} ocudu_prb_args_t;

typedef struct {
  char mcc[4];
  char mnc[4];
  int mnc_digit_len;
} ocudu_plmn_parts_t;

void ocudu_prb_args_init(ocudu_prb_args_t* args);
bool ocudu_prb_parse_args(int argc, char** argv, bool require_du_ue_id, ocudu_prb_args_t* args, char* err, size_t err_len);
bool ocudu_split_plmn(const char* plmn, ocudu_plmn_parts_t* parts);
byte_array_t ocudu_plmn_to_bcd(const char* plmn);
byte_array_t ocudu_uint_to_octets(unsigned value, size_t len);
void ocudu_sd_to_hex(unsigned sd, char out[7]);
void ocudu_print_error_json(const char* action_type, const char* error);
void ocudu_print_success_json(
    const char* action_type,
    int ran_function_id,
    const char* control_name,
    int control_style,
    int control_action,
    const ocudu_prb_args_t* args,
    const char* outcome_text);

#endif
