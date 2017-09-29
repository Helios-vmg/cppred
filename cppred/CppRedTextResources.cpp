#include "CppRedTextResources.h"

TextStore::TextStore(){
	for (size_t i = 0; i < packed_text_data_size;)
		this->parse_command(packed_text_data + i, packed_text_data_size - i);
}

void TextStore::parse_command(const byte_t *buffer, size_t size){
	
}
