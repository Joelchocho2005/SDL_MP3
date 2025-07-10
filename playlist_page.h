#ifndef PLAYLIST_PAGE_H
#define PLAYLIST_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "../Artistas_Canciones/artistas_canciones.h"

// Estructura de colores
typedef struct {
    SDL_Color fondo;
    SDL_Color blanco;
    SDL_Color gris;
    SDL_Color texto;
    SDL_Color hover;
} PlaylistColors;

// Estructura principal de la playlist
typedef struct {
    // Recursos gráficos
    SDL_Texture* background;
    SDL_Texture* albumCover;
    TTF_Font* fontTitle;
    TTF_Font* fontDetails;
    
    // Datos
    const char* playlistName;
    const char* playlistAuthor;
    cancionInfo* canciones;
    int totalCanciones;
    int capacidad;
    
    // Interacción
    SDL_Rect backButton;
    SDL_Rect nextButton;
    bool showNextButton;
    int hoveredItem;
    PlaylistColors colors;
} PlaylistPage;

// Funciones principales
void Playlist_Init(SDL_Renderer* renderer, const char* name, const char* author);
void Playlist_AgregarCancion(PlaylistPage* page, const cancionInfo* cancion);
void Playlist_Render(SDL_Renderer* renderer, PlaylistPage* page);
void Playlist_HandleEvent(SDL_Event* event, bool* running, PlaylistPage* page);
void Playlist_Destroy(PlaylistPage* page);

#endif
