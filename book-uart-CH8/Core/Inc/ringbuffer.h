/* 学习注释：头文件包含保护：仅第一次包含时展开后续声明。 */
#ifndef RING_BUFFER_H__
/* 学习注释：定义包含保护或本模块编译期符号，不产生运行期调用。 */
#define RING_BUFFER_H__

/* 学习注释：引入 <stdint.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <stdint.h>


/* 学习注释：环形缓冲物理容量为1000字节；保留一格后实际可存999字节。 */
#define RING_BUFFER_LENGTH 1000

/* 学习注释：队列对象把字节存储区与读写索引封装在一起。 */
typedef struct {
	/* 学习注释：队列的物理字节存储区，长度由RING_BUFFER_LENGTH统一指定。 */
	uint8_t buf[RING_BUFFER_LENGTH];
/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#if RING_BUFFER_LENGTH < 255
	/* 学习注释：读写索引类型随编译期容量选择；容量1000使用uint16_t。 */
	uint8_t head, tail;
/* 学习注释：选择另一个编译期分支。 */
#else
	/* 学习注释：读写索引类型随编译期容量选择；容量1000使用uint16_t。 */
	uint16_t head, tail;
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif
/* 学习注释：结束环形缓冲结构或写入状态类型定义。 */
} RingBuffer;

/* 学习注释：枚举分别表示写入成功、缓冲已满与剩余空间不足。 */
typedef enum {
	/* 学习注释：环形队列写入结果枚举值，调用方据此区分成功与容量问题。 */
	RING_BUFFER_OK = 0x0,
	/* 学习注释：环形队列写入结果枚举值，调用方据此区分成功与容量问题。 */
	RING_BUFFER_FULL,
	/* 学习注释：环形队列写入结果枚举值，调用方据此区分成功与容量问题。 */
	RING_BUFFER_NO_SUFFICIENT_SPACE
/* 学习注释：结束环形缓冲结构或写入状态类型定义。 */
} RingBuffer_Status;

/* 学习注释：由容量减去空闲空间和保留格，得到队列中可读字节数。 */
uint16_t RingBuffer_GetDataLength(RingBuffer *buf);
/* 学习注释：按 head/tail 距离计算空闲字节，始终保留一格区分满和空。 */
uint16_t RingBuffer_GetFreeSpace(RingBuffer *buf);
/* 学习注释：清空读写索引和存储区；原 EX3 实际未调用，依赖全局对象的静态零初始化。 */
void RingBuffer_Init(RingBuffer *buf);
/* 学习注释：逐字节出队，读取到请求长度或队列空时停止，返回实际读取数。 */
uint16_t RingBuffer_Read(RingBuffer *buf, uint8_t *data, uint16_t len);
/* 学习注释：先检查整段数据是否放得下，再逐字节入队；失败不执行部分写入。 */
uint8_t RingBuffer_Write(RingBuffer *buf, uint8_t *data, uint16_t len);

/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif //#ifndef RING_BUFFER_H__
