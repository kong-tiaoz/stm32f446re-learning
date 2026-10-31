/* 学习注释：引入 "ringbuffer.h"：环形队列结构与状态码、读写接口声明。 */
#include "ringbuffer.h"
/* 学习注释：引入 <string.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <string.h>

/* 学习注释：按 head/tail 距离计算空闲字节，始终保留一格区分满和空。 */
uint16_t RingBuffer_GetFreeSpace(RingBuffer *buf) {
	/* 学习注释：读写索引相等表示队列为空，可用容量为总长度减1。 */
	if(buf->tail == buf->head)
		/* 学习注释：按环形索引距离返回可用空间或已用长度，扣除区分满空的保留格。 */
		return RING_BUFFER_LENGTH - 1;

	/* 学习注释：写索引领先时，从尾部剩余容量扣除已存区和保留格。 */
	if(buf->head > buf->tail)
		/* 学习注释：按环形索引距离返回可用空间或已用长度，扣除区分满空的保留格。 */
		return RING_BUFFER_LENGTH - ((buf->head - buf->tail) + 1);
	/* 学习注释：进入上述条件均不满足时的分支。 */
	else
		/* 学习注释：按环形索引距离返回可用空间或已用长度，扣除区分满空的保留格。 */
		return  (buf->tail - buf->head) - 1;
}

/* 学习注释：由容量减去空闲空间和保留格，得到队列中可读字节数。 */
uint16_t RingBuffer_GetDataLength(RingBuffer *buf) {
	/* 学习注释：按 head/tail 距离计算空闲字节，始终保留一格区分满和空。 */
	return RING_BUFFER_LENGTH - (RingBuffer_GetFreeSpace(buf) + 1);
}


/* 学习注释：清空读写索引和存储区；原 EX3 实际未调用，依赖全局对象的静态零初始化。 */
void RingBuffer_Init(RingBuffer *buf) {
	/* 学习注释：推进/重置写索引，并按容量取模绕回；不覆盖保留空格。 */
	buf->head = buf->tail = 0;
	/* 学习注释：把环形缓冲存储区清零。 */
	memset(buf->buf, 0, RING_BUFFER_LENGTH);
}

/* 学习注释：逐字节出队，读取到请求长度或队列空时停止，返回实际读取数。 */
uint16_t RingBuffer_Read(RingBuffer *buf, uint8_t *data, uint16_t len) {
	/* 学习注释：记录本次已复制字节数，与请求len及队列边界一起限制循环。 */
	uint16_t counter = 0;

	/* 学习注释：条件成立时继续等待/搬运；队列读写边界由当前条件限制。 */
	while(buf->tail != buf->head && counter < len) {
		/* 学习注释：在当前索引处复制一个字节，并推进本次操作计数。 */
		data[counter++] = buf->buf[buf->tail];
		/* 学习注释：推进/重置读索引，按容量取模实现环形回绕。 */
		buf->tail = (buf->tail + 1) % RING_BUFFER_LENGTH;
	}
	/* 学习注释：返回本次实际出队字节数，队列较短时小于请求len。 */
	return counter;
}

/* 学习注释：先检查整段数据是否放得下，再逐字节入队；失败不执行部分写入。 */
uint8_t RingBuffer_Write(RingBuffer *buf, uint8_t *data, uint16_t len) {
	/* 学习注释：记录本次已复制字节数，与请求len及队列边界一起限制循环。 */
	uint16_t counter = 0;
	/* 学习注释：按 head/tail 距离计算空闲字节，始终保留一格区分满和空。 */
	uint16_t freeSpace = RingBuffer_GetFreeSpace(buf);

	/* 学习注释：没有可用格，返回缓冲已满，不写入数据。 */
	if(freeSpace == 0)
		/* 学习注释：队列满：调用者须保留/丢弃源数据，函数没有写入。 */
		return RING_BUFFER_FULL;
	/* 学习注释：整段请求放不下，返回空间不足，不执行部分写入。 */
	else if (freeSpace < len)
		/* 学习注释：空间不足以容纳整段数据，保持队列内容不变。 */
		return RING_BUFFER_NO_SUFFICIENT_SPACE;

	/* 学习注释：条件成立时继续等待/搬运；队列读写边界由当前条件限制。 */
	while(counter < len) {
		/* 学习注释：在当前索引处复制一个字节，并推进本次操作计数。 */
		buf->buf[buf->head] = data[counter++];
		/* 学习注释：推进/重置写索引，并按容量取模绕回；不覆盖保留空格。 */
		buf->head = (buf->head + 1) % RING_BUFFER_LENGTH;
	}
 	/* 学习注释：所有请求字节已入队，返回写入成功状态。 */
 	return RING_BUFFER_OK;
}
