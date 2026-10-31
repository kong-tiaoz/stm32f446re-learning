/* 学习注释：引入 <_ansi.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <_ansi.h>
/* 学习注释：引入 <_syslist.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <_syslist.h>
/* 学习注释：引入 <errno.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <errno.h>
/* 学习注释：引入 <sys/time.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <sys/time.h>
/* 学习注释：引入 <sys/times.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <sys/times.h>
/* 学习注释：引入 <limits.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <limits.h>
/* 学习注释：引入 <signal.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <signal.h>
/* 学习注释：引入 <retarget.h>：标准流到 UART 的适配接口。 */
#include <retarget.h>
/* 学习注释：引入 <stdio.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <stdio.h>

/* 学习注释：标准流文件描述符编号，供newlib适配层识别输入、输出和错误流。 */
#define STDIN_FILENO  0
/* 学习注释：标准流文件描述符编号，供newlib适配层识别输入、输出和错误流。 */
#define STDOUT_FILENO 1
/* 学习注释：标准流文件描述符编号，供newlib适配层识别输入、输出和错误流。 */
#define STDERR_FILENO 2

/* 学习注释：声明本模块的数据对象，后续读写需遵守其类型和有效期。 */
UART_HandleTypeDef *gHuart;

/* 学习注释：保存串口句柄并关闭 stdout 缓冲，让没有换行的交互提示也立即发送。 */
void RetargetInit(UART_HandleTypeDef *huart) {
  /* 学习注释：保存已初始化UART句柄，供标准库读写入口共同使用。 */
  gHuart = huart;

  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  /* 学习注释：关闭stdout缓冲，使无换行提示立即输出。 */
  setvbuf(stdout, NULL, _IONBF, 0);
}

/* 学习注释：向运行库声明 0/1/2 为终端描述符；其他编号设置 EBADF。 */
int _isatty(int fd) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 1;

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return 0;
}

/* 学习注释：newlib 标准输出适配入口：允许 stdout/stderr，经 HAL 阻塞串口发送。 */
int _write(int fd, char* ptr, int len) {
  /* 学习注释：保存HAL收发返回值，供适配层判断正常或错误。 */
  HAL_StatusTypeDef hstatus;

  /* 学习注释：仅stdout和stderr允许经本适配层发送。 */
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
    /* 学习注释：阻塞发送指定字节序列。 */
    hstatus = HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    /* 学习注释：仅HAL收发成功才返回处理字节数。 */
    if (hstatus == HAL_OK)
      /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
      return len;
    /* 学习注释：进入上述条件均不满足时的分支。 */
    else
      /* 学习注释：保留原版直接返回正EIO的错误约定；它与常用-1并设置errno的接口规范不同。 */
      return EIO;
  }
  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return -1;
}

/* 学习注释：标准流关闭占位处理，不会关闭 UART 硬件；非法描述符置 EBADF。 */
int _close(int fd) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 0;

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return -1;
}

/* 学习注释：newlib 标准输入适配入口：只允许 stdin，每次阻塞接收一个字节。 */
int _read(int fd, char* ptr, int len) {
  /* 学习注释：保存HAL收发返回值，供适配层判断正常或错误。 */
  HAL_StatusTypeDef hstatus;

  /* 学习注释：仅stdin允许经本适配层接收。 */
  if (fd == STDIN_FILENO) {
    /* 学习注释：阻塞接收字节到调用者缓冲区。 */
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
    /* 学习注释：仅HAL收发成功才返回处理字节数。 */
    if (hstatus == HAL_OK)
      /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
      return 1;
    /* 学习注释：进入上述条件均不满足时的分支。 */
    else
      /* 学习注释：保留原版直接返回正EIO的错误约定；它与常用-1并设置errno的接口规范不同。 */
      return EIO;
  }
  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return -1;
}

/* 学习注释：向运行库报告标准流为字符设备；原版非法描述符返回值约定保留。 */
int _fstat(int fd, struct stat* st) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
    /* 学习注释：把标准输入输出报告为字符设备。 */
    st->st_mode = S_IFCHR;
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 0;
  }

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return 0;
}
