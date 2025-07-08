#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generalH.h"


// Codigo de armado:
// gcc -I src/include -L src/lib versionConexCrud1-0.c CodGeneral/*.c -o reproductor.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

#define WINDOW_WIDTH 1026
#define WINDOW_HEIGHT 652
//Estados de la app
typedef enum { 
    STATE_HOME, 
    STATE_PLAYER,
    STATE_AGREGAR_A_PLAYLIST,
    STATE_SEARCH    
} AppState;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
} Button;

typedef struct playlistNodo {
    cancionNodo* cancion;
    struct playlistNodo* sig;
    struct playlistNodo* prev;
} playlistNodo;

typedef struct playlist {
    char nombre[longNombres];
    struct playlist* sig;
    playlistNodo* canciones;
} playlist;

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
        TTF_Font* small;
        TTF_Font* medium;
        TTF_Font* large;
} Fonts;

typedef struct {
    char nombreUsuario[100];
    char codigoUsuario[100];
    bool premium;
}UsuarioPrueba;

typedef struct {
        SDL_Window* window;
        SDL_Renderer* renderer;

        SDL_Texture* background;
        SDL_Texture* miniMenutexture;
        SDL_Texture* backHome;
        SDL_Texture *AgregarMusicaAPlaylist;
        SDL_Texture* PageSearch;

        Button playButton;
        Button pauseButton;
        Button prevButton;
        Button nextButton;

        SDL_Rect miniMenu;
        SDL_Rect agregarPlaylist;
        SDL_Rect compartirMusica;
        SDL_Rect verArtista;
        SDL_Rect backButtonRect;
        SDL_Rect miniReproductor;

        bool isPlaying;
        bool premium;
        bool isMiniMenu;
        int cancionesDesdeUltimoAnuncio;
        bool reproduciendoAnuncio;

        AdQueue adQueue;

        bool searchActive;
        char searchText[256];
        SDL_Texture* searchTexture;
        SDL_Rect searchBoxRect;
        SDL_Color searchColor;

        //lista de playlits
        playlist* playlists;
        playlist* currentPlaylist;
        playlistNodo* currentSong;   
        playlistNodo* pendingSong; 
        Mix_Music* currentMusic;

        Uint32 errorInicios;  // tiempo de inicio en milisegundos
        Uint32 errorFin; 
        const char* tempMessage;

        //Conexion con el crud
        cancionNodo *raizCancion;
        Artista *raizArtistas;
        anuncioNodo *inicioAnuncios;   // Cola de anuncios
        anuncioNodo *finAnuncios;
        albumLista *raizAlbumes;
        int contadorCodCanciones;
        int contadorCodAlbumes;
        int contadorCodArtistas;
        
        //SOLO PARA PRUEBAS
        UsuarioPrueba userAccount;

        //STRUCTURA DE FUENTES PARA LA APP
        //NOTA: AGREGAR SI NECESITAN DE OTRO TAMAÑO
        Fonts fonts;
    
        bool running;
        AppState currentState;
    } App;

// --- Declaración de funciones ---
//FUNCIONES BASE PARA EL SDL
bool initSDL(App* app);
SDL_Texture* loadTexture(App* app, const char* path);
//CARGA DE TEXTURAS
bool loadMedia(App* app);
//FUNCION DE ANUNCIOS: PROXIMAMENTE SE QUITARA Y SE UNIRA AL CRUDO
bool loadAds(App* app, const char* filename);
//FUCNION DE CARGADO DE FUENTES
bool loadFonts(App* app);
// RENDERIZA IMAGENES,TEXTO EN CADA ESTADO DE LA APP
//USARLO Y VER SU CONFIGURACION USANDO BOLEANOS PARA COSAS DIFERENTES EN EL APP
void render(App* app);
//RENDERIZA UN TEXTO EN UN CUADRO, Y LO CENTRA
void renderTextInRect(App* app, const char* text, TTF_Font* font,SDL_Rect rect, SDL_Color textColor, SDL_Color bgColor, bool drawBackground);
//RENDERIZA TEXTO 
void renderText(App* app, const char* text, TTF_Font* font, int x, int y);
//USADO PARA MOSTRAR TEXTO CORTO EN UN TIEMPO DETERMINADO
void showTemporaryMessage(App* app, const char* message, Uint32 durationMs);
//FUNCION QUE CONTROLA LOS EVENTOS DE TECLADO, MOUSE Y BOTONES DEL APP GENERAL
void handleEvents(App* app);


