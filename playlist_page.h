#ifndef PLAYLIST_PAGE_H
#define PLAYLIST_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Dimensiones de la interfaz de la playlist
#define IMG_WIDTH 1026
#define IMG_HEIGHT 652

// Estructura de una canción
typedef struct {
    const char* titulo;
    const char* artista;
    const char* album;
    const char* duracion;
} Cancion;

// Variables globales usadas por la vista playlist
extern SDL_Window*  window;
extern SDL_Renderer* renderer;
extern TTF_Font* fontTitulo;
extern TTF_Font* fontAutor;
extern TTF_Font* fontNcanciones;
extern SDL_Texture* backgroundTexture;
extern SDL_Texture* albumTexture;

extern Cancion* canciones; // Lista de canciones que se muestran
extern int numCanciones; // Número de canciones en la playlist

// Macro para renderizar texto en pantalla
#define RENDER_TEXTO(font, txt, x, y, col)                     \
    do {                                                       \
        SDL_Surface* s = TTF_RenderText_Solid(font, txt, col); \
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer,s); \
        SDL_Rect r={x,y,s->w,s->h};                            \
        SDL_RenderCopy(renderer,t,NULL,&r);                    \
        SDL_FreeSurface(s); SDL_DestroyTexture(t);             \
    } while(0)

// Inicializa los recursos necesarios de la playlist
bool inicializarPlaylist(SDL_Renderer* ren);

// Renderiza la pantalla de la playlist
void renderizarPlaylist(const char* nombre, const char* autor, const char* nCanciones);

// Libera todos los recursos usados por la playlist
void destruirPlaylist();

#endif // PLAYLIST_PAGE_H
