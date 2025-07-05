#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 1034
#define WINDOW_HEIGHT 660

typedef enum {
    STATE_HOME,
    STATE_PLAYER,
    STATE_PLAN_GRATUITO,
    STATE_PLAN_PREMIUM,
    STATE_COMPRAR_PREMIUM
} AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
} Button;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* background;
    SDL_Texture* planGratuitoTexture;
    SDL_Texture* planPremiumTexture;
    Button friendsButton;
    Button planButton;
    Button logoutButton;
    Button historyButton;
    Button comprarButton;
    Button pagarButton;
    SDL_Rect backButtonRect;
    bool isPlaying;
    bool hasPremium;
    Mix_Music* music;
    bool running;
    AppState currentState;
    int planTipo;
    char email[50];
    char nickname[30];
    SDL_Texture* emailTexture;
    SDL_Texture* nicknameTexture;
    bool showPaymentSuccess;
    Uint32 paymentSuccessTimer;
    TTF_Font* font;
    SDL_Texture* comprarPremiumTexture;  // Textura para la página de compra
               // Área para el número de tarjeta
    SDL_Rect expireDateRect;             // Área para fecha de caducidad
    SDL_Rect cvRect;                    // Área para CVV
    char tarjetaNumber[20];                 // Almacenar número de tarjeta
    char caducidad[6];                  // Fecha de Almacenar (MM/YYi)
    char cvv[4];                         // Almacenar CVV
    bool boolActivo;                    // Para controlar entrada de texto
    int activoInputField;                // 1=cardNumber, 2=expiryFecha, 3=cvv
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
    app->font = TTF_OpenFont("C:\\Users\\Sebastian\\Desktop\\Ciclo 3 universidad\\Programacion 2\\ProyectoFinal(1)\\src\\assets\\Inder-Regular.ttf", 24);
    if (!app->font) {
        printf("No se pudo cargar la fuente: %s\n", TTF_GetError());
        // Continuamos sin fuente, pero el mensaje se mostrará sin texto
    }

    strcpy(app->email, "usuario@ejemplo.com");
    strcpy(app->nickname, "Usuario123");

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
    app->background = loadTexture(app, "imgs/user page.png");
    app->planGratuitoTexture = loadTexture(app, "imgs/gratuito.png");
    app->planPremiumTexture = loadTexture(app, "imgs/premium.png");
    app->comprarPremiumTexture = loadTexture(app, "imgs/comprar premium.png");
    

    app->isPlaying = false;
    app->hasPremium = false;
    app->showPaymentSuccess = false;
    return true;
}

void setupButtonsForState(App* app) {
    // Resetear todos los botones
    memset(&app->friendsButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->planButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->logoutButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->historyButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->comprarButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->pagarButton.rect, 0, sizeof(SDL_Rect));
    memset(&app->backButtonRect, 0, sizeof(SDL_Rect));

    switch (app->currentState) {
        case STATE_HOME:
            app->friendsButton.rect = (SDL_Rect){446, 421, 142, 47};
            app->planButton.rect = (SDL_Rect){446, 342, 142, 48};
            app->logoutButton.rect = (SDL_Rect){445, 590, 146, 48};
            app->historyButton.rect = (SDL_Rect){426, 500, 184, 48};
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            break;

        case STATE_PLAYER:
            app->backButtonRect = (SDL_Rect){20, 20, 80, 40};
            break;

        case STATE_PLAN_GRATUITO:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->comprarButton.rect = (SDL_Rect){442, 588, 150, 50};
            break;

        case STATE_PLAN_PREMIUM:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->pagarButton.rect = (SDL_Rect){445, 562, 150, 50};
            break;

        case STATE_COMPRAR_PREMIUM:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->comprarButton.rect = (SDL_Rect){442, 588, 150, 50};
            break;
    }
}

