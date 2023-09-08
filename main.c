#include <time.h>
#include <math.h>
#include <float.h>
#include <unistd.h>

#include "zstd_alg.h"
#include "compr_alg.h"
#include "lz4_alg.h"
#include "libdeflate_alg.h"
#include "zlib_alg.h"

CompressionInterface ZSTD_Interface = {
        .trainDict = zstd_trainDict,
        .compress = zstd_compress,
        .decompress = zstd_decompress,
        .name = "ZSTD+"
};

CompressionInterface ZSTD_no_dict_Interface = {
        .trainDict = NULL,
        .compress = zstd_compress,
        .decompress = zstd_decompress,
        .name = "ZSTD"
};

CompressionInterface LZ4_Interface = {
        .trainDict = lz4_zstdTrainDict,
        .compress = lz4_compress,
        .decompress = lz4_decompress,
        .name = "LZ4+"
};

CompressionInterface LZ4_no_dict_Interface = {
        .trainDict = NULL,
        .compress = lz4_compress_noDict,
        .decompress = lz4_decompress_noDict,
        .name = "LZ4"
};

CompressionInterface Libdeflate_no_dict_Interface = {
        .trainDict = NULL,
        .compress = libdeflate_compress_noDict,
        .decompress = libdeflate_decompress_noDict,
        .name = "LibDf"
};

CompressionInterface Zlib_no_dict_Interface = {
        .trainDict = NULL,
        .compress = zlib_compress_noDict,
        .decompress = zlib_decompress_noDict,
        .name = "Zlib"
};



