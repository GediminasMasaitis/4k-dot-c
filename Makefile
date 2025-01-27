ARCH ?= 64
EXE ?= ./build/4kc
CC := gcc
CFLAGS := -std=gnu2x -Wno-deprecated-declarations -Wno-format
LDFILE := 64bit.ld
LDFLAGS :=

ifeq ($(ARCH), 32)
	CFLAGS += -m32
	LDFILE = 32bit.ld
endif

ifeq ($(NOSTDLIB), true)
    CFLAGS += -DNOSTDLIB -ffreestanding -fno-pic -fno-builtin -fno-stack-protector -march=haswell -Oz
	LDFLAGS += -T $(LDFILE) -Map=$(EXE).map
else
	CFLAGS += -march=native -O3
endif

ifneq ($(MINI), true)
	CFLAGS += -DFULL
endif

ifeq ($(ASSERTS), true)
	CFLAGS += -DASSERTS
else
	CFLAGS += -DNDEBUG
endif

all:
	mkdir -p build
	$(CC) $(CFLAGS) -c 4k.c
	$(CC) $(LDFLAGS) -o $(EXE) 4k.o
	rm *.o
	ls -la $(EXE)
	md5sum $(EXE)

mini:
	mkdir -p build
	$(CC) $(CFLAGS) -c 4k.c
	ld $(LDFLAGS) -o $(EXE) 4k.o
	rm *.o
	ls -la $(EXE)
	md5sum $(EXE)

dump:
	$(CC) $(CFLAGS) -c 4k.c
	objdump -s ./4k.o

debug:
	mkdir -p build
	$(CC) -c -g 4k.c
	$(CC) -o $(EXE) 4k.o
	rm *.o
	ls -la ./4kc
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