void renderPaymentSuccess(App* app) {
    if (!app->showPaymentSuccess) return;

    // Fondo semitransparente
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(app->renderer, &overlay);

    // Rectángulo del mensaje (negro con borde blanco)
    SDL_Rect messageRect = {WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 - 50, 300, 100};
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(app->renderer, &messageRect);
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(app->renderer, &messageRect);

    // Texto
    if (app->font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(app->font, "Pago exitoso!", white);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
            if (texture) {
                SDL_Rect textRect = {
                    messageRect.x + (messageRect.w - surface->w)/2,
                    messageRect.y + (messageRect.h - surface->h)/2,
                    surface->w,
                    surface->h
                };
                SDL_RenderCopy(app->renderer, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // Temporizador
    if (SDL_GetTicks() > app->paymentSuccessTimer) {
        app->showPaymentSuccess = false;
    }
}

void render(App* app) {
    SDL_RenderClear(app->renderer);

    // Fondo según estado
    switch (app->currentState) {
        case STATE_HOME:

            if (app->background) {
                SDL_RenderCopy(app->renderer, app->background, NULL, NULL);
            }
            if (app->friendsButton.texture && app->friendsButton.rect.w > 0) {
                SDL_RenderCopy(app->renderer, app->friendsButton.texture, NULL, &app->friendsButton.rect);
            }
            if (app->planButton.texture && app->planButton.rect.w > 0) {
                SDL_RenderCopy(app->renderer, app->planButton.texture, NULL, &app->planButton.rect);
            }
            if (app->logoutButton.texture && app->logoutButton.rect.w > 0) {
                SDL_RenderCopy(app->renderer, app->logoutButton.texture, NULL, &app->logoutButton.rect);
            }
            if (app->historyButton.texture && app->historyButton.rect.w > 0) {
                SDL_RenderCopy(app->renderer, app->historyButton.texture, NULL, &app->historyButton.rect);
            }
            // Mostrar información del usuario
            if (!app->emailTexture) {
                updateTextTexture(app, &app->emailTexture, app->email);
            }
            if (!app->nicknameTexture) {
                updateTextTexture(app, &app->nicknameTexture, app->nickname);
            }
            
            // Posiciones para la información (ajusta según tu diseño)
            SDL_Rect emailRect = {100, 220, 200, 30};  // Ajusta estas coordenadas
            SDL_Rect nicknameRect = {720, 220, 200, 30}; // Ajusta estas coordenadas
                                                                                                                                                                                                                                                                                                                                                                                                   
            if (app->emailTexture) {
                SDL_QueryTexture(app->emailTexture, NULL, NULL, &emailRect.w, &emailRect.h);
                SDL_RenderCopy(app->renderer, app->emailTexture, NULL, &emailRect);
            }
            

            if (app->nicknameTexture) {
                SDL_QueryTexture(app->nicknameTexture, NULL, NULL, &nicknameRect.w, &nicknameRect.h);
                SDL_RenderCopy(app->renderer, app->nicknameTexture, NULL, &nicknameRect);
            }            
            break;
        case STATE_PLAN_GRATUITO:
            if (app->planGratuitoTexture) {
                SDL_RenderCopy(app->renderer, app->planGratuitoTexture, NULL, NULL);
            }
            break;
        case STATE_PLAN_PREMIUM:
            if (app->planPremiumTexture) {
                SDL_RenderCopy(app->renderer, app->planPremiumTexture, NULL, NULL);
            }
            break;
        case STATE_COMPRAR_PREMIUM:
            if (app->comprarPremiumTexture) {
                SDL_RenderCopy(app->renderer, app->comprarPremiumTexture, NULL, NULL);
            } 

            

            break;
    }

    // Botones
    if (app->comprarButton.rect.w > 0) {
        SDL_RenderCopy(app->renderer, app->comprarButton.texture, NULL, &app->comprarButton.rect);
    }

    // Mensaje de pago exitoso
    if (app->showPaymentSuccess) {
        renderPaymentSuccess(app);
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

            // Botón de retroceso
            if (app->backButtonRect.w > 0 && SDL_PointInRect(&p, &app->backButtonRect)) {
                printf("Botón presionado: Retroceso\n");
                app->currentState = STATE_HOME;
                setupButtonsForState(app);
                continue;
            }

            switch (app->currentState) {
                case STATE_HOME:
                    if (SDL_PointInRect(&p, &app->friendsButton.rect)) {
                        printf("Botón presionado: Amigos\n");
                    } else if (SDL_PointInRect(&p, &app->planButton.rect)) {
                        printf("Botón presionado: Plan\n");
                        app->currentState = app->planTipo == 1 ? STATE_PLAN_PREMIUM : STATE_PLAN_GRATUITO;
                        setupButtonsForState(app);
                    } else if (SDL_PointInRect(&p, &app->logoutButton.rect)) {
                        printf("Botón presionado: Cerrar sesión\n");
                    } else if (SDL_PointInRect(&p, &app->historyButton.rect)) {
                        printf("Botón presionado: Historial\n");
                    }
                    break;

                case STATE_PLAN_GRATUITO:
                    if (SDL_PointInRect(&p, &app->comprarButton.rect)) {
                        
                        printf("Botón presionado: Comprar plan gratuito\n");
                        app->currentState = STATE_COMPRAR_PREMIUM;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones

                    }
                    break;
                case STATE_COMPRAR_PREMIUM:
                    if (SDL_PointInRect(&p, &app->comprarButton.rect)) {
                        
                        printf("Botón presionado: Comprar plan gratuito\n");
                        app->planTipo = 1;  // Cambiar a premium
                        app->currentState = STATE_PLAN_PREMIUM;  // Ir a pantalla premium
                        setupButtonsForState(app);  // Actualizar botones
                    }

                    

                    break;
                case STATE_PLAN_PREMIUM:
                    if (SDL_PointInRect(&p, &app->pagarButton.rect)) {
                        printf("Botón presionado: Pagar plan premium\n");
                        app->showPaymentSuccess = true;
                        app->paymentSuccessTimer = SDL_GetTicks() + 2000; // 2 segundos
                    }
                    break;
            }
        }
    }
}
void closeApp(App* app) {
    if (app->planPremiumTexture) SDL_DestroyTexture(app->planPremiumTexture);
    if (app->planGratuitoTexture) SDL_DestroyTexture(app->planGratuitoTexture);
    if (app->background) SDL_DestroyTexture(app->background);
    if (app->comprarButton.texture) SDL_DestroyTexture(app->comprarButton.texture);
    if (app->pagarButton.texture) SDL_DestroyTexture(app->pagarButton.texture);
    if (app->music) Mix_FreeMusic(app->music);
    if (app->font) TTF_CloseFont(app->font);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    if (app->emailTexture) SDL_DestroyTexture(app->emailTexture);
    if (app->nicknameTexture) SDL_DestroyTexture(app->nicknameTexture);
    
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    App app = {0};
    app.running = true;
    app.currentState = STATE_HOME;
    app.planTipo = 0; // 1 = premium, 0 = gratuito
   
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