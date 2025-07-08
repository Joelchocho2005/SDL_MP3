#ifndef PLAYLIST_PAGE_H
#define PLAYLIST_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "generalH.h" // Para las estructuras del proyecto principal

typedef enum {
    PLAYLIST_STATE_IDLE,
    PLAYLIST_STATE_PLAYING,
    PLAYLIST_STATE_HOVER
} PlaylistState;

// Estructura de la página de playlist
typedef struct {
    // Recursos gráficos
    SDL_Texture* background;
    SDL_Texture* albumCover;
    
    // Fuentes
    TTF_Font* fontTitle; // Título grande (60px)
    TTF_Font* fontMetadata; // Metadatos (16px)
    TTF_Font* fontSongs; // Lista de canciones (16px)
    
    // Datos dinámicos
    playlist* currentPlaylist;
    playlistNodo* currentlyPlaying; // Canción en reproducción
    SDL_Rect songItems[50]; // Áreas clickeables de canciones
    int visibleSongCount; // Canciones renderizadas
    
    SDL_Rect backButton; // Botón de retroceso
    PlaylistState state;
    int hoveredItemIndex; // Índice de canción hover
    
    SDL_Color colorNormal;
    SDL_Color colorHover;
    SDL_Color colorPlaying;
} PlaylistPage;

void Playlist_Init(SDL_Renderer* renderer, playlist* pl);

void Playlist_Render(SDL_Renderer* renderer, playlistNodo* currentSong);

playlistNodo* Playlist_HandleEvent(SDL_Event* event, AppState* appState);

void Playlist_Destroy();

void Playlist_RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color);

void Playlist_UpdateScroll(int scrollOffset);

#endif // PLAYLIST_PAGE_H
