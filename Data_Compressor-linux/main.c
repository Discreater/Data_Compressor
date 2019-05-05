#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include "huffman.h"
#include "data_buffer.h"
#include "decompress.h"

#define INLINUX

typedef unsigned int uint;
typedef unsigned char uchar;

int console_print = 1;

long long read_data(int* arr);				// 读取输入生成符号表
void output_compressed_content(data_buffer* symbols);	// 输出

void printhelp();


int main(int argc, char** argv) {

	struct timeval start, end;
	gettimeofday(&start, NULL);

	// 解析命令行参数，具体行为见Usage.txt文件
	int file_from_stdin = 1;
	char* in_file_name;

	int file_to_stdout = 1;
	char* out_file_name;

	int compress = 1;

	console_print = 1;

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
				console_print = 0;
				break;
			case 'h':
				printhelp();
				exit(0);
				break;
			case 'p':
				compress = 1;
				break;
			case 'u':
				compress = 0;
				break;
			default:
				fprintf(stderr, "Error: unknown option '%c'.\n", ch);
				exit(-1);
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
		// 打开输入输出文件
		if ((infile = fopen(in_file_name, "rb")) == NULL) {
			fprintf(stderr, "Error: can't open '%s'\n", in_file_name);
			exit(-1);
		}
		if ((outfile = fopen(out_file_name, "wb")) == NULL) {
			fprintf(stderr, "Error: can't write to '%s'\n", out_file_name);
			exit(-1);
		}

		int num_of_char_table[maxCharsNum] = { 0 }; // 存储每个字符的数量
		long long  tnum;							// 字符总数
		tnum = read_data(num_of_char_table);		// 读取数据

		if (console_print) {
			printf("Read input file('%s') succeed.\n", in_file_name);
		}

		// 生成huaffman树
		node* head = NULL;							// huffman树的根结点
		int ar[maxEncodeLength];					// 符号生成数组，用在遍历huffman树时，存储编码状态
		head = generate_huffman_tree(num_of_char_table, tnum);

		if (console_print) {
			printf("Generate huffman tree succeed.\n");
		}

		data_buffer symbols[maxCharsNum];			// 符号表
		int si = 0;
		for (si = 0; si < maxCharsNum; si++) {		// 初始化符号表
			db_flush(symbols + si);
		}
		double avg_code_len;
	 	avg_code_len = generate_symbole_table(head, ar, symbols);	// 通过huffman树生成符号表, 并写入文件
		output_extra_bit_len(symbols, num_of_char_table);	// 将最后一个缓冲区的位数写入文件

		if (console_print) {
			printf("Generate symbol table succeed.\n");
			printf("Write file head succeed.\n");
		}

		if (console_print) {
			int symbcount = 0;
			for (si = 0; si < maxCharsNum; si++) {
				if (symbols[si].len > 0) {
					symbcount += 2 + symbols[si].len;
					if (si < 10) {
						symbcount += 1;
					}
					else if (si < 100) {
						symbcount += 2;
					}
					else {
						symbcount += 3;
					}
				}
			}
			symbcount++;
			symbcount += 2;		// 存储额外位所占空间
			printf("Size of file head:\t%d B (symbol table with extra bit)\n", symbcount);
		}

		clear_huffman_tree(head);					// 释放内存

		fclose(infile);
		// 重新打开输入文件
		if ((infile = fopen(in_file_name, "rb")) == NULL) {
			fprintf(stderr, "Error: can't open %s\n", in_file_name);
		}
		
		output_compressed_content(symbols);			// 根据符号表，读取输入，输出压缩后的内容

		if (console_print) {
			printf("Write to output file(%s) succeed.\n", out_file_name);
			printf("Average encode length :\t%lf (per Byte)\n", avg_code_len);
			printf("Compress complete.\n");
		}
	}
	else {
		// 打开输入输出文件
		if ((infile = fopen(in_file_name, "rb")) == NULL) {
			fprintf(stderr, "Error: can't open '%s'\n", in_file_name);
			exit(-1);
		}
		if ((outfile = fopen(out_file_name, "wb")) == NULL) {
			fprintf(stderr, "Error: can't write to '%s'\n", out_file_name);
			exit(-1);
		}

		// 读取符号表，通过符号表生成huffman树
		node* head = build_huffman_tree();
		
		if (console_print) {
			printf("Read file head succeed.\n");
		}

		// 读取压缩内容，解压缩，输出
		decode(head, get_extra_bit_len());

		if (console_print) {
			printf("Decompress finished.\n");
		}

		// 释放内存
		clear_huffman_tree(head);
	}

	fclose(infile);
	fclose(outfile);
	gettimeofday(&end, NULL);
	printf("Consume time: %ld ms\n", (end.tv_usec - start.tv_usec)/1000);
	return 0;
}

long long read_data(int* arr)
{
	long long count = 0, err_count = 0;
	int ch;
	uchar tc;
	while ((ch = fgetc(infile)) != EOF) {
		tc = (uchar)(char)ch;
		if (tc >= maxCharsNum || tc < 0u) {
			fprintf(stderr, "ERROR: wrong char %d\n", err_count++);
			exit(-1);
		}
		count++;
		arr[tc]++;
	}
	if (console_print) {
		printf("Size of input file:\t%lld B\n", count);
	}
	return count;
}

void output_compressed_content(data_buffer*symbols) {
	long long tnum = 0;
	// 缓冲需要输出的内容
	data_buffer* temp = malloc(sizeof(data_buffer));
	// 缓冲溢出的内容
	data_buffer* newbuf = malloc(sizeof(data_buffer));
	// 初始化
	db_flush(temp);
	db_flush(newbuf);
	int ch;
	uchar tc;
	while ((ch = fgetc(infile)) != -1) {
		tc = (uchar)(char)ch;
		if (tc >= maxCharsNum || tc < 0u) {
			fprintf(stderr, "Error: wrong char.\n");
			exit(-1);
		}
		// 若缓冲区溢出，则输出，并将newbuf内容移至temp, 清空newbuf
		if (newbuf->len != 0) {
			int k;
			for (k = 0; k < length_in_bytes(temp); k++) {
				fputc((char)temp->databuf[k], outfile);
				tnum++;
			}
			fflush(outfile);			// 清空缓冲区
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
			fputc((char)temp->databuf[k], outfile);
			tnum++;
		}
	}
	if (temp != NULL) {
		free(temp);
	}
	if (newbuf != NULL) {
		free(newbuf);
	}
	fflush(outfile);				// 清空缓冲区
	if (console_print) {
		printf("Size of output file:\t%d B\n", tnum);
	}
}


void printhelp() {
	printf("Usage:\t./Compressor [OPTIONS] [FILES]\n");
	printf("  -i [file]\tThe input file\n");
	printf("  -o [file]\tThe outputfile\n");
	printf("  -n\t\tNo terminal print\n");
	printf("  -h\t\tPrint this help information\n");
	printf("  -p\t\tCompress the inpur file [defalut]\n");
	printf("  -u\t\tUncompress the input file\n");
}
