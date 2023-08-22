/* Zstd functions */
#include <zstd.h>
#include <zdict.h>

#define MAX_COMPRESS_SIZE ZSTD_compressBound(MAX_TRAIN_SIZE)
#define MAX_DICT_SIZE 2048  // 2 KB

size_t
zstd_trainDict(unsigned char *dictBuffer, size_t dictMaxSize, const unsigned char *data, const size_t *samplesSizes,
               size_t sampleCount);

size_t
zstd_compress(const unsigned char *data, size_t dataSize, unsigned char *compressed, size_t compressedMaxSize,
              const unsigned char *dict, size_t dictSize);

size_t
zstd_decompress(const unsigned char *compressed, size_t compressedSize, unsigned char *decompressed,
                size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize);

size_t
zstd_compress_noDict(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char *dict, size_t dictSize);

size_t
zstd_decompress_noDict(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize);
