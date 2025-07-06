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

// Chicos para ejecutar o crear el .exe, basta con usar los comando para todas las librerias que usen, en este caso
gcc -I src/include -L src/lib -o reproductor version9-7.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
este comando hace que mi archivo version9-7c se convierta en un ejecutable llamado reproductor.exe, usando estas librerias:
-lSDL2main 
-lSDL2
-lSDL2_image 
-lSDL2_ttf
-lSDL2_mixer
y usa el motor que les enseñe a descargarse al inicio del proyecto
-lmingw32
-usamos el mingw32 uno porque es mas eficiente que l 64 para programas no tan complicados, y ademas necesitan saber que todas las librerias
del SDL2 estan configuradas en general para este motor de c.
