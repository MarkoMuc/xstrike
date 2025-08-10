#ifndef __XSTRIKE_REGEX_TYPES
#define __XSTRIKE_REGEX_TYPES

#include "xstrike_types.h"

#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/types.h>

#define RGX_NODE_START_SIZE 8

enum rgx_type {
  RGX_TYPE_SEQ = 0,         // Sequence; NO quantifier
  RGX_TYPE_COND = 1,        // |; NO quantifier
  RGX_TYPE_CHARSET = 2,     // [ ]; YES quantifier
  RGX_TYPE_GROUP = 3,       // ( ); YES quantifier
  RGX_TYPE_LITERAL = 4,     // " "; YES quantifier
  RGX_TYPE_SPEC_LITERAL = 5 // str; YES quantifier
};

enum rgx_quantifiers {
  RGX_QUANT_NONE = 0,    // Nan; Apply once, fail if none
  RGX_QUANT_STAR = 1,    // +; Apply multiple, fail if none
  RGX_QUANT_PLUS = 2,    // *; Apply multiple, fail never
  RGX_QUANT_QUES = 3,    // ?; Apply once, fail never
  RGX_QUANT_WILDCARD = 4 // .; Not a quantifier
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

enum rgx_quantifiers rgx_node_check_quant(const char c);

rgx_node *rgx_node_init(void);
xstrike_err_t rgx_node_array_init(rgx_node_array *arr);
xstrike_err_t rgx_node_array_add(rgx_node_array *arr, rgx_node *node);
rgx_node *rgx_node_array_pop(rgx_node_array *arr);
xstrike_err_t rgx_node_array_free(rgx_node_array *arr);
bool rgx_node_replace_father(rgx_node *node);
void rgx_node_check_and_replace(rgx_node **rnode);
#endif
