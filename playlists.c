#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 1026
#define WINDOW_HEIGHT 652
#define MAX_PLAYLISTS 999
#define MAX_COVER_OPTIONS 2

typedef enum {
    STATE_HOME,
    STATE_CREATE_PLAYLIST,
    STATE_SELECT_COVER
} AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    char path[256];
} CoverOption;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    TTF_Font* font;
} Button;

typedef struct {
    SDL_Texture* cover;
    char name[50];
    int songCount;
    bool selected;
} PlaylistItem;

typedef struct {
    CoverOption covers[MAX_COVER_OPTIONS];
    Button backButton;
    Button selectButtons[MAX_COVER_OPTIONS];
    Button acceptButton;
    int selectedCover;
} CoverSelectionScreen;

typedef struct {
    Button backButton;
    Button newPlaylistButton;
    Button addButton;
    PlaylistItem playlists[MAX_PLAYLISTS];
    int playlistCount;
} HomeScreen;

typedef struct {
    Button cancelButton;
    Button createButton;
    Button selectCoverButton;
    SDL_Rect playlistNameRect;
    char playlistName[50];
    bool isEditingName;
    SDL_Texture* playlistNameTexture;
    char selectedCoverPath[256];
} CreatePlaylistScreen;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    AppState currentState;
    bool running;

    SDL_Texture* homeScreenTexture;
    SDL_Texture* createPlaylistTexture;
    SDL_Texture* coverSelectionTexture;
    SDL_Texture* coverPlaceholder;
    SDL_Texture* plusIcon;

    HomeScreen homeScreen;
    CreatePlaylistScreen createScreen;
    CoverSelectionScreen coverSelection;

    int playlistCounter;
} App;

// ------------------------ FUNCIONES AUXILIARES ------------------------

void renderButton(SDL_Renderer* renderer, Button* button, const char* text, SDL_Color bgColor) {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &button->rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &button->rect);

    if (button->font && text) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(button->font, text, white);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {
                button->rect.x + (button->rect.w - textSurface->w)/2,
                button->rect.y + (button->rect.h - textSurface->h)/2,
                textSurface->w, textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}

void updateTextTexture(App* app, SDL_Texture** texture, const char* text, SDL_Color color) {
    if (*texture) SDL_DestroyTexture(*texture);
    if (!text || !app->font) return;

    SDL_Surface* surface = TTF_RenderText_Blended(app->font, text, color);
    if (!surface) return;

    *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
}

