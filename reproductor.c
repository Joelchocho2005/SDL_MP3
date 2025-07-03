#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1026
#define WINDOW_HEIGHT 652
typedef enum { STATE_HOME, STATE_PLAYER } AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
} Button;

typedef struct SongNode {
    char path[256];
    char nombreCancion[256];
    Mix_Music* music;
    struct SongNode* prev;
    struct SongNode* next;
} SongNode;

typedef struct Playlist {
    char name[256];
    SongNode* head;
      int numCanciones;   // ← total de canciones
    int duracionTotal;  // ← duración total en segundos
    struct Playlist* next;
} Playlist;

    typedef struct AdNode {
        char path[256];
        Mix_Music* music;
        struct AdNode* next;
    } AdNode;

typedef struct {
    AdNode* front;
    AdNode* rear;
} AdQueue;

typedef struct {
        TTF_Font* small;
        TTF_Font* medium;
        TTF_Font* large;
} Fonts;

typedef struct {
    char nombreUsuario[100];
    char codigoUsuario[100];
    bool premium;
}Usuario;

    typedef struct {
        SDL_Window* window;
        SDL_Renderer* renderer;

        SDL_Texture* background;

        Button playButton;
        Button pauseButton;
        Button prevButton;
        Button nextButton;
        SDL_Rect backButtonRect;

        bool isPlaying;
        bool premium;
        int cancionesDesdeUltimoAnuncio;
        bool reproduciendoAnuncio;

        AdQueue adQueue;

        Playlist* playlists;  // lista de todas las playlists cargadas
        Playlist* currentPlaylist; // la playlist que está sonando
        SongNode* currentSong;
        SongNode* pendingSong;

        
        Fonts fonts;

        bool running;
        AppState currentState;
    } App;

// --- Declaración de funciones ---
bool initSDL(App* app);
SDL_Texture* loadTexture(App* app, const char* path);
bool loadMedia(App* app);
Playlist* loadPlaylist(const char* filename);
bool loadAds(App* app, const char* filename);
bool loadFonts(App* app);
void playCurrentSong(App* app);
void setupButtonsForState(App* app);
void render(App* app);
void handleEvents(App* app);
void cleanup(App* app);
void updatePlayback(App* app);
Playlist* loadUserPlaylists(const char* playlistsFile, Usuario* user, const char* basePath);
void cargarYMostrarPlaylists(App* app, const Usuario* user, const char* basePath);

int main(int argc, char* argv[]) {
    App app = {0};

    Usuario userAccount;
    strcpy(userAccount.codigoUsuario, "123");
    userAccount.premium=false;

    app.running = true;
    app.currentState = STATE_PLAYER;
    app.premium = false;  // cambia según necesidad
    app.cancionesDesdeUltimoAnuncio = 0;
    app.reproduciendoAnuncio = false;
    app.adQueue.front = app.adQueue.rear = NULL;

  

  
    
    if (!initSDL(&app)) return 1;
    if (!loadMedia(&app)) return 1;
    if (!loadFonts(&app)) return 1;

    // Playlist* base = loadPlaylist("data/playlistListaUsuarios/canciones.txt");
    
    // if (base) {
    // base->next = app.playlists;
    // app.playlists = base;
    // }

    // app.currentPlaylist = base;  // o la que el usuario elija
    // app.currentSong = base->head;
    cargarYMostrarPlaylists(&app, &userAccount, "data/playlistUsuarios");

    if (!loadAds(&app, "data\\anuncios.txt")) {
        printf("No se cargaron anuncios, pero sigue funcionando.\n");
    }

    setupButtonsForState(&app);

    while (app.running) {
        handleEvents(&app);

        updatePlayback(&app); 

        render(&app);
        SDL_Delay(16);  // ~60 FPS
    }

    cleanup(&app);
    return 0;
}

// --- Implementaciones ---

bool initSDL(App* app) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        return false;
    }
    if (TTF_Init() == -1) {
        printf("TTF_Init error: %s\n", TTF_GetError());
        return false;
    }
    app->window = SDL_CreateWindow("Reproductor SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!app->window) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        return false;
    }
    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

SDL_Texture* loadTexture(App* app, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("IMG_Load error %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
    }
    return texture;
}

