#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "huffman.h"
#include "data_buffer.h"
#include "uncompress.h"

#define INLINUX
#define DEBUG

typedef unsigned int uint;
typedef unsigned char uchar;

int read_data(int* arr);				// 读取输入生成符号表
void output_compressed_content(data_buffer* symbols);	// 输出

int main(int argc, char** argv) {
	// 解析命令行参数，具体行为见Usage.txt文件
	int file_from_stdin = 1;
	char* in_file_name;

	int file_to_stdout = 1;
	char* out_file_name;

	int compress = 1;

	int ch;
	while((ch = getopt(argc, argv, "i:o:nhpu")) != -1){
		switch (ch){
			case 'i':
				file_from_stdin = 0;
				in_file_name = optarg;
				break;
			case 'o':
				file_to_stdout = 0;
				out_file_name = optarg;
				break;
			case 'n':
				// unfinished
				break;
			case 'h':
				// unfinished
				break;
			case 'p':
				compress = 1;
				break;
			case 'u':
				compress = 0;
			default:
				// unfinished
				break;
		}
	}

	if (file_from_stdin) {
		fprintf(stderr, "Error: no input file!");
		exit(-1);
	}
	if (file_to_stdout) {
		fprintf(stderr, "Error: no output file!");
		exit(-1);
	}

	if(compress){
		// 输入、输出重定向
		fclose(stdin);
		if ((freopen(in_file_name, "rb", stdin)) == NULL) {
			fprintf(stderr, "Error: can't open %s", in_file_name);
		}
		fclose(stdout);
		if (freopen(out_file_name, "wb", stdout) == NULL) {
			fprintf(stderr, "Error: can't write to %s", out_file_name);
		}

		int num_of_char_table[maxCharsNum] = { 0 }; // 存储每个字符的数量
		int tnum;									// 字符总数
		tnum = read_data(num_of_char_table);		// 读取数据

		// 生成huaffman树
		node* head = NULL;							// huffman树的根结点
		int ar[maxEncodeLength];					// 符号生成数组，用在遍历huffman树时，存储编码状态
		head = generate_huffman_tree(num_of_char_table, tnum);

		data_buffer symbols[maxCharsNum];			// 符号表
		int si = 0;
		for (si = 0; si < maxCharsNum; si++) {		// 初始化符号表
			db_flush(symbols + si);
		}
		
		generate_symbole_table(head, ar, symbols);	// 通过huffman树生成符号表, 并写入文件
		output_extra_bit_len(symbols, num_of_char_table);	// 将最后一个缓冲区的位数写入文件

		clear_huffman_tree(head);					// 释放内存

		// 输入、输出重定向
		fclose(stdin);
		if (freopen(in_file_name, "rb", stdin) == NULL) {
			fprintf(stderr, "Error: can't open %s!", in_file_name);
		}
		fclose(stdout);
		if (freopen(out_file_name, "ab", stdout) == NULL) {		// 此处为从文件结尾开始写入内容
			fprintf(stderr, "Error: can't write to %s", out_file_name);
		}

		output_compressed_content(symbols);			// 根据符号表，读取输入，输出压缩后的内容
		fclose(stdin);
		fclose(stdout);
	}
	else {
		// 输入、输出重定向
		fclose(stdin);
		if (freopen(in_file_name, "rb", stdin) == NULL) {
			fprintf(stderr, "Error: can't read %s", in_file_name);
		}
		fflush(stdout);
		if (freopen(out_file_name, "wb", stdout) == NULL) {
			fprintf(stderr, "Error: can't write to %s", out_file_name);
		}

		// 读取符号表，通过符号表生成huffman树
		node* head = build_huffman_tree();

		// 读取压缩内容，解压缩，输出
		decode(head, get_extra_bit_len());
		// 释放内存
		clear_huffman_tree(head);
		fclose(stdout);
	}	

	return 0;
}

int read_data(int* arr)
{
	int count = 0, err_count = 0;
	uchar tc;
	while ((tc = (uchar)(char)getchar()) != 255u) {
		if (tc >= maxCharsNum || tc < 0u) {
			fprintf(stderr, "ERROR: wrong char %d", err_count++);
			exit(-1);
		}
		count++;
		arr[tc]++;
	}
	return count;
}

void output_compressed_content(data_buffer*symbols) {
	// 缓冲需要输出的内容
	data_buffer* temp = malloc(sizeof(data_buffer));
	// 缓冲溢出的内容
	data_buffer* newbuf = malloc(sizeof(data_buffer));
	// 初始化
	db_flush(temp);
	db_flush(newbuf);
	uchar tc;
	while ((tc = (uchar)(char)getchar()) != 255u) {
		if (tc >= maxCharsNum || tc < 0u) {
			fprintf(stderr, "Error: wrong char");
			exit(-1);
		}
		// 若缓冲区溢出，则输出，并将newbuf内容移至temp, 清空newbuf
		if (newbuf->len != 0) {
			int k;
			for (k = 0; k < length_in_bytes(temp); k++) {
				putchar((char)temp->databuf[k]);
			}
			fflush(stdout);			// 清空缓冲区
			db_copy(temp, newbuf);
			db_flush(newbuf);
		}
		// 插入至缓冲区
		db_insert(temp, symbols + tc, newbuf);
	}
	// 输出未满的缓冲区
	if (temp->len != 0) {
		int k;
		for (k = 0; k < length_in_bytes(temp); k++) {
			putchar((char)temp->databuf[k]);
		}
	}
	if (temp != NULL) {
		free(temp);
	}
	if (newbuf != NULL) {
		free(newbuf);
	}
	fflush(stdout);				// 清空缓冲区
}

