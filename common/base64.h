/*********************************************************************
* Filename:   base64.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding Base64 implementation.
*********************************************************************/

#pragma once

/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include <cstdint>
#include <vector>
#include <string>

/**************************** DATA TYPES ****************************/
typedef unsigned char byte_t;

/*********************** FUNCTION DECLARATIONS **********************/
// Returns the size of the output. If called with out = NULL, will just return
// the size of what the output would have been (without a terminating NULL).
std::vector<byte_t> base64_decode(const std::string &in);
