#ifndef UNCOMPRESS_H
#define UNCOMPRESS_H

#include "huffman.h"

// uncompress
node* build_huffman_tree(void);
int get_extra_bit_len(void);			// 获取最后一个字符的位数
void determine_mode(char* file_name);	// 获取文件权限
void decode(node* head, int extra_len);

#endif // !UNCOMPRESS_H