//FUNCIONES DEL REPRODUCTOR:
//FUNCION CALLBACK PARA EL REPRODUCTOR
void playCurrentSong(App* app);
void updatePlayback(App* app);
//MANEJA LOS CONTROLES DEL REPRODUCTOR
void handlePlaybackControls(App* app, SDL_Point p);

// SETEO DE BOTONES PARA CADA ESTADO
//NOTA: SI NECESITAN CREAR UN BUTTON HAGANLO CON SU CONFIGURACION EN EL RENDER
void setupButtonsForState(App* app);

//CONEXION CON EL CRUDO
/*
    CARGA LA BASE DE DATOS DE CANCIONES-ARTISTAS-ALBUNES
    CARGA LAS PLAYLITS DEL USUARIO USANDO SU CODIGO
*/
cancionNodo* buscarCancionPorNombre(Artista* raiz, char* nombreBuscado);
void insertarCancionEnPlaylist(playlist* pl, cancionNodo* cancion);
playlist* loadPlaylist(char* nombreArchivo, Artista* arbolArtistas);
void loadUserPlaylists(char* nombreArchivo, Artista* arbolArtistas);
void cargarYMostrarPlaylists(App* app, const char* rutaArchivo);

//FUNCION DE LIBERACION DE MEMORIA SDL, STRUCTS ETC
void cleanup(App* app);

int main(int argc, char* argv[]) {

    
    App app = {0};

    UsuarioPrueba userAccount;
    strcpy(userAccount.codigoUsuario, "123");
    userAccount.premium=false;
    app.userAccount = userAccount;

    app.running = true;
    app.currentState = STATE_HOME;
    app.premium = true;  // cambia según necesidad
    app.cancionesDesdeUltimoAnuncio = 0;
    app.reproduciendoAnuncio = false;
    app.adQueue.front = app.adQueue.rear = NULL;
    app.isMiniMenu=false;
    app.searchActive=false;
    
    
    
    if (!initSDL(&app)) return 1;
    if (!loadMedia(&app)) return 1;
    if (!loadFonts(&app)) return 1;

    // Playlist* base = loadPlaylist("data/playlistListaUsuarios/canciones.txt");
    
    // if (base) {
    // base->next = app.playlists;
    // app.playlists = base;
    // }

    // app.currentPlaylist = base;  // o la que el usuario elija
    // app.currentSong = base->head
    app.contadorCodCanciones=0;
    app.contadorCodAlbumes=0;
    app.contadorCodArtistas=0;
    app.raizAlbumes=NULL;
    app.raizArtistas=NULL;
    app.raizCancion=NULL;
    cargarArtistas(&app.raizArtistas,&app.contadorCodArtistas);
    cargarAlbumes(&app.raizArtistas,&app.contadorCodAlbumes);
    cargarCanciones(&app.raizArtistas, &app.raizCancion, &app.contadorCodCanciones);
    
    listarCanciones(1);

    // char nombre[12];
    // if(app.raizArtistas->albumes->canciones->info->nombre!=NULL)
    printf("\n%s",app.raizArtistas->albumes->canciones->info->nombre);

    printf("\n%d",app.contadorCodCanciones);

    char archivoGeneralPlaylist[100];
    strcpy(archivoGeneralPlaylist,"data/playlistListaUsuarios/");
    strcat(archivoGeneralPlaylist,userAccount.codigoUsuario);
    strcat(archivoGeneralPlaylist,"-ListaPlaylist.csv");
    cargarYMostrarPlaylists(&app, archivoGeneralPlaylist);
    app.currentPlaylist = app.playlists; 
    // app->currentSong = app->currentPlaylist->canciones;
    setupButtonsForState(&app);

    while (app.running) {
        handleEvents(&app);

        updatePlayback(&app); 

        render(&app);
        SDL_Delay(16);  // ~60 FPS
    }

    cleanup(&app);
    liberarArtistas(app.raizArtistas);
    liberarAlbumes(app.raizAlbumes);
    liberarArbolCanciones(app.raizCancion);
    return 0;
}

