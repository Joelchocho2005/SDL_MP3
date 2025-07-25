#include "playlist_page.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PlaylistPage page;

// Función auxiliar para renderizar texto
static void renderizarTexto(SDL_Renderer* renderer, TTF_Font* fuente, const char* texto, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(fuente, texto, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Playlist_Init(SDL_Renderer* renderer, const char* name, const char* author) {
    // Configuración de colores
    page.colors.fondo = (SDL_Color){64, 65, 68, 255};
    page.colors.blanco = (SDL_Color){255, 255, 255, 255};
    page.colors.gris = (SDL_Color){200, 200, 200, 255};
    page.colors.texto = (SDL_Color){20, 20, 20, 255};
    page.colors.hover = (SDL_Color){58, 58, 59, 255};

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

void Playlist_Render(SDL_Renderer* renderer, PlaylistPage* page) {
    // Fondo
    SDL_SetRenderDrawColor(renderer, 
                          page->colors.fondo.r, 
                          page->colors.fondo.g, 
                          page->colors.fondo.b, 
                          page->colors.fondo.a);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, page->background, NULL, NULL);

    // Portada del álbum
    SDL_Rect coverRect = {75, 41, 200, 200};
    SDL_RenderCopy(renderer, page->albumCover, NULL, &coverRect);

    // Título de la playlist
    SDL_Surface* titleSurf = TTF_RenderText_Solid(page->fontTitle, page->playlistName, page->colors.blanco);
    if (titleSurf) {
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        if (titleTex) {
            SDL_Rect titleRect = {307, 80, titleSurf->w, titleSurf->h};
            SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
            SDL_DestroyTexture(titleTex);
        }
        SDL_FreeSurface(titleSurf);
    }

    // Renderizar canciones
    int renderCount = page->showNextButton ? 5 : page->totalCanciones;
    for (int i = 0; i < renderCount; i++) {
        SDL_Color bgColor = (i == page->hoveredItem) ? page->colors.hover : page->colors.gris;
        SDL_Color textColor = (i == page->hoveredItem) ? page->colors.blanco : page->colors.texto;

        SDL_Rect songRect = {50, 330 + (i * 45), 925, 40};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &songRect);

        char indexStr[4];
        snprintf(indexStr, sizeof(indexStr), "%d", i + 1);
        
        // Renderizar todos los campos
        renderizarTexto(renderer, page->fontDetails, indexStr, songRect.x + 30, songRect.y + 10, textColor);
        renderizarTexto(renderer, page->fontDetails, page->canciones[i].nombre, songRect.x + 70, songRect.y + 10, textColor);
        renderizarTexto(renderer, page->fontDetails, page->canciones[i].artista, songRect.x + 320, songRect.y + 10, textColor);
        renderizarTexto(renderer, page->fontDetails, page->canciones[i].album, songRect.x + 560, songRect.y + 10, textColor);
        renderizarTexto(renderer, page->fontDetails, page->canciones[i].duracion, songRect.x + 830, songRect.y + 10, textColor);
    }

    // Botón "Siguiente"
    if (page->showNextButton) {
        SDL_SetRenderDrawColor(renderer, 
                              page->colors.hover.r, 
                              page->colors.hover.g, 
                              page->colors.hover.b, 
                              page->colors.hover.a);
        SDL_RenderFillRect(renderer, &page->nextButton);
        renderizarTexto(renderer, page->fontDetails, "Siguiente", 
                       page->nextButton.x + 10, page->nextButton.y + 5, 
                       page->colors.blanco);
    }
}

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

void Playlist_Destroy(PlaylistPage* page) {
    if (page->background) SDL_DestroyTexture(page->background);
    if (page->albumCover) SDL_DestroyTexture(page->albumCover);
    if (page->fontTitle) TTF_CloseFont(page->fontTitle);
    if (page->fontDetails) TTF_CloseFont(page->fontDetails);
    if (page->canciones) free(page->canciones);
}
