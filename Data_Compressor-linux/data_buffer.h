#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#define TRUE 1
#define FALSE 0

#define dataBufLen 32		// 最长符号的字节数
#define bitsPerByte 8		// 每个字节的位数
#define byteMask 0xff		// 截取一个字节的数据

typedef char boolean;
typedef unsigned char uchar;

struct data_buffer {
	uchar databuf[dataBufLen];
	int len;
};


typedef struct data_buffer data_buffer;

// 返回缓冲区的字节数，向下取整
int length_in_chars(data_buffer* db);

// 把newval中从低位逐个放入oldval中，若空间不足，未存下的放到newbuf中
void db_insert(data_buffer* oldval, data_buffer* newval, data_buffer* newbuf);

// 返回缓冲区字节数，向上取整
int length_in_bytes(data_buffer* db);						
// 初始化
void db_init(data_buffer* db, int* ar, int len);
// 把符号的高位放入缓冲区
//void db_fill(data_buffer* oldval, data_buffer* newval, int shiftamt);	

void db_flush(data_buffer* db);

void db_copy(data_buffer* oldval, data_buffer* e);

#endif