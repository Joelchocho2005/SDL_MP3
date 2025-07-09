#include "playlist_page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PlaylistPage page;

// Función auxiliar para renderizar texto
void renderizarTexto(TTF_Font* fuente, const char* texto, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(fuente, texto, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindowFromID(1)), surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindowFromID(1)), texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Inicializa la playlist
void Playlist_Init(SDL_Renderer* renderer, const char* name, const char* author) {
    // Configuración de colores
    page.colors[0] = (SDL_Color){64, 65, 68, 255}; // Fondo
    page.colors[1] = (SDL_Color){255, 255, 255, 255}; // Blanco
    page.colors[2] = (SDL_Color){200, 200, 200, 255}; // Gris
    page.colors[3] = (SDL_Color){20, 20, 20, 255}; // Texto
    page.colors[4] = (SDL_Color){58, 58, 59, 255}; // Hover

    // Carga de recursos
    page.background = IMG_LoadTexture(renderer, "imgs/Playlist_page.png");
    page.albumCover = IMG_LoadTexture(renderer, "imgs/album2.jpg");
    page.fontTitle = TTF_OpenFont("src/assets/Inder.ttf", 60);
    page.fontDetails = TTF_OpenFont("src/assets/Inder.ttf", 16);

    // Inicialización de datos
    page.playlistName = name;
    page.playlistAuthor = author;
    page.totalCanciones = 0;
    page.capacidad = 10;
    page.canciones = (cancionInfo*)malloc(page.capacidad * sizeof(cancionInfo));
    page.hoveredItem = -1;

    // Botones
    page.backButton = (SDL_Rect){30, 10, 40, 40};
    page.showNextButton = false;
}

// Agrega una canción desde otras ventanas
void Playlist_AgregarCancion(PlaylistPage* page, const cancionInfo* cancion) {
    if (page->totalCanciones >= page->capacidad) {
        page->capacidad *= 2;
        page->canciones = (cancionInfo*)realloc(page->canciones, page->capacidad * sizeof(cancionInfo));
    }
    page->canciones[page->totalCanciones] = *cancion;
    page->totalCanciones++;
    page->showNextButton = (page->totalCanciones > 5);
    if (page->showNextButton) {
        page->nextButton = (SDL_Rect){50 + 925 - 150, 330 + (5 * 45) + 10, 150, 30};
    }
}

// Renderiza la playlist con todos los campos
void Playlist_Render(SDL_Renderer* renderer, PlaylistPage* page) {
    // Fondo
    SDL_SetRenderDrawColor(renderer, page->colors[0].r, page->colors[0].g, page->colors[0].b, page->colors[0].a);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, page->background, NULL, NULL);

    // Portada del álbum
    SDL_Rect coverRect = {75, 41, 200, 200};
    SDL_RenderCopy(renderer, page->albumCover, NULL, &coverRect);

    // Título de la playlist
    SDL_Surface* titleSurf = TTF_RenderText_Solid(page->fontTitle, page->playlistName, page->colors[1]);
    SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
    SDL_Rect titleRect = {307, 80, titleSurf->w, titleSurf->h};
    SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);

    // Renderizar canciones
    int renderCount = page->showNextButton ? 5 : page->totalCanciones;
    for (int i = 0; i < renderCount; i++) {
        SDL_Color bgColor = (i == page->hoveredItem) ? page->colors[4] : page->colors[2];
        SDL_Color textColor = (i == page->hoveredItem) ? page->colors[1] : page->colors[3];

        // Fondo del ítem de canción
        SDL_Rect songRect = {50, 330 + (i * 45), 925, 40};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &songRect);

        // Renderizar todos los campos
        char indexStr[4];
        snprintf(indexStr, sizeof(indexStr), "%d", i + 1);
        renderizarTexto(page->fontDetails, indexStr, songRect.x + 30, songRect.y + 10, textColor);
        renderizarTexto(page->fontDetails, page->canciones[i].nombre, songRect.x + 70, songRect.y + 10, textColor);
        renderizarTexto(page->fontDetails, page->canciones[i].artista, songRect.x + 320, songRect.y + 10, textColor);
        renderizarTexto(page->fontDetails, page->canciones[i].album, songRect.x + 560, songRect.y + 10, textColor);
        renderizarTexto(page->fontDetails, page->canciones[i].duracion, songRect.x + 830, songRect.y + 10, textColor);
    }

    // Botón Siguiente
    if (page->showNextButton) {
        SDL_SetRenderDrawColor(renderer, page->colors[4].r, page->colors[4].g, page->colors[4].b, page->colors[4].a);
        SDL_RenderFillRect(renderer, &page->nextButton);
        renderizarTexto(page->fontDetails, "Siguiente", page->nextButton.x + 10, page->nextButton.y + 5, page->colors[1]);
    }

    // Limpieza
    SDL_FreeSurface(titleSurf);
    SDL_DestroyTexture(titleTex);
}

// Maneja eventos como clic, hover
void Playlist_HandleEvent(SDL_Event* event, bool* running, PlaylistPage* page) {
    if (event->type == SDL_MOUSEMOTION) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        page->hoveredItem = -1;

        int checkCount = page->showNextButton ? 5 : page->totalCanciones;
        for (int i = 0; i < checkCount; i++) {
            SDL_Rect songRect = {50, 330 + (i * 45), 925, 40};
            if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &songRect)) {
                page->hoveredItem = i;
                break;
            }
        }
    }
    else if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        if (SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &page->backButton)) {
            printf("Click en atrás\n");
        }
        else if (page->showNextButton && SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &page->nextButton)) {
            printf("Click en siguiente\n");
        }
    }
    else if (event->type == SDL_QUIT) {
        *running = false;
    }
}

// Libera recursos
void Playlist_Destroy(PlaylistPage* page) {
    if (page->background) SDL_DestroyTexture(page->background);
    if (page->albumCover) SDL_DestroyTexture(page->albumCover);
    if (page->fontTitle) TTF_CloseFont(page->fontTitle);
    if (page->fontDetails) TTF_CloseFont(page->fontDetails);
    if (page->canciones) free(page->canciones);
}
