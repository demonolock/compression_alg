#include "zstd_alg.h"

#define ZSTD_COMPRESSION_LEVEL 3  // 1 to 22. 3 is default level

size_t
zstd_trainDict(unsigned char* dictBuffer, size_t dictMaxSize, const unsigned char* data, const size_t* samplesSizes, size_t sampleCount) {
    return ZDICT_trainFromBuffer(dictBuffer, dictMaxSize, data, samplesSizes, sampleCount);
}

size_t
zstd_compress(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize) {
    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    size_t compressedSize = ZSTD_compress_usingDict(cctx, compressed, compressedMaxSize, data, dataSize, dict, dictSize, 3);
    ZSTD_freeCCtx(cctx);
    return compressedSize;
}

size_t
zstd_decompress(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize) {
    ZSTD_DCtx* dctx = ZSTD_createDCtx();
    size_t decompressedSize = ZSTD_decompress_usingDict(dctx, decompressed, decompressedMaxSize, compressed, compressedSize, dict, dictSize);
    ZSTD_freeDCtx(dctx);
    return decompressedSize;
}

size_t
zstd_compress_noDict(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;
    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    size_t compressedSize = ZSTD_compressCCtx(cctx, compressed, compressedMaxSize, data, dataSize, 3);
    ZSTD_freeCCtx(cctx);
    return compressedSize;
}

size_t
zstd_decompress_noDict(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;
    ZSTD_DCtx* dctx = ZSTD_createDCtx();
    size_t decompressedSize = ZSTD_decompressDCtx(dctx, decompressed, decompressedMaxSize, compressed, compressedSize);
    ZSTD_freeDCtx(dctx);
    return decompressedSize;
}





