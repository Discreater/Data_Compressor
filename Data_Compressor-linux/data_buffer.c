#include "data_buffer.h"
#include <stdio.h>
#include <stdlib.h>

int length_in_chars(data_buffer* db)
{
	return db->len / 8;
}

int length_in_bytes(data_buffer* db)
{
	if (db->len % 8 == 0) {
		return db->len / 8;
	}
	else {
		return db->len / 8 + 1;
	}
}

void db_insert(data_buffer* oldval, data_buffer* newval, data_buffer* newbuf)
{
	int offset = oldval->len % 8, j;
	int maxByte;
	maxByte = ((newval->len + oldval->len) > bitsPerByte * dataBufLen) ? dataBufLen - length_in_bytes(oldval) : length_in_bytes(newval);

	int i;
	for (i = 0 ; i < maxByte; i++) {
		oldval->databuf[i + length_in_chars(oldval)] |= newval->databuf[i] << offset;
		oldval->databuf[i + length_in_chars(oldval) + 1] |= newval->databuf[i] >> (8 - offset);
	}

	if(newval->len + oldval->len > dataBufLen * 8)
	{
		offset = (offset == 0) ? 8 : offset;
		oldval->databuf[i + length_in_bytes(oldval) - 1] |= newval->databuf[i] << offset;
		for(i = maxByte, j=0; i<length_in_bytes(newval); i++,j++ )
		{
			newbuf->databuf[j] |= (newval->databuf[i] >> (8-offset));
			newbuf->databuf[j] |= newval->databuf[i+1] << offset;
		}
		newbuf->len = oldval->len + newval->len - dataBufLen * 8;
	}
	oldval->len = oldval->len + newval->len > dataBufLen * 8 ? dataBufLen * 8 : oldval->len + newval->len;
}

void db_init(data_buffer* db, int* ar, int len)
{
	db_flush(db);
	if (len >= dataBufLen * bitsPerByte || db == NULL) {
		fprintf(stderr, "Error: in db_init()");
		exit(-1);
	}
	int i, j;
	for (i = 0; i < len / 8; i++) {
		for (j = 0; j < 8 && i * 8 + j < len; j++) {
			db->databuf[i] = db->databuf[i] << 1;
			db->databuf[i] += ar[(i + 1) * 8 - j - 1];
		}
	}
	if (len % 8 != 0) {
		for (j = 0; j < len % 8; j++) {
			db->databuf[i] = db->databuf[i] << 1;
			db->databuf[i] += ar[i * 8 + len % 8 - j - 1];
		}
	}
	db->len = len;
}

void db_flush(data_buffer* db) {
	int i;
	for (i = 0; i < dataBufLen; i++) {
		db->databuf[i] = 0;
	}
	db->len = 0;
}

void db_copy(data_buffer* oldval, data_buffer* e)
{
	int i;
	for(i = 0; i < dataBufLen ; i++)
		oldval->databuf[i] = e->databuf[i];

	oldval->len = e->len;
}

// void db_fill(data_buffer* oldval, data_buffer* newval, int shiftamt)
// {
	
// }
