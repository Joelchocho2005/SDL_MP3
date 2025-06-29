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

typedef enum {
    STATE_HOME,
    STATE_PLAYER,
} AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
} Button;

typedef struct SongNode {
    char path[256];              // ruta de la canción
    Mix_Music* music;            // puntero a la música
    struct SongNode* prev;
    struct SongNode* next;
} SongNode;

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
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* background;     // imagen fondo reproductor (solo para STATE_PLAYER)
    SDL_Texture* background_Player;

    Button playButton;
    Button pauseButton;
    Button prevButton;
    Button nextButton;

    SDL_Rect backButtonRect;     // solo rect, sin textura (botón "Regresar" está en la imagen de fondo)
    
    bool isPlaying;

    bool premium;
    int cancionesDesdeUltimoAnuncio;
    AdQueue adQueue;

    // Playlist linked list
    SongNode* currentSong;

    bool running;
    AppState currentState;
} App;

bool initSDL(App* app) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL no pudo inicializarse: %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image no pudo inicializarse: %s\n", IMG_GetError());
        return false;
    }
    if (TTF_Init() == -1) {
        printf("SDL_ttf no pudo inicializarse: %s\n", TTF_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer no pudo inicializarse: %s\n", Mix_GetError());
        return false;
    }

    app->window = SDL_CreateWindow(
        "Reproductor de Música",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!app->window) {
        printf("No se pudo crear la ventana: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetWindowResizable(app->window, SDL_FALSE);

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) {
        printf("No se pudo crear el renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

SDL_Texture* loadTexture(App* app, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("No se pudo cargar la imagen %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("No se pudo crear la textura desde %s: %s\n", path, SDL_GetError());
    }
    return texture;
}

bool loadMedia(App* app) {
    // Fondo reproductor solo para estado PLAYER
    app->background = loadTexture(app, "imgs/Img_Reproductor.png");
    if (!app->background) return false;

    app->prevButton.texture = loadTexture(app, "imgs/promo3Atras-removebg-preview.png");
    app->playButton.texture = loadTexture(app, "imgs/promo3Play-removebg-preview.png");
    app->pauseButton.texture = loadTexture(app, "imgs/prom3Pause-removebg-preview.png");
    app->nextButton.texture = loadTexture(app, "imgs/promo3Siguiente-removebg-preview.png");

    if (!app->prevButton.texture || !app->playButton.texture || 
        !app->pauseButton.texture || !app->nextButton.texture) return false;

    app->isPlaying = false;

    return true;
}

bool loadPlaylist(App* app, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("No se pudo abrir %s\n", filename);
        return false;
    }

    SongNode* head = NULL;
    SongNode* tail = NULL;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Quitar salto de línea si existe
        line[strcspn(line, "\r\n")] = 0;

        SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
        if (!newNode) {
            printf("Error de memoria\n");
            fclose(file);
            return false;
        }
        strcpy(newNode->path, line);
        newNode->music = Mix_LoadMUS(line);
        if (!newNode->music) {
            printf("No se pudo cargar: %s\n", Mix_GetError());
            free(newNode);
            continue;
        }

        newNode->prev = tail;
        newNode->next = NULL;

        if (tail) tail->next = newNode;
        else head = newNode;

        tail = newNode;
    }

    fclose(file);

    app->currentSong = head;  // empieza por la primera canción
    return head != NULL;
}

bool loadAds(App* app, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("No se pudo abrir el archivo de anuncios: %s\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        AdNode* newNode = (AdNode*)malloc(sizeof(AdNode));
        if (!newNode) {
            printf("Error de memoria al cargar anuncio\n");
            fclose(file);
            return false;
        }

        strcpy(newNode->path, line);
        newNode->music = Mix_LoadMUS(line);
        if (!newNode->music) {
            printf("No se pudo cargar el anuncio: %s\n", Mix_GetError());
            free(newNode);
            continue;
        }

        newNode->next = NULL;
        if (!app->adQueue.front) {
            app->adQueue.front = newNode;
            app->adQueue.rear = newNode;
        } else {
            app->adQueue.rear->next = newNode;
            app->adQueue.rear = newNode;
        }
    }

    fclose(file);
    return true;
}

void playCurrentSong(App* app) {
    if (!app->currentSong) return;

    // Si no es premium y se han reproducido 2 canciones, toca anuncio
    if (!app->premium && app->cancionesDesdeUltimoAnuncio >= 2) {
        if (app->adQueue.front) {
            Mix_HaltMusic();
            AdNode* ad = app->adQueue.front;

            if (Mix_PlayMusic(ad->music, 1) == -1) {
                printf("Error al reproducir anuncio: %s\n", Mix_GetError());
            } else {
                app->isPlaying = true;
                printf("Reproduciendo anuncio: %s\n", ad->path);
            }

            // Sacar de la cola
            app->adQueue.front = ad->next;
            if (!app->adQueue.front) app->adQueue.rear = NULL;

            Mix_FreeMusic(ad->music);
            free(ad);

            app->cancionesDesdeUltimoAnuncio = 0;
            return; // después del anuncio, no cambia la canción
        }
    }

    // Reproducir la canción actual
    Mix_HaltMusic();
    if (Mix_PlayMusic(app->currentSong->music, -1) == -1) {
        printf("Error al reproducir musica: %s\n", Mix_GetError());
        app->isPlaying = false;
        return;
    }

    app->isPlaying = true;
    app->cancionesDesdeUltimoAnuncio++;
}


void setupButtonsForState(App* app) {
    switch (app->currentState) {
        case STATE_HOME:
            // Solo botón play visible
            app->playButton.rect = (SDL_Rect){ 481, 500, 64, 64 };
            app->prevButton.rect = (SDL_Rect){ 0, 0, 0, 0 };
            app->nextButton.rect = (SDL_Rect){ 0, 0, 0, 0 };
            app->backButtonRect = (SDL_Rect){ 0, 0, 0, 0 };  // oculto
            break;

        case STATE_PLAYER:
            // Todos visibles y back arriba izquierda
            app->playButton.rect = (SDL_Rect){ 481, 500, 64, 64 };
            app->prevButton.rect = (SDL_Rect){ 350, 500, 64, 64 };
            app->nextButton.rect = (SDL_Rect){ 612, 500, 64, 64 };
            app->backButtonRect = (SDL_Rect){ 20, 20, 80, 40 };
            break;
    }
}

void render(App* app) {
    if (app->currentState == STATE_HOME) {
        SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
        SDL_RenderClear(app->renderer);
    } else if (app->currentState == STATE_PLAYER) {
        SDL_RenderClear(app->renderer);
        SDL_RenderCopy(app->renderer, app->background, NULL, NULL);
    }

    // Render botones visibles (w > 0)
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

    // El botón back no tiene textura, está en la imagen de fondo

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

            // Botón play (función según estado)
            if (app->playButton.rect.w > 0 && SDL_PointInRect(&p, &app->playButton.rect)) {
                if (app->currentState == STATE_HOME) {
                    app->currentState = STATE_PLAYER;
                    setupButtonsForState(app);
                    playCurrentSong(app);
                } else if (app->currentState == STATE_PLAYER) {
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
                }
            }

            // Botón Back (solo visible en PLAYER)
            if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                if (app->currentState == STATE_PLAYER) {
                    Mix_HaltMusic();
                    app->isPlaying = false;
                    app->currentState = STATE_HOME;
                    setupButtonsForState(app);
                }
            }

            // Botón Anterior
            if (app->prevButton.rect.w > 0 && SDL_PointInRect(&p, &app->prevButton.rect)) {
                if (app->currentState == STATE_PLAYER && app->currentSong && app->currentSong->prev) {
                    app->currentSong = app->currentSong->prev;
                    playCurrentSong(app);
                }
            }

            // Botón Siguiente
            if (app->nextButton.rect.w > 0 && SDL_PointInRect(&p, &app->nextButton.rect)) {
                if (app->currentState == STATE_PLAYER && app->currentSong && app->currentSong->next) {
                    app->currentSong = app->currentSong->next;
                    playCurrentSong(app);
                }
            }
        }
    }
}

