/* Local toolchain port: a UART is a non-seekable character stream. */
#include <errno.h>
#include <sys/types.h>

/* 本地适配：newlib可能请求定位；UART是不可定位的字符流。 */
_off_t _lseek(int fd, _off_t offset, int whence)
{
    /* 保留运行库签名，描述符不改变UART的不可定位性质。 */
    (void)fd;
    /* UART没有可跳转的文件偏移。 */
    (void)offset;
    /* 无论从头、当前或末尾定位都拒绝。 */
    (void)whence;
    /* ESPIPE向运行库说明流不支持seek。 */
    errno = ESPIPE;
    /* 返回标准失败值，不修改任何硬件状态。 */
    return (_off_t)-1;
}
