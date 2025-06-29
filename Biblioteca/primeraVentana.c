#include <SDL2/SDL.h>
#include <stdio.h>
#define SDL_MAIN_HANDLED
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
            //EN CASO DE QUE EL EVENTO SEA CERRAR
            if(event.type== SDL_QUIT) gameIsRunning=false;
        }
        
    }
        

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
