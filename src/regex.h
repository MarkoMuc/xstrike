#ifndef __XSTRIKE_REGEX
#define __XSTRIKE_REGEX

#include "regex_types.h"
#include "xstrike_ioctl.h"
#include "xstrike_types.h"

#include <linux/printk.h>
#include <linux/types.h>

static inline bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
static inline bool is_num(char c) { return '0' <= c && c <= '9'; }
static inline bool is_alphanum(char c) { return is_alpha(c) && is_num(c); }

xstrike_err_t xstrike_regex_builder(struct rgx_pattern *arg);
xstrike_err_t xstrike_regex_match(struct FileData *pdata, rgx_node *head);

bool rgx_recursive(const char *data, const u64 len, u64 *idx,
                   const rgx_node *rnode);
bool rgx_apply_quant(const char *data, const u64 len, u64 *idx,
                     const rgx_node *rnode);
#endif
