#include "regex.h"

// TODO: how do I handle end of string situations?
// maybe remove [ ] or "? rgx_node_add_quant works like that already
// Illegal pattern

static inline bool rgx_quant_rules(const enum rgx_quantifiers q, const bool res,
                                   const bool p) {
  return (q == RGX_QUANT_NONE && res) || (q == RGX_QUANT_PLUS && p) ||
         (q == RGX_QUANT_STAR) || (q == RGX_QUANT_QUES);
}

static bool rgx_is_rule(const char c) {
  switch (c) {
  case '(':
  case '|':
  case '[':
  case '\"':
    return true;
  default:
    return false;
  }
}

static void rgx_node_add_quant(const char *pstr, rgx_node *rnode, u64 len,
                               u64 *idx) {
  u64 i = *idx;
  if (i + 1 < len) {
    const enum rgx_quantifiers quant = rgx_node_check_quant(pstr[i + 1]);

    if (quant != RGX_QUANT_NONE) {
      // printk(KERN_INFO "%c", pstr[i + 1]);
      rnode->quant = quant;
      i++;
    }

    *idx = i;
  }
}

void rgx_node_check_and_replace(rgx_node **rnode) {
  if (rgx_node_replace_father(*rnode)) {
    rgx_node *new_fnode = rgx_node_init();
    new_fnode->type = RGX_TYPE_SEQ;
    rgx_node_array_init(&new_fnode->body);

    rgx_node *old_node = rgx_node_array_pop(&(*rnode)->body);
    rgx_node_array_add(&(*rnode)->body, new_fnode);
    new_fnode->father = *rnode;

    old_node->father = new_fnode;
    rgx_node_array_add(&new_fnode->body, old_node);

    *rnode = new_fnode;
  }
}

bool rgx_node_replace_father(rgx_node *node) {
  if (!node) {
    return false;
  }

  if (node->type == RGX_TYPE_COND && node->body.len >= 2) {
    return true;
  }

  return false;
}

xstrike_err_t xstrike_regex_builder(struct rgx_pattern *arg,
                                    rgx_node **pattern) {
  const u64 len = arg->len;
  const char *pstr = arg->pattern;

  u64 i = 0;
  u64 slen = 0;

  char prev = 0;
  bool escaped = false;
  rgx_node *head = rgx_node_init();
  rgx_node *rnode = head;

  rgx_node_array_init(&head->body);

  while (i < len && pstr[i] != '\0') {
    const char c = pstr[i];
    if (i > 0) {
      prev = pstr[i - 1];
    }

    slen++;
    escaped = false;

    if (prev == '\\' && c != '\\') {
      escaped = true;
      i++;
    }

    switch (rnode->type) {
    case RGX_TYPE_LITERAL: {
      if (c == '\"') {
        // printk(KERN_INFO "\"");
        rnode->len = slen;
        rgx_node_add_quant(pstr, rnode, len, &i);
        rnode = rnode->father;
      }

      i++;
      continue;
    }

    case RGX_TYPE_SPEC_LITERAL: {
      if (rgx_is_rule(c)) {
        rnode->len = slen;
        rgx_node_add_quant(pstr, rnode, len, &i);
        rnode = rnode->father;
        continue;
      }

      i++;
      continue;
    }

    case RGX_TYPE_CHARSET: {
      if (c == ']') {
        // printk(KERN_INFO "]");
        rnode->len = slen;
        rgx_node_add_quant(pstr, rnode, len, &i);
        rnode = rnode->father;
      }

      i++;
      continue;
    }

    case RGX_TYPE_SEQ: {
      if (c == ')') {
        while (rnode->father && rnode->type != RGX_TYPE_GROUP) {
          rnode = rnode->father;
        }
        continue;
      }
      break;
    }
    case RGX_TYPE_GROUP: {
      if (c == ')') {
        // printk(KERN_INFO ")");
        rgx_node_add_quant(pstr, rnode, len, &i);
        rnode = rnode->father;
        i++;
        continue;
      }
    }

    default:
      break;
    }

    if (c == '[') {
      // printk(KERN_INFO "[");
      rgx_node_check_and_replace(&rnode);
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_CHARSET;
      new_node->str = &pstr[i];

      rgx_node_array_add(&rnode->body, new_node);
      new_node->father = rnode;
      rnode = new_node;

      slen = 0;
    } else if (c == '(') {
      // printk(KERN_INFO "(");
      rgx_node_check_and_replace(&rnode);
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_GROUP;
      rgx_node_array_init(&new_node->body);

      rgx_node_array_add(&rnode->body, new_node);
      new_node->father = rnode;
      rnode = new_node;

    } else if (c == '|') {
      // printk(KERN_INFO "|");
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_COND;
      rgx_node_array_init(&new_node->body);

      if (!rnode->father) {
        printk(KERN_INFO "Sytax error, not a ref expr.");
        return XSTRIKE_ERR_MEM;
      }

      rgx_node *node = rgx_node_array_pop(&rnode->body);
      node->father = new_node;

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
    } else if (c == '\"') {
      // printk(KERN_INFO "\"");
      rgx_node_check_and_replace(&rnode);
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_LITERAL;
      new_node->str = &pstr[i];
      rgx_node_array_add(&rnode->body, new_node);

      new_node->father = rnode;
      rnode = new_node;
      slen = 0;
    } else if (c != ' ' && c != '\t') {
      // printk(KERN_INFO "-");
      rgx_node_check_and_replace(&rnode);
      rgx_node *new_node = rgx_node_init();
      new_node->type = RGX_TYPE_SPEC_LITERAL;
      new_node->str = &pstr[i];
      rgx_node_array_add(&rnode->body, new_node);

      new_node->father = rnode;
      rnode = new_node;
      slen = 0;
    }

    i++;
  }

  if (rnode->type == RGX_TYPE_COND) {
    printk(KERN_INFO "Failure due to an unclosed conditional.");
    *pattern = NULL;
    return XSTRIKE_ERR_INVALID_RGX;
  }

  *pattern = head;
  return XSTRIKE_SUCC;
}