// --- Implementaciones ---

bool initSDL(App* app) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        return false;
    }
    if (TTF_Init() == -1) {
        printf("TTF_Init error: %s\n", TTF_GetError());
        return false;
    }
    app->window = SDL_CreateWindow("Reproductor SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!app->window) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        return false;
    }
    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) {
        printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

SDL_Texture* loadTexture(App* app, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("IMG_Load error %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
    }
    return texture;
}

bool loadMedia(App* app) {
    app->background = loadTexture(app, "imgs/Img_Reproductor.png");
    if (!app->background) return false;

    app->playButton.texture = loadTexture(app, "imgs/promo3Play-removebg-preview.png");
    app->pauseButton.texture = loadTexture(app, "imgs/prom3Pause-removebg-preview.png");
    app->prevButton.texture = loadTexture(app, "imgs/promo3Atras-removebg-preview.png");
    app->nextButton.texture = loadTexture(app, "imgs/promo3Siguiente-removebg-preview.png");
    app->miniMenutexture=loadTexture(app,"imgs/MiniMenu.png");
    app->backHome= loadTexture(app,"imgs/Img_HomePage.png");
    app->AgregarMusicaAPlaylist=loadTexture(app,"imgs/AgregarPlaylist.png");
    app->PageSearch=loadTexture(app,"imgs/Img_Search.png");

    if (!app->playButton.texture || !app->pauseButton.texture ||
        !app->prevButton.texture || !app->nextButton.texture) return false;

    app->isPlaying = false;

    return true;
}




bool loadAds(App* app, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("No se pudo abrir anuncios %s\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        AdNode* ad = malloc(sizeof(AdNode));
        if (!ad) {
            fclose(f);
            return false;
        }
        strcpy(ad->path, line);
        ad->music = NULL;
        // if (!ad->music) {
        //     printf("Error cargando anuncio %s: %s\n", line, Mix_GetError());
        //     free(ad);
        //     continue;
        // }
        ad->next = NULL;
        if (!app->adQueue.front) {
            app->adQueue.front = app->adQueue.rear = ad;
        } else {
            app->adQueue.rear->next = ad;
            app->adQueue.rear = ad;
        }
    }
    fclose(f);
    return true;
}

bool loadFonts(App* app) {
    app->fonts.small = TTF_OpenFont("src/assets/Inder-Regular.ttf", 12);
    if (!app->fonts.small) {
        printf("Error cargando fuente pequeña: %s\n", TTF_GetError());
        return false;
    }

    app->fonts.medium = TTF_OpenFont("src/assets/Inder-Regular.ttf", 24);
    if (!app->fonts.medium) {
        printf("Error cargando fuente mediana: %s\n", TTF_GetError());
        return false;
    }

    app->fonts.large = TTF_OpenFont("src/assets/Inder-Regular.ttf", 48);
    if (!app->fonts.large) {
        printf("Error cargando fuente grande: %s\n", TTF_GetError());
        return false;
    }

    return true;
}
void renderTextInRect(App* app, const char* text, TTF_Font* font,
                      SDL_Rect rect, SDL_Color textColor, SDL_Color bgColor, bool drawBackground) {
    // 1️⃣ Renderizar texto a superficie
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, textColor);
    if (!surface) {
        printf("TTF_RenderUTF8_Blended error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    int textW = surface->w;
    int textH = surface->h;
    SDL_FreeSurface(surface);

    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        return;
    }

    // 2️⃣ Dibujar rectángulo de fondo si se quiere
    if (drawBackground) {
        SDL_SetRenderDrawColor(app->renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(app->renderer, &rect);
    }

    // 3️⃣ Centrar el texto dentro del rectángulo
    SDL_Rect dstRect;
    dstRect.w = textW;
    dstRect.h = textH;
    dstRect.x = rect.x + (rect.w - textW) / 2;
    dstRect.y = rect.y + (rect.h - textH) / 2;

    SDL_RenderCopy(app->renderer, texture, NULL, &dstRect);

    SDL_DestroyTexture(texture);
}


void renderText(App* app, const char* text, TTF_Font* font, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};  // blanco, o puedes parametrizar
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        printf("TTF_RenderUTF8_Blended error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        return;
    }
    int textW, textH;
    SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
    SDL_Rect dstRect = { x, y, textW, textH };
    SDL_RenderCopy(app->renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
}
//crudo:
cancionNodo* buscarCancionPorNombre(Artista* raiz, char* nombreBuscado) {
    if (raiz == NULL) return NULL;

    // Buscar en la izquierda
    cancionNodo* encontrada = buscarCancionPorNombre(raiz->izq, nombreBuscado);
    if (encontrada != NULL) return encontrada;

    // Revisar los álbumes del artista actual
    albumLista* albumActual = raiz->albumes;
    while (albumActual != NULL) {
        cancionNodo* cancionActual = albumActual->canciones;
        while (cancionActual != NULL) {
            if (strcmp(cancionActual->info->nombre, nombreBuscado) == 0) {
                return cancionActual;
            }
            cancionActual = cancionActual->sig;
        }
        albumActual = albumActual->sig;
    }

    // Buscar en la derecha
    return buscarCancionPorNombre(raiz->der, nombreBuscado);
}
void insertarCancionEnPlaylist(playlist* pl, cancionNodo* cancion) {
    playlistNodo* nuevo = (playlistNodo*)malloc(sizeof(playlistNodo));
    if (nuevo == NULL) {
        printf("Error al asignar memoria para playlistNodo\n");
        exit(1);
    }
    nuevo->cancion = cancion;
    nuevo->sig = NULL;
    nuevo->prev = NULL;

    if (pl->canciones == NULL) {
        pl->canciones = nuevo;
    } else {
        playlistNodo* actual = pl->canciones;
        while (actual->sig != NULL) {
            actual = actual->sig;
        }
        actual->sig = nuevo;
        nuevo->prev = actual;
    }
}

playlist* loadPlaylist(char* nombreArchivo, Artista* arbolArtistas) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo de playlist: %s\n", nombreArchivo);
        return NULL;
    }

    playlist* pl = (playlist*)malloc(sizeof(playlist));
    if (pl == NULL) {
        printf("Error al asignar memoria para playlist.\n");
        exit(1);
    }

    strcpy(pl->nombre, nombreArchivo);
    pl->canciones = NULL;

    char linea[longNombres];
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        // Elimina salto de línea
        linea[strcspn(linea, "\n")] = 0;

        cancionNodo* encontrada = buscarCancionPorNombre(arbolArtistas, linea);
        if (encontrada != NULL) {
            insertarCancionEnPlaylist(pl, encontrada);
            printf("Agregada: %s\n", linea);
        } else {
            printf("Canción no encontrada: %s\n", linea);
        }
    }

    fclose(archivo);
    return pl;
}

