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
// 学习：引入 ioLibrary_Driver/Ethernet/socket.h，为本文件提供类型、常量和接口声明
#include <ioLibrary_Driver/Ethernet/socket.h>
// 学习：引入 retarget-tcp.h，为本文件提供类型、常量和接口声明
#include <retarget-tcp.h>
// 学习：引入 stdint.h，为本文件提供类型、常量和接口声明
#include <stdint.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>

// 学习：按 #ifdef RETARGET_TCP 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef RETARGET_TCP

// 学习：定义 STDIN_FILENO 为 0，保留本章原始配置
#define STDIN_FILENO  0
// 学习：定义 STDOUT_FILENO 为 1，保留本章原始配置
#define STDOUT_FILENO 1
// 学习：定义 STDERR_FILENO 为 2，保留本章原始配置
#define STDERR_FILENO 2

// 学习：按 #ifndef RETARGET_PORT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef RETARGET_PORT
// 学习：定义 RETARGET_PORT 为 5000，保留本章原始配置
#define RETARGET_PORT 5000
#endif

// 学习：保存/更新 int8_t gSock；本行实际值为 -1
int8_t gSock = -1;

// 学习：初始化当前 UART 或 TCP 标准输入输出重定向
uint8_t RetargetInit(int8_t sn) {
  // 学习：保存/更新 gSock；本行实际值为 sn
  gSock = sn;

  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  // 学习：关闭 stdout 缓冲使每次输出立即进入重定向层
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Open 'sn' socket in TCP mode with a port number equal
   * to the value of RETARGET_PORT macro */
  // 学习：按条件 if(socket(sn, Sn_MR_TCP, RETARGET_PORT, 0) == sn) { 决定本分支/循环，原判断与次数保持不变
  if(socket(sn, Sn_MR_TCP, RETARGET_PORT, 0) == sn) {
    // 学习：按条件 if(listen(sn) == SOCK_OK) 决定本分支/循环，原判断与次数保持不变
    if(listen(sn) == SOCK_OK)
      // 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
      return 1;
  }
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
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
  // 学习：保存/更新 int sentlen；本行实际值为 0
  int sentlen = 0;
  // 学习：保存/更新 int buflen；本行实际值为 len
  int buflen = len;

  // 学习：按条件 if(getSn_SR(gSock) == SOCK_ESTABLISHED) { 决定本分支/循环，原判断与次数保持不变
  if(getSn_SR(gSock) == SOCK_ESTABLISHED) {
    // 学习：按条件 if (fd == STDOUT_FILENO || fd == STDERR_FILENO) { 决定本分支/循环，原判断与次数保持不变
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
      // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
      while(1) {
        // 学习：向已连接的 W5500 TCP socket 发送字节
        sentlen = send(gSock, (void*) ptr, buflen);
        // 学习：按条件 if (sentlen == buflen) 决定本分支/循环，原判断与次数保持不变
        if (sentlen == buflen)
          // 学习：把 len 返回给调用者；成功/错误含义由本接口约定决定
          return len;
        else if (sentlen > 0 && sentlen < buflen) {
          // 学习：保存/更新 buflen -；本行实际值为 sentlen
          buflen -= sentlen;
          // 学习：保存/更新 ptr +；本行实际值为 (len - buflen)
          ptr += (len - buflen);
        }
        else if (sentlen < 0)
          // 学习：把 EIO 返回给调用者；成功/错误含义由本接口约定决定
          return EIO;
      }
    }
  // 学习：读取 W5500 socket 的硬件状态寄存器
  } else if(getSn_SR(gSock) != SOCK_LISTEN) {
    /* Remote peer close the connection? */
    // 学习：关闭 W5500 socket，准备重建监听
    close(gSock);
    // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
    RetargetInit(gSock);
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

// 学习：拒绝标准流定位并设置 errno
int _lseek(int fd, int ptr, int dir) {
  // 学习：声明 (void) fd，供当前函数或模块保存状态
  (void) fd;
  // 学习：声明 (void) ptr，供当前函数或模块保存状态
  (void) ptr;
  // 学习：声明 (void) dir，供当前函数或模块保存状态
  (void) dir;

  // 学习：保存/更新 errno；本行实际值为 EBADF
  errno = EBADF;
  // 学习：把 -1 返回给调用者；成功/错误含义由本接口约定决定
  return -1;
}

// 学习：newlib 标准输入适配入口，返回已读取字节数
int _read(int fd, char* ptr, int len) {
  // 学习：按条件 if(getSn_SR(gSock) == SOCK_ESTABLISHED) { 决定本分支/循环，原判断与次数保持不变
  if(getSn_SR(gSock) == SOCK_ESTABLISHED) {
    // 学习：按条件 if (fd == STDIN_FILENO) { 决定本分支/循环，原判断与次数保持不变
    if (fd == STDIN_FILENO) {
      // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
      while(1) {
        // 学习：从 W5500 TCP socket 接收输入字节
        len = recv(gSock, (void*) ptr, len);
        // 学习：按条件 if (len > 0) 决定本分支/循环，原判断与次数保持不变
        if (len > 0)
          // 学习：把 len 返回给调用者；成功/错误含义由本接口约定决定
          return len;
        else
          // 学习：把 EIO 返回给调用者；成功/错误含义由本接口约定决定
          return EIO;
      }
    }
  // 学习：读取 W5500 socket 的硬件状态寄存器
  } else if(getSn_SR(gSock) != SOCK_LISTEN) {
    /* Remote peer close the connection? */
    // 学习：关闭 W5500 socket，准备重建监听
    close(gSock);
    // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
    RetargetInit(gSock);
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
#endif //#if !defined(OS_USE_SEMIHOSTING)
