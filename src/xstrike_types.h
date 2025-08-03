#ifndef __XSTRIKE_TYPES
#define __XSTRIKE_TYPES

#include "xstrike_ioctl.h"
#include <linux/stddef.h>
#include <linux/types.h>

struct FileData {
  struct rgx_pattern rules;
  char *data;
  size_t count;
  size_t size;
  u64 id;
  bool processed;
};

#endif
