all: Dictionary

Dictionary: code/main.c
	gcc `pkg-config --cflags gtk+-3.0` -o Dictionary code/main.c `pkg-config --libs gtk+-3.0` -w code/inc/libbt.a code/inc/libfdr.a -rdynamic

clean:
	rm -f *.o Dictionary
