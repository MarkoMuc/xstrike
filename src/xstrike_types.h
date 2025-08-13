#ifndef __XSTRIKE_TYPES
#define __XSTRIKE_TYPES

#include "regex_types.h"
#include "xstrike_ioctl.h"
#include <linux/stddef.h>
#include <linux/types.h>

enum xstrike_err {
  XSTRIKE_SUCC = 0,
  XSTRIKE_ERR_MEM = 1,
  XSTRIKE_ERR_IDX_OVER = 2,
  XSTRIKE_ERR_IDX_UNDER = 3,
  XSTRIKE_ERR_NOT_INIT = 4,
  XSTRIKE_ERR_INVALID_RGX = 5,
  XSTRIKE_ERR_NULLPTR = 6,
  XSTRIKE_ERR_MISSING_PATTERN = 7
};

typedef enum xstrike_err xstrike_err_t;

struct FileData {
  struct rgx_pattern rules;
  char *data;
  size_t len;
  size_t size;
  u64 id;
  bool processed;
  struct rgx_node *pattern;
};

#endif
