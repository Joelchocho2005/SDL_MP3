#ifndef PLAYLIST_PAGE_H
#define PLAYLIST_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Colores
typedef struct {
    SDL_Color fondo;
    SDL_Color blanco;
    SDL_Color gris;
    SDL_Color texto;
    SDL_Color hover;
} PlaylistColors;

// Estructura principal
typedef struct {
    // Recursos
    SDL_Texture* background;
    SDL_Texture* albumCover;
    TTF_Font* fontTitle;
    TTF_Font* fontDetails;
    
    // Datos
    const char* playlistName;
    const char* playlistAuthor;
    int songCount;
    SDL_Rect* songRects;
    
    // Interacción
    SDL_Rect backButton;
    SDL_Rect nextButton;
    bool showNextButton;
    int hoveredItem;
    PlaylistColors colors;
} PlaylistPage;

// Inicialización
void Playlist_Init(SDL_Renderer* renderer, const char* name, const char* author, int totalSongs);

// Renderizado
void Playlist_Render(SDL_Renderer* renderer, const char** titles, const char** artists, 
                    const char** albums, const char** durations);

// Eventos
void Playlist_HandleEvent(SDL_Event* event, bool* running);

// Limpieza
void Playlist_Destroy();

#endif