bool loadMedia(App* app) {
    app->background = loadTexture(app, "imgs/Img_Reproductor.png");
    if (!app->background) return false;

    app->playButton.texture = loadTexture(app, "imgs/promo3Play-removebg-preview.png");
    app->pauseButton.texture = loadTexture(app, "imgs/prom3Pause-removebg-preview.png");
    app->prevButton.texture = loadTexture(app, "imgs/promo3Atras-removebg-preview.png");
    app->nextButton.texture = loadTexture(app, "imgs/promo3Siguiente-removebg-preview.png");

    if (!app->playButton.texture || !app->pauseButton.texture ||
        !app->prevButton.texture || !app->nextButton.texture) return false;

    app->isPlaying = false;

    return true;
}
Playlist* loadUserPlaylists(const char* playlistsFile, Usuario* user, const char* basePath) {
    FILE* f = fopen(playlistsFile, "r");
    if (!f) {
        printf("No se pudo abrir archivo de playlists: %s\n", playlistsFile);
        return NULL;
    }

    Playlist* first = NULL;
    Playlist* last = NULL;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;  // Limpiar salto de línea
        if (strlen(line) == 0) continue;

        // Armar nombre de archivo: "<codigoUsuario>-<nombrePlaylist>.csv"
        char playlistFilePath[512];
        snprintf(playlistFilePath, sizeof(playlistFilePath), "%s/%s-%s.csv", basePath, user->codigoUsuario, line);

        FILE* pf = fopen(playlistFilePath, "r");
        if (!pf) {
            printf("No se pudo abrir archivo de playlist: %s\n", playlistFilePath);
            continue;
        }

        Playlist* pl = malloc(sizeof(Playlist));
        if (!pl) {
            fclose(pf);
            continue;
        }

        strcpy(pl->name, line);
        pl->head = NULL;
        pl->next = NULL;

        // === 📌 Leer cabecera: numCanciones,duracionTotal ===
        char headerLine[256];
        if (fgets(headerLine, sizeof(headerLine), pf)) {
            headerLine[strcspn(headerLine, "\r\n")] = 0;

            char* numStr = strtok(headerLine, ",");
            char* durStr = strtok(NULL, ",");

            if (numStr && durStr) {
                pl->numCanciones = atoi(numStr);
                pl->duracionTotal = atoi(durStr);
            } else {
                pl->numCanciones = 0;
                pl->duracionTotal = 0;
            }
        } else {
            pl->numCanciones = 0;
            pl->duracionTotal = 0;
        }

        // === 📌 Leer canciones ===
        SongNode* tail = NULL;
        char songLine[512];
        while (fgets(songLine, sizeof(songLine), pf)) {
            songLine[strcspn(songLine, "\r\n")] = 0;

            char* nombreCancion = strtok(songLine, ",");
            char* pathCancion = strtok(NULL, ",");

            if (!nombreCancion || !pathCancion) continue;
            

            SongNode* node = malloc(sizeof(SongNode));
            if (!node) continue;

            strcpy(node->nombreCancion, nombreCancion);
            strcpy(node->path, pathCancion);

        

            node->music = Mix_LoadMUS(pathCancion);

            if (!node->music) {
                printf("Error cargando canción: %s\n", pathCancion);
                free(node);
                continue;
            }

            node->prev = tail;
            node->next = NULL;

            if (tail) tail->next = node;
            else pl->head = node;

            tail = node;
        }

        fclose(pf);

        if (!first) first = pl;
        else last->next = pl;

        last = pl;
    }

    fclose(f);
    return first;
}
void cargarYMostrarPlaylists(App* app, const Usuario* user, const char* basePath) {
    char archivoPlaylist[150] = "";
    strcat(archivoPlaylist, "data/playlistListaUsuarios/");
    strcat(archivoPlaylist, user->codigoUsuario);
    strcat(archivoPlaylist, "-ListaPlaylist.csv");

    app->playlists = loadUserPlaylists(archivoPlaylist, (Usuario*)user, basePath);
    app->currentPlaylist = app->playlists;
    app->currentSong = app->currentPlaylist ? app->currentPlaylist->head : NULL;

    // Mostrar playlists y canciones
    Playlist* p = app->playlists;
    while (p) {
        printf("\nPlaylist: %s\n", p->name);
        SongNode* s = p->head;
        while (s) {
            printf("  Cancion: %s (%s)\n", s->nombreCancion, s->path);
            s = s->next;
        }
        p = p->next;
    }
}
Playlist* loadPlaylist(const char* filename) {
      FILE* f = fopen(filename, "r");
    if (!f) return NULL;

    Playlist* pl = malloc(sizeof(Playlist));
    if (!pl) {
        fclose(f);
        return NULL;
    }
    strcpy(pl->name, filename);
    pl->head = NULL;
    pl->next = NULL;

    SongNode* tail = NULL;
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        SongNode* node = malloc(sizeof(SongNode));
        if (!node) continue;

        strcpy(node->path, line);
        node->music = Mix_LoadMUS(line);
        if (!node->music) {
            free(node);
            continue;
        }
        node->next = NULL;
        node->prev = tail;

        if (tail) tail->next = node;
        else pl->head = node;

        tail = node;
    }

    fclose(f);
    if (!pl->head) {
        free(pl);
        return NULL;
    }
    return pl;
}

