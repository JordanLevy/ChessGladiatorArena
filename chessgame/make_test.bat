gcc -c -fPIC test.c -o test.o
gcc test.o -shared -o test.so