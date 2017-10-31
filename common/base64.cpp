/*********************************************************************
* Filename:   base64.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the Base64 encoding algorithm.
*********************************************************************/

/*************************** HEADER FILES ***************************/
#include <stdlib.h>
#include "base64.h"

/****************************** MACROS ******************************/
#define NEWLINE_INVL 76

/**************************** VARIABLES *****************************/
// Note: To change the charset to a URL encoding, replace the '+' and '/' with '*' and '-'
static const char charset[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

/*********************** FUNCTION DEFINITIONS ***********************/
byte_t revchar(char ch){
	if (ch >= 'A' && ch <= 'Z')
		ch -= 'A';
	else if (ch >= 'a' && ch <='z')
		ch = ch - 'a' + 26;
	else if (ch >= '0' && ch <='9')
		ch = ch - '0' + 52;
	else if (ch == '+')
		ch = 62;
	else if (ch == '/')
		ch = 63;

	return(ch);
}

std::vector<byte_t> base64_decode(const std::string &in){
	auto len = in.size();
	if (in[len - 1] == '=')
		len--;
	if (in[len - 1] == '=')
		len--;

	auto blks = len / 4;
	auto left_over = len % 4;

	std::vector<byte_t> ret;
	auto blk_ceiling = blks * 4;
	size_t idx2 = 0;
	for (size_t idx = 0; idx2 < blk_ceiling; idx += 3, idx2 += 4){
		if (in[idx2] == '\n')
			idx2++;
		ret.push_back((revchar(in[idx2]) << 2) | ((revchar(in[idx2 + 1]) & 0x30) >> 4));
		ret.push_back((revchar(in[idx2 + 1]) << 4) | (revchar(in[idx2 + 2]) >> 2));
		ret.push_back((revchar(in[idx2 + 2]) << 6) | revchar(in[idx2 + 3]));
	}

	if (left_over == 2){
		ret.push_back((revchar(in[idx2]) << 2) | ((revchar(in[idx2 + 1]) & 0x30) >> 4));
	}else if (left_over == 3){
		ret.push_back((revchar(in[idx2]) << 2) | ((revchar(in[idx2 + 1]) & 0x30) >> 4));
		ret.push_back((revchar(in[idx2 + 1]) << 4) | (revchar(in[idx2 + 2]) >> 2));
	}

	return ret;
}
