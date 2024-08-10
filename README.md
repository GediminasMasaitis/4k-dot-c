# 4k.c
A chess engine designed to fit into 4kB. ... Or at least it will be, hopefully.

The plan is to make a <= 4096 byte UCI engine executable, without:
* Any external dependencies (including libc)
* Compression
* Minification
* On-the-fly compilation

### Current specifications
Executable size:

* 64 bit: 3776 bytes

## Thanks
* **zamfofex** for helping with initial set up of a minimal Hello World executable and helping with 64 bit compilation