SDL_Texture* loadTexture(App* app, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Error cargando imagen %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool isAnyPlaylistSelected(App* app) {
    for (int i = 0; i < app->homeScreen.playlistCount; i++) {
        if (app->homeScreen.playlists[i].selected) return true;
    }
    return false;
}

// ------------------------ INICIALIZACIÓN ------------------------

bool initSDL(App* app) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) || TTF_Init() == -1) {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow("Reproductor de Playlists", SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!app->window) {
        printf("Error creando ventana: %s\n", SDL_GetError());
        return false;
    }

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!app->renderer) {
        printf("Error creando renderer: %s\n", SDL_GetError());
        return false;
    }

    app->font = TTF_OpenFont("src/assets/Inder-Regular.ttf", 24);
    if (!app->font) {
        printf("Error cargando fuente: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

bool loadMedia(App* app) {
    // Cargar imágenes de fondo
    app->homeScreenTexture = loadTexture(app, "imgs/agregar_playlist.png");
    app->createPlaylistTexture = loadTexture(app, "imgs/create_playlist.png");
    app->coverSelectionTexture = loadTexture(app, "imgs/portadas.png");
    app->plusIcon = loadTexture(app, "imgs/plus.png");
    
    // Crear placeholder para portadas
    SDL_Surface* placeholderSurface = SDL_CreateRGBSurface(0, 200, 200, 32, 0, 0, 0, 0);
    SDL_FillRect(placeholderSurface, NULL, SDL_MapRGB(placeholderSurface->format, 200, 200, 200));
    app->coverPlaceholder = SDL_CreateTextureFromSurface(app->renderer, placeholderSurface);
    SDL_FreeSurface(placeholderSurface);

    if (!app->homeScreenTexture || !app->createPlaylistTexture || 
        !app->coverSelectionTexture || !app->coverPlaceholder) {
        printf("Error cargando texturas\n");
        return false;
    }

    app->playlistCounter = 1;
    return true;
}

void setupHomeScreen(App* app) {
    // Configurar botones de la pantalla principal
    app->homeScreen.backButton.rect = (SDL_Rect){20, 20, 80, 40};
    app->homeScreen.backButton.font = app->font;
    
    app->homeScreen.newPlaylistButton.rect = (SDL_Rect){65, 188, 65, 65};
    
    app->homeScreen.addButton.rect = (SDL_Rect){850, 550, 150, 50};
    app->homeScreen.addButton.font = app->font;

    // Configurar playlists de ejemplo
    app->homeScreen.playlistCount = 3;
    for (int i = 0; i < app->homeScreen.playlistCount; i++) {
        app->homeScreen.playlists[i].cover = app->coverPlaceholder;
        snprintf(app->homeScreen.playlists[i].name, sizeof(app->homeScreen.playlists[i].name), "Playlist %d", i+1);
        app->homeScreen.playlists[i].songCount = i * 5 + 10;
        app->homeScreen.playlists[i].selected = false;
    }
}

void setupCreatePlaylistScreen(App* app) {
    app->createScreen.cancelButton.rect = (SDL_Rect){585, 485, 120, 40};
    app->createScreen.cancelButton.font = app->font;
    
    app->createScreen.createButton.rect = (SDL_Rect){300, 485, 120, 40};
    app->createScreen.createButton.font = app->font;
    
    app->createScreen.selectCoverButton.rect = (SDL_Rect){405, 115, 200, 200};
    
    app->createScreen.playlistNameRect = (SDL_Rect){352, 420, 300, 40};
    snprintf(app->createScreen.playlistName, sizeof(app->createScreen.playlistName), "Playlist #%d", app->playlistCounter);
    updateTextTexture(app, &app->createScreen.playlistNameTexture, app->createScreen.playlistName, (SDL_Color){0, 0, 0, 255});
    
    app->createScreen.isEditingName = false;
    app->createScreen.selectedCoverPath[0] = '\0';
}

void setupCoverSelectionScreen(App* app) {
    // Configurar opciones de portada
    const char* coverPaths[MAX_COVER_OPTIONS] = {
        "imgs/portada1.png",
        "imgs/portada2.png",
    };
    
    for (int i = 0; i < MAX_COVER_OPTIONS; i++) {
        app->coverSelection.covers[i].texture = loadTexture(app, coverPaths[i]);
        strncpy(app->coverSelection.covers[i].path, coverPaths[i], sizeof(app->coverSelection.covers[i].path));
        app->coverSelection.covers[i].rect = (SDL_Rect){300 + i*250, 200, 200, 200};
        
        app->coverSelection.selectButtons[i].rect = (SDL_Rect){300 + i*250, 420, 200, 40};
        app->coverSelection.selectButtons[i].font = app->font;
    }
    
    app->coverSelection.backButton.rect = (SDL_Rect){200, 550, 120, 40};
    app->coverSelection.backButton.font = app->font;
    
    app->coverSelection.acceptButton.rect = (SDL_Rect){725, 550, 120, 40};
    app->coverSelection.acceptButton.font = app->font;
    
    app->coverSelection.selectedCover = -1;
}

// ------------------------ RENDERIZADO ------------------------

void renderHomeScreen(App* app) {
    // Fondo
    SDL_RenderCopy(app->renderer, app->homeScreenTexture, NULL, NULL);
    
    // Botones
    renderButton(app->renderer, &app->homeScreen.backButton, "Atras", (SDL_Color){180, 180, 180, 255});
    
    // Botón de nueva playlist con icono +
    SDL_RenderCopy(app->renderer, app->plusIcon, NULL, &app->homeScreen.newPlaylistButton.rect);
    
    // Playlists
    for (int i = 0; i < app->homeScreen.playlistCount; i++) {
        SDL_Rect playlistRect = {65, 290 + i*110, 600, 100}; 
        
        // Fondo de la playlist
        SDL_SetRenderDrawColor(app->renderer, 
                             app->homeScreen.playlists[i].selected ? 200 : 240, 
                             app->homeScreen.playlists[i].selected ? 230 : 240, 
                             app->homeScreen.playlists[i].selected ? 255 : 240, 
                             255);
        SDL_RenderFillRect(app->renderer, &playlistRect);
        SDL_SetRenderDrawColor(app->renderer, 180, 180, 180, 255);
        SDL_RenderDrawRect(app->renderer, &playlistRect);
        
        // Portada
        SDL_Rect coverRect = {playlistRect.x + 10, playlistRect.y + 10, 80, 80};
        SDL_RenderCopy(app->renderer, app->homeScreen.playlists[i].cover, NULL, &coverRect);
        
        // Nombre y número de canciones
        SDL_Color black = {0, 0, 0, 255};
        char infoText[100];
        snprintf(infoText, sizeof(infoText), "%s (%d canciones)", 
                app->homeScreen.playlists[i].name, app->homeScreen.playlists[i].songCount);
        
        SDL_Surface* textSurface = TTF_RenderText_Blended(app->font, infoText, black);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(app->renderer, textSurface);
            SDL_Rect textRect = {
                playlistRect.x + 100,
                playlistRect.y + 20,
                textSurface->w,
                textSurface->h
            };
            SDL_RenderCopy(app->renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
        
        // Indicador de selección
        if (app->homeScreen.playlists[i].selected) {
            SDL_SetRenderDrawColor(app->renderer, 70, 130, 180, 255);
            SDL_Rect selectionRect = {playlistRect.x + 5, playlistRect.y + 5, 10, 10};
            SDL_RenderFillRect(app->renderer, &selectionRect);
        }
    }
    
    // Botón de agregar
    if (isAnyPlaylistSelected(app)) {
        renderButton(app->renderer, &app->homeScreen.addButton, "Agregar", (SDL_Color){80, 180, 90, 255});
    }
}

void renderCreatePlaylistScreen(App* app) {
    // Fondo
    SDL_RenderCopy(app->renderer, app->createPlaylistTexture, NULL, NULL);
    
    // Portada
    SDL_Texture* coverToShow = app->coverPlaceholder;
    if (strlen(app->createScreen.selectedCoverPath)) {
        SDL_Texture* loadedCover = IMG_LoadTexture(app->renderer, app->createScreen.selectedCoverPath);
        if (loadedCover) coverToShow = loadedCover;
    }
    SDL_RenderCopy(app->renderer, coverToShow, NULL, &app->createScreen.selectCoverButton.rect);
    
    // Nombre de la playlist
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(app->renderer, &app->createScreen.playlistNameRect);
    SDL_SetRenderDrawColor(app->renderer, app->createScreen.isEditingName ? 70 : 180, 
                          app->createScreen.isEditingName ? 130 : 180, 
                          app->createScreen.isEditingName ? 180 : 180, 255);
    SDL_RenderDrawRect(app->renderer, &app->createScreen.playlistNameRect);
    
    if (app->createScreen.playlistNameTexture) {
        SDL_RenderCopy(app->renderer, app->createScreen.playlistNameTexture, NULL, &app->createScreen.playlistNameRect);
    }
    
    // Botones
    renderButton(app->renderer, &app->createScreen.cancelButton, "Cancelar", (SDL_Color){220, 80, 60, 255});
    renderButton(app->renderer, &app->createScreen.createButton, "Crear", (SDL_Color){80, 180, 90, 255});
}

void renderCoverSelectionScreen(App* app) {
    // Fondo
    SDL_RenderCopy(app->renderer, app->coverSelectionTexture, NULL, NULL);
    
    // Opciones de portada
    for (int i = 0; i < MAX_COVER_OPTIONS; i++) {
        if (app->coverSelection.covers[i].texture) {
            SDL_RenderCopy(app->renderer, app->coverSelection.covers[i].texture, NULL, &app->coverSelection.covers[i].rect);
            
            // Resaltar selección
            if (app->coverSelection.selectedCover == i) {
                SDL_SetRenderDrawColor(app->renderer, 70, 130, 180, 255);
                SDL_RenderDrawRect(app->renderer, &app->coverSelection.covers[i].rect);
            }
        }
        
        // Botones de selección
        SDL_Color btnColor = (app->coverSelection.selectedCover == i) ? 
            (SDL_Color){100, 200, 100, 255} : (SDL_Color){200, 200, 200, 255};
        
        renderButton(app->renderer, &app->coverSelection.selectButtons[i], "Seleccionar", btnColor);
    }
    
    // Botones de acción
    renderButton(app->renderer, &app->coverSelection.backButton, "Volver", (SDL_Color){220, 80, 60, 255});
    
    if (app->coverSelection.selectedCover >= 0) {
        renderButton(app->renderer, &app->coverSelection.acceptButton, "Aceptar", (SDL_Color){80, 180, 90, 255});
    }
}

void render(App* app) {
    SDL_SetRenderDrawColor(app->renderer, 240, 240, 240, 255);
    SDL_RenderClear(app->renderer);

    switch (app->currentState) {
        case STATE_HOME:
            renderHomeScreen(app);
            break;
        case STATE_CREATE_PLAYLIST:
            renderCreatePlaylistScreen(app);
            break;
        case STATE_SELECT_COVER:
            renderCoverSelectionScreen(app);
            break;
    }

    SDL_RenderPresent(app->renderer);
}

// ------------------------ MANEJO DE EVENTOS ------------------------

void handleHomeEvents(App* app, SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        SDL_Point p = {e->button.x, e->button.y};
        
        // Botón de retroceso
        if (SDL_PointInRect(&p, &app->homeScreen.backButton.rect)) {
            // Implementar acción de retroceso si es necesario
        }
        
        // Botón de nueva playlist
        if (SDL_PointInRect(&p, &app->homeScreen.newPlaylistButton.rect)) {
            app->currentState = STATE_CREATE_PLAYLIST;
            setupCreatePlaylistScreen(app);
            return;
        }
        
        // Selección de playlists
        for (int i = 0; i < app->homeScreen.playlistCount; i++) {
            SDL_Rect playlistRect = {65, 290 + i*110, 600, 100};
            if (SDL_PointInRect(&p, &playlistRect)) {
                app->homeScreen.playlists[i].selected = !app->homeScreen.playlists[i].selected;
            }
        }
        
        // Botón de agregar
        if (isAnyPlaylistSelected(app) && SDL_PointInRect(&p, &app->homeScreen.addButton.rect)) {
            printf("Playlists seleccionadas agregadas\n");
            // Implementar lógica de agregar
        }
    }
}

void handleCreatePlaylistEvents(App* app, SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        SDL_Point p = {e->button.x, e->button.y};
        
        // Campo de nombre
        if (SDL_PointInRect(&p, &app->createScreen.playlistNameRect)) {
            app->createScreen.isEditingName = true;
            SDL_StartTextInput();
        } else {
            app->createScreen.isEditingName = false;
            SDL_StopTextInput();
        }
        
        // Botón de cancelar
        if (SDL_PointInRect(&p, &app->createScreen.cancelButton.rect)) {
            app->currentState = STATE_HOME;
            return;
        }
        
        // Botón de crear
        if (SDL_PointInRect(&p, &app->createScreen.createButton.rect)) {
            // Agregar nueva playlist
            if (app->homeScreen.playlistCount < MAX_PLAYLISTS) {
                PlaylistItem* newItem = &app->homeScreen.playlists[app->homeScreen.playlistCount];
                strncpy(newItem->name, app->createScreen.playlistName, sizeof(newItem->name));
                newItem->songCount = 0;
                newItem->selected = false;
                
                if (strlen(app->createScreen.selectedCoverPath)) {
                    newItem->cover = IMG_LoadTexture(app->renderer, app->createScreen.selectedCoverPath);
                } else {
                    newItem->cover = app->coverPlaceholder;
                }
                
                app->homeScreen.playlistCount++;
                app->playlistCounter++;
            }
            
            app->currentState = STATE_HOME;
            return;
        }
        
        // Selección de portada
        if (SDL_PointInRect(&p, &app->createScreen.selectCoverButton.rect)) {
            app->currentState = STATE_SELECT_COVER;
            return;
        }
    }
    
    // Edición de texto
    if (e->type == SDL_TEXTINPUT && app->createScreen.isEditingName) {
        if (strlen(app->createScreen.playlistName) + strlen(e->text.text) < sizeof(app->createScreen.playlistName)) {
            strcat(app->createScreen.playlistName, e->text.text);
            updateTextTexture(app, &app->createScreen.playlistNameTexture, 
                            app->createScreen.playlistName, (SDL_Color){0, 0, 0, 255});
        }
    }
    
    if (e->type == SDL_KEYDOWN && app->createScreen.isEditingName) {
        if (e->key.keysym.sym == SDLK_BACKSPACE && strlen(app->createScreen.playlistName) > 0) {
            app->createScreen.playlistName[strlen(app->createScreen.playlistName)-1] = '\0';
            updateTextTexture(app, &app->createScreen.playlistNameTexture, 
                            app->createScreen.playlistName, (SDL_Color){0, 0, 0, 255});
        }
    }
}

void handleCoverSelectionEvents(App* app, SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        SDL_Point p = {e->button.x, e->button.y};
        
        // Botón de volver
        if (SDL_PointInRect(&p, &app->coverSelection.backButton.rect)) {
            app->currentState = STATE_CREATE_PLAYLIST;
            return;
        }
        
        // Botón de aceptar
        if (app->coverSelection.selectedCover >= 0 && 
            SDL_PointInRect(&p, &app->coverSelection.acceptButton.rect)) {
            strncpy(app->createScreen.selectedCoverPath, 
                   app->coverSelection.covers[app->coverSelection.selectedCover].path, 
                   sizeof(app->createScreen.selectedCoverPath));
            app->currentState = STATE_CREATE_PLAYLIST;
            return;
        }
        
        // Selección de portada
        for (int i = 0; i < MAX_COVER_OPTIONS; i++) {
            if (SDL_PointInRect(&p, &app->coverSelection.covers[i].rect) || 
                SDL_PointInRect(&p, &app->coverSelection.selectButtons[i].rect)) {
                app->coverSelection.selectedCover = i;
                break;
            }
        }
    }
}

