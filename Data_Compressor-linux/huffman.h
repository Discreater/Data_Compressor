#ifndef HUFFMAN_H
#define HUFFMAN_H


#define maxCharsNum 256u		// 最大字符数
#define maxEncodeLength 256		// 最大编码长度
#define LEFT 0
#define RIGHT 1

#include "data_buffer.h"

typedef unsigned char uchar;
typedef struct node node;

FILE* infile, * outfile;

struct node {
	node* left;		// 左子节点
	node* right;	// 右子节点
	node* next;		// 优先队列中下一节点
	double value;	// 概率
	uchar c;			// 单字节字符
};

void node_init(node* nnode, uchar c, double v);

void node_insert(node ** head, node* ins);

node* pop(node** head);

node* generate_huffman_tree(int* char_table, long long tnum);		// 生成huffman树，返回树root结点
double generate_symbole_table(node* head, int* ar, data_buffer* symbols);			// 生成符号表

void output_extra_bit_len(data_buffer* symbols, int* table);	// 计算最后一个缓冲区的位数，并输出
void travel_huffman_tree(node* fnode, int len, int zo, int* ar, data_buffer* symbols);	// 递归遍历huffman树

void clear_huffman_tree(node* head);					// 释放内存

#endif