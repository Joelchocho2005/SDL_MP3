#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INPUT_LENGTH 256

// Proporciones relativas a la imagen (1026x652)
#define INPUT_X_RATIO 172.0f/1026.0f  // 172px desde izquierda
#define EMAIL_Y_RATIO 222.0f/652.0f    // 222px desde arriba (email)
#define PASS_Y_RATIO  287.0f/652.0f    // 287px desde arriba (password)
#define INPUT_W_RATIO 682.0f/1026.0f   // 682px de ancho
#define INPUT_H_RATIO 30.0f/652.0f     // 30px de alto

typedef struct {
    SDL_Rect rect;
    char text[MAX_INPUT_LENGTH];
    bool active;
    SDL_Color textColor;
    SDL_Color bgColor;
    SDL_Color borderColor;
} InputField;

void renderInputField(SDL_Renderer *renderer, TTF_Font *font, InputField *field, bool isPassword) {
    // Dibujar borde
    SDL_SetRenderDrawColor(renderer, field->borderColor.r, field->borderColor.g, field->borderColor.b, 255);
    SDL_Rect border = { field->rect.x - 2, field->rect.y - 2, field->rect.w + 4, field->rect.h + 4 };
    SDL_RenderFillRect(renderer, &border);

    // Dibujar fondo
    SDL_SetRenderDrawColor(renderer, field->bgColor.r, field->bgColor.g, field->bgColor.b, 255);
    SDL_RenderFillRect(renderer, &field->rect);

    // Preparar texto
    const char *displayText = field->text;
    char hidden[MAX_INPUT_LENGTH] = "";

    if (isPassword) {
        memset(hidden, '*', strlen(field->text));
        hidden[strlen(field->text)] = '\0';
        displayText = hidden;
    }

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, displayText, field->textColor);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            field->rect.x + 5,
            field->rect.y + (field->rect.h - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    // Crear ventana centrada
    SDL_Window *window = SDL_CreateWindow("Login System",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1026, 652, SDL_WINDOW_RESIZABLE);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *imagen = IMG_Load("imgs/Login4.png");
    
    if (!imagen) {
        printf("Error al cargar imagen: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, imagen);
    SDL_FreeSurface(imagen);

    TTF_Font *font = TTF_OpenFont("src/assets/Inder-Regular.ttf", 20);
    if (!font) {
        printf("No se pudo cargar la fuente: %s\n", TTF_GetError());
        return 1;
    }

    InputField emailField = {
        .text = "",
        .active = true,
        .textColor = {0, 0, 0},
        .bgColor = {255, 255, 255},
        .borderColor = {0, 0, 0}
    };

    InputField passField = {
        .text = "",
        .active = false,
        .textColor = {0, 0, 0},
        .bgColor = {255, 255, 255},
        .borderColor = {0, 0, 0}
    };

    SDL_StartTextInput();

    bool running = true;
    bool mostrarError = false;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x, y = event.button.y;
                emailField.active = SDL_PointInRect(&(SDL_Point){x, y}, &emailField.rect);
                passField.active = SDL_PointInRect(&(SDL_Point){x, y}, &passField.rect);
            }
            else if (event.type == SDL_TEXTINPUT) {
                InputField *active = emailField.active ? &emailField : &passField;
                if (strlen(active->text) + strlen(event.text.text) < MAX_INPUT_LENGTH) {
                    strcat(active->text, event.text.text);
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                InputField *active = emailField.active ? &emailField : &passField;

                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    size_t len = strlen(active->text);
                    if (len > 0) {
                        active->text[len - 1] = '\0';
                    }
                }
                else if (event.key.keysym.sym == SDLK_TAB) {
                    emailField.active = !emailField.active;
                    passField.active = !passField.active;
                }
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    printf("Email: %s\n", emailField.text);
                    printf("Password: %s\n", passField.text);
                }
            }
        }

        // Limpiar pantalla
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Obtener tamaño actual de ventana
        int winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);

        // Calcular posición para centrar la imagen
        SDL_Rect imageRect;
        imageRect.w = 1026;  // Ancho original de la imagen
        imageRect.h = 652;   // Alto original de la imagen
        imageRect.x = (winWidth - imageRect.w) / 2;
        imageRect.y = (winHeight - imageRect.h) / 2;

        // Dibujar imagen centrada
        SDL_RenderCopy(renderer, texture, NULL, &imageRect);

        // Calcular posiciones de los campos de entrada relativas a la imagen centrada
        emailField.rect.x = imageRect.x + (int)(INPUT_X_RATIO * imageRect.w)-61;
        emailField.rect.y = imageRect.y + (int)(EMAIL_Y_RATIO * imageRect.h)+8;
        emailField.rect.w = (int)(INPUT_W_RATIO * imageRect.w)-156;
        emailField.rect.h = (int)(INPUT_H_RATIO * imageRect.h)+12;

        passField.rect.x = imageRect.x + (int)(INPUT_X_RATIO * imageRect.w)-61;
        passField.rect.y = imageRect.y + (int)(PASS_Y_RATIO * imageRect.h)+42;
        passField.rect.w = (int)(INPUT_W_RATIO * imageRect.w)-156;
        passField.rect.h = (int)(INPUT_H_RATIO * imageRect.h)+16;

        // Dibujar campos de entrada
        renderInputField(renderer, font, &emailField, false);
        renderInputField(renderer, font, &passField, true);
        // Actualizar pantalla
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}