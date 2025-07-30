#include <linux/ioctl.h>
#include <linux/types.h>

struct rgx_pattern {
  const char *pattern;
  __u64 len;
};

#define XSTRIKE_MAGIC_NUM 'k'
#define XSTRIKE_SET _IOW(XSTRIKE_MAGIC_NUM, 0, struct rgx_pattern)