void loadUserPlaylists(char* nombreArchivo, Artista* arbolArtistas) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo CSV de playlists de usuario.\n");
        return;
    }

    char linea[longNombres];
    int lineaActual = 0;

    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        linea[strcspn(linea, "\n")] = 0;

        if (lineaActual == 0) {
            printf("Info de cabecera: %s\n", linea);
        } else {
            printf("\nCargando playlist: %s\n", linea);
            playlist* pl = loadPlaylist(linea, arbolArtistas);
            if (pl != NULL) {
                printf("Playlist cargada: %s\n", pl->nombre);
                // Aquí puedes agregar la playlist al usuario, si tienes esa estructura
            }
        }
        lineaActual++;
    }

    fclose(archivo);
}

void cargarYMostrarPlaylists(App* app, const char* rutaArchivo) {
    FILE* archivo = fopen(rutaArchivo, "r");
    if (!archivo) {
        printf("No se pudo abrir archivo de playlists de usuario.\n");
        return;
    }

    char linea[longNombres];
    bool primera = true;

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0;  // Elimina salto de línea

        char rutaCompleta[256];
        snprintf(rutaCompleta, sizeof(rutaCompleta), "data/playlistUsuarios/%s-%s.csv", app->userAccount.codigoUsuario, linea);

        playlist* nueva = loadPlaylist(rutaCompleta, app->raizArtistas);
        if (nueva) {
            nueva->sig = app->playlists;
            app->playlists = nueva;
            printf("Playlist cargada: %s\n", nueva->nombre);

            if (primera) {
                app->currentPlaylist = nueva;
                app->currentSong = nueva->canciones;
                primera = false;
            }
        } else {
            printf("No se pudo cargar: %s\n", rutaCompleta);
        }
    }
    fclose(archivo);
}



