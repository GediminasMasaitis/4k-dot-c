# 4k.c
A chess engine designed to fit into 4kB.

The engine is a <= 4096 byte UCI engine executable, without:
* Any external dependencies (including libc)
* Compression
* Minification
* On-the-fly compilation

### Current specifications
64 bit: 3966 bytes

Perft: 16843009 nps

## Thanks
* **zamfofex** for helping with initial set up of a minimal Hello World executable and helping with 64 bit compilation