bool loadAds(App* app, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("No se pudo abrir anuncios %s\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        AdNode* ad = malloc(sizeof(AdNode));
        if (!ad) {
            fclose(f);
            return false;
        }
        strcpy(ad->path, line);
        ad->music = Mix_LoadMUS(line);
        if (!ad->music) {
            printf("Error cargando anuncio %s: %s\n", line, Mix_GetError());
            free(ad);
            continue;
        }
        ad->next = NULL;
        if (!app->adQueue.front) {
            app->adQueue.front = app->adQueue.rear = ad;
        } else {
            app->adQueue.rear->next = ad;
            app->adQueue.rear = ad;
        }
    }
    fclose(f);
    return true;
}

bool loadFonts(App* app) {
    app->fonts.small = TTF_OpenFont("src/assets/Inder-Regular.ttf", 12);
    if (!app->fonts.small) {
        printf("Error cargando fuente pequeña: %s\n", TTF_GetError());
        return false;
    }

    app->fonts.medium = TTF_OpenFont("src/assets/Inder-Regular.ttf", 24);
    if (!app->fonts.medium) {
        printf("Error cargando fuente mediana: %s\n", TTF_GetError());
        return false;
    }

    app->fonts.large = TTF_OpenFont("src/assets/Inder-Regular.ttf", 48);
    if (!app->fonts.large) {
        printf("Error cargando fuente grande: %s\n", TTF_GetError());
        return false;
    }

    return true;
}
void renderText(App* app, const char* text, TTF_Font* font, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};  // blanco, o puedes parametrizar
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        printf("TTF_RenderUTF8_Blended error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        return;
    }
    int textW, textH;
    SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
    SDL_Rect dstRect = { x, y, textW, textH };
    SDL_RenderCopy(app->renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
}

void playCurrentSong(App* app) {
    if (!app->currentSong) return;
    // Si no es premium y debe sonar anuncio
    if (!app->premium && app->cancionesDesdeUltimoAnuncio >= 2 && !app->reproduciendoAnuncio && app->adQueue.front) {
        app->pendingSong = app->currentSong;
        AdNode* ad = app->adQueue.front;

        Mix_HaltMusic();
        if (Mix_PlayMusic(ad->music, 1) == -1) {
            printf("Error reproduciendo anuncio: %s\n", Mix_GetError());
        } else {
            app->reproduciendoAnuncio = true;
            app->isPlaying = true;
            printf("Reproduciendo anuncio: %s\n", ad->path);
           
        }
        return;
    }

    // Reproducir canción normal
    Mix_HaltMusic();
    if (Mix_PlayMusic(app->currentSong->music, 0) == -1) {
        printf("Error reproduciendo canción: %s\n", Mix_GetError());
        app->isPlaying = false;
        return;
    }
    app->isPlaying = true;
    app->reproduciendoAnuncio = false;
    printf("Reproduciendo canción: %s\n", app->currentSong->path);
}

void setupButtonsForState(App* app) {
    if (app->currentState == STATE_HOME) {
        app->playButton.rect = (SDL_Rect){ 481, 500, 64, 64 };
        app->prevButton.rect = (SDL_Rect){ 0, 0, 0, 0 };
        app->nextButton.rect = (SDL_Rect){ 0, 0, 0, 0 };
        app->backButtonRect = (SDL_Rect){ 0, 0, 0, 0 };
    } else if (app->currentState == STATE_PLAYER) {
        app->playButton.rect = (SDL_Rect){ 481, 520, 80, 80 };
        app->prevButton.rect = (SDL_Rect){ 350, 523, 80, 80 };
        app->nextButton.rect = (SDL_Rect){ 612, 520, 80, 80 };
        app->backButtonRect = (SDL_Rect){ 20, 20, 80, 40 };
    }
}

