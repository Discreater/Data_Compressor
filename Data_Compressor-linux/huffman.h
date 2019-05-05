#ifndef HUFFMAN_H
#define HUFFMAN_H


#define maxCharsNum 256u		// ����ַ���
#define maxEncodeLength 256		// �����볤��
#define LEFT 0
#define RIGHT 1

#include "data_buffer.h"

typedef unsigned char uchar;
typedef struct node node;

FILE* infile, * outfile;

struct node {
	node* left;		// ���ӽڵ�
	node* right;	// ���ӽڵ�
	node* next;		// ���ȶ�������һ�ڵ�
	double value;	// ����
	uchar c;			// ���ֽ��ַ�
};

void node_init(node* nnode, uchar c, double v);

void node_insert(node ** head, node* ins);

node* pop(node** head);

node* generate_huffman_tree(int* char_table, long long tnum);		// ����huffman����������root���
double generate_symbole_table(node* head, int* ar, data_buffer* symbols);			// ���ɷ��ű�

void output_extra_bit_len(data_buffer* symbols, int* table);	// �������һ����������λ���������
void travel_huffman_tree(node* fnode, int len, int zo, int* ar, data_buffer* symbols);	// �ݹ����huffman��

void clear_huffman_tree(node* head);					// �ͷ��ڴ�

#endif