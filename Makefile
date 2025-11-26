ARCH ?= 64
EXE ?= ./build/4kc
CC := gcc
CFLAGS := -std=gnu2x -Wno-deprecated-declarations -Wno-format
LDFLAGS :=
NOSTDLIBLDFLAGS :=

ifeq ($(NOSTDLIB), true)
    CFLAGS += -DNOSTDLIB -nostdlib -fno-pic -fno-builtin -fno-stack-protector -march=haswell -Oz
	LDFLAGS += -nostdlib -Wl,-Map=$(EXE).map
	NOSTDLIBLDFLAGS += -Wl,-T 64bit.ld
else
	CFLAGS += -march=native -static -Oz
endif

ifneq ($(MINI), true)
	CFLAGS += -DFULL
endif

ifeq ($(LOWSTACK), true)
	CFLAGS += -DLOWSTACK
endif

ifeq ($(ASSERTS), true)
	CFLAGS += -DASSERTS
else
	CFLAGS += -DNDEBUG
endif

all:
	mkdir -p build
	$(CC) $(CFLAGS) -c 4k.c
	$(CC) $(LDFLAGS) $(NOSTDLIBLDFLAGS) -o $(EXE) 4k.o
	ls -la $(EXE)
	@if [ -f $(EXE).map ]; then grep fill $(EXE).map || true; fi
	md5sum $(EXE)

.PHONY: compress loader

compress:
	mkdir -p build
	$(CC) $(CFLAGS) -S -masm=intel -o 4k.s 4k.c
	$(CC) $(CFLAGS) -c -o 4k.o 4k.s
	$(CC) $(LDFLAGS) -Wl,-T 64bit-noheader.ld -o $(EXE) 4k.o
	ls -la $(EXE)
	@if [ -f $(EXE).map ]; then grep fill $(EXE).map || true; fi
	apultra -stats -v $(EXE) $(EXE).ap

loader: compress
	nasm -felf64 -DSTART_LOCATION=$$(grep '_start' $(EXE).map | awk '{print $$1}') loader.asm -o loader.o
	$(CC) -nostdlib -Wl,-T 64bit-loader.ld -Wl,-Map=./build/loader.map -o $(EXE) loader.o
	ls -la $(EXE)
	md5sum $(EXE)

win:
	if not exist build mkdir build
	$(CC) $(CFLAGS) -o $(EXE) 4k.c

pgo:
	mkdir -p build
	$(CC) $(CFLAGS) -fprofile-generate -ftest-coverage -fprofile-update=atomic -c 4k.c
	$(CC) $(LDFLAGS) -fprofile-generate -o $(EXE) 4k.o
	$(EXE) bench
	$(CC) $(CFLAGS) -fprofile-use -fprofile-correction -c 4k.c
	$(CC) $(LDFLAGS) -fprofile-use -o $(EXE) 4k.o
	rm *.o
	ls -la $(EXE)
	md5sum $(EXE)

format:
	dos2unix ./*.asm
	dos2unix ./*.c
	dos2unix ./*.ld
	clang-format -i ./4k.c

clean:
	rm -rf build/**
	rm -f *.map
	rm -f *.s
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.c.temp*