void playCurrentSong(App* app) {
    if (!app->currentSong || !app->currentSong->cancion || !app->currentSong->cancion->info) return;

    // Si no es premium y le toca anuncio
    if (!app->premium && app->cancionesDesdeUltimoAnuncio >= 2 && !app->reproduciendoAnuncio && app->adQueue.front) {
        app->pendingSong = app->currentSong;

        AdNode* ad = app->adQueue.front;

        Mix_HaltMusic();

        ad->music = Mix_LoadMUS(ad->path);
        if (!ad->music) {
            printf("Error cargando anuncio %s: %s\n", ad->path, Mix_GetError());
            return;
        }

        if (Mix_PlayMusic(ad->music, 1) == -1) {
            printf("Error reproduciendo anuncio: %s\n", Mix_GetError());
        } else {
            app->reproduciendoAnuncio = true;
            app->isPlaying = true;
            printf("Reproduciendo anuncio: %s\n", ad->path);
        }

        return;
    }

    // Si no hay anuncio, reproduce canción normal
    Mix_HaltMusic();

    if (app->currentMusic) {
        Mix_FreeMusic(app->currentMusic);
        app->currentMusic = NULL;
    }

    const char* ruta = app->currentSong->cancion->info->rutaAudio;
    app->currentMusic = Mix_LoadMUS(ruta);
    if (!app->currentMusic) {
        printf("Error cargando canción: %s\n", Mix_GetError());
        app->isPlaying = false;
        return;
    }

    if (Mix_PlayMusic(app->currentMusic, 0) == -1) {
        printf("Error reproduciendo canción: %s\n", Mix_GetError());
        app->isPlaying = false;
        Mix_FreeMusic(app->currentMusic);
        app->currentMusic = NULL;
        return;
    }

    app->isPlaying = true;
    app->reproduciendoAnuncio = false;

    printf("Reproduciendo canción: %s\n", ruta);
}

void setupButtonsForState(App* app) {
        app->playButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->prevButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->nextButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->backButtonRect  = (SDL_Rect){0, 0, 0, 0};
        app->miniMenu        = (SDL_Rect){0, 0, 0, 0};
        app->miniReproductor = (SDL_Rect){0, 0, 0, 0};
        app->agregarPlaylist = (SDL_Rect){0, 0, 0, 0};
        app->compartirMusica = (SDL_Rect){0, 0, 0, 0};
        app->searchBoxRect   = (SDL_Rect){0, 0, 0, 0};

        switch (app->currentState) {
        case STATE_HOME:
            app->playButton.rect = (SDL_Rect){ 490, 587, 64, 64 };
            app->prevButton.rect = (SDL_Rect){ 400, 590, 60, 60 };
            app->nextButton.rect = (SDL_Rect){ 580, 586, 64, 64 };
            // app->backButtonRect = (SDL_Rect){ 0, 0, 0, 0 };
            // app->miniMenu = (SDL_Rect){ 0, 0, 0, 0 };
            app->miniReproductor=(SDL_Rect){275,541,454,111};
            app->searchActive = false;
            strcpy(app->searchText, "");
            app->searchBoxRect = (SDL_Rect) {150, 45, 725, 70} ;  // Usa coordenadas reales
            app->searchColor = (SDL_Color){ 255, 255, 255, 255};
            break;

        case STATE_PLAYER:
            app->playButton.rect = (SDL_Rect){ 481, 520, 80, 80 };
            app->prevButton.rect = (SDL_Rect){ 350, 523, 80, 80 };
            app->nextButton.rect = (SDL_Rect){ 612, 520, 80, 80 };
            app->backButtonRect = (SDL_Rect){ 20, 20, 80, 40 };
            app->miniMenu = (SDL_Rect){ 930, 540, 50, 50 };
            app->agregarPlaylist=(SDL_Rect){750,250,225,102};
            app->compartirMusica =(SDL_Rect){750,352,225,102};
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            app->searchBoxRect   = (SDL_Rect){150, 45, 725, 70};
            
            break;
        case STATE_AGREGAR_A_PLAYLIST:
            app->backButtonRect = (SDL_Rect){30, 10, 40, 40};
            
        break;
        case STATE_SEARCH:
            app->backButtonRect = (SDL_Rect){50, 70, 40, 40};
            
        break;

        default:
            // Opcional: Maneja otros estados si es necesario
            break;
    }
}

