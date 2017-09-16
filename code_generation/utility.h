#pragma once
#include <string>
#include <vector>
#include <map>

typedef std::map<std::string, std::string> known_hashes_t;

extern const char * const generated_file_warning;

unsigned to_unsigned(const std::string &s);
unsigned to_unsigned_default(const std::string &s, unsigned def = 0);
unsigned hex_no_prefix_to_unsigned(const std::string &s);
unsigned hex_no_prefix_to_unsigned_default(const std::string &s, unsigned def = 0);
std::string hash_buffer(const void *, size_t);
std::string hash_file(const std::string &path);
std::string hash_files(const std::vector<std::string> &files);
//Returns true if the key is found and the hash matches, otherwise returns false.
bool check_for_known_hash(const known_hashes_t &, const std::string &key, const std::string &value);
bool is_hex(char c);

template <typename T, size_t N>
size_t array_length(const T(&)[N]){
	return N;
}
