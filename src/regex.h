#ifndef __XSTRIKE_REGEX
#define __XSTRIKE_REGEX

#include "../xstrike_ioctl.h"
#include "../xstrike_types.h"

#include <linux/types.h>

static inline bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static inline bool is_num(char c) { return '0' <= c && c <= '9'; }

static inline bool is_alhanum(char c) { return is_alpha(c) && is_num(c); }

#endif
