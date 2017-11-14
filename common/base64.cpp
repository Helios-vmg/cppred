/*********************************************************************
* Filename:   base64.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the Base64 encoding algorithm.
*********************************************************************/

#include <stdlib.h>
#include "base64.h"

static const char charset[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

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

std::string base64_encode(const std::vector<byte_t> &in){
	size_t idx, blk_ceiling, newline_count = 0;

	auto blks = in.size() / 3;
	auto left_over = in.size() % 3;

	std::string ret;
	//Allocate a few bytes more to ensure no reallocations are necessary.
	ret.reserve(in.size() * 3 / 2 + 16);
	
	blk_ceiling = blks * 3;
	for (idx = 0; idx < blk_ceiling; idx += 3){
		ret.push_back(charset[in[idx] >> 2]);
		ret.push_back(charset[((in[idx] & 0x03) << 4) | (in[idx + 1] >> 4)]);
		ret.push_back(charset[((in[idx + 1] & 0x0f) << 2) | (in[idx + 2] >> 6)]);
		ret.push_back(charset[in[idx + 2] & 0x3F]);
	}

	if (left_over == 1) {
		ret.push_back(charset[in[idx] >> 2]);
		ret.push_back(charset[(in[idx] & 0x03) << 4]);
		ret.push_back('=');
		ret.push_back('=');
	}else if (left_over == 2){
		ret.push_back(charset[in[idx] >> 2]);
		ret.push_back(charset[((in[idx] & 0x03) << 4) | (in[idx + 1] >> 4)]);
		ret.push_back(charset[(in[idx + 1] & 0x0F) << 2]);
		ret.push_back('=');
	}

	return ret;
}