void handleEvents(App* app) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            app->running = false;
            return;
        }
        
        switch (app->currentState) {
            case STATE_HOME:
                handleHomeEvents(app, &e);
                break;
            case STATE_CREATE_PLAYLIST:
                handleCreatePlaylistEvents(app, &e);
                break;
            case STATE_SELECT_COVER:
                handleCoverSelectionEvents(app, &e);
                break;
        }
    }
}

// ------------------------ LIMPIEZA ------------------------

void closeApp(App* app) {
    if (app->createScreen.playlistNameTexture) SDL_DestroyTexture(app->createScreen.playlistNameTexture);
    if (app->homeScreenTexture) SDL_DestroyTexture(app->homeScreenTexture);
    if (app->createPlaylistTexture) SDL_DestroyTexture(app->createPlaylistTexture);
    if (app->coverSelectionTexture) SDL_DestroyTexture(app->coverSelectionTexture);
    if (app->coverPlaceholder) SDL_DestroyTexture(app->coverPlaceholder);
    if (app->plusIcon) SDL_DestroyTexture(app->plusIcon);
    
    for (int i = 0; i < app->homeScreen.playlistCount; i++) {
        if (app->homeScreen.playlists[i].cover != app->coverPlaceholder) {
            SDL_DestroyTexture(app->homeScreen.playlists[i].cover);
        }
    }
    
    for (int i = 0; i < MAX_COVER_OPTIONS; i++) {
        if (app->coverSelection.covers[i].texture) {
            SDL_DestroyTexture(app->coverSelection.covers[i].texture);
        }
    }
    
    if (app->font) TTF_CloseFont(app->font);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// ------------------------ MAIN ------------------------

int main(int argc, char* argv[]) {
    App app = {0};
    app.running = true;
    app.currentState = STATE_HOME;

    if (!initSDL(&app) || !loadMedia(&app)) {
        printf("Error al inicializar SDL o cargar medios\n");
        return 1;
    }

    setupHomeScreen(&app);
    setupCoverSelectionScreen(&app);

    while (app.running) {
        handleEvents(&app);
        render(&app);
        SDL_Delay(16);
    }

    closeApp(&app);
    return 0;
}