void render(App* app) {
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    SDL_Rect rectMusic = {340, 360, 343, 50};  // x, y, w, h
    SDL_Rect rectNextMusic = {105, 115, 184, 78};  // x, y, w, h
    SDL_Rect rectPlaylist = {340, 30, 343, 50};  // x, y, w, h
    SDL_Rect miniMenu = {750, 250, 225, 308};
    // SDL_Rect boton3Puntos = {50, 70, 40, 40};

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color bgColor = {0, 0, 255, 255};

    switch (app->currentState) {
        case STATE_HOME:
            SDL_RenderCopy(app->renderer, app->backHome, NULL, NULL);

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

            if (!app->reproduciendoAnuncio && app->currentSong && app->currentSong->cancion && app->currentSong->cancion->info) {
                renderText(app,
                    app->currentSong->cancion->info->nombre,
                    app->fonts.small,
                    430, 560);
            }

            break;

        case STATE_PLAYER:
            SDL_RenderCopy(app->renderer, app->background, NULL, NULL);

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

            if (!app->reproduciendoAnuncio) {
                if (app->currentPlaylist)
                    renderTextInRect(app,
                        app->currentPlaylist->nombre,  // ✅ campo correcto
                        app->fonts.medium,
                        rectPlaylist,
                        textColor,
                        bgColor,
                        false);

                if (app->currentSong && app->currentSong->cancion && app->currentSong->cancion->info)
                    renderTextInRect(app,
                        app->currentSong->cancion->info->nombre,
                        app->fonts.medium,
                        rectMusic,
                        textColor,
                        bgColor,
                        false);

                if (app->currentSong && app->currentSong->sig && app->currentSong->sig->cancion && app->currentSong->sig->cancion->info)
                    renderTextInRect(app,
                        app->currentSong->sig->cancion->info->nombre,
                        app->fonts.small,
                        rectNextMusic,
                        textColor,
                        bgColor,
                        false);
            }

            if (app->isMiniMenu) {
                SDL_RenderCopy(app->renderer, app->miniMenutexture, NULL, &miniMenu);
                Uint32 now = SDL_GetTicks();
                if (app->tempMessage && now - app->errorInicios < app->errorFin) {
                    renderText(app, app->tempMessage, app->fonts.small, 820, 540);
                } else {
                    app->tempMessage = NULL;
                }
            }

            break;
        case STATE_AGREGAR_A_PLAYLIST:
            SDL_RenderCopy(app->renderer, app->AgregarMusicaAPlaylist, NULL, NULL);

        break;
        case STATE_SEARCH:
            SDL_RenderCopy(app->renderer, app->PageSearch, NULL, NULL);
            // renderTextInRect(app,"Hola", app->fonts.medium,
            //             boton3Puntos,
            //             textColor,
            //             bgColor,
            //             true); 
        break; 
        default:
            break;
    }

    SDL_RenderPresent(app->renderer);
}


