ARCH ?= 64
CC := gcc
CFLAGS := -std=gnu2x -Wno-deprecated-declarations -Wno-format
LDFLAGS :=
NOSTDLIBLDFLAGS :=

ifeq ($(OS),Windows_NT)
    ifdef EXE
        ifeq ($(suffix $(EXE)),)
            override EXE := $(EXE).exe
        endif
    else
        EXE := .\build\4kc.exe
    endif
    MKDIR = if not exist build mkdir build
    LS = dir
    MD5 = certutil -hashfile $(EXE) MD5
    MAP_CHECK = if exist $(EXE).map findstr fill $(EXE).map
else
    EXE ?= ./build/4kc
    MKDIR = mkdir -p build
    LS = ls -la
    MD5 = md5sum $(EXE)
    MAP_CHECK = if [ -f $(EXE).map ]; then grep fill $(EXE).map || true; fi
endif

ifeq ($(NOSTDLIB), true)
    CFLAGS += -DNOSTDLIB -nostdlib -fno-pic -fno-builtin -fno-stack-protector -fno-schedule-insns2 -march=haswell -Oz
	LDFLAGS += -nostdlib -Wl,-Map=$(EXE).map
	NOSTDLIBLDFLAGS += -Wl,-T 64bit.ld
else
	CFLAGS += -march=native -static -O3
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
	$(MKDIR)
	$(CC) $(CFLAGS) -c 4k.c
	$(CC) $(LDFLAGS) $(NOSTDLIBLDFLAGS) -o $(EXE) 4k.o
	$(LS) $(EXE)
	@$(MAP_CHECK)
	$(MD5)

compressor:
	$(CC) -march=native -O3 -std=gnu2x -lpthread -lm -o compressor compressor.c

compress: compressor
	$(MKDIR)
	$(CC) $(CFLAGS) -S -masm=intel -o 4k.s 4k.c
	$(CC) $(CFLAGS) -c -o 4k.o 4k.s
	$(CC) $(LDFLAGS) -Wl,-Map=$(EXE).map -Wl,-T 64bit-noheader.ld -o $(EXE) 4k.o
	$(LS) $(EXE)
	@$(MAP_CHECK)
	./compressor -m slow -o $(EXE).4kc $(EXE)

loader: compress
	nasm -f bin -DSTART_LOCATION=$$(grep '_start' $(EXE).map | awk '{print $$1}') -o $(EXE) loader.asm
	$(LS) $(EXE)
	$(MD5)

pgo:
	$(MKDIR)
	$(CC) $(CFLAGS) -fprofile-generate -ftest-coverage -fprofile-update=atomic -c 4k.c
	$(CC) $(LDFLAGS) -fprofile-generate -o $(EXE) 4k.o
	$(EXE) bench
	$(CC) $(CFLAGS) -fprofile-use -fprofile-correction -c 4k.c
	$(CC) $(LDFLAGS) -fprofile-use -o $(EXE) 4k.o
	rm *.o
	$(LS) $(EXE)
	$(MD5)

format:
	dos2unix ./*.asm
	dos2unix ./*.c
	dos2unix ./*.ld
	clang-format -i ./4k.c
	clang-format -i ./compressor.c

clean:
	rm -rf build/**
	rm -f *.map
	rm -f *.s
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.c.temp*
	rm -f compressor
