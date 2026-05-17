#include "ocudu_prb_control_common.h"

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_int_range(const char* text, int min_value, int max_value, int* out)
{
  if (text == NULL || *text == '\0' || out == NULL)
    return false;
  errno = 0;
  char* end = NULL;
  long value = strtol(text, &end, 0);
  if (errno != 0 || end == text || *end != '\0')
    return false;
  if (value < min_value || value > max_value)
    return false;
  *out = (int)value;
  return true;
}

static bool parse_uint32(const char* text, uint32_t* out)
{
  if (text == NULL || *text == '\0' || out == NULL)
    return false;
  errno = 0;
  char* end = NULL;
  unsigned long value = strtoul(text, &end, 0);
  if (errno != 0 || end == text || *end != '\0' || value > UINT32_MAX)
    return false;
  *out = (uint32_t)value;
  return true;
}

static bool parse_sd(const char* text, unsigned* out)
{
  if (text == NULL || *text == '\0' || out == NULL)
    return false;
  errno = 0;
  char* end = NULL;
  unsigned long value = strtoul(text, &end, 0);
  if (errno != 0 || end == text || *end != '\0' || value > 0xFFFFFFul)
    return false;
  *out = (unsigned)value;
  return true;
}

static bool all_digits(const char* text)
{
  for (const char* ptr = text; ptr != NULL && *ptr != '\0'; ++ptr) {
    if (!isdigit((unsigned char)*ptr))
      return false;
  }
  return true;
}

void ocudu_prb_args_init(ocudu_prb_args_t* args)
{
  memset(args, 0, sizeof(*args));
  strncpy(args->ric_addr, OCUDU_PRB_DEFAULT_RIC, sizeof(args->ric_addr) - 1);
  args->port = OCUDU_PRB_DEFAULT_PORT;
  strncpy(args->plmn, OCUDU_PRB_DEFAULT_PLMN, sizeof(args->plmn) - 1);
  args->sst = OCUDU_PRB_DEFAULT_SST;
  args->sd = OCUDU_PRB_DEFAULT_SD;
  args->dedicated_prb = 0;
  strncpy(args->conf_path, OCUDU_PRB_DEFAULT_CONF, sizeof(args->conf_path) - 1);
  strncpy(args->nci_hex, OCUDU_PRB_DEFAULT_NCI_HEX, sizeof(args->nci_hex) - 1);
}

bool ocudu_prb_parse_args(int argc, char** argv, bool require_du_ue_id, ocudu_prb_args_t* args, char* err, size_t err_len)
{
  static const struct option options[] = {
      {"ric", required_argument, NULL, 1},
      {"port", required_argument, NULL, 2},
      {"plmn", required_argument, NULL, 3},
      {"sst", required_argument, NULL, 4},
      {"sd", required_argument, NULL, 5},
      {"min-prb-policy-ratio", required_argument, NULL, 6},
      {"max-prb-policy-ratio", required_argument, NULL, 7},
      {"dedicated-ratio", required_argument, NULL, 8},
      {"du-ue-id", required_argument, NULL, 9},
      {"json", no_argument, NULL, 10},
      {"conf", required_argument, NULL, 11},
      {"nci", required_argument, NULL, 12},
      {"help", no_argument, NULL, 'h'},
      {0, 0, 0, 0},
  };

  ocudu_prb_args_init(args);
  optind = 1;
  int opt = 0;
  while ((opt = getopt_long(argc, argv, "h", options, NULL)) != -1) {
    switch (opt) {
      case 1:
        strncpy(args->ric_addr, optarg, sizeof(args->ric_addr) - 1);
        args->ric_addr[sizeof(args->ric_addr) - 1] = '\0';
        break;
      case 2:
        if (!parse_int_range(optarg, 1, 65535, &args->port)) {
          snprintf(err, err_len, "invalid --port");
          return false;
        }
        break;
      case 3:
        if ((strlen(optarg) != 5 && strlen(optarg) != 6) || !all_digits(optarg)) {
          snprintf(err, err_len, "invalid --plmn");
          return false;
        }
        strncpy(args->plmn, optarg, sizeof(args->plmn) - 1);
        args->plmn[sizeof(args->plmn) - 1] = '\0';
        break;
      case 4:
        if (!parse_int_range(optarg, 0, 255, &args->sst)) {
          snprintf(err, err_len, "invalid --sst");
          return false;
        }
        break;
      case 5:
        if (!parse_sd(optarg, &args->sd)) {
          snprintf(err, err_len, "invalid --sd");
          return false;
        }
        args->sd_present = true;
        break;
      case 6:
        if (!parse_int_range(optarg, 0, 100, &args->min_prb)) {
          snprintf(err, err_len, "invalid --min-prb-policy-ratio");
          return false;
        }
        args->min_present = true;
        break;
      case 7:
        if (!parse_int_range(optarg, 0, 100, &args->max_prb)) {
          snprintf(err, err_len, "invalid --max-prb-policy-ratio");
          return false;
        }
        args->max_present = true;
        break;
      case 8:
        if (!parse_int_range(optarg, 0, 100, &args->dedicated_prb)) {
          snprintf(err, err_len, "invalid --dedicated-ratio");
          return false;
        }
        args->dedicated_present = true;
        break;
      case 9:
        if (!parse_uint32(optarg, &args->du_ue_id)) {
          snprintf(err, err_len, "invalid --du-ue-id");
          return false;
        }
        args->du_ue_id_present = true;
        break;
      case 10:
        args->json = true;
        break;
      case 11:
        strncpy(args->conf_path, optarg, sizeof(args->conf_path) - 1);
        args->conf_path[sizeof(args->conf_path) - 1] = '\0';
        break;
      case 12:
        if (strlen(optarg) >= sizeof(args->nci_hex)) {
          snprintf(err, err_len, "invalid --nci");
          return false;
        }
        strncpy(args->nci_hex, optarg, sizeof(args->nci_hex) - 1);
        args->nci_hex[sizeof(args->nci_hex) - 1] = '\0';
        for (char* ptr = args->nci_hex; *ptr != '\0'; ++ptr)
          *ptr = (char)toupper((unsigned char)*ptr);
        break;
      case 'h':
        snprintf(err, err_len, "usage requested");
        return false;
      default:
        snprintf(err, err_len, "unknown argument");
        return false;
    }
  }

  if (!args->min_present || !args->max_present) {
    snprintf(err, err_len, "missing required PRB min/max policy ratio");
    return false;
  }
  if (args->min_prb > args->max_prb) {
    snprintf(err, err_len, "min PRB policy ratio must be <= max PRB policy ratio");
    return false;
  }
  if (require_du_ue_id && !args->du_ue_id_present) {
    snprintf(err, err_len, "missing required --du-ue-id");
    return false;
  }
  return true;
}