void handleEvents(App* app) {
    SDL_Event e;
    SDL_Rect miniMenu={750,250,225,308};

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            app->running = false;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point p = {x, y};
           

            switch (app->currentState) {
                case STATE_HOME:
                    if (app->reproduciendoAnuncio) {
                        printf("Anuncio en reproducción: controles bloqueados.\n");
                        if(SDL_PointInRect(&p, &app->miniReproductor)&&!SDL_PointInRect(&p, &app->nextButton.rect)&&!SDL_PointInRect(&p, &app->playButton.rect)&&!SDL_PointInRect(&p, &app->prevButton.rect)){
                            app->currentState=STATE_PLAYER;
                            setupButtonsForState(app);
                        }
                        break;
                    }else{
                        
                        handlePlaybackControls(app, p);
                    }

                   
                    if(SDL_PointInRect(&p, &app->miniReproductor)&&!SDL_PointInRect(&p, &app->nextButton.rect)&&!SDL_PointInRect(&p, &app->playButton.rect)&&!SDL_PointInRect(&p, &app->prevButton.rect)){
                        app->currentState=STATE_PLAYER;
                        setupButtonsForState(app);
                    }

                    if(SDL_PointInRect(&p, &app->searchBoxRect)){
                        app->currentState=STATE_SEARCH;
                        setupButtonsForState(app);


                    }


                    break;

                case STATE_PLAYER:
                    if (app->reproduciendoAnuncio) {
                        printf("Anuncio en reproducción: controles bloqueados.\n");
                        break;
                    }else { 
                        handlePlaybackControls(app, p);
                    }
                    
                    if(SDL_PointInRect(&p, &app->miniMenu)){
                        app->isMiniMenu=true;
                        // printf("Mini menu activo");
                       
                    }
                    if(app->isMiniMenu&&SDL_PointInRect(&p,&app->agregarPlaylist)){
                            printf("Agregar a playlisst");
                            app->currentState=STATE_AGREGAR_A_PLAYLIST;
                        }
                    if(app->isMiniMenu&&SDL_PointInRect(&p,&app->compartirMusica)){
                        showTemporaryMessage(app, "Link copiado", 3000);  // 3 segundos
                          printf("Link copiado");   
                    }
                    else if(!SDL_PointInRect(&p, &app->miniMenu) && !SDL_PointInRect(&p, &miniMenu)){
                        app->isMiniMenu=false;
                        // printf("Min menu desactivado");
                    }
                    if(SDL_PointInRect(&p, &app->backButtonRect)){
                        app->currentState= STATE_HOME;
                        setupButtonsForState(app);
                    }

                    break;
                case STATE_AGREGAR_A_PLAYLIST:
                    if(SDL_PointInRect(&p, &app->backButtonRect)){
                        app->currentState= STATE_PLAYER;
                        setupButtonsForState(app);
                    }
                break;  
                case STATE_SEARCH:
                    if(SDL_PointInRect(&p, &app->backButtonRect)){
                        app->currentState= STATE_HOME;
                        setupButtonsForState(app);
                    }
                break; 
            }
        }
    }
}
void handleSearchInput(App* app, SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point p = { x, y };

        if (SDL_PointInRect(&p, &app->searchBoxRect)) {
            app->searchActive = true;
            SDL_StartTextInput();
        } else {
            app->searchActive = false;
            SDL_StopTextInput();
        }
    }
    else if (e->type == SDL_TEXTINPUT && app->searchActive) {
        strcat(app->searchText, e->text.text);
    }
    else if (e->type == SDL_KEYDOWN && app->searchActive) {
        if (e->key.keysym.sym == SDLK_BACKSPACE && strlen(app->searchText) > 0) {
            app->searchText[strlen(app->searchText) - 1] = '\0';
        }
        else if (e->key.keysym.sym == SDLK_RETURN) {
            printf("Texto ingresado: %s\n", app->searchText);
            app->searchActive = false;
            SDL_StopTextInput();
            // Aquí haces lo que necesites con searchText
        }
    }
}

void handlePlaybackControls(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->playButton.rect)) {
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
    } else if (SDL_PointInRect(&p, &app->prevButton.rect)) {
        if (app->currentSong && app->currentSong->prev) {
            app->currentSong = app->currentSong->prev;
            app->cancionesDesdeUltimoAnuncio++;
            playCurrentSong(app);
        }
    } else if (SDL_PointInRect(&p, &app->nextButton.rect)) {
        if (app->currentSong && app->currentSong->sig) {
            app->currentSong = app->currentSong->sig;
            app->cancionesDesdeUltimoAnuncio++;
            playCurrentSong(app);
        }
    }
}


void updatePlayback(App* app) {
    if (app->reproduciendoAnuncio && !Mix_PlayingMusic()) {
        app->reproduciendoAnuncio = false;
        app->cancionesDesdeUltimoAnuncio = 0;

        AdNode* ad = app->adQueue.front;
        if (ad) {
            app->adQueue.front = ad->next;
            if (!app->adQueue.front) app->adQueue.rear = NULL;

            if (ad->music) {
                Mix_FreeMusic(ad->music);
                ad->music = NULL;
            }
            free(ad);
        }

        if (app->pendingSong) {
            app->currentSong = app->pendingSong;
            app->pendingSong = NULL;
        }

        playCurrentSong(app);
        return;
    }

    if (!app->reproduciendoAnuncio && !Mix_PlayingMusic() && app->isPlaying) {
        if (app->currentMusic) {
            Mix_FreeMusic(app->currentMusic);
            app->currentMusic = NULL;
        }

        if (app->currentSong && app->currentSong->sig) {
            app->currentSong = app->currentSong->sig;
            app->cancionesDesdeUltimoAnuncio++;
            playCurrentSong(app);
        } else {
            app->isPlaying = false;
        }
    }
}





