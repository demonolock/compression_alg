#include "libdeflate.h"


size_t
libdeflate_trainDict(unsigned char* dictBuffer, size_t dictMaxSize, const unsigned char* data, const size_t* samplesSizes, size_t sampleCount) {
    (void)dictBuffer; // to suppress warnings
    (void)dictMaxSize;
    (void)data;
    (void)samplesSizes;
    (void)sampleCount;
    return 0; // libdeflate doesn't support dictionary training
}

size_t
libdeflate_compress(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;

    struct libdeflate_compressor* compressor = libdeflate_alloc_compressor();
    size_t compressedSize = libdeflate_deflate_compress(compressor, data, dataSize, compressed, compressedMaxSize);
    libdeflate_free_compressor(compressor);
    return compressedSize;
}

size_t
libdeflate_decompress(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize) {
    (void)dict; // to suppress warnings
    (void)dictSize;

    struct libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();
    size_t actualOutSize;
    enum libdeflate_result result = libdeflate_deflate_decompress(decompressor, compressed, compressedSize, decompressed, decompressedMaxSize, &actualOutSize);
    libdeflate_free_decompressor(decompressor);
    if (result == LIBDEFLATE_SUCCESS) {
        return actualOutSize;
    } else {
        return 0;
    }
}
