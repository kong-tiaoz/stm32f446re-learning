// 学习：引入 _ansi.h，为本文件提供类型、常量和接口声明
#include <_ansi.h>
// 学习：引入 _syslist.h，为本文件提供类型、常量和接口声明
#include <_syslist.h>
// 学习：引入 errno.h，为本文件提供类型、常量和接口声明
#include <errno.h>
// 学习：引入 sys/time.h，为本文件提供类型、常量和接口声明
#include <sys/time.h>
// 学习：引入 sys/times.h，为本文件提供类型、常量和接口声明
#include <sys/times.h>
// 学习：引入 limits.h，为本文件提供类型、常量和接口声明
#include <limits.h>
// 学习：引入 signal.h，为本文件提供类型、常量和接口声明
#include <signal.h>
// 学习：引入 retarget.h，为本文件提供类型、常量和接口声明
#include <retarget.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>

// 学习：定义 STDIN_FILENO 为 0，保留本章原始配置
#define STDIN_FILENO  0
// 学习：定义 STDOUT_FILENO 为 1，保留本章原始配置
#define STDOUT_FILENO 1
// 学习：定义 STDERR_FILENO 为 2，保留本章原始配置
#define STDERR_FILENO 2

// 学习：声明 UART_HandleTypeDef *gHuart，供当前函数或模块保存状态
UART_HandleTypeDef *gHuart;

// 学习：初始化当前 UART 或 TCP 标准输入输出重定向
void RetargetInit(UART_HandleTypeDef *huart) {
  // 学习：保存/更新 gHuart；本行实际值为 huart
  gHuart = huart;

  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  // 学习：关闭 stdout 缓冲使每次输出立即进入重定向层
  setvbuf(stdout, NULL, _IONBF, 0);
}

// 学习：向 newlib 说明标准输入输出是字符设备
int _isatty(int fd) {
  // 学习：按条件 if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) 决定本分支/循环，原判断与次数保持不变
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    // 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
    return 1;

  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
}

// 学习：newlib 标准输出适配入口，返回实际发送字节数或原例错误值
int _write(int fd, char* ptr, int len) {
  // 学习：声明 HAL_StatusTypeDef hstatus，供当前函数或模块保存状态
  HAL_StatusTypeDef hstatus;

  // 学习：按条件 if (fd == STDOUT_FILENO || fd == STDERR_FILENO) { 决定本分支/循环，原判断与次数保持不变
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
    // 学习：阻塞发送给定字节缓冲区
    hstatus = HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    // 学习：按条件 if (hstatus == HAL_OK) 决定本分支/循环，原判断与次数保持不变
    if (hstatus == HAL_OK)
      // 学习：把 len 返回给调用者；成功/错误含义由本接口约定决定
      return len;
    else
      // 学习：把 EIO 返回给调用者；成功/错误含义由本接口约定决定
      return EIO;
  }
  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 -1 返回给调用者；成功/错误含义由本接口约定决定
  return -1;
}

// 学习：处理标准流关闭请求，不关闭硬件 UART
int _close(int fd) {
  // 学习：按条件 if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) 决定本分支/循环，原判断与次数保持不变
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
    return 0;

  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 -1 返回给调用者；成功/错误含义由本接口约定决定
  return -1;
}

// 学习：newlib 标准输入适配入口，返回已读取字节数
int _read(int fd, char* ptr, int len) {
  // 学习：声明 HAL_StatusTypeDef hstatus，供当前函数或模块保存状态
  HAL_StatusTypeDef hstatus;

  // 学习：按条件 if (fd == STDIN_FILENO) { 决定本分支/循环，原判断与次数保持不变
  if (fd == STDIN_FILENO) {
    // 学习：阻塞接收指定字节数供协议或标准输入读取
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
    // 学习：按条件 if (hstatus == HAL_OK) 决定本分支/循环，原判断与次数保持不变
    if (hstatus == HAL_OK)
      // 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
      return 1;
    else
      // 学习：把 EIO 返回给调用者；成功/错误含义由本接口约定决定
      return EIO;
  }
  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 -1 返回给调用者；成功/错误含义由本接口约定决定
  return -1;
}

// 学习：为标准流填写字符设备类型
int _fstat(int fd, struct stat* st) {
  // 学习：按条件 if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) { 决定本分支/循环，原判断与次数保持不变
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
    // 学习：保存/更新 st->st_mode；本行实际值为 S_IFCHR
    st->st_mode = S_IFCHR;
    // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
    return 0;
  }

  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
}
