#include "compr_alg.h"

size_t
libdeflate_compress_noDict(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize);

size_t
libdeflate_decompress_noDict(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize);
