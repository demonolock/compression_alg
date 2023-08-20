CC=gcc
CFLAGS=-I. # -g
LIBS=-lzstd -llz4 -lm -ldeflate -lz

build:
	$(CC) $(CFLAGS) compr_alg.c zstd_alg.c zlib_alg.c lz4_alg.c libdeflate_alg.c main.c $(LIBS) -o compr_alg

install:
	sudo apt update
	sudo apt install libzstd-dev -y
	sudo apt install liblz4-dev -y
	sudo apt install libdeflate-dev -y
	sudo apt install zlib1g-dev -y

clean:
	rm -f *.o compr_alg
