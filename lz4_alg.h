size_t
lz4_zstdTrainDict(unsigned char* dictBuffer, size_t dictMaxSize, const unsigned char* data, const size_t* samplesSizes, size_t sampleCount);

size_t
lz4_trainDict(unsigned char *dictBuffer, size_t dictMaxSize, const unsigned char *data, const size_t *samplesSizes,
              size_t sampleCount);

size_t
lz4_compress(const unsigned char *data, size_t dataSize, unsigned char *compressed, size_t compressedMaxSize,
             const unsigned char *dict, size_t dictSize);

size_t
lz4_decompress(const unsigned char *compressed, size_t compressedSize, unsigned char *decompressed,
               size_t decompressedMaxSize, const unsigned char *dict, size_t dictSize);