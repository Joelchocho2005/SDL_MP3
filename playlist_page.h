#ifndef PLAYLIST_PAGE_H
#define PLAYLIST_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Estructura para la información de una canción
typedef struct {
    int codCancion;
    char nombre[100];
    char artista[100];
    char album[100];
    char duracion[10];
} CancionPlaylist;

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
    CancionPlaylist* canciones;
    int totalCanciones;
    int capacidad;
    
    // Interacción
    SDL_Rect backButton;
    SDL_Rect nextButton;
    bool showNextButton;
    int hoveredItem;
    PlaylistColors colors;
} PlaylistPage;

// Inicialización
void Playlist_Init(SDL_Renderer* renderer, const char* name, const char* author);

// Agregar canción desde otras páginas
void Playlist_AgregarCancion(PlaylistPage* page, const char* nombre, const char* artista, 
                            const char* album, const char* duracion);

// Renderizado
void Playlist_Render(SDL_Renderer* renderer, PlaylistPage* page);

// Eventos
void Playlist_HandleEvent(SDL_Event* event, bool* running, PlaylistPage* page);

// Limpieza
void Playlist_Destroy(PlaylistPage* page);

#endif
