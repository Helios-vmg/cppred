#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <memory>

#define DELETE_COPY_CONSTRUCTORS(x) \
	x(const x &) = delete; \
	x(x &&) = delete; \
	void operator=(const x &) = delete; \
	void operator=(x &&) = delete;

typedef std::map<std::string, std::string> known_hashes_t;
typedef std::uint8_t byte_t;

extern const char * const generated_file_warning;

unsigned to_unsigned(const std::string &s);
int to_int(const std::string &s);
unsigned to_unsigned_default(const std::string &s, unsigned def = 0);
unsigned hex_no_prefix_to_unsigned(const std::string &s);
unsigned hex_no_prefix_to_unsigned_default(const std::string &s, unsigned def = 0);
bool to_bool(const std::string &s);
const char *bool_to_string(bool);
std::string hash_file(const std::string &path, const char *date_string);
std::string hash_files(const std::vector<std::string> &files, const char *date_string);
//Returns true if the key is found and the hash matches, otherwise returns false.
bool check_for_known_hash(const known_hashes_t &, const std::string &key, const std::string &value);
bool is_hex(char c);
void write_buffer_to_stream(std::ostream &, const std::vector<std::uint8_t> &);
void write_varint(std::vector<std::uint8_t> &dst, std::uint32_t);
void write_ascii_string(std::vector<std::uint8_t> &dst, const std::string &);
typedef std::map<std::string, std::shared_ptr<std::vector<byte_t>>> data_map_t;
data_map_t read_data_csv(const char *path);

template <typename T>
void write_collection_to_stream(std::ostream &stream, const T &begin, const T &end){
	stream << "{\n";
	bool new_line = true;
	std::stringstream accum;
	for (auto it = begin; it != end; ++it){
		if (new_line){
			accum << "   ";
			new_line = false;
		}
		accum << " " << *it << ",";
		if (accum.str().size() >= 80){
			new_line = true;
			stream << accum.str() << std::endl;
			auto temp = std::move(accum);
		}
	}
	if (!new_line)
		stream << accum.str() << std::endl;
	stream << "}";
}

template <typename T, size_t N>
size_t array_length(const T(&)[N]){
	return N;
}
