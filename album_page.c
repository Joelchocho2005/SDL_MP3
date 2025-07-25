#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define WINDOW_WIDTH 1026
#define WINDOW_HEIGHT 652

typedef enum {
    STATE_ALBUM_PAGE,
    STATE_ALBUM_PAGE_SIGUIENTE,
    STATE_ARTIST_PAGE,
    STATE_POPULARES_PAGE,
    STATE_DISCOGRAFIA_PAGE
} AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
} Button;

typedef struct {
    
    SDL_Color blanco;
    SDL_Color gris; // gris medio
    SDL_Rect rect; // x, y, ancho, alto

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* background;
    SDL_Texture* albumpage;
    SDL_Texture* artistpage;
    SDL_Texture* popularespage;
    SDL_Texture* discografiapage;


    Button siguienteButton;
    Button artistaButton;
    Button popularesButton;
    Button discografiaButton;

    char nombreCancion[20];
    char reproduccionesCancion[10];
    char duracionCancion[10];
    int cantidad;
    char nums[20];
    int posicionesX;
    int posicionesY;
    int ancho;
    int alto;
    char mensaje[4];  // suficiente para 3 dígitos + '\0'

    SDL_Texture* nombreCancionTexture;
    SDL_Texture* nombreArtistaTexture;
    SDL_Texture* nombreAlbumTexture;

    SDL_Rect backButtonRect;
    bool isPlaying;
    Mix_Music* music;
    bool running;
    AppState currentState;
    TTF_Font* font;
    TTF_Font* bigfont;
    SDL_Rect cvRect;                    // Área para CVV
    char cvv[4];                         // Almacenar CVV
    bool boolActivo;                    // Para controlar entrada de texto
    SDL_Event event;
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
        "Página de Usuario",
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
    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!app->renderer) {
        printf("No se pudo crear el renderer: %s\n", SDL_GetError());
        return false;
    }

    // Cargar fuente
    app->font = TTF_OpenFont("src/assets/Inder-Regular.ttf", 15);
    if (!app->font) {
        printf("No se pudo cargar la fuente: %s\n", TTF_GetError());
        // Continuamos sin fuente, pero el mensaje se mostrará sin texto
    }

    app->bigfont = TTF_OpenFont("src/assets/Inder-Regular.ttf", 60);
    if (!app->bigfont) {
        printf("No se pudo cargar la fuente: %s\n", TTF_GetError());
        // Continuamos sin fuente, pero el mensaje se mostrará sin texto
    }

    return true;
}

void updateTextTexture(App* app, SDL_Texture** texture, const char* text) {
    if (*texture) SDL_DestroyTexture(*texture);
    
    if (app->font && text) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(app->font, text, white);
        if (surface) {
            *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
}

void updateTextTextureBig(App* app, SDL_Texture** texture, const char* text) {
    if (*texture) SDL_DestroyTexture(*texture);
    
    if (app->bigfont && text) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(app->bigfont, text, white);
        if (surface) {
            *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
}


void renderizarRectangulos(SDL_Renderer* renderer, int cantidad, int posicionesX, int posicionesY, int ancho, int alto) {
    
    SDL_Rect rect;
    rect.x = posicionesX;
    rect.y = posicionesY;
    rect.w = ancho;
    rect.h = alto;

    // Color de los rectángulos (por ejemplo, blanco)
    SDL_SetRenderDrawColor(renderer, 63, 80, 93, 255);
    SDL_RenderFillRect(renderer, &rect);
}

SDL_Texture* loadTexture(App* app, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Error al cargar la imagen %s: %s\n", path, IMG_GetError());
        // Crear superficie de fallback roja
        surface = SDL_CreateRGBSurface(0, 100, 100, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 0, 0));
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error al crear textura desde %s: %s\n", path, SDL_GetError());
    }
    return texture;
}

bool loadMedia(App* app) {
    // Cargar texturas
    app->albumpage = loadTexture(app, "imgs/Album Main Page.png");
    app->artistpage = loadTexture(app, "imgs/Artist Main Page.png");
    app->popularespage = loadTexture(app, "imgs/Populares Page.png");
    app->discografiapage = loadTexture(app, "imgs/Discografia Page.png");

    app->isPlaying = false;
    return true;
}

