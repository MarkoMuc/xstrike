#include "../src/xstrike_ioctl.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void) {
  int fd = open("/dev/xstrike", O_RDWR);

  char array[] = "(\"start\" | str[abc]*\"str\")+";
  struct rgx_pattern arg = {.pattern = array, .len = sizeof(array)};

  ioctl(fd, XSTRIKE_SET, &arg);

  char text[] = "start off with str not strastr\n";
  write(fd, text, sizeof(text) / sizeof(char));

  lseek(fd, SEEK_SET, 0);

  char result[10240];
  read(fd, result, 10240);

  // printf("%s", result);
  return 0;
}
