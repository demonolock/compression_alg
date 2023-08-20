#include "compr_alg.h"

size_t
zlib_compress(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize);

size_t
zlib_decompress(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize);

size_t
zlib_compress_noDict(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char *dict, size_t dictSize);

size_t
zlib_decompress_noDict(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize);