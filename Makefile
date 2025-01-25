ARCH ?= 64
EXE ?= ./build/4kc
CFLAGS := -std=gnu23 -march=haswell -nostdlib -fno-pic -fno-builtin -fno-stack-protector -Oz
LDFILE := 64bit.ld

ifeq ($(ARCH), 32)
	CFLAGS += -m32
	LDFILE = 32bit.ld
endif

ifneq ($(MINI), true)
	CFLAGS += -DFULL
endif

ifeq ($(ASSERTS), true)
	CFLAGS += -DASSERTS
endif

all:
	mkdir -p build
	gcc $(CFLAGS) -c 4k.c
	ld -T $(LDFILE) -Map=$(EXE).map -o$(EXE) 4k.o
	rm *.o
	ls -la ./
	md5sum $(EXE)

dump:
	gcc $(CFLAGS) -c 4k.c
	objdump -s ./4k.o

debug:
	mkdir -p build
	gcc -c -g 4k.c
	gcc -o $(EXE) 4k.o
	rm *.o
	ls -la ./
	md5sum $(EXE)
	./$(EXE)

format:
	dos2unix ./*.c
	dos2unix ./*.ld
	clang-format -i ./4k.c

clean:
	rm -rf build
	rm -f *.map
	rm -f *.o
	rm -f *.c.temp*
