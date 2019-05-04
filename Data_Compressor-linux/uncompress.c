#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uncompress.h"
#include "huffman.h"


node* build_huffman_tree(void)
{
	node* head = malloc(sizeof(node));
	node* nnode = NULL;
	char ar[maxEncodeLength];
	int c;
	char tc;
	do {
		fscanf(infile, "%s%d%c", ar, &c, &tc);
		int i;
		node* tmp = head;
		for (i = 0; ar[i] != '\0'; i++) {
			if (ar[i] == '0') {
				if (head->left == NULL) {
					nnode = malloc(sizeof(node));
					memset(nnode, 0, sizeof(node));
					head->left = nnode;
				}
				head = head->left;
			}
			else if (ar[i] == '1') {
				if (head->right == NULL) {
					nnode = malloc(sizeof(node));
					memset(nnode, 0, sizeof(node));
					head->right = nnode;
				}
				head = head->right;
			}
			else {
				fprintf(stderr, "Error: unknown file head");
				exit(-1);
			}
			// 叶子结点
			if (ar[i + 1] == '\0') {
				head->c = (uchar)(unsigned)c;
			}
		}
		head = tmp;
	} while (tc != '\n');

	return head;
}

int get_extra_bit_len(void) {
	int len = 0;
	char tmp;
	fscanf(infile, "%d%c", &len, &tmp);
	return len;
}

void decode(node * head, int extra_len) {
	// 增加一字节缓冲，以便处理最后一字节
	int ch, cht;
	node* root = head;
	cht = getchar();
	ch = cht;
	int i, bit0;
	while ((cht = fgetc(infile)) != EOF) {
		for (i = 0; i < 8; i++) {
			bit0 = ch & 1;
			if (bit0 == LEFT) {		// 向左
				head = head->left;
			}
			else {					// 向右
				head = head->right;
			}
			if (head->c != 0) {		// 若为叶子结点
				fputc((char)(head->c), outfile);
				if (head->c == '\n') {
					fflush(outfile);
				}
				head = root;
			}
			ch = ch >> 1;
		}
		ch = cht;
	}
	// 处理最后一字节
	for (i = 0; i < extra_len; i++) {
		bit0 = ch & 1;
		if (bit0 == LEFT) {		// 向左
			head = head->left;
		}
		else {					// 向右
			head = head->right;
		}
		if (head->c != 0) {		// 若为叶子结点
			fputc((char)(head->c), outfile);
			if (head->c == '\n') {
				fflush(outfile);
			}
			head = root;
		}
		ch = ch >> 1;
	}
	ch = cht;
	fflush(outfile);
}
