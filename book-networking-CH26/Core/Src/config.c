/*
 * config.c
 *
 *  Created on: 31 ott 2016
 *      Author: cnoviello
 */
// 学习：引入 stdlib.h，为本文件提供类型、常量和接口声明
#include <stdlib.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 fatfs.h，为本文件提供类型、常量和接口声明
#include "fatfs.h"
// 学习：引入 ioLibrary_Driver/Ethernet/wizchip_conf.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Ethernet/wizchip_conf.h"
// 学习：引入 ioLibrary_Driver/Internet/httpServer/httpParser.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Internet/httpServer/httpParser.h"

// 学习：逐行解析 net.cfg，写入 DHCP/MAC/IP/掩码/网关/DNS
uint8_t ReadNetCfgFromFile(wiz_NetInfo *netInfo) {
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
  // 学习：声明 FIL cfgFile，供当前函数或模块保存状态
  FIL cfgFile;
  // 学习：声明 UINT br，供当前函数或模块保存状态
  UINT br;
// 学习：按 #elif defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#elif defined(OS_USE_SEMIHOSTING)
  // 学习：保存/更新 FILE *cfgFile；本行实际值为 NULL
  FILE *cfgFile = NULL;
#endif
  // 学习：声明 char buf[30], *p，供当前函数或模块保存状态
  char buf[30], *p;

  // 学习：保存/更新 p；本行实际值为 buf
  p = buf;
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
  // 学习：按路径和标志打开 FatFs 文件
  FRESULT res = f_open(&cfgFile, "/net.cfg", FA_READ);
  // 学习：按条件 if(res == FR_OK) { 决定本分支/循环，原判断与次数保持不变
  if(res == FR_OK) {
// 学习：按 #elif defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#elif defined(OS_USE_SEMIHOSTING)
  // 学习：按条件 if((cfgFile = fopen(OS_BASE_FS_PATH"net.cfg","r")) != NULL) { 决定本分支/循环，原判断与次数保持不变
  if((cfgFile = fopen(OS_BASE_FS_PATH"net.cfg","r")) != NULL) {
#endif
    // 学习：按条件 for(uint8_t i = 0; i < 6; i++) { 决定本分支/循环，原判断与次数保持不变
    for(uint8_t i = 0; i < 6; i++) {
      // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
      while(1) {
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
        // 学习：按条件 if(f_read(&cfgFile, p, 1, &br) == FR_OK) { 决定本分支/循环，原判断与次数保持不变
        if(f_read(&cfgFile, p, 1, &br) == FR_OK) {
#else
        // 学习：按条件 if(fread(p, sizeof(char), 1, cfgFile)) { 决定本分支/循环，原判断与次数保持不变
        if(fread(p, sizeof(char), 1, cfgFile)) {
#endif
          // 学习：按条件 if (*p == '\r') { 决定本分支/循环，原判断与次数保持不变
          if (*p == '\r') {
            // 学习：跳过本次迭代的后续处理，继续读取/等待下一项
            continue;
          }
          // 学习：当前字符为换行符时结束这一配置行，并补NUL终止符
          else if(*p == '\n') {
            // 学习：保存/更新 *p；本行实际值为 '\0'
            *p = '\0';
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          }
          else {
            // 学习：声明 p++，供当前函数或模块保存状态
            p++;
          }
        } else { /* Reached the EOF */
          // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
          break;
        }
      }

      // 学习：按条件 if(p != buf)  { 决定本分支/循环，原判断与次数保持不变
      if(p != buf)  {
          // 学习：按条件 switch(i) { 决定本分支/循环，原判断与次数保持不变
          switch(i) {
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 0: //DHCP
            // 学习：按条件 if(strcmp(buf, "NODHCP") == 0) 决定本分支/循环，原判断与次数保持不变
            if(strcmp(buf, "NODHCP") == 0)
              // 学习：保存/更新 netInfo->dhcp；本行实际值为 NETINFO_STATIC
              netInfo->dhcp = NETINFO_STATIC;
            else
              // 学习：保存/更新 netInfo->dhcp；本行实际值为 NETINFO_DHCP
              netInfo->dhcp = NETINFO_DHCP;
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 1: //MAC Address
            // 学习：把冒号分隔 MAC 文本解析为 6 字节地址
            mac_addr_((uint8_t*)buf, netInfo->mac);
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 2: //IP Address
            // 学习：把点分十进制 IPv4 文本解析为 4 字节数组
            inet_addr_((uint8_t*)buf, netInfo->ip);
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 3: //Netmask
            // 学习：把点分十进制 IPv4 文本解析为 4 字节数组
            inet_addr_((uint8_t*)buf, netInfo->sn);
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 4: //Gateway Address
            // 学习：把点分十进制 IPv4 文本解析为 4 字节数组
            inet_addr_((uint8_t*)buf, netInfo->gw);
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case 5: //DNS Address
            // 学习：把点分十进制 IPv4 文本解析为 4 字节数组
            inet_addr_((uint8_t*)buf, netInfo->dns);
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          }
          // 学习：保存/更新 p；本行实际值为 buf
          p = buf;
        }
      else {
        // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
        return  0;
      }
    }
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_close(&cfgFile) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_close(&cfgFile) != FR_OK)
#else
    // 学习：按条件 if(fclose(cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(fclose(cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
    return 1;
  }
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
}

// 学习：把当前网络参数按六行格式写回 net.cfg
uint8_t WriteNetCfgInFile(wiz_NetInfo *netInfo) {
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
  // 学习：声明 FIL cfgFile，供当前函数或模块保存状态
  FIL cfgFile;
  // 学习：声明 UINT bw，供当前函数或模块保存状态
  UINT bw;
#else
  // 学习：保存/更新 FILE *cfgFile；本行实际值为 NULL
  FILE *cfgFile = NULL;
#endif
  // 学习：声明 char buf[30]，供当前函数或模块保存状态
  char buf[30];

// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
  // 学习：按条件 if(f_open(&cfgFile, "0:/net.cfg",FA_WRITE) == FR_OK) { 决定本分支/循环，原判断与次数保持不变
  if(f_open(&cfgFile, "0:/net.cfg",FA_WRITE) == FR_OK) {
#else
  // 学习：按条件 if((cfgFile = fopen(OS_BASE_FS_PATH"net.cfg","w ")) != NULL) { 决定本分支/循环，原判断与次数保持不变
  if((cfgFile = fopen(OS_BASE_FS_PATH"net.cfg","w ")) != NULL) {
#endif
    // 学习：按条件 if(netInfo->dhcp == NETINFO_DHCP) 决定本分支/循环，原判断与次数保持不变
    if(netInfo->dhcp == NETINFO_DHCP)
      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(buf, "DHCP\n");
    else
      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(buf, "NODHCP\n");
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：按格式生成文本到标准输出或指定缓冲区
    sprintf(buf, "%x:%x:%x:%x:%x:%x\n", netInfo->mac[0], netInfo->mac[1], netInfo->mac[2],
                                        // 学习：声明 netInfo->mac[3], netInfo->mac[4], netInfo->mac[5])，供当前函数或模块保存状态
                                        netInfo->mac[3], netInfo->mac[4], netInfo->mac[5]);
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：按格式生成文本到标准输出或指定缓冲区
    sprintf(buf, "%d.%d.%d.%d\n", netInfo->ip[0], netInfo->ip[1], netInfo->ip[2], netInfo->ip[3]);
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：按格式生成文本到标准输出或指定缓冲区
    sprintf(buf, "%d.%d.%d.%d\n", netInfo->sn[0], netInfo->sn[1], netInfo->sn[2], netInfo->sn[3]);
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：按格式生成文本到标准输出或指定缓冲区
    sprintf(buf, "%d.%d.%d.%d\n", netInfo->gw[0], netInfo->gw[1], netInfo->gw[2], netInfo->gw[3]);
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：按格式生成文本到标准输出或指定缓冲区
    sprintf(buf, "%d.%d.%d.%d\n", netInfo->dns[0], netInfo->dns[1], netInfo->dns[2], netInfo->dns[3]);
// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_write(&cfgFile, buf, strlen(buf), &bw) != FR_OK)
#else
    // 学习：按条件 if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(!fwrite(buf, sizeof(char), strlen(buf), cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
    // 学习：按条件 if(f_close(&cfgFile) != FR_OK) 决定本分支/循环，原判断与次数保持不变
    if(f_close(&cfgFile) != FR_OK)
#else
    // 学习：按条件 if(fclose(cfgFile)) 决定本分支/循环，原判断与次数保持不变
    if(fclose(cfgFile))
#endif
      // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
      return 0;

    // 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
    return 1;
  }
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
}
