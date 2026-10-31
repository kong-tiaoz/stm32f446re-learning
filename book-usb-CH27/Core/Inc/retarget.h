// 学习：按 #ifndef _RETARGET_H__ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef _RETARGET_H__
// 学习：定义 _RETARGET_H__ 为 包含保护/编译开关，保留本章原始配置
#define _RETARGET_H__

// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 sys/stat.h，为本文件提供类型、常量和接口声明
#include <sys/stat.h>

// 学习：初始化当前 UART 或 TCP 标准输入输出重定向
void RetargetInit(UART_HandleTypeDef *huart);
// 学习：向 newlib 说明标准输入输出是字符设备
int _isatty(int fd);
// 学习：newlib 标准输出适配入口，返回实际发送字节数或原例错误值
int _write(int fd, char* ptr, int len);
// 学习：处理标准流关闭请求，不关闭硬件 UART
int _close(int fd);
// 学习：newlib 标准输入适配入口，返回已读取字节数
int _read(int fd, char* ptr, int len);
// 学习：为标准流填写字符设备类型
int _fstat(int fd, struct stat* st);
#endif //#ifndef _RETARGET_H__
