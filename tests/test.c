#include "../src/xstrike_ioctl.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void) {
  int fd = open("/dev/xstrike", O_RDWR);

  const char pattern[] = "dog";
  struct rgx_pattern arg = {.pattern = pattern,
                            .len = sizeof(pattern) / sizeof(char)};

  ioctl(fd, XSTRIKE_SET, &arg);

  const char text[] = "The quick brown fox jumps over the lazy dog.\n"
                      "Dogs are great companions, often very loyal.\n"
                      "A dog's life is simple and full of joy.";
  write(fd, text, sizeof(text) / sizeof(char));

  lseek(fd, SEEK_SET, 0);

  char result[10240];
  read(fd, result, 10240);

  printf("%s", result);
  return 0;
}
