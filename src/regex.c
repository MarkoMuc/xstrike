#include "regex.h"

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

  return arr->items[arr->len--];
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

xstrike_err_t xstrike_regex_builder(struct rgx_pattern *arg) {
  const u64 len = arg->len;
  const char *pstr = arg->pattern;

  u64 i = 0;
  u64 slen = 0;

  char prev = 0;
  bool special = false;
  rgx_node *head = rgx_node_init();
  rgx_node *rnode = head;

  rgx_node_array_init(&head->body);

  while (i < len) {
    slen++;
    const char c = pstr[i];

    if (prev == '\\' && c != '\\') {
      i++;
      continue;
    }

    switch (rnode->type) {
    case RGX_TYPE_LITERAL: {
      if (c == '\"') {
        rnode->len = slen;
        rnode = rnode->father;
        special = false;
      } else {
        i++;
      }
      continue;
    }
    case RGX_TYPE_CHARSET: {
      if (c == '[') {
        rnode->len = slen;
        special = false;
      } else {
        i++;
      }

      if (i + 1 < len) {
        const enum rgx_quantifiers quant = rgx_node_check_quant(pstr[i + 1]);
        if (quant != RGX_QUANT_NONE) {
          rnode->quant = quant;
          i++;
        }
      }
      rnode = rnode->father;

      continue;
    }
    case RGX_TYPE_GROUP: {
      if (c == ')') {
        special = false;
      } else {
        i++;
      }

      if (i + 1 < len) {
        const enum rgx_quantifiers quant = rgx_node_check_quant(pstr[i + 1]);
        if (quant != RGX_QUANT_NONE) {
          rnode->quant = quant;
          i++;
        }
      }
      rnode = rnode->father;
    }

    default:
      break;
    }

    if (!special && c == '[') {
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_CHARSET;
      new_node->str = &pstr[i];

      rgx_node_array_add(&rnode->body, new_node);
      new_node->father = rnode;
      rnode = new_node;

      special = true;
    } else if (!special && c == '(') {
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_GROUP;

      rgx_node_array_add(&rnode->body, new_node);
      new_node->father = rnode;
      rnode = new_node;

      special = true;
    } else if (!special && c == '|') {
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_COND;
      rgx_node_array_init(&new_node->body);

      if (!rnode->father) {
        printk(KERN_INFO "Sytax error, not a ref expr.");
        return XSTRIKE_ERR_MEM;
      }

      rgx_node *node = rgx_node_array_pop(&rnode->body);

      if (rgx_node_array_add(&new_node->body, node) != XSTRIKE_SUCC) {
        printk(KERN_INFO "Failure during add conditional, dirty exit");
        return XSTRIKE_ERR_MEM;
      }

      if (rgx_node_array_add(&rnode->body, new_node) != XSTRIKE_SUCC) {
        printk(KERN_INFO "Failure during add conditional, dirty exit");
        return XSTRIKE_ERR_MEM;
      }

      new_node->father = rnode;
      rnode = new_node;
      special = true;
    } else if (!special && c == '\"') {
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_LITERAL;
      new_node->str = &pstr[i];
      rgx_node_array_add(&rnode->body, new_node);

      new_node->father = rnode;
      rnode = new_node;
      special = true;
    }
    prev = c;
    i++;
  }
  return XSTRIKE_SUCC;
}
