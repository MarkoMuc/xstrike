#ifndef __XSTRIKE_TYPES
#define __XSTRIKE_TYPES

#include "linux/types.h"
#include "regex.h"

struct FileData {
  struct rgx_pattern pattern;
  char *data;
  size_t count;
  size_t size;
  u64 id;
  bool processed;
};

#endif
