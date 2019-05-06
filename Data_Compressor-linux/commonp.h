#ifndef COMMONP_H
#define COMMONP_H

#include <stdio.h>

int ceild(int a, int b);

union file_mode
{
	unsigned mode;
	char buf[4];
};

void put64D(FILE* outfile,int* ar, int len);
void trans64DtoBin(char* ars, char* ar);

#endif // !COMMONP_H