xstrike_err_t xstrike_regex_match(struct FileData *pdata, char **result) {
  const rgx_node *head = pdata->pattern;
  if (!head) {
    printk(KERN_INFO "Regex pattern is NULL");
    return XSTRIKE_ERR_NULLPTR;
  }

  if (head->type == RGX_TYPE_SEQ && head->body.len < 1) {
    printk(KERN_INFO "Regex pattern is missing.");
    return XSTRIKE_ERR_MISSING_PATTERN;
  }

  u64 idx = 0;
  rgx_recursive(pdata->data, pdata->len, &idx, head);

  return XSTRIKE_SUCC;
}

bool rgx_recursive(const char *data, const u64 len, u64 *idx,
                   const rgx_node *rnode) {
  u64 cidx = *idx;
  bool res = true;
  const enum rgx_quantifiers quant = rnode->quant;

  if (!rnode) {
    printk(KERN_INFO "Regex pattern is NULL during recursion.");
    return false;
  }

  switch (rnode->type) {
  case RGX_TYPE_SEQ: {
    if (rnode->body.len < 1) {
      printk(KERN_INFO "Missing body from seq");
      res = false;
      break;
    }

    for (size_t i = 0; i < rnode->body.len; i++) {
      res = rgx_recursive(data, len, idx, rnode->body.items[i]);
      if (!res) {
        break;
      }
    }
    break;
  }

  case RGX_TYPE_CHARSET: {
    const u64 charset_len = rnode->len;
    const char *charset = rnode->str;
    u64 pidx = cidx;
    bool prev = false;

    while (cidx < len) {
      const char c = data[cidx];
      do {
        for (u64 i = 0; i < charset_len; i++) {
          res = charset[i] == c;
          if (!res) {
            break;
          }
          cidx++;
        }
        prev |= res;
        if (res) {
          pidx = cidx;
        }
      } while (!(quant == RGX_QUANT_NONE || quant == RGX_QUANT_QUES) && res);
    }

    res = rgx_quant_rules(quant, res, prev);
    cidx = pidx;
    break;
  }

  case RGX_TYPE_COND: {
    if (rnode->body.len < 2) {
      printk(KERN_INFO "Missing condition");
      res = false;
      break;
    }

    res = rgx_recursive(data, len, idx, rnode->body.items[0]);
    if (!res) {
      res = rgx_recursive(data, len, idx, rnode->body.items[1]);
    }
    break;
  }
  case RGX_TYPE_GROUP: {
    if (rnode->body.len < 1) {
      printk(KERN_INFO "Missing body from group");
      res = false;
      break;
    }

    const u64 group_len = rnode->body.len;
    bool prev = false;
    u64 pidx = cidx;

    do {
      for (size_t i = 0; i < group_len && cidx < len; i++) {
        res = rgx_recursive(data, len, &cidx, rnode->body.items[i]);
        if (!res) {
          break;
        }
      }

      prev |= res;
      if (res) {
        pidx = cidx;
      }

      if (cidx < len) {
        break;
      }
    } while (!(quant == RGX_QUANT_NONE || quant == RGX_QUANT_QUES) && res);

    res = rgx_quant_rules(quant, res, prev);
    cidx = pidx;
    break;
  }

  case RGX_TYPE_LITERAL:
  case RGX_TYPE_SPEC_LITERAL: {
    const u64 literal_len = rnode->len;
    const char *literal = rnode->str;
    u64 pidx = cidx;
    bool prev = false;

    do {
      u64 i = 0;
      for (; i < literal_len && cidx < len; i++) {
        res = literal[i] == data[cidx];
        if (!res) {
          break;
        }
        cidx++;
      }
      prev |= res;

      if (res) {
        pidx = cidx;
      }

      if (i + 1 != literal_len && cidx >= len) {
        break;
      }
    } while (!(quant == RGX_QUANT_NONE || quant == RGX_QUANT_QUES) && res);

    res = rgx_quant_rules(quant, res, prev);
    cidx = pidx;
    break;
  }
  }

  *idx = cidx;
  return res;
}
