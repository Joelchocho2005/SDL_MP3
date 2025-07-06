#include "playlist_page.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

// RENDER_TEXTO: Macro para evitar repetir lógica al dibujar texto
#define RENDER_TEXTO(font, txt, x, y, col)                         \
    do {                                                           \
        SDL_Surface* s = TTF_RenderText_Solid(font, txt, col);     \
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);\
        SDL_Rect r = {x, y, s->w, s->h};                            \
        SDL_RenderCopy(renderer, t, NULL, &r);                     \
        SDL_FreeSurface(s); SDL_DestroyTexture(t);                 \
    } while(0)

// Variables globales definidas en .h
SDL_Texture* backgroundTexture = NULL;
SDL_Texture* albumTexture = NULL;
TTF_Font* fontTitulo = NULL;
TTF_Font* fontAutor = NULL;
TTF_Font* fontNcanciones = NULL;
Cancion* canciones = NULL;
int numCanciones = 0;

bool inicializarPlaylist(SDL_Renderer* ren) {
    renderer = ren;

    backgroundTexture = IMG_LoadTexture(renderer, "imgs/Playlist_page.png");
    albumTexture      = IMG_LoadTexture(renderer, "imgs/album2.jpg");

    fontTitulo     = TTF_OpenFont("src/assets/Inder.ttf", 60);
    fontAutor      = TTF_OpenFont("src/assets/Inder.ttf", 16);
    fontNcanciones = TTF_OpenFont("src/assets/Inder.ttf", 16);

    if (!backgroundTexture || !albumTexture || !fontTitulo || !fontAutor || !fontNcanciones) {
        printf("Error cargando recursos playlist: %s\n", SDL_GetError());
        return false;
    }

    // EJEMLO DE CANCIONES
    // canciones = malloc(sizeof(Cancion) * 3);
    // canciones[0] = (Cancion){"Canción 1", "Artista A", "Álbum X", "3:25"};
    // canciones[1] = (Cancion){"Canción 2", "Artista B", "Álbum Y", "4:10"};
    // canciones[2] = (Cancion){"Canción 3", "Artista C", "Álbum Z", "2:58"};
    // numCanciones = 3;

    return true;
}

void renderizarPlaylist(const char* nombrePlaylist, const char* autor, const char* totalStr) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    SDL_Color fondoColor = {64, 65, 68, 255};
    SDL_Color blanco     = {255,255,255,255};
    SDL_Color grisClaro  = {200,200,200,255};
    SDL_Color textoNegro = {20, 20, 20,255};
    SDL_Color hoverColor = {58, 58, 59,255};

    SDL_SetRenderDrawColor(renderer, fondoColor.r, fondoColor.g, fondoColor.b, fondoColor.a);
    SDL_RenderClear(renderer);

    int winW, winH;
    SDL_GetWindowSize(SDL_GetWindowFromID(1), &winW, &winH);

    SDL_Rect bg = {(winW-IMG_WIDTH)/2, (winH-IMG_HEIGHT)/2, IMG_WIDTH, IMG_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &bg);

    // Imagen del álbum
    SDL_Rect albumRect = {bg.x+75, bg.y+41, 200, 200};
    SDL_RenderCopy(renderer, albumTexture, NULL, &albumRect);

    // Texto cabecera
    RENDER_TEXTO(fontTitulo, nombrePlaylist, bg.x+307, bg.y+80 , blanco);
    RENDER_TEXTO(fontAutor,  autor,          bg.x+390, bg.y+152, blanco);
    RENDER_TEXTO(fontNcanciones, totalStr,   bg.x+410, bg.y+183, blanco);

    // Canciones
    int baseY = bg.y + 330;
    for (int i=0; i<numCanciones; ++i) {
        SDL_Rect fila = {bg.x+50, baseY+i*45, 925, 40};
        SDL_Point p = {mouseX, mouseY};
        bool hover = SDL_PointInRect(&p, &fila);

        SDL_Color cf = hover ? hoverColor : grisClaro;
        SDL_Color ct = hover ? blanco     : textoNegro;

        SDL_SetRenderDrawColor(renderer, cf.r, cf.g, cf.b, cf.a);
        SDL_RenderFillRect(renderer, &fila);

        char idx[4]; snprintf(idx, sizeof(idx), "%d", i + 1);
        RENDER_TEXTO(fontAutor, idx,                 fila.x+30,  fila.y+10, ct);
        RENDER_TEXTO(fontAutor, canciones[i].titulo, fila.x+70,  fila.y+10, ct);
        RENDER_TEXTO(fontAutor, canciones[i].artista,fila.x+320, fila.y+10, ct);
        RENDER_TEXTO(fontAutor, canciones[i].album,  fila.x+560, fila.y+10, ct);
        RENDER_TEXTO(fontAutor, canciones[i].duracion,fila.x+830,fila.y+10, ct);
    }

    SDL_RenderPresent(renderer);
}

void destruirPlaylist() {
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (albumTexture) SDL_DestroyTexture(albumTexture);
    if (fontTitulo) TTF_CloseFont(fontTitulo);
    if (fontAutor) TTF_CloseFont(fontAutor);
    if (fontNcanciones) TTF_CloseFont(fontNcanciones);
    backgroundTexture = albumTexture = NULL;
    fontTitulo = fontAutor = fontNcanciones = NULL;
    
    canciones = NULL;
    numCanciones = 0;
}
