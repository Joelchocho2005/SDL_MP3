#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

#define IMG_WIDTH 1026
#define IMG_HEIGHT 652

// SDL objetos globales
SDL_Window*  window;
SDL_Renderer* renderer;
TTF_Font* fontTitulo;
TTF_Font* fontAutor;
TTF_Font* fontNcanciones;
SDL_Texture* backgroundTexture;
SDL_Texture* albumTexture;

// Estructura de canción
typedef struct {
    const char* titulo;
    const char* artista;
    const char* album;
    const char* duracion;
} Cancion;

// Lista de canciones
Cancion* canciones   = NULL; // Se llenará desde lógica
int numCanciones = 0;

// Cargar recursos (fondos, fuentes, imágenes)
bool cargarRecursos() {
    backgroundTexture = IMG_LoadTexture(renderer, "imgs/Playlist_page.png");
    if (!backgroundTexture) { printf("Error fondo: %s\n", IMG_GetError()); return false; }

    albumTexture = IMG_LoadTexture(renderer, "imgs/album2.jpg");
    if (!albumTexture) { printf("Error álbum: %s\n", IMG_GetError()); return false; }

    fontTitulo     = TTF_OpenFont("src/assets/Inder.ttf", 60);
    fontAutor      = TTF_OpenFont("src/assets/Inder.ttf", 16);
    fontNcanciones = TTF_OpenFont("src/assets/Inder.ttf", 16);

    if (!fontTitulo || !fontAutor || !fontNcanciones) {
        printf("Error fuentes: %s\n", TTF_GetError());
        return false;
    }
    return true;
}

void renderizarTexto(TTF_Font* f, const char* txt, int x, int y, SDL_Color c) {
    SDL_Surface* s = TTF_RenderText_Solid(f, txt, c);
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_Rect r     = {x, y, s->w, s->h};
    SDL_RenderCopy(renderer, t, NULL, &r);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || !IMG_Init(IMG_INIT_PNG) || TTF_Init() < 0) {
        printf("Error SDL init: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Playlist", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              IMG_WIDTH, IMG_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!cargarRecursos()) return 1;

    // Colores
    SDL_Color fondoColor = {64, 65, 68, 255};
    SDL_Color blanco     = {255, 255, 255, 255};
    SDL_Color grisClaro  = {200, 200, 200, 255};
    SDL_Color textoNegro = {20, 20, 20, 255};
    SDL_Color hoverColor = {58, 58, 59, 255};

    // Placeholders de cabecera
    const char* nombrePlaylist    = "";
    const char* autorPlaylist     = "";
    const char* ncancionesPlaylist = "";

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        // Posición mouse
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Fondo
        SDL_SetRenderDrawColor(renderer, fondoColor.r, fondoColor.g, fondoColor.b, fondoColor.a);
        SDL_RenderClear(renderer);

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        SDL_Rect bgRect = {(winW - IMG_WIDTH) / 2, (winH - IMG_HEIGHT) / 2, IMG_WIDTH, IMG_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        // Álbum
        SDL_Rect albumRect = {bgRect.x + 75, bgRect.y + 41, 200, 200};
        SDL_RenderCopy(renderer, albumTexture, NULL, &albumRect);

        // Cabecera
        renderizarTexto(fontTitulo,     nombrePlaylist,      bgRect.x + 307, bgRect.y + 80,  blanco);
        renderizarTexto(fontAutor,      autorPlaylist,       bgRect.x + 390, bgRect.y + 152, blanco);
        renderizarTexto(fontNcanciones, ncancionesPlaylist,  bgRect.x + 410, bgRect.y + 183, blanco);

        // Fila dinámica de canciones
        int baseY = bgRect.y + 330;
        for (int i = 0; i < numCanciones; ++i) {
            SDL_Rect fila = {bgRect.x + 50, baseY + i * 45, 925, 40};

            SDL_Point m = {mouseX, mouseY};
            bool hover  = SDL_PointInRect(&m, &fila);
            SDL_Color cf = hover ? hoverColor : grisClaro;
            SDL_Color ct = hover ? blanco : textoNegro;

            SDL_SetRenderDrawColor(renderer, cf.r, cf.g, cf.b, cf.a);
            SDL_RenderFillRect(renderer, &fila);

            char idx[4]; snprintf(idx, sizeof(idx), "%d", i + 1);

            renderizarTexto(fontAutor, idx,                 fila.x + 30,  fila.y + 10, ct);
            renderizarTexto(fontAutor, canciones[i].titulo, fila.x + 70,  fila.y + 10, ct);
            renderizarTexto(fontAutor, canciones[i].artista,fila.x + 320, fila.y + 10, ct);
            renderizarTexto(fontAutor, canciones[i].album,  fila.x + 560, fila.y + 10, ct);
            renderizarTexto(fontAutor, canciones[i].duracion,fila.x + 830,fila.y + 10, ct);
        }

        SDL_RenderPresent(renderer);
    }

    // Limpieza
    SDL_DestroyTexture(albumTexture);
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(fontTitulo);
    TTF_CloseFont(fontAutor);
    TTF_CloseFont(fontNcanciones);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
