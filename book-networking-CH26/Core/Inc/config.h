/*
 * config.h
 *
 *  Created on: 31 ott 2016
 *      Author: cnoviello
 */

// 学习：按 #ifndef CONFIG_H_ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef CONFIG_H_
// 学习：定义 CONFIG_H_ 为 包含保护/编译开关，保留本章原始配置
#define CONFIG_H_

// 学习：逐行解析 net.cfg，写入 DHCP/MAC/IP/掩码/网关/DNS
uint8_t ReadNetCfgFromFile(wiz_NetInfo *netInfo);
// 学习：把当前网络参数按六行格式写回 net.cfg
uint8_t WriteNetCfgInFile(wiz_NetInfo *netInfo);
#endif /* CONFIG_H_ */
