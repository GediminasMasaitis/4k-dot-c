all:
	mkdir -p build
	gcc -m32 -nostdlib -fno-pic -fno-builtin -Os -c 4k.c
	strip -x 4k.o
	ld -T tiny.ld -o ./build/4k-dot-c 4k.o
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
