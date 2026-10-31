/* 学习注释：头文件包含保护：仅第一次包含时展开后续声明。 */
#ifndef _RETARGET_H__
/* 学习注释：定义包含保护或本模块编译期符号，不产生运行期调用。 */
#define _RETARGET_H__

/* 学习注释：引入 "stm32f4xx_hal.h"：本文件使用的类型、外设接口或标准库声明。 */
#include "stm32f4xx_hal.h"
/* 学习注释：引入 <sys/stat.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <sys/stat.h>

/* 学习注释：保存串口句柄并关闭 stdout 缓冲，让没有换行的交互提示也立即发送。 */
void RetargetInit(UART_HandleTypeDef *huart);
/* 学习注释：向运行库声明 0/1/2 为终端描述符；其他编号设置 EBADF。 */
int _isatty(int fd);
/* 学习注释：newlib 标准输出适配入口：允许 stdout/stderr，经 HAL 阻塞串口发送。 */
int _write(int fd, char* ptr, int len);
/* 学习注释：标准流关闭占位处理，不会关闭 UART 硬件；非法描述符置 EBADF。 */
int _close(int fd);
/* 学习注释：newlib 标准输入适配入口：只允许 stdin，每次阻塞接收一个字节。 */
int _read(int fd, char* ptr, int len);
/* 学习注释：向运行库报告标准流为字符设备；原版非法描述符返回值约定保留。 */
int _fstat(int fd, struct stat* st);

/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif //#ifndef _RETARGET_H__
