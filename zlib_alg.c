#include <zlib.h>
#include "zlib_alg.h"

#define ZLIB_COMPRESSION_LEVEL Z_BEST_COMPRESSION  // from Z_NO_COMPRESSION (0) to Z_BEST_COMPRESSION (9)

size_t
zlib_compress(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    deflateInit(&strm, ZLIB_COMPRESSION_LEVEL);
    if (dict != NULL) {
        deflateSetDictionary(&strm, dict, dictSize);
    }

    strm.avail_in = dataSize;
    strm.next_in = (unsigned char *)data;
    strm.avail_out = compressedMaxSize;
    strm.next_out = compressed;

    deflate(&strm, Z_FINISH);
    size_t compressedSize = compressedMaxSize - strm.avail_out;

    deflateEnd(&strm);

    return compressedSize;
}

size_t
zlib_decompress(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    inflateInit(&strm);
    if (dict != NULL) {
        inflateSetDictionary(&strm, dict, dictSize);
    }

    strm.avail_in = compressedSize;
    strm.next_in = (unsigned char *)compressed;
    strm.avail_out = decompressedMaxSize;
    strm.next_out = decompressed;

    inflate(&strm, Z_FINISH);
    size_t decompressedSize = decompressedMaxSize - strm.avail_out;

    inflateEnd(&strm);

    return decompressedSize;
}

size_t
zlib_compress_noDict(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;

    return zlib_compress(data, dataSize, compressed, compressedMaxSize, NULL, 0);
}

size_t
zlib_decompress_noDict(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize) {
    (void)dict;  // to suppress warnings
    (void)dictSize;

    return zlib_decompress(compressed, compressedSize, decompressed, decompressedMaxSize, NULL, 0);
}
