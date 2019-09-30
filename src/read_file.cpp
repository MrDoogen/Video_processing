#include <fstream>
#include <cstdint>
#include <cstddef>

#include "fsize.h"
#include "read_file.h"


uint8_t *read_file(char *in_filename) {
	size_t file_size = fsize(in_filename);
	if (!file_size) {
		return nullptr;
	}
	std::ifstream infile;
	try {
        infile.open(in_filename, std::ios::in|std::ios::binary);
    }
    catch (const std::ios_base::failure &) {
        return nullptr;
    }

    infile.seekg(0, std::ios::beg);
    
    uint8_t *data;
    try {
        data = new uint8_t [file_size];
    }
    catch (const std::bad_alloc &) {
        return nullptr;
    }

    infile.read(reinterpret_cast<char*>(data), file_size * sizeof(char));
    infile.close();
    return data;
}
