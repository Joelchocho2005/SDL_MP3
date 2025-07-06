#include "playlist_page.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

// Dimensiones de la imagen de fondo
#define IMG_WIDTH   1026
#define IMG_HEIGHT   652

typedef struct {
    const char* titulo;
    const char* artista;
    const char* album;
    const char* duracion;
} Cancion;

void mostrarPlaylist(SDL_Window* window, SDL_Renderer* renderer)
{
    // Carga de texturas y fuentes
    SDL_Texture* backgroundTexture = IMG_LoadTexture(renderer, "imgs/Playlist_page.png");
    SDL_Texture* albumTexture      = IMG_LoadTexture(renderer, "imgs/album2.jpg");
    TTF_Font* fontTitulo = TTF_OpenFont("src/assets/Inder.ttf", 60);
    TTF_Font* fontTexto  = TTF_OpenFont("src/assets/Inder.ttf", 16);

    if (!backgroundTexture || !albumTexture || !fontTitulo || !fontTexto) {
        printf("Error cargando recursos en playlist_page: %s\n", SDL_GetError());
        return;
    }

    // Canciones ejemplo
    // Cancion cancionesDemo[] = {
    //     {"Song 1", "Artist A", "Album X", "3:25"},
    //     {"Song 2", "Artist B", "Album Y", "4:10"},
    //     {"Song 3", "Artist C", "Album Z", "2:58"},
    // };
    Cancion* canciones   = NULL;
    int      numCanciones = 0;

    // Datos de cabecera
    const char* nombrePlaylist    = "Mi Playlist";     
    const char* autorPlaylist     = "Usuario";
    char ncancionesStr[8];
    snprintf(ncancionesStr, sizeof(ncancionesStr), "%d", numCanciones);

    // Colores
    SDL_Color fondoColor = {64, 65, 68, 255};
    SDL_Color blanco     = {255,255,255,255};
    SDL_Color grisClaro  = {200,200,200,255};
    SDL_Color textoNegro = { 20, 20, 20,255};
    SDL_Color hoverColor = { 58, 58, 59,255};

    #define RENDER_TEXTO(font, txt, x, y, col)                     \
        do {                                                       \
            SDL_Surface* s = TTF_RenderText_Solid(font, txt, col); \
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer,s); \
            SDL_Rect r={x,y,s->w,s->h};                            \
            SDL_RenderCopy(renderer,t,NULL,&r);                    \
            SDL_FreeSurface(s); SDL_DestroyTexture(t);             \
        } while(0)

    // Loop interno
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)          running = false;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX,&mouseY);

        SDL_SetRenderDrawColor(renderer,fondoColor.r,fondoColor.g,fondoColor.b,fondoColor.a);
        SDL_RenderClear(renderer);

        int winW, winH;
        SDL_GetWindowSize(window,&winW,&winH);
        SDL_Rect bg = {(winW-IMG_WIDTH)/2,(winH-IMG_HEIGHT)/2,IMG_WIDTH,IMG_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture,NULL,&bg);

        // Álbum
        SDL_Rect albumRect = {bg.x+75,bg.y+41,200,200};
        SDL_RenderCopy(renderer, albumTexture,NULL,&albumRect);

        // Cabecera
        RENDER_TEXTO(fontTitulo,nombrePlaylist, bg.x+307,bg.y+80 , blanco);
        RENDER_TEXTO(fontTexto ,autorPlaylist ,  bg.x+390,bg.y+152, blanco);
        RENDER_TEXTO(fontTexto ,ncancionesStr , bg.x+410,bg.y+183, blanco);

        // Lista de canciones
        int baseY = bg.y + 330;
        for (int i=0;i<numCanciones;++i){
            SDL_Rect fila = {bg.x+50, baseY+i*45, 925, 40};

            SDL_Point p={mouseX,mouseY};
            bool hover = SDL_PointInRect(&p,&fila);

            SDL_Color cf = hover?hoverColor:grisClaro;
            SDL_Color ct = hover?blanco:textoNegro;

            SDL_SetRenderDrawColor(renderer,cf.r,cf.g,cf.b,cf.a);
            SDL_RenderFillRect(renderer,&fila);

            char idx[4]; snprintf(idx,sizeof(idx),"%d",i+1);
            RENDER_TEXTO(fontTexto, idx                , fila.x+30 , fila.y+10, ct);
            RENDER_TEXTO(fontTexto, canciones[i].titulo, fila.x+70 , fila.y+10, ct);
            RENDER_TEXTO(fontTexto, canciones[i].artista,fila.x+320, fila.y+10, ct);
            RENDER_TEXTO(fontTexto, canciones[i].album , fila.x+560, fila.y+10, ct);
            RENDER_TEXTO(fontTexto, canciones[i].duracion,fila.x+830, fila.y+10, ct);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Liberar
    SDL_DestroyTexture(albumTexture);
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(fontTitulo);
    TTF_CloseFont(fontTexto);
}
