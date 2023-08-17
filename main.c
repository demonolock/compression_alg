#include <time.h>
#include <math.h>
#include <float.h>
#include "zstd_alg.h"
#include "compr_alg.h"
#include "lz4_alg.h"

CompressionInterface ZSTD_Interface = {
        .trainDict = zstd_trainDict,
        .compress = zstd_compress,
        .decompress = zstd_decompress
};


CompressionInterface LZ4_Interface = {
        .trainDict = lz4_zstdTrainDict,
        .compress = lz4_compress,
        .decompress = lz4_decompress
};

int main() {
    CompressionInterface* compression = &LZ4_Interface;

    double minCompressionTime = DBL_MAX, maxCompressionTime = 0.0, totalCompressionTime = 0.0;
    double minDecompressionTime = DBL_MAX, maxDecompressionTime = 0.0, totalDecompressionTime = 0.0;
    double minDictTrainingTime = DBL_MAX, maxDictTrainingTime = 0.0, totalDictTrainingTime = 0.0;
    double minCompressionRatio = DBL_MAX, maxCompressionRatio = 0.0, totalCompressionRatio = 0.0;
    int successfulOperationsCount = 0;

    unsigned char data[MAX_TRAIN_SIZE];
    char fileNames[MAX_FILES][256];
    size_t numFiles = getAllFiles("./data", fileNames);
    printf("Found %zu files\n", numFiles);

    FileState remainingFiles[MAX_FILES];
    for (size_t i = 0; i < numFiles; ++i) {
        strncpy(remainingFiles[i].fileName, fileNames[i], 256);
        remainingFiles[i].offset = 0;
    }

    unsigned char dictBuffer[MAX_DICT_SIZE];
    size_t samplesSizes[SAMPLE_COUNT];
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samplesSizes[i] = PAGE_SIZE;
    }

    while (numFiles > 0) {
        size_t bytesRead = readDataFromMultipleFiles(remainingFiles, &numFiles, data);
        if (bytesRead <= 0) {
            fprintf(stderr, "Failed to read data or empty files\n");
            continue;
        }

        if (bytesRead > MAX_TRAIN_SIZE) {
            fprintf(stderr, "File size exceeds the maximum training size\n");
            continue;
        }

        // Dictionary Training
        clock_t dictStartTime = clock();
        size_t dictSize = compression->trainDict(dictBuffer, MAX_DICT_SIZE, data, samplesSizes, SAMPLE_COUNT);
        clock_t dictEndTime = clock();
        if (ZSTD_isError(dictSize)) {
            fprintf(stderr, "Dictionary training failed: %s. numFiles: %lu; filename: %s\n", ZSTD_getErrorName(dictSize), numFiles, remainingFiles[numFiles].fileName);
            continue;
        }
        double dictTime = (double)(dictEndTime - dictStartTime) / CLOCKS_PER_SEC;
        minDictTrainingTime = fmin(minDictTrainingTime, dictTime);
        maxDictTrainingTime = fmax(maxDictTrainingTime, dictTime);
        totalDictTrainingTime += dictTime;

        for (size_t pageOffset = 0; pageOffset < bytesRead; pageOffset += PAGE_SIZE) {
            size_t currentPageSize = (bytesRead - pageOffset < PAGE_SIZE) ? bytesRead - pageOffset : PAGE_SIZE;

            // Compression for the current page
            unsigned char compressedData[MAX_COMPRESS_SIZE];
            clock_t compressionStartTime = clock();
            size_t compressedSize = compression->compress(&data[pageOffset], currentPageSize, compressedData, MAX_COMPRESS_SIZE, dictBuffer, dictSize);
            clock_t compressionEndTime = clock();
            if (ZSTD_isError(compressedSize)) {
                fprintf(stderr, "Compression failed: %s\n", ZSTD_getErrorName(compressedSize));
                continue;
            }

            double compressionTime = (double)(compressionEndTime - compressionStartTime) / CLOCKS_PER_SEC;
            minCompressionTime = fmin(minCompressionTime, compressionTime);
            maxCompressionTime = fmax(maxCompressionTime, compressionTime);
            totalCompressionTime += compressionTime;

            double compressionRatio = (double)compressedSize / (double)currentPageSize;
            minCompressionRatio = fmin(minCompressionRatio, compressionRatio);
            maxCompressionRatio = fmax(maxCompressionRatio, compressionRatio);
            totalCompressionRatio += compressionRatio;

            // Decompression for the current page
            unsigned char decompressedData[PAGE_SIZE];
            clock_t decompressionStartTime = clock();
            size_t decompressedSize = compression->decompress(compressedData, compressedSize, decompressedData, PAGE_SIZE, dictBuffer, dictSize);
            clock_t decompressionEndTime = clock();
            if (ZSTD_isError(decompressedSize)) {
                fprintf(stderr, "Decompression failed: %s\n", ZSTD_getErrorName(decompressedSize));
                continue;
            }
            double decompressionTime = (double)(decompressionEndTime - decompressionStartTime) / CLOCKS_PER_SEC;
            minDecompressionTime = fmin(minDecompressionTime, decompressionTime);
            maxDecompressionTime = fmax(maxDecompressionTime, decompressionTime);
            totalDecompressionTime += decompressionTime;

            successfulOperationsCount++;

            // Validate decompressed data for the current page
            if (currentPageSize != decompressedSize || memcmp(&data[pageOffset], decompressedData, currentPageSize) != 0) {
                fprintf(stderr, "Original and decompressed files for page at offset %zu do not match!\n", pageOffset);
                continue;
            }
        }
    }
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    printf("| Metric                           | Average      | Min          | Max          |\n");
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    printf("| Dictionary training time (s)     | %12.6lf | %12.6lf | %12.6lf |\n", totalDictTrainingTime / successfulOperationsCount, minDictTrainingTime, maxDictTrainingTime);
    printf("| Compression time (s)             | %12.6lf | %12.6lf | %12.6lf |\n", totalCompressionTime / successfulOperationsCount, minCompressionTime, maxCompressionTime);
    printf("| Decompression time (s)           | %12.6lf | %12.6lf | %12.6lf |\n", totalDecompressionTime / successfulOperationsCount, minDecompressionTime, maxDecompressionTime);
    printf("| Compression ratio                | %12.6lf | %12.6lf | %12.6lf |\n", totalCompressionRatio / successfulOperationsCount, minCompressionRatio, maxCompressionRatio);
    printf("+----------------------------------+--------------+--------------+--------------+\n");


    printf("Compression and decompression for all pages were successful! Original and decompressed pages match.\n");
    return 0;
}