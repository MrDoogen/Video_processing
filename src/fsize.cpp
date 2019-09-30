#include <fstream>
#include <cstddef>


size_t fsize(char *in_filename) {
    std::ifstream infile;
    try {
        infile.open(in_filename, std::ios::in|std::ios_base::binary);
    }
    catch (const std::ios_base::failure &) {
        return 0;
    }
    infile.seekg(0, std::ios_base::end);
    size_t size = infile.tellg();

    infile.close();

    return size;
}
