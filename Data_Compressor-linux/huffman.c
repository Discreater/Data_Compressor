#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"
#include "data_buffer.h"

typedef unsigned char uchar;
double avg = 0;		// 平均编码长度

void node_init(node* nnode, uchar c, double v)
{
	nnode->c = c;
	nnode->value = v;
	nnode->left = nnode->right = nnode->next = NULL;
}

void node_insert(node ** head, node* ins)
{
	if (ins == NULL) {
		fprintf(stderr, "Error: insert a null pointer.\n");
		exit(-1);
	}
	if (*head == NULL) {
		*head = ins;
		return;
	}
	node* p = *head;
	node* pre = 0;
	if (p->value >= ins->value) {
		ins->next = p;
		*head = ins;
		return;
	}
	while (p->next != NULL && p->value < ins->value) {
		pre = p;
		p = p->next;
	}
	if (p->value < ins->value) {
		p->next = ins;
		return;
	}
	ins->next = p;
	pre->next = ins;
}

node* pop(node** head)
{
	node* tmp = *head;
	*head = (*head)->next;
	return tmp;
}

node* generate_huffman_tree(int* char_table, long long tnum)
{
	node* head = NULL;
	int i;
	for (i = 0; i < maxCharsNum; i++) {
		if (char_table[i] > 0) {
			// 若该符号出现过，则生成新结点，并插入优先队列中
			node* nnode = malloc(sizeof(node));
			node_init(nnode, (uchar)(unsigned)i, (double)char_table[i] / tnum);
			node_insert(&head, nnode);
		}
	}

	if (head == NULL) {
		fprintf(stderr, "Error: no input");
		exit(-1);
	}
	// 弹出优先队列的头两个结点，组成新的结点（概率相加），再插回优先队列
	while (head->next != NULL) {
		node* tmp1 = pop(&head);
		node* tmp2 = pop(&head);
		tmp1->next = tmp2->next = NULL;

		node* nnode = malloc(sizeof(node));
		nnode->c = 0;
		nnode->value = tmp1->value + tmp2->value;
		nnode->left = tmp1;
		nnode->right = tmp2;
		nnode->next = NULL;

		node_insert(&head, nnode);
	}
	return head;
}

double generate_symbole_table(node * head, int* ar, data_buffer * symbols)
{
	avg = 0;
	travel_huffman_tree(head->left, 0, LEFT, ar, symbols);
	travel_huffman_tree(head->right, 0, RIGHT, ar, symbols);
	fprintf(outfile, "\n");
	return avg;
}

void output_extra_bit_len(data_buffer* symbols, int* table) {
	int i, re = 0;
	for (i = 0; i < maxCharsNum; i++) {
		if (table[i] > 0) {
			re = (re + (table[i] * symbols[i].len) % 8) % 8;
		}
	}
	re = (re == 0) ? 8 : re;
	fprintf(outfile, "%d\n", re);
}

void travel_huffman_tree(node* fnode, int len, int zo, int* ar, data_buffer* symbols)
{
	ar[len++] = zo;
	if (fnode->left == NULL) {
		// 输出符号表到文件中
		fprintf(outfile, " ");
		int i;
		for (i = 0; i < len; i++) {
			fprintf(outfile, "%d", ar[i]);
		}
		uchar tc = fnode->c;
		fprintf(outfile, " %u", tc);

		avg += fnode->value * len;
		db_init(symbols + fnode->c, ar, len);
		return;
	}
	travel_huffman_tree(fnode->left, len, LEFT, ar, symbols);
	travel_huffman_tree(fnode->right, len, RIGHT, ar, symbols);
}

void clear_huffman_tree(node* head) {
	if (head == NULL) {
		return;
	}
	if (head->left != NULL) {
		clear_huffman_tree(head->left);
	}
	if (head->right != NULL) {
		clear_huffman_tree(head->right);
	}
	if (head != NULL) {
		free(head);
	}
}



