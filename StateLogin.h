#ifndef STATE_LOGIN_H
#define STATE_LOGIN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

// Inicializa el estado login (ventana, renderer, fuentes, etc.)
bool inicializarLogin(SDL_Window **window, SDL_Renderer **renderer);

// Maneja eventos de login y registro
void manejarEventosLogin(SDL_Event *event, bool *corriendo);

// Renderiza la pantalla actual (login o registro)
void renderizarLogin();

// Libera los recursos asociados al estado login
void destruirLogin();

void cambiarPantallaLogin();
void cambiarPantallaRegistro();

typedef enum {
    LOGIN_CONTINUAR,
    LOGIN_CERRAR_SESION,
    LOGIN_SALIR
} EstadoLogin;

EstadoLogin obtenerEstadoLogin();

#endif // STATE_LOGIN_H