void setupButtonsForState(App* app) {
    // Resetear todos los botones

    memset(&app->backButtonRect, 0, sizeof(SDL_Rect));
    memset(&app->siguienteButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->artistaButton, 0, sizeof(SDL_Rect));
    memset(&app->popularesButton, 0, sizeof(SDL_Rect));
    memset(&app->discografiaButton, 0, sizeof(SDL_Rect));
    
    switch (app->currentState) {
        
        case STATE_ALBUM_PAGE:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->siguienteButton.rect = (SDL_Rect){800, 600, 200, 40};
            app->artistaButton.rect = (SDL_Rect){310, 140, 100, 30};
            break;

        case STATE_ALBUM_PAGE_SIGUIENTE:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->artistaButton.rect = (SDL_Rect){310, 140, 100, 30};
            break;

        case STATE_ARTIST_PAGE:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->popularesButton.rect = (SDL_Rect){128, 310, 100, 20};
            app->discografiaButton.rect = (SDL_Rect){128, 470, 100, 20};
            break;

        case STATE_POPULARES_PAGE:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};

        case STATE_DISCOGRAFIA_PAGE:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
    }
}

void render(App* app) {
    SDL_RenderClear(app->renderer);

    // Fondo según estado
    switch (app->currentState) {

        case STATE_ALBUM_PAGE:
            if (app->albumpage) {
                SDL_RenderCopy(app->renderer, app->albumpage, NULL, NULL);
            }
            
            updateTextTextureBig(app, &app->nombreAlbumTexture, "YHLQMDLG");
            if (app->nombreAlbumTexture) {
                SDL_Rect textRect = {320, 70, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreAlbumTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreAlbumTexture, NULL, &textRect);
            }

            updateTextTexture(app, &app->nombreArtistaTexture, "Bad Bunny");
            if (app->nombreArtistaTexture) {
                SDL_Rect textRect = {322, 144, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreArtistaTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreArtistaTexture, NULL, &textRect);
            }

            app->cantidad = 5;
            for(int i = 0; i<20; i++) {
                app->nums[i] = i+1;
            }
            app->posicionesX = 35;
            app->ancho = 960;
            app->alto = 42;

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 330 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                sprintf(app->mensaje, "%d", app->nums[i]);  // convierte número a string, ej: "1"

                updateTextTexture(app, &app->nombreCancionTexture, app->mensaje);
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {60, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "Yo Perreo Sola");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {129, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {440, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }
            break;
            
        case STATE_ALBUM_PAGE_SIGUIENTE:
            if (app->albumpage) {
                SDL_RenderCopy(app->renderer, app->albumpage, NULL, NULL);
            }
             
            updateTextTextureBig(app, &app->nombreAlbumTexture, "YHLQMDLG");
            if (app->nombreAlbumTexture) {
                SDL_Rect textRect = {320, 70, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreAlbumTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreAlbumTexture, NULL, &textRect);
            }

            updateTextTexture(app, &app->nombreArtistaTexture, "Bad Bunny");
            if (app->nombreArtistaTexture) {
                SDL_Rect textRect = {322, 144, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreArtistaTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreArtistaTexture, NULL, &textRect);
            }

            app->cantidad = 3;
            for(int i = 0; i<20; i++) {
                app->nums[i] = i+1;
            }
            app->posicionesX = 35;
            app->ancho = 960;
            app->alto = 42;

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 330 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                sprintf(app->mensaje, "%d", app->nums[i]);  // convierte número a string, ej: "1"

                updateTextTexture(app, &app->nombreCancionTexture, app->mensaje);
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {60, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "Yo Perreo Sola");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {129, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {440, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 342 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }
            break;

        case STATE_ARTIST_PAGE:
            if (app->artistpage) {
                SDL_RenderCopy(app->renderer, app->artistpage, NULL, NULL);
            }

            updateTextTextureBig(app, &app->nombreArtistaTexture, "Bad Bunny");
            if (app->nombreArtistaTexture) {
                SDL_Rect textRect = {320, 70, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreArtistaTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreArtistaTexture, NULL, &textRect);
            }

            updateTextTexture(app, &app->nombreArtistaTexture, "# Reproducciones");
            if (app->nombreArtistaTexture) {
                SDL_Rect textRect = {322, 165, 0, 0}; // Ajusta la posición
                SDL_QueryTexture(app->nombreArtistaTexture, NULL, NULL, &textRect.w, &textRect.h);
                SDL_RenderCopy(app->renderer, app->nombreArtistaTexture, NULL, &textRect);
            }

            app->cantidad = 2;
            app->posicionesX = 35;
            app->ancho = 960;
            app->alto = 42;

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 350 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                updateTextTexture(app, &app->nombreCancionTexture, "Cancion #");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {129, 362 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {440, 362 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 362 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 510 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                updateTextTexture(app, &app->nombreCancionTexture, "Album #");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {129, 520 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {440, 520 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 520 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }
            break;
        case STATE_POPULARES_PAGE:
            if (app->popularespage) {
                SDL_RenderCopy(app->renderer, app->popularespage, NULL, NULL);
            }

            app->cantidad = 8;
            for(int i = 0; i<20; i++) {
                app->nums[i] = i+1;
            }
            app->posicionesX = 35;
            app->ancho = 960;
            app->alto = 42;

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 213 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                sprintf(app->mensaje, "%d", app->nums[i]);  // convierte número a string, ej: "1"

                updateTextTexture(app, &app->nombreCancionTexture, app->mensaje);
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {62, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "Cancion #");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {128, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {435, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }

            break;
        
        case STATE_DISCOGRAFIA_PAGE:
            if (app->discografiapage) {
                SDL_RenderCopy(app->renderer, app->discografiapage, NULL, NULL);
            }

            app->cantidad = 8;
            for(int i = 0; i<20; i++) {
                app->nums[i] = i+1;
            }
            app->posicionesX = 35;
            app->ancho = 960;
            app->alto = 42;

            for (int i = 0; i < app->cantidad; i++) {
                app->posicionesY = 213 + i*50;

                renderizarRectangulos(app->renderer, app->cantidad, app->posicionesX, app->posicionesY, app->ancho, app->alto);

                sprintf(app->mensaje, "%d", app->nums[i]);  // convierte número a string, ej: "1"

                updateTextTexture(app, &app->nombreCancionTexture, app->mensaje);
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {62, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "Cancion #");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {128, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {435, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }

                updateTextTexture(app, &app->nombreCancionTexture, "#########");
                if (app->nombreCancionTexture) {
                    SDL_Rect textRect = {800, 225 + i*50, 0, 0}; // Ajusta la posición
                    SDL_QueryTexture(app->nombreCancionTexture, NULL, NULL, &textRect.w, &textRect.h);
                    SDL_RenderCopy(app->renderer, app->nombreCancionTexture, NULL, &textRect);
                }
            }
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
                
                case STATE_ALBUM_PAGE:
                    if (SDL_PointInRect(&p, &app->siguienteButton.rect)) {
                        printf("Botón presionado: Siguiente\n");
                        app->currentState = STATE_ALBUM_PAGE_SIGUIENTE;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }
                    if (SDL_PointInRect(&p, &app->artistaButton.rect)) {
                        printf("Botón presionado: Artista\n");
                        app->currentState = STATE_ARTIST_PAGE;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }
                    if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                        printf("Botón presionado: Retroceso\n");
                        app->currentState = STATE_ARTIST_PAGE;
                        setupButtonsForState(app);
                    }
                    break;

                case STATE_ALBUM_PAGE_SIGUIENTE:
                    if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                        printf("Botón presionado: Retroceso\n");
                        app->currentState = STATE_ALBUM_PAGE;
                        setupButtonsForState(app);
                    }
                    if (SDL_PointInRect(&p, &app->artistaButton.rect)) {
                        printf("Botón presionado: Artista\n");
                        app->currentState = STATE_ARTIST_PAGE;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }
                    break;

                case STATE_ARTIST_PAGE:
                    if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                        printf("Botón presionado: Retroceso\n");
                        app->currentState = STATE_ALBUM_PAGE;
                        setupButtonsForState(app);
                    }
                    if (SDL_PointInRect(&p, &app->popularesButton.rect)) {
                        printf("Botón presionado: Populares\n");
                        app->currentState = STATE_POPULARES_PAGE;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }
                    if (SDL_PointInRect(&p, &app->discografiaButton.rect)) {
                        printf("Botón presionado: Discografia\n");
                        app->currentState = STATE_DISCOGRAFIA_PAGE;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }
                    break;

                case STATE_POPULARES_PAGE:
                    if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                        printf("Botón presionado: Retroceso\n");
                        app->currentState = STATE_ARTIST_PAGE;
                        setupButtonsForState(app);
                    }
                    break;

                case STATE_DISCOGRAFIA_PAGE:
                    if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                        printf("Botón presionado: Retroceso\n");
                        app->currentState = STATE_ARTIST_PAGE;
                        setupButtonsForState(app);
                    }
                    break;

            }
        }
    }
}
void closeApp(App* app) {
    
    if (app->background) SDL_DestroyTexture(app->background);
    if (app->music) Mix_FreeMusic(app->music);
    if (app->font) TTF_CloseFont(app->font);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    App app = {0};
    app.running = true;
    app.currentState = STATE_ALBUM_PAGE;
    
    if (!initSDL(&app)) {
        printf("Error al inicializar SDL\n");
        return 1;
    }
    if (!loadMedia(&app)) {
        printf("Error al cargar medios\n");
        closeApp(&app);
        return 1;
    }

    setupButtonsForState(&app);

    while (app.running) {
        handleEvents(&app);
        render(&app);
        SDL_Delay(16); // ~60 FPS
    }

    closeApp(&app);
    return 0;
}