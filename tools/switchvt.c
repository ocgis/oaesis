#include <fcntl.h>
#include <linux/vt.h>
#include <sys/ioctl.h>

int
main (void) {
  int fd = open ("/dev/console", O_WRONLY);

  if (fd == -1) {
    return -1;
  } else {
    return ioctl (fd, VT_ACTIVATE, 7);
  }
}
