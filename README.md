# 4k.c
A chess engine designed to fit into 4kB.

The engine is a <= 4096 byte UCI engine executable, without:
* Any external dependencies (including libc)
* Minification
* On-the-fly compilation

### How to contribute

#### For general contributions

* To get the latest bench, run `make && ./build/4kc bench`
* If you have a potential idea, just PR it.
* All PRs are welcome, I will sort though them.
* No need to touch the changelog. I will amend the commit and you will get credited there (as well as the git history)

#### For size measurements
* You must have the *correct* `gcc` version on your system in your PATH. It is a version somewhere in-between 14.2 and 15.1. See below for installation details
* You must have `apultra` on your PATH. https://github.com/GediminasMasaitis/apultra. Just get the source, `make` it, and `sudo mv apultra ./usr/local/bin`.
* Run `make NOSTDLIB=true MINI=true loader`, you will see the binary size in the console output


### How to install the exact gcc version:
```bash
sudo apt install build-essential
sudo apt install libmpfr-dev libgmp3-dev libmpc-dev -y

wget -O gcc-dev.zip https://github.com/GediminasMasaitis/gcc/archive/refs/heads/master.zip
unzip gcc-dev.zip
cd gcc-dev

./configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=/usr/local/gcc-dev --enable-checking=release --enable-languages=c --disable-multilib --program-suffix=-dev
make -j 4
sudo make install

sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/gcc-dev/bin/gcc-dev 99
```

Change `-j 4` to your core count for faster building

## Thanks
* **zamfofex** for helping with initial set up of a minimal Hello World executable and helping with 64 bit compilation, reducing code size
* **sqrmax** for helping optimize the loader
