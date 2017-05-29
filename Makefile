CFLAGS := -O

all: imgdata

imgdata: 16bit.o build.o extract.o main.o preview.o Makefile
	gcc $(CFLAGS) 16bit.o build.o extract.o main.o preview.o -o imgdata

main.o: main.c 16bit.h build.h extract.h preview.h Makefile
	gcc -c $(CFLAGS) main.c

%.o: %.c %.h Makefile
	gcc -c $(CFLAGS) $<

clean:
	rm -f imgdata
	rm -f *.o
