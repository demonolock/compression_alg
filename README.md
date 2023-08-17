# Compression Project

This project provides a platform for testing and benchmarking compression algorithms using trained dictionaries.

## Features

- Training of compression dictionaries.
- Compression and decompression of files using trained dictionaries.
- Measurement of the execution time for dictionary training, compression, and decompression.
- Reporting of average, minimum, and maximum performance metrics and compression ratios.

## Getting Started

### Prerequisites

- Ensure you have a C compiler, preferably GCC or Clang.
- Install the ZSTD, LZ4, libdeflate libraries. You can use `make install` if you use Linux.

### Compilation

```bash
make clean build
```

### Usage

After compilation, run the program using:

```bash
./compr_alg
```

The program will search for data samples in the `./data` directory, train a dictionary using these samples, and then compress and decompress each file using the trained dictionary.

## Output

The program provides a comprehensive report on the compression and decompression processes, including:

- Time taken for dictionary training, compression, and decompression.
- Compression ratio statistics.
- Any discrepancies between the original and decompressed files.

```bash
+----------------------------------+--------------+--------------+--------------+
| Metric                           | Average      | Min          | Max          |
+----------------------------------+--------------+--------------+--------------+
| Dictionary training time (s)     |     0.000624 |     0.005911 |     0.017405 |
| Compression time (s)             |     0.000018 |     0.000007 |     0.000062 |
| Decompression time (s)           |     0.000012 |     0.000009 |     0.000036 |
| Compression ratio                |     0.101090 |     0.021484 |     1.013672 |
+----------------------------------+--------------+--------------+--------------+
```