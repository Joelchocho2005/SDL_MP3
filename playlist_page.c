#include "playlist_page.h"
#include "reproductor.h"  // Para las funciones de reproducción

static PlaylistPage page;

void initPlaylistPage(SDL_Renderer* renderer, playlist* pl) {
    // Cargar recursos
    page.background = IMG_LoadTexture(renderer, "imgs/Playlist_page.png");
    page.cover = IMG_LoadTexture(renderer, "imgs/album2.jpg");
    
    page.fontTitle = TTF_OpenFont("src/assets/Inder.ttf", 60);
    page.fontDetails = TTF_OpenFont("src/assets/Inder.ttf", 16);
    page.fontSongs = TTF_OpenFont("src/assets/Inder.ttf", 16);
    
    page.playlistActual = pl;
    page.backButton = (SDL_Rect){30, 10, 40, 40};
    
    // Inicializar rectángulos de canciones
    page.totalSongs = 0;
    playlistNodo* actual = pl->canciones;
    int yPos = 330;  // Posición Y inicial para las canciones
    
    while(actual != NULL && page.totalSongs < 50) {
        page.songRects[page.totalSongs] = (SDL_Rect){50, yPos, 925, 40};
        yPos += 45;
        page.totalSongs++;
        actual = actual->sig;
    }
}

void renderPlaylistPage(SDL_Renderer* renderer) {
    // Fondo
    SDL_RenderCopy(renderer, page.background, NULL, NULL);
    
    // Portada
    SDL_Rect coverRect = {75, 41, 200, 200};
    SDL_RenderCopy(renderer, page.cover, NULL, &coverRect);
    
    // Textos
    SDL_Color white = {255, 255, 255, 255};
    
    // Título playlist
    SDL_Surface* titleSurf = TTF_RenderText_Solid(page.fontTitle, page.playlistActual->nombre, white);
    SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
    SDL_Rect titleRect = {307, 80, titleSurf->w, titleSurf->h};
    SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
    
    // Número de canciones
    char songCount[20];
    snprintf(songCount, 20, "%d canciones", page.playlistActual->numeroCanciones);
    SDL_Surface* countSurf = TTF_RenderText_Solid(page.fontDetails, songCount, white);
    SDL_Texture* countTex = SDL_CreateTextureFromSurface(renderer, countSurf);
    SDL_Rect countRect = {410, 183, countSurf->w, countSurf->h};
    SDL_RenderCopy(renderer, countTex, NULL, &countRect);
    
    // Lista de canciones
    playlistNodo* actual = page.playlistActual->canciones;
    for(int i = 0; i < page.totalSongs && actual != NULL; i++) {
        SDL_Color songColor = {200, 200, 200, 255};
        
        // Resaltar si está reproduciéndose
        if(app.currentSong && app.currentSong->cancion == actual->cancion) {
            songColor = (SDL_Color){65, 135, 95, 255};
        }
        
        // Dibujar fondo de canción
        SDL_SetRenderDrawColor(renderer, 64, 65, 68, 255);
        SDL_RenderFillRect(renderer, &page.songRects[i]);
        
        // Número
        char numStr[4];
        snprintf(numStr, 4, "%d", i+1);
        renderText(renderer, page.fontSongs, numStr, page.songRects[i].x+30, page.songRects[i].y+10, songColor);
        
        // Título
        renderText(renderer, page.fontSongs, actual->cancion->info->nombre, page.songRects[i].x+70, page.songRects[i].y+10, songColor);
        
        // Artista
        renderText(renderer, page.fontSongs, actual->cancion->album->punteroArtista->info->nombre, 
                 page.songRects[i].x+320, page.songRects[i].y+10, songColor);
        
        // Duración
        char durStr[10];
        snprintf(durStr, 10, "%.2f", actual->cancion->info->duracion);
        renderText(renderer, page.fontSongs, durStr, page.songRects[i].x+830, page.songRects[i].y+10, songColor);
        
        actual = actual->sig;
    }
    
    // Limpieza
    SDL_FreeSurface(titleSurf);
    SDL_FreeSurface(countSurf);
    SDL_DestroyTexture(titleTex);
    SDL_DestroyTexture(countTex);
}

void handlePlaylistEvents(SDL_Event* e, AppState* currentState) {
    if(e->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point click = {x, y};
        
        // Botón Atrás
        if(SDL_PointInRect(&click, &page.backButton)) {
            *currentState = STATE_HOME;
            return;
        }
        
        // Verificar clic en canciones
        for(int i = 0; i < page.totalSongs; i++) {
            if(SDL_PointInRect(&click, &page.songRects[i])) {
                // Buscar la canción correspondiente
                playlistNodo* actual = page.playlistActual->canciones;
                for(int j = 0; j < i && actual != NULL; j++) {
                    actual = actual->sig;
                }
                
                if(actual) {
                    // Reproducir canción
                    app.currentSong = actual;
                    playCurrentSong(&app);
                }
                break;
            }
        }
    }
}

void destroyPlaylistPage() {
    if(page.background) SDL_DestroyTexture(page.background);
    if(page.cover) SDL_DestroyTexture(page.cover);
    if(page.fontTitle) TTF_CloseFont(page.fontTitle);
    if(page.fontDetails) TTF_CloseFont(page.fontDetails);
    if(page.fontSongs) TTF_CloseFont(page.fontSongs);
    
    memset(&page, 0, sizeof(PlaylistPage));
}

// Renderizar texto
void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect rect = {x, y, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &rect);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}
