ARCH ?= 64
EXE ?= ./build/4kc
CFLAGS := -std=gnu2x -march=native -Wno-deprecated-declarations -Wno-format
LDFILE := 64bit.ld
LDFLAGS := 

ifeq ($(ARCH), 32)
	CFLAGS += -m32
	LDFILE = 32bit.ld
endif

ifeq ($(NOSTDLIB), true)
    CFLAGS += -ffreestanding -fno-pic -fno-builtin -fno-stack-protector
	LDFlags += -T $(LDFILE) -Map=$(EXE).map
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
	gcc $(CFLAGS) -c 4k.c
	gcc $(CFLAGS) -o $(EXE) 4k.o
	rm *.o
	ls -la $(EXE)
	md5sum $(EXE)

mini:
	mkdir -p build
	gcc $(CFLAGS) -c 4k.c
	ld $(LDFLAGS) -o $(EXE) 4k.o
	rm *.o
	ls -la $(EXE)
	md5sum $(EXE)

dump:
	gcc $(CFLAGS) -c 4k.c
	objdump -s ./4k.o

debug:
	mkdir -p build
	gcc -c -g 4k.c
	gcc -o $(EXE) 4k.o
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
