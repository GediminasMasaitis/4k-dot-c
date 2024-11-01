ARCH ?= 64
CFLAGS := -march=haswell -nostdlib -fno-pic -fno-builtin -fno-stack-protector -Oz
LDFILE := 64bit.ld

ifeq ($(ARCH), 32)
	CFLAGS += -m32
	LDFILE = 32bit.ld
endif

ifeq ($(FULL), true)
	CFLAGS += -DFULL
endif

ifeq ($(ASSERTS), true)
	CFLAGS += -DASSERTS
endif

all:
	mkdir -p build
	gcc $(CFLAGS) -c 4k.c
	ld -T $(LDFILE) -Map=./build/4k-dot-c.map -o./build/4k-dot-c 4k.o
	rm *.o
	ls -la ./build
	md5sum ./build/4k-dot-c
	./build/4k-dot-c

dump:
	gcc $(CFLAGS) -c 4k.c
	objdump -s ./4k.o

debug:
	mkdir -p build
	gcc -c -g 4k.c
	gcc -o ./build/4k-dot-c 4k.o
	rm *.o
	ls -la ./build
	md5sum ./build/4k-dot-c
	./build/4k-dot-c

format:
	dos2unix ./*.*
	clang-format -i ./4k.c

clean:
	rm -rf build
	rm -f *.map
	rm -f *.o
	rm -f *.c.temp*
