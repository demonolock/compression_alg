CC=gcc
CFLAGS=-I. # -g
LIBS=-lzstd -llz4 -lm # -ldeflate

build:
	$(CC) $(CFLAGS) zstd_alg.c lz4_alg.c compr_alg.c main.c $(LIBS) -o compr_alg

install:
	sudo apt update
	sudo apt install libzstd-dev -y
	sudo apt install liblz4-dev -y

install-libdeflate:
	rm -rf libdeflate
	git clone https://github.com/ebiggers/libdeflate.git
	cd libdeflate
	make
	sudo make install

clean:
	rm -f *.o compr_alg