bool ocudu_split_plmn(const char* plmn, ocudu_plmn_parts_t* parts)
{
  if (plmn == NULL || parts == NULL)
    return false;
  size_t len = strlen(plmn);
  if ((len != 5 && len != 6) || !all_digits(plmn))
    return false;
  memset(parts, 0, sizeof(*parts));
  memcpy(parts->mcc, plmn, 3);
  parts->mcc[3] = '\0';
  memcpy(parts->mnc, plmn + 3, len - 3);
  parts->mnc[len - 3] = '\0';
  parts->mnc_digit_len = (int)(len - 3);
  return true;
}

byte_array_t ocudu_plmn_to_bcd(const char* plmn)
{
  ocudu_plmn_parts_t parts = {0};
  if (!ocudu_split_plmn(plmn, &parts)) {
    return (byte_array_t){0};
  }
  uint8_t mcc1 = (uint8_t)(parts.mcc[0] - '0');
  uint8_t mcc2 = (uint8_t)(parts.mcc[1] - '0');
  uint8_t mcc3 = (uint8_t)(parts.mcc[2] - '0');
  uint8_t mnc1 = (uint8_t)(parts.mnc[0] - '0');
  uint8_t mnc2 = (uint8_t)(parts.mnc[1] - '0');
  uint8_t mnc3 = parts.mnc_digit_len == 3 ? (uint8_t)(parts.mnc[2] - '0') : 0x0f;

  byte_array_t dst = {.len = 3, .buf = calloc(3, sizeof(uint8_t))};
  if (dst.buf == NULL)
    return (byte_array_t){0};
  dst.buf[0] = (uint8_t)((mcc2 << 4) | mcc1);
  dst.buf[1] = (uint8_t)((mnc3 << 4) | mcc3);
  dst.buf[2] = (uint8_t)((mnc2 << 4) | mnc1);
  return dst;
}

byte_array_t ocudu_uint_to_octets(unsigned value, size_t len)
{
  byte_array_t dst = {.len = len, .buf = calloc(len, sizeof(uint8_t))};
  if (dst.buf == NULL)
    return (byte_array_t){0};
  for (size_t i = 0; i < len; ++i) {
    size_t shift = (len - 1 - i) * 8;
    dst.buf[i] = (uint8_t)((value >> shift) & 0xffu);
  }
  return dst;
}

void ocudu_sd_to_hex(unsigned sd, char out[7])
{
  snprintf(out, 7, "%06X", sd & 0xFFFFFFu);
}

void ocudu_print_error_json(const char* action_type, const char* error)
{
  printf("{\"action_type\":\"%s\",\"accepted\":false,\"error\":\"%s\"}\n", action_type, error);
}

void ocudu_print_success_json(
    const char* action_type,
    int ran_function_id,
    const char* control_name,
    int control_style,
    int control_action,
    const ocudu_prb_args_t* args,
    const char* outcome_text)
{
  char sd_hex[7] = {0};
  ocudu_sd_to_hex(args->sd, sd_hex);
  printf(
      "{\"accepted\":true,"
      "\"action_type\":\"%s\","
      "\"ran_function_id\":%d,"
      "\"control_name\":\"%s\","
      "\"control_style\":%d,"
      "\"control_action\":%d,"
      "\"request\":{\"ric\":\"%s\",\"port\":%d,\"plmn\":\"%s\",\"sst\":%d,\"sd\":\"%s\","
      "\"min_prb_policy_ratio\":%d,\"max_prb_policy_ratio\":%d,\"dedicated_ratio\":%d",
      action_type,
      ran_function_id,
      control_name,
      control_style,
      control_action,
      args->ric_addr,
      args->port,
      args->plmn,
      args->sst,
      sd_hex,
      args->min_prb,
      args->max_prb,
      args->dedicated_prb);
  if (args->du_ue_id_present)
    printf(",\"du_ue_id\":%u", args->du_ue_id);
  printf("},\"outcome\":{\"acknowledged\":true,\"evidence\":\"%s\"}}\n", outcome_text);
}