void freePlaylist(App* app) {
    SongNode* node = app->currentSong;
    // Navegar al inicio
    while (node && node->prev) node = node->prev;
    // Liberar toda la lista
    while (node) {
        SongNode* next = node->next;
        Mix_FreeMusic(node->music);
        free(node);
        node = next;
    }
}

void freeAdQueue(App* app) {
    AdNode* node = app->adQueue.front;
    while (node) {
        AdNode* next = node->next;
        Mix_FreeMusic(node->music);
        free(node);
        node = next;
    }
    app->adQueue.front = NULL;
    app->adQueue.rear = NULL;
}

void cleanup(App* app) {
    freePlaylist(app);

    if (app->background) SDL_DestroyTexture(app->background);

    if (app->playButton.texture) SDL_DestroyTexture(app->playButton.texture);
    if (app->pauseButton.texture) SDL_DestroyTexture(app->pauseButton.texture);
    if (app->prevButton.texture) SDL_DestroyTexture(app->prevButton.texture);
    if (app->nextButton.texture) SDL_DestroyTexture(app->nextButton.texture);

    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);

    // Liberar anuncios
    freeAdQueue(app);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    App app;
    memset(&app, 0, sizeof(App));

    app.running = true;
    app.currentState = STATE_PLAYER;
    app.premium = false;  // O true si quieres simular cuenta premium
    app.cancionesDesdeUltimoAnuncio = 0;

    if (!initSDL(&app)) {
        return 1;
    }

    if (!loadMedia(&app)) {
        cleanup(&app);
        return 1;
    }

    if (!loadPlaylist(&app, "data/canciones.txt")) {
        printf("No se pudo cargar la playlist\n");
        cleanup(&app);
        return 1;
    }
    if (!loadAds(&app, "data/anuncios.txt")) {
        printf("No se pudieron cargar los anuncios\n");
        cleanup(&app);
        return 1;
    }
    setupButtonsForState(&app);

    while (app.running) {
        handleEvents(&app);
        render(&app);
        SDL_Delay(16); // ~60 fps
    }

    cleanup(&app);
    return 0;
}
