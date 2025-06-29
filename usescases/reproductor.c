#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_RUTA 256

typedef struct Nodo {
    char ruta[MAX_RUTA];
    struct Nodo *anterior;
    struct Nodo *siguiente;
} Nodo;
typedef enum {
    NINGUNO,
    BTN_ANTERIOR,
    BTN_PAUSA,
    BTN_SIGUIENTE
} Boton;

Nodo* cargarCanciones(const char* archivo) {
    FILE* f = fopen(archivo, "r");
    if (!f) {
        printf("No se pudo abrir el archivo: %s\n", archivo);
        return NULL;
    }

    Nodo *primero = NULL, *actual = NULL;
    char linea[MAX_RUTA];
    printf("\nLista de Canciones agregadas\n");
    while (fgets(linea, sizeof(linea), f)) {
        // Eliminar salto de línea
        linea[strcspn(linea, "\r\n")] = 0;
        printf("%s\n",linea);
        Nodo* nuevo = malloc(sizeof(Nodo));
        strcpy(nuevo->ruta, linea);
        nuevo->anterior = actual;
        nuevo->siguiente = NULL;
        
        if (actual)
            actual->siguiente = nuevo;
        else
            primero = nuevo;

        actual = nuevo;
    }
    printf("------------------------------\n");
    fclose(f);
    return primero;
}

void liberarCanciones(Nodo* inicio) {
    while (inicio) {
        Nodo* temp = inicio;
        inicio = inicio->siguiente;
        free(temp);
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        printf("Error SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error Mix_OpenAudio: %s\n", Mix_GetError());
        return 1;
    }

    Nodo* actual = cargarCanciones("data/canciones.txt");
    if (!actual) return 1;

    SDL_Window *window = SDL_CreateWindow("Reproductor MP3",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 400, SDL_WINDOW_SHOWN);
    SDL_Surface *screen = SDL_GetWindowSurface(window);

    SDL_Rect btnAnterior = {100, 150, 100, 50};
    SDL_Rect btnPausa = {300, 150, 100, 50};
    SDL_Rect btnSiguiente = {500, 150, 100, 50};
    SDL_FillRect(screen, &btnAnterior, SDL_MapRGB(screen->format, 150, 150, 255));   // Azul
    SDL_FillRect(screen, &btnPausa, SDL_MapRGB(screen->format, 180, 180, 180));      // Gris
    SDL_FillRect(screen, &btnSiguiente, SDL_MapRGB(screen->format, 150, 255, 150));  // Verde
    SDL_UpdateWindowSurface(window);
    
    Mix_Music *musica = Mix_LoadMUS(actual->ruta);
    printf("Cargando: %s\n", actual->ruta);

    if (!musica) {
        printf("Error al cargar %s: %s\n", actual->ruta, Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(musica, -1);

    bool ejecutando = true;
    bool pausado = false;
    SDL_Event evento;

    while (ejecutando) {
        while (SDL_PollEvent(&evento)) {
    if (evento.type == SDL_QUIT) {
        ejecutando = false;
    } else if (evento.type == SDL_MOUSEBUTTONDOWN) {
        int mx = evento.button.x;
        int my = evento.button.y;

        Boton botonPresionado = NINGUNO;

        if (mx >= btnPausa.x && mx <= btnPausa.x + btnPausa.w &&
            my >= btnPausa.y && my <= btnPausa.y + btnPausa.h) {
            botonPresionado = BTN_PAUSA;
        } else if (mx >= btnSiguiente.x && mx <= btnSiguiente.x + btnSiguiente.w &&
                   my >= btnSiguiente.y && my <= btnSiguiente.y + btnSiguiente.h) {
            botonPresionado = BTN_SIGUIENTE;
        } else if (mx >= btnAnterior.x && mx <= btnAnterior.x + btnAnterior.w &&
                   my >= btnAnterior.y && my <= btnAnterior.y + btnAnterior.h) {
            botonPresionado = BTN_ANTERIOR;
        }

        switch (botonPresionado) {
            case BTN_PAUSA:
                if (pausado) {
                    Mix_ResumeMusic();
                    printf("->Reanudado\n");
                } else {
                    Mix_PauseMusic();
                    printf("->Pausado\n");
                }
                pausado = !pausado;
                break;

            case BTN_SIGUIENTE:
                if (actual->siguiente) {
                    Mix_HaltMusic();
                    Mix_FreeMusic(musica);
                    actual = actual->siguiente;
                    musica = Mix_LoadMUS(actual->ruta);
                    printf("Cargando: %s\n", actual->ruta);
                    if (musica) Mix_PlayMusic(musica, -1);
                    else printf("Error al cargar %s: %s\n", actual->ruta, Mix_GetError());
                    pausado = false;
                }
                break;

            case BTN_ANTERIOR:
                if (actual->anterior) {
                    Mix_HaltMusic();
                    Mix_FreeMusic(musica);
                    actual = actual->anterior;
                    musica = Mix_LoadMUS(actual->ruta);
                    printf("Cargando: %s\n", actual->ruta);
                    if (musica) Mix_PlayMusic(musica, -1);
                    else printf("Error al cargar %s: %s\n", actual->ruta, Mix_GetError());
                    pausado = false;
                }
                break;

            default:
                break;
        }
    }
}
}

    Mix_HaltMusic();
    Mix_FreeMusic(musica);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    liberarCanciones(actual);
    return 0;
}
