gcc -c -fPIC ./ChessEngine/chess_game.c -o ./ChessEngine/chess_game.o
gcc ./ChessEngine/chess_game.o -shared -o ./chess_game.so