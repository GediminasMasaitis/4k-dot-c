ARCH ?= 64
CFLAGS := -nostdlib -fno-pic -fno-builtin -Os
LDFILE := 64bit.ld

ifeq ($(ARCH), 32)
	CFLAGS += -m32
	LDFILE = 32bit.ld
endif

all:
	mkdir -p build
	gcc $(CFLAGS) -c 4k.c
	strip -x 4k.o
	ld -T $(LDFILE) -o ./build/4k-dot-c 4k.o
	rm *.o
	ls -la ./build
	./build/4k-dot-c

format:
	dos2unix ./*.*
	clang-format -i ./4k.c

clean:
	rm -rf build
	rm -f *.o
	rm -f *.c.temp*