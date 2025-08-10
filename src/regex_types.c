#include "regex_types.h"

enum rgx_quantifiers rgx_node_check_quant(const char c) {
  switch (c) {
  case '+':
    return RGX_QUANT_PLUS;
  case '*':
    return RGX_QUANT_STAR;
  case '?':
    return RGX_QUANT_QUES;
  default:
    return RGX_QUANT_NONE;
  }
}

xstrike_err_t rgx_node_array_free(rgx_node_array *arr) {
  if (!arr || !arr->items) {
    printk(KERN_INFO
           "Failed to insert node, array not initialized or passing null");
    return XSTRIKE_ERR_NOT_INIT;
  }

  printk(KERN_INFO "TODO:IMPLEMENT FREES.");

  return XSTRIKE_SUCC;
}

rgx_node *rgx_node_array_pop(rgx_node_array *arr) {
  if (!arr || !arr->items) {
    printk(KERN_INFO
           "Failed to insert node, array not initialized or passing null");
    return NULL;
  }

  if (arr->len - 1 < 0) {
    printk(KERN_INFO "Failed to rm last node, index underflow");
    return NULL;
  }

  return arr->items[--arr->len];
}

xstrike_err_t rgx_node_array_add(rgx_node_array *arr, rgx_node *node) {
  if (!arr || !arr->items) {
    printk(KERN_INFO
           "Failed to insert node, array not initialized or passing null");
    return XSTRIKE_ERR_NOT_INIT;
  }

  if (!node) {
    printk(KERN_INFO "Failed to insert node, node is null.");
    return XSTRIKE_ERR_NOT_INIT;
  }

  if (arr->len >= arr->size) {
    arr->size *= 2;
    arr->items =
        krealloc_array(arr->items, arr->size, sizeof(rgx_node *), GFP_KERNEL);
    if (!arr) {
      printk(KERN_INFO "Failed to realloc rgx_node array.");
      return XSTRIKE_ERR_MEM;
    }
  }

  arr->items[arr->len++] = node;

  return XSTRIKE_SUCC;
}

xstrike_err_t rgx_node_array_init(rgx_node_array *arr) {
  if (!arr) {
    printk(KERN_INFO "Failed to malloc for rgx_node_array.");
    return XSTRIKE_ERR_MEM;
  }

  arr->items = kmalloc(sizeof(rgx_node *) * RGX_NODE_START_SIZE, GFP_KERNEL);

  if (!arr) {
    printk(KERN_INFO "Failed to malloc items rgx_node_array.");
    return XSTRIKE_ERR_MEM;
  }

  arr->size = RGX_NODE_START_SIZE;
  arr->len = 0;

  return XSTRIKE_SUCC;
}

rgx_node *rgx_node_init() {
  rgx_node *node = kmalloc(sizeof(rgx_node), GFP_KERNEL);
  if (!node) {
    printk(KERN_INFO "Failed to malloc for rgx_node.");
  }

  node->type = RGX_TYPE_SEQ;
  node->quant = RGX_QUANT_NONE;
  node->len = 0;
  node->str = NULL;
  node->father = NULL;
  return node;
}
