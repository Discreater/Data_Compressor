#include <stdlib.h>
#include <string.h>

#include "commonp.h"

int ceild(int a, int b)
{
	if (a % b == 0) {
		return a / b;
	}
	return a / b + 1;
}

void put64D(FILE* outfile,int* ar, int len)
{
	fputc(' ', outfile);
	int i, j;
	for (i = 0; i < ceild(len, 6); i++) {
		unsigned res = 0;
		int sbeg, send;
		sbeg = len - (ceild(len, 6) - i) * 6;
		send = sbeg + 6;
		if (sbeg < 0) sbeg = 0;
		for (j = 0; j < send - sbeg; j++) {
			res = res << 1;
			res += (unsigned)ar[j + sbeg];
		}
		if (res < 10) {
			fputc(res + '0', outfile);
		}
		else if (res < 36) {
			fputc(res - 10 + 'A', outfile);
		}
		else if (res < 62) {
			fputc(res - 36 + 'a', outfile);
		}
		else if (res == 62) {
			fputc('!', outfile);
		}
		else {
			fputc('?', outfile);
		}
	}
}

void trans64DtoBin(char* ars, char* ar)
{
	int i;
	int step = 0;
	for (i = 0; ars[i] != '\0'; i++) {
		int res = 0;
		if (ars[i] >= '0' && ars[i] <= '9') {
			res = ars[i] - '0';
		}
		else if (ars[i] >= 'A' && ars[i] <= 'Z') {
			res = ars[i] - 'A' + 10;
		}
		else if (ars[i] >= 'a' && ars[i] <= 'z') {
			res = ars[i] - 'a' + 36;
		}
		else if (ars[i] == '!') {
			res = 62;
		}
		else if (ars[i] == '?') {
			res = 63;
		}
		else {
			fprintf(stderr, "Error: unknown file head.\n");
			exit(-1);
		}

		char binchar[6];
		memset(binchar, 0, 6);
		int bincharlen = 0;
		if (res == 0) {
			if (i == 0) {
				fprintf(stderr, "Error: unknown file head.\n");
				exit(-1);
			}
		}
		while (res != 0) {
			if (bincharlen >= 6) {
				fprintf(stderr, "Error: unknown file head.\n");
				exit(-1);
			}
			binchar[bincharlen++] = res % 2;
			res /= 2;
		}
		int j = i ? 5 : bincharlen - 1;
		for (; j >= 0; j--) {
			ar[step++] = binchar[j] + '0';
		}
	}
	ar[step] = '\0';
}
