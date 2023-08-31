#include "compr_alg.h"

// Exclude non data files
int
is_excluded(const char *dir_path, const char *filename) {
    const char *excluded_files[] = {".map", ".init", "PG_VERSION", "pg_control", "..", NULL};
    const char *not_excluded_dirs[] = {"/base", "/global", "/pg_tblspc", NULL};

    int dir_is_excluded = 1;
    for (int i = 0; not_excluded_dirs[i] != NULL; i++) {
        if (strstr(dir_path, not_excluded_dirs[i])) {
            dir_is_excluded = 0;
            break;
        }
    }
    if (dir_is_excluded) return 1;

    for (int i = 0; excluded_files[i] != NULL; i++) {
        if (strstr(filename, excluded_files[i])) return 1;
    }
    return 0;
}

size_t
getFilesFromDirectoryRecursively(const char *directoryPath, char fileNames[][256], size_t currentCount) {
    DIR *dir = opendir(directoryPath);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Failed to open directory");
        return currentCount;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", directoryPath, entry->d_name);

        if (entry->d_type == DT_REG) {
            if (is_excluded(directoryPath, entry->d_name)) {
                //printf("File %s/%s was excluded\n", directoryPath, entry->d_name);
                continue;
            }
            struct stat st;
            if (stat(fullPath, &st) != 0) {
                perror("Error obtaining file stats");
                continue;
            }

            if (st.st_size == 0) {
                //printf("File %s size is 0\n", fullPath);
                continue;
            }

            //printf("Adding file to list: %s\n", fullPath);  // DEBUG OUTPUT

            strncpy(fileNames[currentCount], fullPath, 256);
            currentCount++;

            if (currentCount >= MAX_FILES) {
                fprintf(stderr, "Too many files in directory: %zu\n", currentCount);
                closedir(dir);
                return currentCount;
            }
        } else if (entry->d_type == DT_DIR) {
            //printf("Entering directory: %s\n", fullPath);  // DEBUG OUTPUT
            currentCount = getFilesFromDirectoryRecursively(fullPath, fileNames, currentCount);
        }
    }

    closedir(dir);
    return currentCount;

}

size_t
getAllFiles(const char *directoryPath, char fileNames[][256]) {
    return getFilesFromDirectoryRecursively(directoryPath, fileNames, 0);
}

// Get file size
long
fsize(FILE *file) {
    long prevPos = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, prevPos, SEEK_SET);
    return size;
}


size_t
readDataFromMultipleFiles(FileState remainingFiles[], size_t *numRemainingFiles, unsigned char *buffer) {
    size_t totalBytesRead = 0;
    size_t blockIndex = 0;
    size_t currentFile = 0;

    while (blockIndex < SAMPLE_COUNT && currentFile < *numRemainingFiles) {
        FILE *file = fopen(remainingFiles[currentFile].fileName, "rb");
        if (!file) {
            perror("Failed to open file for reading");
            currentFile++;
            continue;
        }

        fseek(file, remainingFiles[currentFile].offset, SEEK_SET);
        size_t blocksReadFromCurrentFile = 0;

        while (blockIndex < SAMPLE_COUNT && blocksReadFromCurrentFile < SAMPLE_COUNT) {
            size_t bytesRead = fread(buffer + blockIndex * PAGE_SIZE, 1, PAGE_SIZE, file);
            if (bytesRead < PAGE_SIZE) {
                if (feof(file)) {
                    // Reached end of file
                    break;
                } else if (ferror(file)) {
                    perror("Error reading file");
                    fclose(file);
                    return totalBytesRead;
                }
            }

            totalBytesRead += bytesRead;
            blockIndex++;
            blocksReadFromCurrentFile++;
        }

        fclose(file);

        if (ftell(file) >= remainingFiles[currentFile].offset + blocksReadFromCurrentFile * PAGE_SIZE) {
            for (size_t i = currentFile; i < *numRemainingFiles - 1; ++i) {
                remainingFiles[i] = remainingFiles[i + 1];
            }
            (*numRemainingFiles)--;
        } else {
            remainingFiles[currentFile].offset += blocksReadFromCurrentFile * PAGE_SIZE;
            currentFile++;
        }
    }

    if (blockIndex < SAMPLE_COUNT) {
        int sample_count = SAMPLE_COUNT;
        fprintf(stderr, "Could not read %d blocks from file %s\n", sample_count, remainingFiles[currentFile].fileName);
        return 0;
    }

    return totalBytesRead;
}
