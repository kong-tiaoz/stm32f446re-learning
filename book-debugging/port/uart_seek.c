/* Local toolchain port: a UART is a non-seekable character stream. */
#include <errno.h>
#include <sys/types.h>

_off_t _lseek(int fd, _off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return (_off_t)-1;
}