void showTemporaryMessage(App* app, const char* message, Uint32 durationMs) {
    app->tempMessage = message;
    app->errorInicios = SDL_GetTicks();
    app->errorFin = durationMs;
}
void cleanup(App* app) {
    // ---------------------------
    // 1️⃣ Parar la música si suena
    // ---------------------------
    Mix_HaltMusic();
    Mix_HookMusicFinished(NULL);

    // ---------------------------
    // 2️⃣ Liberar playlistNodo de currentSong (ir al inicio y liberar todos)
    // ---------------------------
    playlistNodo* s = app->currentSong;
    while (s && s->prev) s = s->prev; // ir al inicio
    while (s) {
        playlistNodo* next = s->sig;
        // No liberes cancionNodo porque esos apuntan a canciones compartidas en arbolArtistas
        free(s);
        s = next;
    }
    app->currentSong = NULL;

    // ---------------------------
    // 3️⃣ Liberar TODAS las playlists y sus playlistNodo
    // ---------------------------
    playlist* p = app->playlists;
    while (p) {
        playlistNodo* sn = p->canciones;
        while (sn) {
            playlistNodo* next = sn->sig;
            // Igual que antes, no liberes cancionNodo
            free(sn);
            sn = next;
        }
        playlist* nextP = p->sig;
        free(p);
        p = nextP;
    }
    app->playlists = NULL;

    // ---------------------------
    // 4️⃣ Liberar pendingSong si existe (y no es parte de otra lista)
    // ---------------------------
    if (app->pendingSong) {
        free(app->pendingSong);
        app->pendingSong = NULL;
    }

    // ---------------------------
    // 5️⃣ Liberar anuncios de la cola y sus audios
    // ---------------------------
    AdNode* a = app->adQueue.front;
    while (a) {
        AdNode* next = a->next;
        if (a->music) Mix_FreeMusic(a->music);
        free(a);
        a = next;
    }
    app->adQueue.front = NULL;
    app->adQueue.rear = NULL;

    // ---------------------------
    // 6️⃣ Destruir texturas de botones, fondos, mini menú y backHome
    // ---------------------------
    if (app->playButton.texture) SDL_DestroyTexture(app->playButton.texture);
    if (app->pauseButton.texture) SDL_DestroyTexture(app->pauseButton.texture);
    if (app->prevButton.texture) SDL_DestroyTexture(app->prevButton.texture);
    if (app->nextButton.texture) SDL_DestroyTexture(app->nextButton.texture);
    if (app->background) SDL_DestroyTexture(app->background);
    if (app->miniMenutexture) SDL_DestroyTexture(app->miniMenutexture);
    if (app->backHome) SDL_DestroyTexture(app->backHome);
    if(app->AgregarMusicaAPlaylist) SDL_DestroyTexture(app->AgregarMusicaAPlaylist);
    if (app->PageSearch) SDL_DestroyTexture(app->PageSearch);

    // ---------------------------
    // 7️⃣ Cerrar fuentes
    // ---------------------------
    if (app->fonts.small) TTF_CloseFont(app->fonts.small);
    if (app->fonts.medium) TTF_CloseFont(app->fonts.medium);
    if (app->fonts.large) TTF_CloseFont(app->fonts.large);

    // ---------------------------
    // 8️⃣ Destruir renderer y ventana
    // ---------------------------
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    app->renderer = NULL;
    app->window = NULL;

    // ---------------------------
    // 9️⃣ Liberar tempMessage si es necesario (solo si fue asignado dinámicamente)
    // ---------------------------
    // if (app->tempMessage) free((void*)app->tempMessage);

    // ---------------------------
    // 🔟 Cerrar SDL y subsistemas
    // ---------------------------
    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}
