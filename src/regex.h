#ifndef __XSTRIKE_REGEX
#define __XSTRIKE_REGEX

#include "xstrike_ioctl.h"
#include "xstrike_types.h"

#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/types.h>

#define RGX_NODE_START_SIZE 8

enum rgx_type {
  RGX_TYPE_SEQ = 0,         // Sequence
  RGX_TYPE_COND = 1,        // |
  RGX_TYPE_CHARSET = 2,     // [ ]
  RGX_TYPE_GROUP = 3,       // ( )
  RGX_TYPE_LITERAL = 4,     // " " or str
  RGX_TYPE_SPEC_LITERAL = 5 // " " or str in context
};

enum rgx_quantifiers {
  RGX_QUANT_NONE = 0,    // No quantifier
  RGX_QUANT_STAR = 1,    // +
  RGX_QUANT_PLUS = 2,    // *
  RGX_QUANT_QUES = 3,    // ?
  RGX_QUANT_WILDCARD = 4 // . -> technically not a quantifier
};

typedef struct rgx_node rgx_node;

struct rgx_node_array {
  rgx_node **items;
  size_t len;
  size_t size;
};

typedef struct rgx_node_array rgx_node_array;

struct rgx_node {
  enum rgx_type type;
  enum rgx_quantifiers quant;
  const char *str;
  u64 len;
  rgx_node *father;
  rgx_node_array body;
};

static inline bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
static inline bool is_num(char c) { return '0' <= c && c <= '9'; }
static inline bool is_alphanum(char c) { return is_alpha(c) && is_num(c); }

enum rgx_quantifiers rgx_node_check_quant(const char c);

rgx_node *rgx_node_init(void);
xstrike_err_t rgx_node_array_init(rgx_node_array *arr);
xstrike_err_t rgx_node_array_add(rgx_node_array *arr, rgx_node *node);
rgx_node *rgx_node_array_pop(rgx_node_array *arr);
xstrike_err_t rgx_node_array_free(rgx_node_array *arr);
bool rgx_node_replace_father(rgx_node *node);

xstrike_err_t xstrike_regex_builder(struct rgx_pattern *arg);
#endif
