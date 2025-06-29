#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        printf("Error SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error Mix_OpenAudio: %s\n", Mix_GetError());
        return 1;
    }

    Mix_Music *musica = Mix_LoadMUS("CECILIO ALVA - LA OTRA.mp3");
    if (!musica) {
        printf("Error Mix_LoadMUS: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Reproductor MP3",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 400, SDL_WINDOW_SHOWN);
    SDL_Surface *screen = SDL_GetWindowSurface(window);

    // Crear botón (rectángulo gris)
    SDL_Rect boton = {300, 150, 100, 50};  // x, y, ancho, alto
    SDL_FillRect(screen, &boton, SDL_MapRGB(screen->format, 180, 180, 180)); // Gris
    SDL_UpdateWindowSurface(window);

    Mix_PlayMusic(musica, -1); // Reproducir en bucle

    bool ejecutando = true;
    bool pausado = false;
    SDL_Event evento;

    while (ejecutando) {
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                ejecutando = false;
            } else if (evento.type == SDL_KEYDOWN) {
                if (evento.key.keysym.sym == SDLK_q) {
                    ejecutando = false;
                }
            } else if (evento.type == SDL_MOUSEBUTTONDOWN) {
                int mx = evento.button.x;
                int my = evento.button.y;
                if (mx >= boton.x && mx <= boton.x + boton.w &&
                    my >= boton.y && my <= boton.y + boton.h) {
                    if (pausado) {
                        Mix_ResumeMusic();
                        printf("▶️ Reanudado\n");
                    } else {
                        Mix_PauseMusic();
                        printf("⏸ Pausado\n");
                    }
                    pausado = !pausado;
                }
            }
        }
        SDL_Delay(100);
    }

    Mix_FreeMusic(musica);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
