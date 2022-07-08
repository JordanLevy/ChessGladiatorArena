gcc -c -fPIC chess_game.c -o chess_game.o
gcc chess_game.o -shared -o chess_game.so