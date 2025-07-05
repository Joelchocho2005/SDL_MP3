
#include "StateLogin.h" // Incluye la interfaz de funciones y tipos para manejar el sistema de login
#include <stdio.h> // Necesario para usar printf

// SDL exige que el punto de entrada se llame SDL_main en algunos entornos (especialmente Windows con SDL2)
int SDL_main(int argc, char *argv[]) {
    SDL_Window *ventana = NULL;         // Ventana principal del programa
    SDL_Renderer *renderizador = NULL;  // Renderizador asociado a la ventana (para dibujar contenido)

    // Inicializa todo lo necesario para mostrar el login (ventana, renderizador, fuentes, texturas, etc.)
    if (!inicializarLogin(&ventana, &renderizador)) {
        printf("Error al iniciar login\n");
        return 1; // Si falla la inicialización, termina el programa
    }

    bool corriendo = true;         // Controla si la aplicación sigue corriendo
    SDL_Event evento;              // Estructura para capturar eventos del sistema (teclado, mouse, etc.)

    // Bucle principal: sigue mientras esté corriendo y no se haya solicitado salir
    while (corriendo && obtenerEstadoLogin() == LOGIN_CONTINUAR) {
        // Manejo de todos los eventos que haya en la cola
        while (SDL_PollEvent(&evento)) {
            manejarEventosLogin(&evento, &corriendo); // Pasamos el evento a nuestro manejador
        }

        renderizarLogin();         // Dibuja la interfaz gráfica de login (o crear cuenta)
        SDL_Delay(16);             // Pausa ~16ms para limitar a ~60 FPS
    }

    destruirLogin();               // Libera memoria, texturas, fuentes, y cierra SDL correctamente
    return 0;                      // Finaliza el programa
}