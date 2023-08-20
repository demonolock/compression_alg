#include <lz4hc.h>
#include <zdict.h>

#define LZ4_COMPRESSION_LEVEL 1  // 1 to 12. 1 is default level


size_t
lz4_zstdTrainDict(unsigned char* dictBuffer, size_t dictMaxSize, const unsigned char* data, const size_t* samplesSizes, size_t sampleCount) {
    return ZDICT_trainFromBuffer(dictBuffer, dictMaxSize, data, samplesSizes, sampleCount);
}

size_t
lz4_compress(const unsigned char *data, size_t dataSize, unsigned char *compressed, size_t compressedMaxSize,
             const unsigned char *dict, size_t dictSize) {
    LZ4_streamHC_t* lz4Stream = LZ4_createStreamHC();
    LZ4_loadDictHC(lz4Stream, (const char *)dict, dictSize);

    int compressedSize = LZ4_compress_HC_continue(lz4Stream, (const char *)data, (char *)compressed, dataSize, compressedMaxSize);

    LZ4_freeStreamHC(lz4Stream);
    return (size_t)compressedSize;
}

size_t
lz4_decompress(const unsigned char *compressed, size_t compressedSize, unsigned char *decompressed,
               size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize) {
    LZ4_streamDecode_t* lz4StreamDecode = LZ4_createStreamDecode();
    LZ4_setStreamDecode(lz4StreamDecode, (const char *)dict, dictSize);

    int decompressedSize = LZ4_decompress_safe_continue(lz4StreamDecode, (const char *)compressed, (char *)decompressed, compressedSize, decompressedMaxSize);

    LZ4_freeStreamDecode(lz4StreamDecode);
    return (size_t)decompressedSize;
}

size_t
lz4_compress_noDict(const unsigned char *data, size_t dataSize, unsigned char *compressed, size_t compressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;
    return (size_t) LZ4_compress_default((const char *)data, (char *)compressed, (int)dataSize, (int)compressedMaxSize);
}

size_t
lz4_decompress_noDict(const unsigned char *compressed, size_t compressedSize, unsigned char *decompressed, size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;
    return (size_t) LZ4_decompress_safe((const char *)compressed, (char *)decompressed, (int)compressedSize, (int)decompressedMaxSize);
}