/* Compress 16 blocks together, train dict on 16 blocks*/
int
compress_16_blk(CompressionInterface *compression) {
    double minCompressionTime = DBL_MAX, maxCompressionTime = 0.0, totalCompressionTime = 0.0;
    double minDecompressionTime = DBL_MAX, maxDecompressionTime = 0.0, totalDecompressionTime = 0.0;
    double minDictTrainingTime = DBL_MAX, maxDictTrainingTime = 0.0, totalDictTrainingTime = 0.0;
    double minCompressionRatio = DBL_MAX, maxCompressionRatio = 0.0, totalCompressionRatio = 0.0;
    double minCompressionRatioWithDict = DBL_MAX, maxCompressionRatioWithDict = 0.0, totalCompressionRatioWithDict = 0.0;
    int successfulOperationsCount = 0;

    unsigned char data[MAX_TRAIN_SIZE];
    static char fileNames[MAX_FILES][256];
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
    size_t dictSize = 0;

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
        if (compression->trainDict && dictSize==0) {  /* If using dictionary */
            clock_t dictStartTime = clock();
            dictSize = compression->trainDict(dictBuffer, MAX_DICT_SIZE, data, samplesSizes, SAMPLE_COUNT);
            clock_t dictEndTime = clock();
            if (ZSTD_isError(dictSize)) {
                fprintf(stderr, "Dictionary training failed: %s. numFiles: %lu; filename: %s\n",
                        ZSTD_getErrorName(dictSize), numFiles, remainingFiles[numFiles].fileName);
                continue;
            }
            double dictTime = (double) (dictEndTime - dictStartTime) / CLOCKS_PER_SEC / SAMPLE_COUNT; // For 1 block
            minDictTrainingTime = fmin(minDictTrainingTime, dictTime);
            maxDictTrainingTime = fmax(maxDictTrainingTime, dictTime);
            totalDictTrainingTime += dictTime;
        }
        for (size_t pageOffset = 0; pageOffset < bytesRead; pageOffset += SAMPLE_COUNT * PAGE_SIZE) {
            size_t currentPageSize = (bytesRead - pageOffset < SAMPLE_COUNT * PAGE_SIZE) ? bytesRead - pageOffset :
                                     SAMPLE_COUNT * PAGE_SIZE;

            // Compression for the current page block
            unsigned char compressedData[MAX_COMPRESS_SIZE];
            clock_t compressionStartTime = clock();
            size_t compressedSize = compression->compress(&data[pageOffset], currentPageSize, compressedData,
                                                          MAX_COMPRESS_SIZE, dictBuffer, dictSize);
            clock_t compressionEndTime = clock();
            if (ZSTD_isError(compressedSize)) {
                fprintf(stderr, "Compression failed: %s\n", ZSTD_getErrorName(compressedSize));
                continue;
            }

            double compressionTime = (double) (compressionEndTime - compressionStartTime) / CLOCKS_PER_SEC;
            minCompressionTime = fmin(minCompressionTime, compressionTime);
            maxCompressionTime = fmax(maxCompressionTime, compressionTime);
            totalCompressionTime += compressionTime;

            double compressionRatio = (double) compressedSize / (double) currentPageSize;
            minCompressionRatio = fmin(minCompressionRatio, compressionRatio);
            maxCompressionRatio = fmax(maxCompressionRatio, compressionRatio);
            totalCompressionRatio += compressionRatio;

            // Decompression for the current page block
            unsigned char decompressedData[SAMPLE_COUNT * PAGE_SIZE];
            clock_t decompressionStartTime = clock();
            size_t decompressedSize = compression->decompress(compressedData, compressedSize, decompressedData,
                                                              SAMPLE_COUNT * PAGE_SIZE, dictBuffer, dictSize);
            clock_t decompressionEndTime = clock();
            if (ZSTD_isError(decompressedSize)) {
                fprintf(stderr, "Decompression failed: %s\n", ZSTD_getErrorName(decompressedSize));
                continue;
            }
            double decompressionTime = (double) (decompressionEndTime - decompressionStartTime) / CLOCKS_PER_SEC;
            minDecompressionTime = fmin(minDecompressionTime, decompressionTime);
            maxDecompressionTime = fmax(maxDecompressionTime, decompressionTime);
            totalDecompressionTime += decompressionTime;

            successfulOperationsCount += 1;

            // Validate decompressed data for the current page block
            if (currentPageSize != decompressedSize ||
                memcmp(&data[pageOffset], decompressedData, currentPageSize) != 0) {
                fprintf(stderr, "Original and decompressed files for page block at offset %zu do not match!\n",
                        pageOffset);
                continue;
            }

            // Calculate the effective compressed size considering the dictionary size (1 dict on 16 pages)
            size_t effectiveCompressedSize = compressedSize + dictSize;
            double compressionRatioWithDict = (double) effectiveCompressedSize / (double) currentPageSize;
            //printf("compressedSize: %lu, dictSize: %lu, currentPageSize: %lu, compressionRatioWithDict: %f\n", compressedSize, dictSize, currentPageSize, compressionRatioWithDict);
            minCompressionRatioWithDict = fmin(minCompressionRatioWithDict, compressionRatioWithDict);
            maxCompressionRatioWithDict = fmax(maxCompressionRatioWithDict, compressionRatioWithDict);
            totalCompressionRatioWithDict += compressionRatioWithDict;
        }
    }
    FILE *csvFile = fopen("./view/results.csv", "a+");
    if (csvFile == NULL) {
        perror("Failed to open CSV file for writing");
        return 1;
    }
    // dict training time got 1 block
    float dictTrainingTime = totalDictTrainingTime / successfulOperationsCount;
    // compression time without dict for 1 block
    float compressionTime = (totalCompressionTime / successfulOperationsCount) / SAMPLE_COUNT;
    // compression time + dict train time for 1 block
    float compressionAndDictTime = compressionTime + dictTrainingTime;
    // average compression ratio with dict
    float compressionRatioWithDict = totalCompressionRatioWithDict / successfulOperationsCount;
    // average compression without dict
    float compressionRatio = totalCompressionRatio / successfulOperationsCount;


    printf("\nCompression algorithm: %s\n", compression->name);
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    printf("| Metric                           | Average      | Min          | Max          |\n");
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    if (compression->trainDict) {  /* If using dictionary */
        printf("| Dictionary training time (s)     | %12.6lf | %12.6lf | %12.6lf |\n",
               dictTrainingTime, minDictTrainingTime, maxDictTrainingTime);
        printf("| Compression time (s) + dict time | %12.6lf | %12.6lf | %12.6lf |\n",
               compressionAndDictTime, minCompressionTime, maxCompressionTime);
        printf("| Compression ratio + dict size    | %12.6lf | %12.6lf | %12.6lf |\n",
               compressionRatioWithDict, minCompressionRatioWithDict, maxCompressionRatioWithDict);
        fprintf(csvFile, "%s16,%12.6lf,%12.6lf\n", compression->name, compressionAndDictTime, compressionRatioWithDict);
    }
    else {
        fprintf(csvFile, "%s16,%12.6lf,%12.6lf\n", compression->name, compressionTime, compressionRatio);
    }
    printf("| Compression time (s)             | %12.6lf | %12.6lf | %12.6lf |\n",
           compressionTime, minCompressionTime, maxCompressionTime);
    printf("| Decompression time (s)           | %12.6lf | %12.6lf | %12.6lf |\n",
           (totalDecompressionTime / successfulOperationsCount) / SAMPLE_COUNT, minDecompressionTime, maxDecompressionTime);
    printf("| Compression ratio                | %12.6lf | %12.6lf | %12.6lf |\n",
           compressionRatio, minCompressionRatio, maxCompressionRatio);
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    fclose(csvFile);
    //printf("Compression and decompression for all page blocks were successful! Original and decompressed blocks match.\n");
    return 0;
}

