#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
int main(int argc, char *argv[]) {
    
    SDL_Window *window;
    if (SDL_Init(SDL_INIT_VIDEO)<0){
        printf("Error al iniciar");
        return -1;
    }

    window=SDL_CreateWindow("MI PRIMERA VENTANA",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_SHOWN);
    if(window==NULL){
        printf("NO SE PUDO CREAR LA VENTANA: %S\n",SDL_GetError());
        return 1;
    }
    bool gameIsRunning=true;
    while (gameIsRunning){
        //Eventos apartir de lo que haga el usuario
        SDL_Event event;
        while (SDL_PollEvent(&event)){
           switch (event.type) {
                case SDL_QUIT:
                    printf("Evento: Cerrar ventana\n");
                    gameIsRunning = false;
                    break;
                case SDL_KEYDOWN:
                    printf("Tecla presionada: %s (Código: %d)\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    printf("Tecla soltada: %s (Código: %d)\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.sym);
                    break;
                case SDL_MOUSEMOTION:
                    printf("Movimiento del mouse en (%d, %d)\n", event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    printf("Botón del mouse presionado: %d en (%d, %d)\n", event.button.button, event.button.x, event.button.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    printf("Botón del mouse soltado: %d en (%d, %d)\n", event.button.button, event.button.x, event.button.y);
                    break;
                case SDL_MOUSEWHEEL:
                    printf("Scroll del mouse: %d horizontal, %d vertical\n", event.wheel.x, event.wheel.y);
                    break;
                default:
                    printf("Evento desconocido: %d\n", event.type);
                    break;
            }
        }
        
    }
        

    SDL_DestroyWindow(window);

    SDL_Quit();
    getchar();
    return 0;
}
