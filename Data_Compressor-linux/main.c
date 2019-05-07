#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// 命令行参数解析
#include <getopt.h>
// memset函数
#include <string.h>
// 文件状态相关
#include <sys/stat.h>
// 时间相关
#include <time.h>
// 本地头文件
#include "commonp.h"
#include "huffman.h"
#include "data_buffer.h"
#include "decompress.h"

// 跨平台预备， 未实现
// #define INLINUX	4					

typedef unsigned int uint;
typedef unsigned char uchar;

// 是否需要终端输出
int console_print = 1;

long long read_data(int* arr);				// 读取输入生成符号表
long long output_compressed_content(data_buffer* symbols);	// 输出
void output_extra_bit_len(data_buffer* symbols, int* table);	// 计算最后一个缓冲区的位数，并输出
void output_mode_content(struct stat* state);

void Hello(void);
void printhelp(void);

int main(int argc, char** argv) {
	clock_t start, end;
	start = clock();

	// 解析命令行参数，具体行为见Usage.txt文件
	int file_from_stdin = 1;
	char* in_file_name = NULL;

	int file_to_stdout = 1;
	char* out_file_name = NULL;

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

	if (console_print) {
		Hello();
	}

	if (file_from_stdin) {
		fprintf(stderr, "Error: no input file!\n");
		exit(-1);
	}
	if (file_to_stdout) {
		out_file_name = malloc(sizeof(char) * (strlen(in_file_name) + 5));
		if (compress) {
			strcpy(out_file_name, in_file_name);
			strcat(out_file_name, ".hcp");
		}
		else {
			if (strcmp(in_file_name + strlen(in_file_name) - 4, ".hcp")) {
				strcpy(out_file_name, in_file_name);
				strcat(out_file_name, ".dcp");
			}
			else {
				strcpy(out_file_name, in_file_name);
				out_file_name[strlen(in_file_name) - 4] = '\0';
			}
		}
		if (access(out_file_name, 0) == 0) {
			printf("-Output file('%s') has existed. Do you want to cover it?([Y]/n):", out_file_name);
			int yn;
			yn = getchar();
			if (yn != 'y' && yn != 'Y') {
				exit(-1);
			}
			printf("\n");
		}
		if (console_print) {
			printf("Default output file: '%s'\n\n", out_file_name);
		}
	}

	if(compress){
		if (console_print) {
			printf("********** Compress **********\n\n");
		}
		// 打开输入输出文件
		if ((infile = fopen(in_file_name, "rb")) == NULL) {
			fprintf(stderr, "Error: can't open '%s'\n", in_file_name);
			exit(-1);
		}
		if ((outfile = fopen(out_file_name, "wb")) == NULL) {
			fprintf(stderr, "Error: can't write to '%s'\n", out_file_name);
			exit(-1);
		}
		// 获取输入文件的权限信息
		struct stat state;
		if (stat(in_file_name, &state) == -1) {
			// 失败
			fprintf(stderr, "Error: read file(%s) attribute error.\n", in_file_name);
		}

		if (console_print) {
			printf("reading...\n");
		}

		int num_of_char_table[maxCharsNum] = { 0 }; // 存储每个字符的数量
		long long  tnum;							// 字符总数
		tnum = read_data(num_of_char_table);		// 读取数据

		if (console_print) {
			printf("Read input file('%s') succeed.\n\n", in_file_name);
		}

		// 生成huaffman树
		node* head = NULL;							// huffman树的根结点
		int ar[maxEncodeLength + 1];					// 符号生成数组，用在遍历huffman树时，存储编码状态
		head = generate_huffman_tree(num_of_char_table, tnum);

		data_buffer symbols[maxCharsNum];			// 符号表
		int si = 0;
		for (si = 0; si < maxCharsNum; si++) {		// 初始化符号表
			db_flush(symbols + si);
		}
		double avg_code_len;
	 	avg_code_len = generate_symbole_table(head, ar, symbols);	// 通过huffman树生成符号表, 并写入文件
		output_extra_bit_len(symbols, num_of_char_table);	// 将最后一个缓冲区的位数写入文件
		output_mode_content(&state);							// 将文件权限信息写入文件
		
		// 计算符号表所占空间
		int symbcount = 0;
		if (console_print) {
			symbcount = 0;
			for (si = 0; si < maxCharsNum; si++) {
				if (symbols[si].len > 0) {
					symbcount += 2 + ceild(symbols[si].len, 6);
					if (si < 16) {
						symbcount += 1;
					}
					else {
						symbcount += 2;
					}
				}
			}
			symbcount++;
			symbcount += 2 + 4;		// 额外位与文件权限所占空间
		}

		clear_huffman_tree(head);					// 释放内存

		fclose(infile);
		// 重新打开输入文件
		if ((infile = fopen(in_file_name, "rb")) == NULL) {
			fprintf(stderr, "Error: can't open %s\n", in_file_name);
		}
		if (console_print) {
			printf("writing...\n");
		}
		long long outnum;
		outnum = output_compressed_content(symbols);			// 根据符号表，读取输入，输出压缩后的内容

		if (console_print) {
			printf("Write to output file('%s') succeed.\n\n", out_file_name);
			printf("--Size of input file:\t%lld B\n\n", tnum);
			printf("--Size of file head:\t%d B (symbol table with extra bit)\n", symbcount);
			printf("--Size of output file:\t%lld B = %lld B + %d B (include file head)\n\n", outnum + symbcount, outnum, symbcount);
			printf("Average encode length :\t%lf (per Byte)\n\n", avg_code_len);
			printf("Compress complete.\n");
		}
	}
	else {
		if (console_print) {
			printf("********* DeCompress *********\n\n");
		}
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
		int extra_bits = get_extra_bit_len();
		determine_mode(out_file_name);

		if (console_print) {
			printf("Read file head succeed.\n");
		}

		// 读取压缩内容，解压缩，输出
		decode(head, extra_bits);

		if (console_print) {
			printf("Decompress finished.\n");
		}

		// 释放内存
		clear_huffman_tree(head);
	}
	fclose(infile);
	fclose(outfile);
	if (console_print) {
		printf("\n******************************\n\n");
	}

	end = clock();
	if (console_print) {
		printf("Consume time: %lf s\n", (double)(end - start) / CLOCKS_PER_SEC);
	}
	return 0;
}

long long read_data(int* arr)
{
	long long count = 0;
	int ch;
	uchar tc;
	while ((ch = fgetc(infile)) != EOF) {
		tc = (uchar)(char)ch;
		if (tc >= maxCharsNum || tc < 0u) {
			fprintf(stderr, "Error: wrong char.\n");
			exit(-1);
		}
		count++;
		arr[tc]++;
	}
	return count;
}

long long output_compressed_content(data_buffer*symbols) {
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
		// 若data_buffer溢出，则输出，并将newbuf内容移至temp, 清空newbuf
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
		// 插入至新data_buffer
		db_insert(temp, symbols + tc, newbuf);
	}
	// 输出未满的data_buffer
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
	return tnum;
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


void output_mode_content(struct stat* state) {
	union file_mode mod;
	mod.mode = state->st_mode;
	int i;
	for (i = 0; i < 4; i++) {
		fputc(mod.buf[i], outfile);
	}
}

void Hello() {
	printf("******************************\n");
	printf("*** DATA COMPRESSOR        ***\n");
	printf("******************************\n");
	printf("Base on huffman encode method.\n\n");
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