int
compress_1_blk(CompressionInterface *compression) {
    double minCompressionTime = DBL_MAX, maxCompressionTime = 0.0, totalCompressionTime = 0.0;
    double minDecompressionTime = DBL_MAX, maxDecompressionTime = 0.0, totalDecompressionTime = 0.0;
    double minDictTrainingTime = DBL_MAX, maxDictTrainingTime = 0.0, totalDictTrainingTime = 0.0;
    double minCompressionRatio = DBL_MAX, maxCompressionRatio = 0.0, totalCompressionRatio = 0.0;
    double minCompressionRatioWithDict = DBL_MAX, maxCompressionRatioWithDict = 0.0, totalCompressionRatioWithDict = 0.0;
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
    size_t dictSize = 0;
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
        if (compression->trainDict && dictSize==0) {  /* If using dictionary */
            clock_t dictStartTime = clock();
            dictSize = compression->trainDict(dictBuffer, MAX_DICT_SIZE, data, samplesSizes, SAMPLE_COUNT);
            clock_t dictEndTime = clock();
            if (ZSTD_isError(dictSize)) {
                fprintf(stderr, "Dictionary training failed: %s. numFiles: %lu; filename: %s\n",
                        ZSTD_getErrorName(dictSize), numFiles, remainingFiles[numFiles].fileName);
                continue;
            }
            double dictTime = (double) (dictEndTime - dictStartTime) / CLOCKS_PER_SEC / SAMPLE_COUNT;
            // printf("dict full time = %f\n", (double) (dictEndTime - dictStartTime) / CLOCKS_PER_SEC );
            minDictTrainingTime = fmin(minDictTrainingTime, dictTime);
            maxDictTrainingTime = fmax(maxDictTrainingTime, dictTime);
            totalDictTrainingTime += dictTime;
        }
        for (size_t pageOffset = 0; pageOffset < bytesRead; pageOffset += PAGE_SIZE) {
            size_t currentPageSize = (bytesRead - pageOffset < PAGE_SIZE) ? bytesRead - pageOffset : PAGE_SIZE;

            // Compression for the current page
            unsigned char compressedData[MAX_COMPRESS_SIZE];
            clock_t compressionStartTime = clock();
            size_t compressedSize = compression->compress(&data[pageOffset], currentPageSize, compressedData,
                                                          MAX_COMPRESS_SIZE, dictBuffer, dictSize);
            clock_t compressionEndTime = clock();
            if (ZSTD_isError(compressedSize)) {
                fprintf(stderr, "Compression failed: %s\n", ZSTD_getErrorName(compressedSize));
                continue;
            }

            double compressionTime = (double) (compressionEndTime - compressionStartTime) / CLOCKS_PER_SEC;
            minCompressionTime = fmin(minCompressionTime, compressionTime);
            maxCompressionTime = fmax(maxCompressionTime, compressionTime);
            totalCompressionTime += compressionTime;

            double compressionRatio = (double) compressedSize / (double) currentPageSize;
            minCompressionRatio = fmin(minCompressionRatio, compressionRatio);
            maxCompressionRatio = fmax(maxCompressionRatio, compressionRatio);
            totalCompressionRatio += compressionRatio;

            // Decompression for the current page
            unsigned char decompressedData[PAGE_SIZE];
            clock_t decompressionStartTime = clock();
            size_t decompressedSize = compression->decompress(compressedData, compressedSize, decompressedData,
                                                              PAGE_SIZE, dictBuffer, dictSize);
            clock_t decompressionEndTime = clock();
            if (ZSTD_isError(decompressedSize)) {
                fprintf(stderr, "Decompression failed: %s\n", ZSTD_getErrorName(decompressedSize));
                continue;
            }
            double decompressionTime = (double) (decompressionEndTime - decompressionStartTime) / CLOCKS_PER_SEC;
            minDecompressionTime = fmin(minDecompressionTime, decompressionTime);
            maxDecompressionTime = fmax(maxDecompressionTime, decompressionTime);
            totalDecompressionTime += decompressionTime;

            successfulOperationsCount++; // the number of pages

            // Validate decompressed data for the current page
            if (currentPageSize != decompressedSize ||
                memcmp(&data[pageOffset], decompressedData, currentPageSize) != 0) {
                fprintf(stderr, "Original and decompressed files for page at offset %zu do not match!\n", pageOffset);
                continue;
            }

            // Calculate the effective compressed size considering the dictionary size (1 dict on 16 pages)
            size_t effectiveCompressedSize = compressedSize + dictSize;
            double compressionRatioWithDict = (double) effectiveCompressedSize / (double) currentPageSize;
            //printf("compressedSize: %lu, dictSize: %lu, currentPageSize: %lu, compressionRatioWithDict: %f\n", compressedSize, dictSize, currentPageSize, compressionRatioWithDict);
            minCompressionRatioWithDict = fmin(minCompressionRatioWithDict, compressionRatioWithDict);
            maxCompressionRatioWithDict = fmax(maxCompressionRatioWithDict, compressionRatioWithDict);
            totalCompressionRatioWithDict += compressionRatioWithDict;
        }
    }
    FILE *csvFile = fopen("./view/results.csv", "a+");
    if (csvFile == NULL) {
        perror("Failed to open CSV file for writing");
        return 1;
    }
    // dict training time for 1 block
    float dictTrainingTime = totalDictTrainingTime / successfulOperationsCount * SAMPLE_COUNT;
    // compression time without dict
    float compressionTime = totalCompressionTime / successfulOperationsCount;
    // compression time + dict train time for 1 block
    float compressionTimeWithDict = compressionTime + dictTrainingTime;
    // compression ratio + fict for 1 block
    float compressionRatioWithDict = totalCompressionRatioWithDict / successfulOperationsCount;
    // compression ratio without dict
    float compressionRatio = totalCompressionRatio / successfulOperationsCount;


    printf("\nCompression algorithm: %s\n", compression->name);
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    printf("| Metric                           | Average      | Min          | Max          |\n");
    printf("+----------------------------------+--------------+--------------+--------------+\n");
    if (compression->trainDict) {  /* If using dictionary */
        printf("| Dictionary training time (s)     | %12.6lf | %12.6lf | %12.6lf |\n",
               dictTrainingTime, minDictTrainingTime, maxDictTrainingTime);
        printf("| Compression time (s) + dict time | %12.6lf | %12.6lf | %12.6lf |\n",
               compressionTimeWithDict, minCompressionTime, maxCompressionTime);
        printf("| Compression ratio + dict size    | %12.6lf | %12.6lf | %12.6lf |\n",
               compressionRatioWithDict, minCompressionRatioWithDict, maxCompressionRatioWithDict);
        fprintf(csvFile, "%s,%12.6lf,%12.6lf\n", compression->name, compressionTimeWithDict, compressionRatioWithDict);
    }
    else {
        fprintf(csvFile, "%s,%12.6lf,%12.6lf\n", compression->name, compressionTime, compressionRatio);
    }
    printf("| Compression time (s)             | %12.6lf | %12.6lf | %12.6lf |\n",
           compressionTime, minCompressionTime, maxCompressionTime);
    printf("| Decompression time (s)           | %12.6lf | %12.6lf | %12.6lf |\n",
           totalDecompressionTime / successfulOperationsCount, minDecompressionTime, maxDecompressionTime);
    printf("| Compression ratio                | %12.6lf | %12.6lf | %12.6lf |\n",
           compressionRatio, minCompressionRatio, maxCompressionRatio);
    printf("+----------------------------------+--------------+--------------+--------------+\n");

    fclose(csvFile);
    //printf("Compression and decompression for all pages were successful! Original and decompressed pages match.\n");
    return 0;
}

int main() {
    printf("16 blocks compressed together");
    compress_16_blk(&ZSTD_Interface);
    compress_16_blk(&ZSTD_no_dict_Interface);
    compress_16_blk(&LZ4_Interface);
    compress_16_blk(&LZ4_no_dict_Interface);
    compress_16_blk(&Libdeflate_no_dict_Interface);
    compress_16_blk(&Zlib_no_dict_Interface);
    printf("Compress by 1 block");
    compress_1_blk(&ZSTD_Interface);
    compress_1_blk(&ZSTD_no_dict_Interface);
    compress_1_blk(&LZ4_Interface);
    compress_1_blk(&LZ4_no_dict_Interface);
    compress_1_blk(&Libdeflate_no_dict_Interface);
    compress_1_blk(&Zlib_no_dict_Interface);
    return 0;
}