void render(App* app) {
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    switch (app->currentState) {
        case STATE_HOME:
            // Renderiza cosas de la pantalla de inicio
            break;

        case STATE_PLAYER:
            SDL_RenderCopy(app->renderer, app->background, NULL, NULL);

            if (app->playButton.rect.w > 0) {
                if (app->isPlaying)
                    SDL_RenderCopy(app->renderer, app->pauseButton.texture, NULL, &app->playButton.rect);
                else
                    SDL_RenderCopy(app->renderer, app->playButton.texture, NULL, &app->playButton.rect);
            }
            if (app->prevButton.rect.w > 0)
                SDL_RenderCopy(app->renderer, app->prevButton.texture, NULL, &app->prevButton.rect);
            if (app->nextButton.rect.w > 0)
                SDL_RenderCopy(app->renderer, app->nextButton.texture, NULL, &app->nextButton.rect);
                renderText(app, app->currentPlaylist->name, app->fonts.medium, 280, 20);
                
                if(!app->reproduciendoAnuncio)
                renderText(app, app->currentSong->nombreCancion, app->fonts.small, 100, 150);
                else renderText(app, app->adQueue.front->path, app->fonts.small, 50, 150);
            break;
        default:
            break;
    }

    SDL_RenderPresent(app->renderer);
}


void handleEvents(App* app) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            app->running = false;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point p = {x, y};

            switch (app->currentState) {
                case STATE_HOME:
                    if (SDL_PointInRect(&p, &app->playButton.rect)) {
                        app->currentState = STATE_PLAYER;
                        setupButtonsForState(app);
                        playCurrentSong(app);
                    }
                    break;

                case STATE_PLAYER:
                    if (app->reproduciendoAnuncio) {
                        printf("Anuncio en reproducción: controles bloqueados.\n");
                        break;
                    }

                    if (SDL_PointInRect(&p, &app->playButton.rect)) {
                        if (app->isPlaying) {
                            Mix_PauseMusic();
                            app->isPlaying = false;
                        } else {
                            if (Mix_PlayingMusic() == 0) {
                                playCurrentSong(app);
                            } else {
                                Mix_ResumeMusic();
                                app->isPlaying = true;
                            }
                        }
                    } else if (SDL_PointInRect(&p, &app->prevButton.rect)) {
                        if (app->currentSong && app->currentSong->prev) {
                            app->currentSong = app->currentSong->prev;
                            app->cancionesDesdeUltimoAnuncio++;
                            playCurrentSong(app);
                        }
                    } else if (SDL_PointInRect(&p, &app->nextButton.rect)) {
                        if (app->currentSong && app->currentSong->next) {
                            app->currentSong = app->currentSong->next;
                            app->cancionesDesdeUltimoAnuncio++;
                            playCurrentSong(app);
                        }
                    }
                    break;
            }
        }
    }
}
void updatePlayback(App* app) {
    // Si terminó anuncio, pasa a canción
    if (app->reproduciendoAnuncio && !Mix_PlayingMusic()) {
        app->reproduciendoAnuncio = false;
        app->cancionesDesdeUltimoAnuncio = 0;

        // Quitar anuncio de la cola y liberar
        AdNode* ad = app->adQueue.front;
        if (ad) {
            app->adQueue.front = ad->next;
            if (!app->adQueue.front) app->adQueue.rear = NULL;
            Mix_FreeMusic(ad->music);
            free(ad);
        }

        // Si no quieres volver a la pendiente, avanza a la siguiente:
        if (app->currentSong && app->currentSong->next) {
            app->currentSong = app->currentSong->next;
        } else {
            app->currentSong = app->currentPlaylist->head;
        }

        playCurrentSong(app);
    }

    // Si canción terminó y no es anuncio
    if (!app->reproduciendoAnuncio && !Mix_PlayingMusic() && app->isPlaying) {
        if (app->currentSong && app->currentSong->next) {
            app->currentSong = app->currentSong->next;
            app->cancionesDesdeUltimoAnuncio++;
            playCurrentSong(app);
        } else {
            app->isPlaying = false;  // Fin de lista
        }
    }
}

void cleanup(App* app) {
    // Liberar lista canciones
    SongNode* s = app->currentSong;
    while (s && s->prev) s = s->prev; // ir al inicio
    while (s) {
        SongNode* next = s->next;
        Mix_FreeMusic(s->music);
        free(s);
        s = next;
    }

    // Liberar anuncios restantes
    AdNode* a = app->adQueue.front;
    while (a) {
        AdNode* next = a->next;
        Mix_FreeMusic(a->music);
        free(a);
        a = next;
    }

    SDL_DestroyTexture(app->playButton.texture);
    SDL_DestroyTexture(app->pauseButton.texture);
    SDL_DestroyTexture(app->prevButton.texture);
    SDL_DestroyTexture(app->nextButton.texture);
    SDL_DestroyTexture(app->background);

    if (app->fonts.small) TTF_CloseFont(app->fonts.small);
    if (app->fonts.medium) TTF_CloseFont(app->fonts.medium);
    if (app->fonts.large) TTF_CloseFont(app->fonts.large);

    Mix_CloseAudio();
    IMG_Quit();
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}
