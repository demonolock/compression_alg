/* Common for all compression algorithms */
#ifndef COMPR_ALG_H
#define COMPR_ALG_H
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define SAMPLE_COUNT 16
#define PAGE_SIZE 1024 * 8
#define MAX_TRAIN_SIZE (SAMPLE_COUNT * PAGE_SIZE)  // 128KB (16 Pages)
#define MAX_FILES 10000

typedef struct {
    char fileName[256];
    size_t offset; // Position for continue reading
} FileState;


typedef struct CompressionInterface {
    size_t (*trainDict)(unsigned char* dictBuffer, size_t dictMaxSize, const unsigned char* data, const size_t* samplesSizes, size_t sampleCount);
    size_t (*compress)(const unsigned char* data, size_t dataSize, unsigned char* compressed, size_t compressedMaxSize, const unsigned char* dict, size_t dictSize);
    size_t (*decompress)(const unsigned char* compressed, size_t compressedSize, unsigned char* decompressed, size_t decompressedMaxSize, const unsigned char* dict, size_t dictSize);
} CompressionInterface;

size_t
getAllFiles(const char* directoryPath, char fileNames[][256]);

size_t
readDataFromMultipleFiles(FileState remainingFiles[], size_t* numRemainingFiles, unsigned char* buffer);

#endif // COMPR_ALG_H
