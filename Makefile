/*PARA CPP*/
all:
	g++ -I src/include -L src/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2
/*PARA C*/
all:
	gcc -I src/include -L src/lib -o main main.c -lmingw32 -lSDL2main -lSDL2
/*CON SDL IMAGE*/
all:
	gcc -I src/include -L src/lib -o main main.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
/*CON SDL MIXER*/
all:
gcc -I src/include -L src/lib mp3.c -o mp3 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer
// COMANDO DE EJECUCION PROGRAMA EN GENERAL
gcc -I src/include -L src/lib -o reproductor reproductor.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
