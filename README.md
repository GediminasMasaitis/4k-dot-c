# 4k.c
A chess engine designed to fit into 4kB.

The engine is a <= 4096 byte UCI engine executable, without:
* Any external dependencies (including libc)
* Compression
* Minification
* On-the-fly compilation

### Current specifications
32 bit: 5264 bytes
64 bit: 3967 bytes

Perft: 16843009 nps

### How to install gcc 14.2:
```bash
sudo apt install build-essential
sudo apt install libmpfr-dev libgmp3-dev libmpc-dev -y
wget http://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz
tar -xf gcc-14.2.0.tar.gz
cd gcc-14.2.0
./configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=/usr/local/gcc-14.2.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib --program-suffix=-14.2.0
make -j 4
sudo make install

sudo update-alternatives --install /usr/bin/g++ g++ /usr/local/gcc-14.2.0/bin/g++-14.2.0 14
sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/gcc-14.2.0/bin/gcc-14.2.0 14
```

Change `-j 4` to your core count for faster building

## Thanks
* **zamfofex** for helping with initial set up of a minimal Hello World executable and helping with 64 bit compilation
