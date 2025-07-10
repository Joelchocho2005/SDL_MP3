#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#define NOMINMAX
#include <windows.h>
#include "generalH.h"
#include "StateLogin.h" // Incluye la interfaz de funciones y tipos para manejar el sistema de login

#define PLAYLISTS_PER_PAGE 6

// Codigo de armado:
// gcc -I src/include -L src/lib versionConexCrud1-0.c CodGeneral/*.c -o reproductor.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
//gcc -I src/include -L src/lib Repro-Login-Crud.c CodGeneral/*.c StateLogin.c ValidacionesLogin.c -o unionLogin.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

#define WINDOW_WIDTH 1026
#define WINDOW_HEIGHT 652
//Estados de la app
typedef enum { 
    STATE_PLAN_GRATUITO,
    STATE_PLAN_PREMIUM,
    STATE_COMPRAR_PREMIUM,
    STATE_USERPAGE,
    STATE_HOME, 
    STATE_PLAYER,
    STATE_AGREGAR_A_PLAYLIST,
    STATE_SEARCH    
} AppState;

typedef struct {
    SDL_Rect rect;
    char placeholder[32];
    char text[32];
    bool active;
    SDL_Color textColor;
} TextField;



typedef struct ResultadoBusqueda {
    cancionNodo* cancion;
    albumLista* album;
    Artista* artista;
    struct ResultadoBusqueda* sig;
} ResultadoBusqueda;

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
    char ruta[100];
    int numeroCanciones;
    int duracionPlaylist;
    bool album;
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
        TTF_Font* small1;
        TTF_Font* medium;
        TTF_Font* large;
} Fonts;

typedef struct {
    char nombreUsuario[100];
    char codigoUsuario[100];
    char correo[100];
    bool premium;
}UsuarioPrueba;

typedef struct {
        SDL_Window* window;
        SDL_Renderer* renderer;

        SDL_Texture* backPlayer;
        SDL_Texture* miniMenutexture;
        SDL_Texture* backHome;
        SDL_Texture *AgregarMusicaAPlaylist;
        SDL_Texture* PageSearch;
        SDL_Texture* canciontexture;
        //Pagina de user
        SDL_Texture* backUser;
        SDL_Texture* planGratuitoTexture;
        SDL_Texture* planPremiumTexture;
        SDL_Texture* comprarPremiumTexture;  // Textura para la página de compra
        bool showPaymentSuccess;
        Uint32 paymentSuccessTimer;
        SDL_Rect tarjetaNumberRect;               // Área para el número de tarjeta
        SDL_Rect expireDateRect;             // Área para fecha de caducidad
        SDL_Rect cvRect;                    // Área para CVV
        char tarjetaNumber[20];                 // Almacenar número de tarjeta
        char caducidad[6];                  // Fecha de Almacenar (MM/YYi)
        char cvv[4];                         // Almacenar CVV
        char tipotarjeta[100];                  //tipo de tarjeta 
        bool boolActivo;                    // Para controlar entrada de texto
        int activoInputField;                // 1=cardNumber, 2=expiryFecha, 3=cvv
        char fechaStr[100];                // para guardar la fecha de renovacion
        bool pagoRealizado;  // Para que el botón se ejecute solo una vez
        // Crear campos para recivir datos de la tarjeta
        TextField numeroTarjetaField;
        TextField fechaCaducidadFieldMes;
        TextField fechaCaducidadFieldAnio;
        TextField cvvField;
        int intentosTarjeta; //intentos de validacion de la tarjeta
        //Reproductor
        Button playButton;
        Button pauseButton;
        Button prevButton;
        Button nextButton;
        Button friendsButton;
        Button planButton;
        Button logoutButton;
        Button historyButton;
        Button comprarButton;
        Button pagarButton;

        SDL_Rect miniMenu;
        SDL_Rect agregarPlaylist;
        SDL_Rect compartirMusica;
        SDL_Rect verArtista;
        SDL_Rect backButtonRect;
        SDL_Rect miniReproductor;
        SDL_Rect infoCancion;
        SDL_Texture* emailTexture;
        SDL_Texture* nicknameTexture;
        SDL_Texture* fechaTexture;

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
        int playlistPageIndex; 
        SDL_Rect playlistRects[6];
        playlist* playlistPtrs[6];  
        SDL_Rect pagSiguiente;
        SDL_Rect pagAnterior;
        playlist* selectedPlaylist;
        //Tiempo de reproduccion
        Uint32 songStartTime;
        Uint32 songElapsedTime;
        Uint32 pausedTime;


        //Variables del app(funciones y llamados basicos de render)
        int planTipo;
        char email[50];
        char nickname[30];
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
        
        //INFORMACION DE MUSICA CUADRO PEQUENIO
        // En tu struct App:
        SDL_Texture* tempImage;       // textura temporal para mostrar
        SDL_Rect tempImageRect;       // dónde dibujar la imagen
        Uint32 tempImageStartTime;    // momento cuando empezó a mostrarse (SDL_GetTicks)
        Uint32 tempImageDuration;     // duración en ms que debe mostrarse
        bool showingTempImage;  

        //SOLO PARA PRUEBAS
        UsuarioPrueba userAccount;

        //STRUCTURA DE FUENTES PARA LA APP
        //NOTA: AGREGAR SI NECESITAN DE OTRO TAMAÑO
        Fonts fonts;
        //MOTOR DE BUSQUEDA
        ResultadoBusqueda* resultados;


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
//Rederizar el tiempo de reproduccion
void renderSongTime(App* app, int x, int y);
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

//FUNCIONES DEL SEARCH
void handleSearchEvents(App* app, SDL_Event* e);

//FUNCION DE INFORMACION MUSICA
void startTempImage(App* app, SDL_Texture* image, SDL_Rect rect, Uint32 durationMs);


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
playlist* crearPlaylistDesdeAlbum(albumLista* album);
void agregarPlaylistALista(App* app, playlist* nueva);
void crearPlaylistsDesdeTodosLosAlbumes(App* app);
void recorrerABByCrearPlaylists(App* app, Artista* nodo);


//FUNCIONES USUARIO
//TODO:SEBAS GUZMAN
static int esSoloDigitos(char *cadena);
static int esNumeroExacto(char *cadena, int longitudEsperada);
static int validarLuhn(char *numero);
static char* tipoTarjeta(char *num);
static int esFechaExpValida(char *fecha);
int ValidarTarjeta(char *numero, char *fecha, char *cvv, char *tipotarjeta);
char* FechaMasUnMes();
void renderTextField(App* app, TextField* field);
void updateTextTexture(App* app, SDL_Texture** texture, const char* text);
void renderPaymentSuccess(App* app);
void deactivateAllFields(App* app);
void activateClickedField(App* app, SDL_Point p);
void handleBackButton(App* app, SDL_Point p);
void handleHomeState(App* app, SDL_Point p);
void handlePlanGratuitoState(App* app, SDL_Point p);
void handleComprarPremiumState(App* app, SDL_Point p);
void handlePlanPremiumState(App* app, SDL_Point p);
void handleTextInput(App* app, const SDL_Event* e);
void handleBackspace(App* app);
void handleTabNavigation(App* app);

//MOTOR DE BUSQUEDA
cancionNodo* buscarMotorDeBusqueda(cancionNodo* raiz, const char* nombre);
albumLista* buscarAlbumPorNombreMT(Artista* raiz, const char* nombre);
Artista* buscarArtistaPorNombreMT(Artista* raiz, const char* nombre);
void motorBusqueda(App* app, const char* textoBuscado);
playlist* buscarPlaylistPorNombre(playlist* lista, const char* nombre);
//FUNCION DE LIBERACION DE MEMORIA SDL, STRUCTS ETC
void cleanup(App* app);

int main(int argc, char* argv[]) {

    // //PRRIMERA PARTE LOGIN AND CREATE
    UsuarioPrueba userAccount;
    strcpy(userAccount.correo,"");
    
    // SDL_Window *ventana = NULL;         // Ventana principal del programa
    // SDL_Renderer *renderizador = NULL;  // Renderizador asociado a la ventana (para dibujar contenido)
    

    // fflush(stdin);

    // // Inicializa todo lo necesario para mostrar el login (ventana, renderizador, fuentes, texturas, etc.)
    // if (!inicializarLogin(&ventana, &renderizador)) {
    //     printf("Error al iniciar login\n");
    //     return 1; // Si falla la inicialización, termina el programa
    // }

    // bool corriendo = true;         // Controla si la aplicación sigue corriendo
    // SDL_Event evento;              // Estructura para capturar eventos del sistema (teclado, mouse, etc.)
    // fflush(stdin);
    // // Bucle principal: sigue mientras esté corriendo y no se haya solicitado salir
    // while (corriendo && obtenerEstadoLogin() == LOGIN_CONTINUAR) {
    //     // Manejo de todos los eventos que haya en la cola
    //     while (SDL_PollEvent(&evento)) {
    //         manejarEventosLogin(&evento, &corriendo); // Pasamos el evento a nuestro manejador
    //     }

    //     renderizarLogin();         // Dibuja la interfaz gráfica de login (o crear cuenta)
    //     SDL_Delay(16);             // Pausa ~16ms para limitar a ~60 FPS
    // }
    // fflush(stdin);
    // // Si fue login exitoso, mostrar la pantalla unos segundos antes de continuar
    // if (obtenerEstadoLogin() == LOGIN_EXITOSO) {
    //     Uint32 inicio = SDL_GetTicks();
    //     while (SDL_GetTicks() - inicio < 2000) {  // Mostrar durante 3 segundos
    //         renderizarLogin();                   // Sigue dibujando la pantalla
    //         SDL_Delay(16);
    //     }
    //     fflush(stdin);
    //     const char *correo = obtenerCorreoUsuario(); // esto devulve el correo del usuario logueado
    //     printf("Usuario logueado: %s\n", correo);
    //     strcpy(userAccount.correo,correo);
    // }

    // destruirLogin();  

    //     printf("Usuario logueado: %s\n", userAccount.correo);

    // if(userAccount.correo==NULL||strlen(userAccount.correo)==0||strlen(userAccount.correo)==1){
    //     return 0;
    // }

    App app = {0};

    strcpy(userAccount.codigoUsuario, "123");
    userAccount.premium=false;
    app.userAccount = userAccount;

    app.running = true;
    app.currentState = STATE_AGREGAR_A_PLAYLIST;
    app.premium = false;  // cambia según necesidad
    app.cancionesDesdeUltimoAnuncio = 0;
    app.reproduciendoAnuncio = false;
    app.adQueue.front = app.adQueue.rear = NULL;
    app.isMiniMenu=false;
    app.searchActive=false;
    app.planTipo = 0; // 1 = premium, 0 = gratuito
    app.pagoRealizado = false;  // Inicializar bandera
    strcpy(app.fechaStr, "05/10/2024");
    strcpy(app.email, "usuario@ejemplo.com");
    strcpy(app.nickname, "Usuario123");
    app.songStartTime=0;
    app.songElapsedTime=0;
    app.pausedTime=0;
    
    if (!initSDL(&app)) return 1;
    if (!loadMedia(&app)) return 1;
    if (!loadFonts(&app)) return 1;
    if (!loadAds(&app, "data/anuncios.txt")) {
        printf("Error cargando anuncios.\n");
        return 1;
    }
    app.contadorCodCanciones=0;
    app.contadorCodAlbumes=0;
    app.contadorCodArtistas=0;
    app.raizAlbumes=NULL;
    app.raizArtistas=NULL;
    app.raizCancion=NULL;
    app.playlistPageIndex=0;
    
    cargarArtistas(&app.raizArtistas,&app.contadorCodArtistas);
    cargarAlbumes(&app.raizArtistas,&app.contadorCodAlbumes);
    cargarCanciones(&app.raizArtistas, &app.raizCancion, &app.contadorCodCanciones);
    cargarAnuncios(&app.inicioAnuncios,&app.finAnuncios);
    listarCanciones(1);

    // char nombre[12];
    // if(app.raizArtistas->albumes->canciones->info->nombre!=NULL)
    printf("\n%s",app.raizArtistas->albumes->canciones->info->nombre);

    printf("\n%d",app.contadorCodCanciones);
    printf("\n%d",app.contadorCodAlbumes);
    printf("\n%d",app.contadorCodArtistas);

    char archivoGeneralPlaylist[100];
    strcpy(archivoGeneralPlaylist,"data/playlistListaUsuarios/");
    strcat(archivoGeneralPlaylist,userAccount.codigoUsuario);
    strcat(archivoGeneralPlaylist,"-ListaPlaylist.csv");
    crearPlaylistsDesdeTodosLosAlbumes(&app);
    cargarYMostrarPlaylists(&app, archivoGeneralPlaylist);
    // app.currentPlaylist = app.playlists; 
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
    // system("programa.exe");
    // ShellExecute(NULL, "open", "crud.exe", NULL, NULL, SW_SHOWNORMAL);
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
    //INICIALIZACIONES DE PAGEUSUARIO
     app->numeroTarjetaField.rect = (SDL_Rect){330, 245, 260, 24};
    strcpy(app->numeroTarjetaField.placeholder, "Numero de la tarjeta");
    app->numeroTarjetaField.text[0] = '\0';
    app->numeroTarjetaField.active = false;
    app->numeroTarjetaField.textColor = (SDL_Color){255, 255, 255, 255};

    // Inicializar campo: Mes de caducidad
    app->fechaCaducidadFieldMes.rect = (SDL_Rect){330, 340, 115, 24};
    strcpy(app->fechaCaducidadFieldMes.placeholder, "MM");
    app->fechaCaducidadFieldMes.text[0] = '\0';
    app->fechaCaducidadFieldMes.active = false;
    app->fechaCaducidadFieldMes.textColor = (SDL_Color){255, 255, 255, 255};

    // Inicializar campo: Año de caducidad
    app->fechaCaducidadFieldAnio.rect = (SDL_Rect){460, 340, 115, 24};
    strcpy(app->fechaCaducidadFieldAnio.placeholder, "AA");
    app->fechaCaducidadFieldAnio.text[0] = '\0';
    app->fechaCaducidadFieldAnio.active = false;
    app->fechaCaducidadFieldAnio.textColor = (SDL_Color){255, 255, 255, 255};

    // Inicializar campo: CVV
    app->cvvField.rect = (SDL_Rect){330, 432, 175, 24};
    strcpy(app->cvvField.placeholder, "CVV");
    app->cvvField.text[0] = '\0';
    app->cvvField.active = false;
    app->cvvField.textColor = (SDL_Color){255, 255, 255, 255};

    app->intentosTarjeta=0; //el numero de intenctos para validar la tarjeta



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
    app->backPlayer = loadTexture(app, "imgs/Img_Reproductor.png");
    if (!app->backPlayer) return false;

    app->playButton.texture = loadTexture(app, "imgs/promo3Play-removebg-preview.png");
    app->pauseButton.texture = loadTexture(app, "imgs/prom3Pause-removebg-preview.png");
    app->prevButton.texture = loadTexture(app, "imgs/promo3Atras-removebg-preview.png");
    app->nextButton.texture = loadTexture(app, "imgs/promo3Siguiente-removebg-preview.png");
    app->miniMenutexture=loadTexture(app,"imgs/MiniMenu.png");
    app->backHome= loadTexture(app,"imgs/Img_HomePage.png");
    app->AgregarMusicaAPlaylist=loadTexture(app,"imgs/AgregarPlaylist.png");
    app->PageSearch=loadTexture(app,"imgs/Img_Search.png");
    app->canciontexture=loadTexture(app,"imgs/Mini_InfoMusica.png");

    //TEXTURAS DE PAGINAS DE USUARIO
    app->backUser = loadTexture(app, "imgs/user page.png");
    app->planGratuitoTexture = loadTexture(app, "imgs/gratuito.png");
    app->planPremiumTexture = loadTexture(app, "imgs/premium.png");
    app->comprarPremiumTexture = loadTexture(app, "imgs/comprar premium.png");
    

    app->isPlaying = false;
    app->showPaymentSuccess = false;


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
    app->fonts.small1 = TTF_OpenFont("src/assets/Inder-Regular.ttf", 12);
    if (!app->fonts.small1) {
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
    if(app->isMiniMenu){
        SDL_Color color = {93, 109, 126, 255}; 
    }
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
void renderSongTime(App* app, int x, int y) {
    // Actualiza solo si está reproduciendo
    if (app->reproduciendoAnuncio) return; 
    if (app->isPlaying) {
        app->songElapsedTime = (SDL_GetTicks() - app->songStartTime) + app->pausedTime;
    }

    int totalSeconds = app->songElapsedTime / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    char timeStr[16];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);

    renderText(app, timeStr, app->fonts.small1, x, y);
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
    pl->album=false;
    if (pl == NULL) {
        printf("Error al asignar memoria para playlist.\n");
        exit(1);
    }

    pl->canciones = NULL;
    pl->sig = NULL;
    pl->album=false;
    // Guarda la ruta
    strcpy(pl->ruta, nombreArchivo);

    // Extrae el nombre de la playlist del nombre del archivo: codUsuario-Nombre.csv
    const char* nombreBase = strrchr(nombreArchivo, '/');
    if (nombreBase) nombreBase++; else nombreBase = nombreArchivo;

    const char* guion = strchr(nombreBase, '-');
    const char* punto = strrchr(nombreBase, '.');

    if (guion && punto && punto > guion) {
        int len = punto - (guion + 1);
        strncpy(pl->nombre, guion + 1, len);
        pl->nombre[len] = '\0';
    } else {
        strcpy(pl->nombre, "SinNombre");
    }

    // Leer cabecera: numeroCanciones;duracionPlaylist
    char linea[longNombres];
    if (fgets(linea, sizeof(linea), archivo) != NULL) {
        int numCanciones = 0, duracion = 0;
        sscanf(linea, "%d;%d", &numCanciones, &duracion);
        pl->numeroCanciones = numCanciones;
        pl->duracionPlaylist = duracion;
    }

    // Leer canciones
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        linea[strcspn(linea, "\n")] = 0;  // Elimina salto de línea

        if (strlen(linea) == 0) continue; // Ignora líneas vacías

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
        nueva->album=false;
        if (nueva) {
            // Insertar al final de la lista ligada app->playlists
            if (app->playlists == NULL) {
                app->playlists = nueva;
            } else {
                playlist* actual = app->playlists;
                while (actual->sig != NULL) {
                    actual = actual->sig;
                }
                actual->sig = nueva;
            }

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

playlist* crearPlaylistDesdeAlbum(albumLista* album) {
    if (!album || !album->info) return NULL;

    playlist* nueva = (playlist*)malloc(sizeof(playlist));
    if (!nueva) return NULL;

    strcpy(nueva->nombre, album->info->nombre);
    nueva->numeroCanciones = 0;
    nueva->duracionPlaylist = 0;
    nueva->sig = NULL;
    nueva->canciones = NULL;
    nueva->album=true;
    // Copiar canciones del álbum a la playlist
    cancionNodo* actual = album->canciones;
    playlistNodo* anterior = NULL;

    while (actual) {
        playlistNodo* nodoPlaylist = (playlistNodo*)malloc(sizeof(playlistNodo));
        nodoPlaylist->cancion = actual;
        nodoPlaylist->sig = NULL;
        nodoPlaylist->prev = anterior;

        if (!nueva->canciones) {
            nueva->canciones = nodoPlaylist;
        } else {
            anterior->sig = nodoPlaylist;
        }

        anterior = nodoPlaylist;
        nueva->numeroCanciones++;
        nueva->duracionPlaylist += actual->info->duracion;

        actual = actual->sig;
    }

    return nueva;
}
void agregarPlaylistALista(App* app, playlist* nueva) {
    if (!app || !nueva) return;

    nueva->sig = NULL;
    nueva->album=true;
    if (!app->playlists) {
        app->playlists = nueva;
    } else {
        playlist* aux = app->playlists;
        while (aux->sig) {
            aux = aux->sig;
        }
        aux->sig = nueva;
    }
}


void crearPlaylistsDesdeTodosLosAlbumes(App* app) {
    if (!app || !app->raizArtistas) return;

    recorrerABByCrearPlaylists(app, app->raizArtistas);
}
void recorrerABByCrearPlaylists(App* app, Artista* nodo) {
    if (!nodo) return;

    recorrerABByCrearPlaylists(app, nodo->izq);

    albumLista* album = nodo->albumes;
    while (album) {
        playlist* nueva = crearPlaylistDesdeAlbum(album);
        nueva->album=true;
        if (nueva) {
            agregarPlaylistALista(app, nueva);
        }
        album = album->sig;
    }

    recorrerABByCrearPlaylists(app, nodo->der);
}

void playCurrentSong(App* app) {
    if (!app->currentSong || !app->currentSong->cancion || !app->currentSong->cancion->info) return;
    app->songStartTime = SDL_GetTicks();
    app->pausedTime = 0;
    app->songElapsedTime = 0;

    // Si no es premium y le toca anuncio
    if (!app->premium && app->cancionesDesdeUltimoAnuncio >= 2 && !app->reproduciendoAnuncio && app->inicioAnuncios) {
        app->pendingSong = app->currentSong;

        anuncioNodo* ad = app->inicioAnuncios;

        Mix_HaltMusic();

        app->currentMusic = Mix_LoadMUS(ad->anuncio.dirAnuncio);
        if (!app->currentMusic) {
            printf("Error cargando anuncio %s: %s\n", ad->anuncio.dirAnuncio, Mix_GetError());
            return;
        }

        if (Mix_PlayMusic(app->currentMusic, 1) == -1) {
            printf("Error reproduciendo anuncio: %s\n", Mix_GetError());
        } else {
            app->reproduciendoAnuncio = true;
            app->isPlaying = true;
            printf("Reproduciendo anuncio: %s\n", ad->anuncio.dirAnuncio);
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
        //Resetear todos los botones
        app->playButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->prevButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->nextButton.rect = (SDL_Rect){0, 0, 0, 0};
        app->backButtonRect  = (SDL_Rect){0, 0, 0, 0};
        app->miniMenu        = (SDL_Rect){0, 0, 0, 0};
        app->miniReproductor = (SDL_Rect){0, 0, 0, 0};
        app->agregarPlaylist = (SDL_Rect){0, 0, 0, 0};
        app->compartirMusica = (SDL_Rect){0, 0, 0, 0};
        app->searchBoxRect   = (SDL_Rect){0, 0, 0, 0};
        app->searchActive = false;
        app->searchColor = (SDL_Color){ 0, 0, 0, 0};
        app->infoCancion   = (SDL_Rect){0, 0, 0, 0};
        app->pagSiguiente=(SDL_Rect){0,0,0,0};
            app->pagAnterior=(SDL_Rect){0,0,0,0};
        memset(&app->friendsButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->planButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->logoutButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->historyButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->comprarButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->pagarButton.rect, 0, sizeof(SDL_Rect));
        memset(&app->backButtonRect, 0, sizeof(SDL_Rect));
        app->searchActive = false;
        app->playlistPageIndex=0;
        switch (app->currentState) {
        case STATE_HOME:
            app->playButton.rect = (SDL_Rect){ 490, 587, 64, 64 };
            app->prevButton.rect = (SDL_Rect){ 400, 590, 60, 60 };
            app->nextButton.rect = (SDL_Rect){ 580, 586, 64, 64 };
            app->backButtonRect = (SDL_Rect){65, 40, 80, 80 };
            // app->miniMenu = (SDL_Rect){ 0, 0, 0, 0 };
            app->miniReproductor=(SDL_Rect){275,541,454,111};
            app->searchActive = false;
            strcpy(app->searchText, "");
            app->searchBoxRect = (SDL_Rect) {150, 45, 725, 70} ;  // Usa coordenadas reales
            // app->searchColor = (SDL_Color){ 255, 255, 255, 255};
            app->pagSiguiente=(SDL_Rect){35,520, 148, 44};
            app->pagAnterior=(SDL_Rect){825,520, 148, 44};
            break;

        case STATE_PLAYER:
            app->playButton.rect = (SDL_Rect){ 481, 520, 80, 80 };
            app->prevButton.rect = (SDL_Rect){ 350, 523, 80, 80 };
            app->nextButton.rect = (SDL_Rect){ 612, 520, 80, 80 };
            app->miniMenu = (SDL_Rect){ 930, 540, 50, 50 };
            app->agregarPlaylist=(SDL_Rect){750,250,225,102};
            app->compartirMusica =(SDL_Rect){750,352,225,102};
            app->backButtonRect = (SDL_Rect){30, 20, 60, 60};
            app->searchBoxRect   = (SDL_Rect){150, 45, 725, 70};
            app->searchColor = (SDL_Color){ 0, 0, 0, 255};
            app->infoCancion   = (SDL_Rect){750,454,225,102};
            
            break;
        case STATE_AGREGAR_A_PLAYLIST:
            app->backButtonRect = (SDL_Rect){0, 0, 50, 50};
            app->pagSiguiente=(SDL_Rect){35,590, 148, 44};
            app->pagAnterior=(SDL_Rect){835,590, 148, 44};
        break;
        case STATE_SEARCH:
            app->backButtonRect = (SDL_Rect){50, 70, 60, 60};
            app->searchBoxRect   = (SDL_Rect){150, 45, 725, 70};

            SDL_StartTextInput();
            app->searchActive = true;
            app->searchText[0] = '\0'; 
        break;
        case STATE_USERPAGE:
            app->friendsButton.rect = (SDL_Rect){446, 421, 142, 47};
            app->planButton.rect = (SDL_Rect){446, 342, 142, 48};
            app->logoutButton.rect = (SDL_Rect){445, 590, 146, 48};
            app->historyButton.rect = (SDL_Rect){426, 500, 184, 48};
            app->backButtonRect = (SDL_Rect){30, 10, 60, 60};
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
    SDL_Rect boton3Puntos = {35,520, 148, 44};
    // app->pagSiguiente=(SDL_Rect){35,520, 148, 44};
    // app->pagAnterior=(SDL_Rect){825,520, 148, 44};
    
    SDL_Rect playlistRects[PLAYLISTS_PER_PAGE] = {
        {105,175, 303, 78},  // playlist1
        {625,175, 303, 78},  // playlist2
        {105,290, 303, 78},  // playlist3
        {615,290, 303, 78},  // playlist4
        {105,415, 303, 78},  // playlist5
        {615,415, 303, 78}   // playlist6
    };
    SDL_Rect playlistRects2[PLAYLISTS_PER_PAGE] = {
        {200,340, 303, 78},  // playlist1
        {645,340, 303, 78},  // playlist2
        {200,440, 303, 78},  // playlist3
        {645,440, 303, 78},  // playlist4
        {200,540, 303, 78},  // playlist5
        {645,540, 303, 78}   // playlist6
    };
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color bgColor = {0, 0, 255, 255};

    // playlist* renderPlaylits=app->playlists;
    

    int minutes = app->currentSong->cancion->info->duracion;  // Este es tu int, en MINUTOS.
    int seconds = 0;  // No tienes segundos, entonces 00.
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d", minutes, seconds);


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
                    app->fonts.small1,
                    430, 560);
            }
            //  renderTextInRect(app,"Hola", app->fonts.medium,
            //                     app->backButtonRect,
            //                     textColor,
            //                     bgColor,
            //                     true); 

            // 💡 Avanza a la playlist de inicio de la página actual
            playlist* renderPlaylists = app->playlists;
            int skip = app->playlistPageIndex * PLAYLISTS_PER_PAGE;
            for (int i = 0; i < skip && renderPlaylists != NULL; i++) {
                renderPlaylists = renderPlaylists->sig;
            }

            // Renderiza hasta 6 desde ahí
            for (int i = 0; i < PLAYLISTS_PER_PAGE && renderPlaylists != NULL; i++) {
                renderTextInRect(app, renderPlaylists->nombre, app->fonts.medium,
                                playlistRects[i],
                                textColor,
                                bgColor,
                                false);
                renderPlaylists = renderPlaylists->sig;
            }
            renderSongTime(app,  700, 550);
            // renderText(app, timeStr, app->fonts.small1, 690, 503);
          
            break;

        case STATE_PLAYER:
            SDL_RenderCopy(app->renderer, app->backPlayer, NULL, NULL);

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
                        app->fonts.small1,
                        rectNextMusic,
                        textColor,
                        bgColor,
                        false);
                    renderSongTime(app,  300, 503);
                    

                    // Y ahora lo pasas a render:
                    renderText(app, timeStr, app->fonts.small1, 690, 503);
                    
            }

            if (app->isMiniMenu) {
                SDL_RenderCopy(app->renderer, app->miniMenutexture, NULL, &miniMenu);
                Uint32 now = SDL_GetTicks();
                if (app->tempMessage && now - app->errorInicios < app->errorFin) {
                        renderText(app, app->tempMessage, app->fonts.small1, 820, 540);
                    } else {
                        app->tempMessage = NULL;
                    }
                }
           
            if (app->showingTempImage) {
                Uint32 now = SDL_GetTicks();
                if (now - app->tempImageStartTime < app->tempImageDuration) {
                    SDL_RenderCopy(app->renderer, app->canciontexture, NULL, &app->tempImageRect);
                    
                    // printf("%s",app->currentSong->cancion->album->info->nombre);
                    renderText(app,app->currentSong->cancion->album->punteroArtista->info->nombre,app->fonts.medium,793, 315);
                    renderText(app,app->currentSong->cancion->album->info->nombre,app->fonts.medium,793, 400);
                    app->isMiniMenu=false;
                    
                } else {
                    app->showingTempImage = false;  // termina la muestra
                }
            }
            // renderTextInRect(app,"Hola", app->fonts.medium,
            //                     app->backButtonRect,
            //                     textColor,
            //                     bgColor,
            //                     true); 

            break;
        case STATE_AGREGAR_A_PLAYLIST:
            SDL_RenderCopy(app->renderer, app->AgregarMusicaAPlaylist, NULL, NULL);
            playlist* renderPlaylists2 = app->playlists;
            int skip2 = app->playlistPageIndex * PLAYLISTS_PER_PAGE;

            // Avanza skip2 playlists que **NO sean album==true**
            int skipped = 0;
            while (renderPlaylists2 != NULL && skipped < skip2) {
                if (renderPlaylists2->album == false) {
                    skipped++;
                }
                renderPlaylists2 = renderPlaylists2->sig;
            }

            // Ahora renderiza hasta PLAYLISTS_PER_PAGE playlists que no sean album==true
            int rendered = 0;
            playlist* current = renderPlaylists2;
            while (current != NULL && rendered < PLAYLISTS_PER_PAGE) {
                if (current->album == false) {
                    // Usa solo x, y del rect
                    int x = playlistRects2[rendered].x;
                    int y = playlistRects2[rendered].y;

                    renderText(app, current->nombre, app->fonts.medium, x, y);
                    rendered++;
                }
                current = current->sig;
            }
        break;
        case STATE_SEARCH:
            SDL_RenderCopy(app->renderer, app->PageSearch, NULL, NULL);
                    // Renderizar cuadro de búsqueda (fondo)
            // SDL_SetRenderDrawColor(app->renderer, 50, 50, 50, 255);  // gris oscuro
            // SDL_RenderFillRect(app->renderer, &app->searchBoxRect);

            // // Renderizar borde
            // SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);  // blanco
            // SDL_RenderDrawRect(app->renderer, &app->searchBoxRect);

            // Renderizar texto actual dentro del rectángulo
            if (strlen(app->searchText) > 0) {
                renderTextInRect(app, app->searchText, app->fonts.medium, app->searchBoxRect, app->searchColor, (SDL_Color){0,0,0,255}, false);
            } 
                    // renderTextInRect(app,"Hola", app->fonts.medium,
                    //             boton3Puntos,
                    //             textColor,
                    //             bgColor,
                    //             true);
                    // renderTextInRect(app,"Hola", app->fonts.medium,
                    //             app->backButtonRect,
                    //             textColor,
                    //             bgColor,
                    //             true); 
        break;
        
        case STATE_USERPAGE:

            if (app->backUser) {
                SDL_RenderCopy(app->renderer, app->backUser, NULL, NULL);
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
            // renderTextInRect(app,"Hola", app->fonts.medium,
            //                     app->backButtonRect,
            //                     textColor,
            //                     bgColor,
            //                     true);     
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

            if (!app->fechaTexture) {
                updateTextTexture(app, &app->fechaTexture, app->fechaStr);
            }

            SDL_Rect fechaRect = {470, 235, 190, 25}; // Ajusta estas coordenadas
            
                                                                                                                                                                                                                                                                                                                                                                                                   
            if (app->fechaTexture) {
                SDL_QueryTexture(app->fechaTexture, NULL, NULL, &fechaRect.w, &fechaRect.h);
                SDL_RenderCopy(app->renderer, app->fechaTexture, NULL, &fechaRect);
            }

            break;
        case STATE_COMPRAR_PREMIUM:
            if (app->comprarPremiumTexture) {
                SDL_RenderCopy(app->renderer, app->comprarPremiumTexture, NULL, NULL);
            } 

            
            renderTextField(app, &app->numeroTarjetaField);         // ← Función que tú defines
            renderTextField(app, &app->fechaCaducidadFieldMes);
            renderTextField(app, &app->fechaCaducidadFieldAnio);
            renderTextField(app, &app->cvvField);
            

            break;
    
        default:
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
    SDL_Rect miniMenu={750,250,225,308};
    SDL_Rect playlist1={};
    SDL_Rect playlist2={};
    SDL_Rect playlist3={};
    SDL_Rect playlist4={};
    SDL_Rect playlist5={};
    SDL_Rect playlist6={};
    
    // SDL_Rect pagSiguiente={35,520, 148, 44};
    // SDL_Rect pagAnterior={825,520, 148, 44};
    SDL_Rect playlistRects[PLAYLISTS_PER_PAGE] = {
        {105,175, 303, 78},  // playlist1
        {625,175, 303, 78},  // playlist2
        {105,290, 303, 78},  // playlist3
        {615,290, 303, 78},  // playlist4
        {105,415, 303, 78},  // playlist5
        {615,415, 303, 78}   // playlist6
    };
    SDL_Rect playlistRects2[PLAYLISTS_PER_PAGE] = {
        {200,340, 303, 78},  // playlist1
        {645,340, 303, 78},  // playlist2
        {200,440, 303, 78},  // playlist3
        {645,440, 303, 78},  // playlist4
        {200,540, 303, 78},  // playlist5
        {645,540, 303, 78}   // playlist6
    };
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            app->running = false;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point p = {x, y};

            if (app->currentState == STATE_COMPRAR_PREMIUM) {
                deactivateAllFields(app);
                activateClickedField(app, p);
                
            }

            // handleBackButton(app, p);

            switch (app->currentState) {
                case STATE_USERPAGE:
                    handleHomeState(app, p);
                       if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->currentState = STATE_HOME;
                            setupButtonsForState(app);
                        }
                        
                    break;
                case STATE_PLAN_GRATUITO:
                    handlePlanGratuitoState(app, p);
                      if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->currentState = STATE_USERPAGE;
                            setupButtonsForState(app);
                        }
                    break;
                case STATE_COMPRAR_PREMIUM:
                    handleComprarPremiumState(app, p);
                     if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->boolActivo = false;
                            app->activoInputField = 0;
                            SDL_StopTextInput();
                            app->currentState = STATE_USERPAGE;
                            setupButtonsForState(app);
                            
                        }
                        if (app->boolActivo) {
                            SDL_StartTextInput();
                        } else {
                            SDL_StopTextInput();
                        }
                    if(SDL_PointInRect(&p,&app->numeroTarjetaField.rect)){
                            app->boolActivo = true;

                    }
                    break;
                case STATE_PLAN_PREMIUM:
                    handlePlanPremiumState(app, p);
                     if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->currentState = STATE_USERPAGE;
                            setupButtonsForState(app);
                        }
                    break;
                case STATE_HOME:
                    if (app->reproduciendoAnuncio) {
                        printf("Anuncio en reproducción: controles bloqueados.\n");
                        if(SDL_PointInRect(&p, &app->miniReproductor)&&!SDL_PointInRect(&p, &app->nextButton.rect)&&!SDL_PointInRect(&p, &app->playButton.rect)&&!SDL_PointInRect(&p, &app->prevButton.rect)){
                            app->currentState=STATE_PLAYER;
                            setupButtonsForState(app);
                        }
                         if(SDL_PointInRect(&p, &app->searchBoxRect)){
                            app->currentState=STATE_SEARCH;
                            setupButtonsForState(app);
                        }
                        if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->currentState = STATE_USERPAGE;
                            setupButtonsForState(app);
                        }

                        break;
                    }else{
                        
                        handlePlaybackControls(app, p);
                    }
                    
                    playlist* clickedPlaylist = NULL; // Para guardar la que toque

                    playlist* checkPlaylists = app->playlists;
                    int skip = app->playlistPageIndex * PLAYLISTS_PER_PAGE;

                    // Salta las playlists que no van en la página actual
                    for (int i = 0; i < skip && checkPlaylists != NULL; i++) {
                        checkPlaylists = checkPlaylists->sig;
                    }

                    // Verifica cada rect de esta página
                    for (int i = 0; i < PLAYLISTS_PER_PAGE && checkPlaylists != NULL; i++) {
                        if (SDL_PointInRect(&p, &playlistRects[i])) {
                            clickedPlaylist = checkPlaylists;
                            break; // Ya encontraste cuál fue tocada
                        }
                        checkPlaylists = checkPlaylists->sig;
                    }

                    if (clickedPlaylist != NULL) {
                        printf("Playlist clickeada: %s\n", clickedPlaylist->nombre);
                        app->selectedPlaylist = clickedPlaylist; 
                    }


                    if(SDL_PointInRect(&p, &app->miniReproductor)&&!SDL_PointInRect(&p, &app->nextButton.rect)&&!SDL_PointInRect(&p, &app->playButton.rect)&&!SDL_PointInRect(&p, &app->prevButton.rect)){
                        app->currentState=STATE_PLAYER;
                        setupButtonsForState(app);
                    }
                    
                    if(SDL_PointInRect(&p, &app->searchBoxRect)){
                        app->currentState=STATE_SEARCH;
                        setupButtonsForState(app);
                    }
                    if(SDL_PointInRect(&p, &app->pagSiguiente)){
                          if (app->playlistPageIndex > 0)
                            app->playlistPageIndex--;
                       
                    }
                    if(SDL_PointInRect(&p, &app->pagAnterior)){
                       app->playlistPageIndex++;
                    }
                       if (SDL_PointInRect(&p, &app->backButtonRect)) {
                            app->currentState = STATE_USERPAGE;
                            setupButtonsForState(app);
                        }
                    break;

                case STATE_PLAYER:
                    if (app->reproduciendoAnuncio) {
                        printf("Anuncio en reproducción: controles bloqueados.\n");
                        if(SDL_PointInRect(&p, &app->backButtonRect)){
                            app->currentState= STATE_HOME;
                            setupButtonsForState(app);
                        }
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
                            app->isMiniMenu=false;
                    }
                    if(app->isMiniMenu&&SDL_PointInRect(&p,&app->compartirMusica)){
                        showTemporaryMessage(app, "Link copiado", 3000);  // 3 segundos
                          printf("Link copiado");   
                    }
                    if(app->isMiniMenu&&SDL_PointInRect(&p,&app->infoCancion)){
                            printf("INFOCANCION");
                            app->showingTempImage=true;
                            // app->isMiniMenu=false;
                            SDL_Rect rect = {750, 250, 289, 313};  // o el rect que quieras
                            startTempImage(app, app->canciontexture, rect, 5000); 
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
                    playlist* clickedPlaylist2 = NULL;

                    playlist* checkPlaylists2 = app->playlists;
                    int skip2 = app->playlistPageIndex * PLAYLISTS_PER_PAGE;

                    // Avanza skip2 playlists que NO sean álbum
                    int skipped = 0;
                    while (checkPlaylists2 != NULL && skipped < skip2) {
                        if (checkPlaylists2->album == false) {
                            skipped++;
                        }
                        checkPlaylists2 = checkPlaylists2->sig;
                    }

                    // Ahora revisa hasta PLAYLISTS_PER_PAGE playlists
                    int rendered = 0;
                    playlist* current = checkPlaylists2;

                    while (current != NULL && rendered < PLAYLISTS_PER_PAGE) {
                        if (current->album == false) {
                            SDL_Rect rect;
                            rect.x = playlistRects2[rendered].x;
                            rect.y = playlistRects2[rendered].y;
                            rect.w = 303;  // O el ancho de tu rect si quieres usarlo
                            rect.h = 78;   // O el alto que definas

                            if (SDL_PointInRect(&p, &rect)) {
                                clickedPlaylist2 = current;
                                break;
                            }

                            rendered++;
                        }
                        current = current->sig;
                    }

                    if (clickedPlaylist2 != NULL) {
                        printf("Playlist clickeada para agregar: %s\n", clickedPlaylist2->nombre);
                        app->selectedPlaylist = clickedPlaylist2; 
                    }
                    if(SDL_PointInRect(&p, &app->pagSiguiente)){
                          if (app->playlistPageIndex > 0)
                            app->playlistPageIndex--;
                       
                    }
                    if(SDL_PointInRect(&p, &app->pagAnterior)){
                       app->playlistPageIndex++;
                    }
                break;  
                case STATE_SEARCH:
                    //   handleSearchEvents(app, &e);

                    if (SDL_PointInRect(&p, &app->backButtonRect)) {
                        app->currentState = STATE_HOME;
                        SDL_StopTextInput();         // 👈 Apaga TextInput
                        app->searchActive = false;   // 👈 Flag a false
                        setupButtonsForState(app);
                    } else if (SDL_PointInRect(&p, &app->searchBoxRect)) {
                        if (!app->searchActive) {    // 👈 Evita reiniciar en cada clic
                            SDL_StartTextInput();
                            app->searchActive = true;
                        }
                    } 
                break; 
            }

        } else if (e.type == SDL_TEXTINPUT) {
             if (app->currentState == STATE_SEARCH && app->searchActive) {
                handleSearchEvents(app, &e);  // ya lo tienes
            }
            if (app->currentState == STATE_COMPRAR_PREMIUM && app->boolActivo) {
                handleTextInput(app, &e);     // mete texto en tarjeta/fecha/cvv
            }
        } else if (e.type == SDL_KEYDOWN) {
                if (app->currentState == STATE_SEARCH && app->searchActive) {
                    handleSearchEvents(app, &e);
                }
            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                handleBackspace(app);
            } else if (e.key.keysym.sym == SDLK_TAB && app->currentState == STATE_COMPRAR_PREMIUM) {
                handleTabNavigation(app);
            }
        }
    }
    
}

void handlePlaybackControls(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->playButton.rect)) {
        if (app->isPlaying) {
            Mix_PauseMusic();
            app->pausedTime = app->songElapsedTime;
            app->isPlaying = false;
        } else {
            if (Mix_PlayingMusic() == 0) {
                playCurrentSong(app);
            } else {
                app->songStartTime = SDL_GetTicks();
                Mix_ResumeMusic();
                app->isPlaying = true;
            }
        }
    } else if (SDL_PointInRect(&p, &app->prevButton.rect)) {
        if (app->currentSong && app->currentSong->prev) {
            app->currentSong = app->currentSong->prev;
            app->cancionesDesdeUltimoAnuncio++;
            app->songStartTime = SDL_GetTicks();
            app->pausedTime = 0;
            app->songElapsedTime = 0;
            playCurrentSong(app);
        }
    } else if (SDL_PointInRect(&p, &app->nextButton.rect)) {
        if (app->currentSong && app->currentSong->sig) {
            app->currentSong = app->currentSong->sig;
            app->cancionesDesdeUltimoAnuncio++;
            app->songStartTime = SDL_GetTicks();
            app->pausedTime = 0;    
            app->songElapsedTime = 0;
            playCurrentSong(app);
        }
    }
}

void handleSearchEvents(App* app, SDL_Event* e) {
    if (e->type == SDL_TEXTINPUT) {
        if (strlen(app->searchText) + strlen(e->text.text) < sizeof(app->searchText) - 1) {
            strcat(app->searchText, e->text.text);
        }
    } else if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_BACKSPACE && strlen(app->searchText) > 0) {
            app->searchText[strlen(app->searchText) - 1] = '\0';
        } else if (e->key.keysym.sym == SDLK_RETURN || e->key.keysym.sym == SDLK_KP_ENTER) {
            printf("Busqueda: %s\n", app->searchText);
            motorBusqueda(app, app->searchText);
            // app->currentState = STATE_HOME;
            setupButtonsForState(app);
            SDL_StopTextInput();
            app->searchActive = false;
        } else if (e->key.keysym.sym == SDLK_ESCAPE) {
            app->currentState = STATE_HOME;
            setupButtonsForState(app);
            SDL_StopTextInput();
            app->searchActive = false;
        }
    }
}

void startTempImage(App* app, SDL_Texture* image, SDL_Rect rect, Uint32 durationMs) {
    app->tempImage = image;
    app->tempImageRect = rect;
    app->tempImageStartTime = SDL_GetTicks();
    app->tempImageDuration = durationMs;
    app->showingTempImage = true;
}
void updatePlayback(App* app) {
    if (app->reproduciendoAnuncio && !Mix_PlayingMusic()) {
        app->reproduciendoAnuncio = false;
        app->cancionesDesdeUltimoAnuncio = 0;

        anuncioNodo* ad = app->inicioAnuncios;
        if (ad) {
            app->inicioAnuncios = ad->sig;
            if (!app->inicioAnuncios) app->finAnuncios = NULL;

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

//FUNCIONES DE USUARIO
static int esSoloDigitos(char *cadena) {
    if (!cadena) return 0;
    for (int i = 0; cadena[i]; i++) {
        if (!isdigit(cadena[i])) return 0;
    }
    return 1;
}
// Valida que una cadena numérica tenga la longitud exacta

static int esNumeroExacto(char *cadena, int longitudEsperada) {
    return strlen(cadena) == longitudEsperada && esSoloDigitos(cadena);
}
// Algoritmo de Luhn para tarjetas
static int validarLuhn(char *numero) {
    int suma = 0, alternar = 0;
    for (int i = 15; i >= 0; i--) {
        int digito = numero[i] - '0';
        if (alternar) {
            digito *= 2;
            if (digito > 9) digito -= 9;
        }
        suma += digito;
        alternar = !alternar;
    }
    return suma % 10 == 0;
}
// Detecta el tipo de tarjeta
static char* tipoTarjeta(char *num) {
    if (num[0] == '4') return "Visa";
    if (num[0] == '5' && num[1] >= '1' && num[1] <= '5') return "MasterCard";
    if (num[0] == '3' && (num[1] == '4' || num[1] == '7')) return "American Express";
    return "Desconocida";
}
// Valida fecha de expiración en formato MM/AA
static int esFechaExpValida(char *fecha) {
    if (strlen(fecha) != 5 || fecha[2] != '/') return 0;
    int mm, aa;
    if (sscanf(fecha, "%2d/%2d", &mm, &aa) != 2) return 0;
    if (mm < 1 || mm > 12) return 0;

    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int anio = now->tm_year % 100;
    int mes = now->tm_mon + 1;

    return (aa > anio || (aa == anio && mm >= mes));
}

// Función principal de validación
int ValidarTarjeta(char *numero, char *fecha, char *cvv, char *tipotarjeta) {
    // Validar número (16 dígitos + Luhn)
    if (!esNumeroExacto(numero, 16) || !validarLuhn(numero)) {
        return 0;  // Número inválido
    }

    // Validar fecha (MM/AA)
    if (!esFechaExpValida(fecha)) {
        return 0;  // Fecha inválida
    }

    // Validar CVV (3 o 4 dígitos)
    if (!esNumeroExacto(cvv, 3) && !esNumeroExacto(cvv, 4)) {
        return 0;  // CVV inválido
    }

    // Guardar el tipo de tarjeta si se proporcionó el buffer
    if (tipotarjeta) {
        char *tipo = tipoTarjeta(numero);
        strncpy(tipotarjeta, tipo, 20);
    }

    return 1;  // Todo válido
}

//la fecha de renovacion
char* FechaMasUnMes() {
    time_t tiempoActual = time(NULL);
    struct tm *tiempoLocal = localtime(&tiempoActual);

    // Aumentar un mes
    tiempoLocal->tm_mon += 1;

    // Normalizar la fecha
    mktime(tiempoLocal);

    // Reservar memoria para la cadena de fecha
    char *fechaStr = malloc(100);
    if (fechaStr == NULL) {
        return NULL; // Error de memoria
    }

    // Formatear la fecha
    strftime(fechaStr, 100, "%d/%m/%Y", tiempoLocal);

    return fechaStr;
}

//-----------------------------------------------------

void renderTextField(App* app, TextField* field) {
    char displayText[32] = {0};  // Inicializar a cero
    const char* sourceText = strlen(field->text) > 0 ? field->text : field->placeholder;
    
    // Formatear número de tarjeta (16 dígitos con espacios)
    if (field == &app->numeroTarjetaField) {
        if (strlen(field->text) > 0) {
            // Tomar solo los primeros 16 dígitos
            char digits[17] = {0}; // 16 + null terminator
            int digitCount = 0;
            
            // Extraer solo dígitos (hasta 16)
            for (int i = 0; field->text[i] && digitCount < 16; i++) {
                if (isdigit(field->text[i])) {
                    digits[digitCount++] = field->text[i];
                }
            }
            digits[digitCount] = '\0';
            
            // Formatear con espacios cada 4 dígitos
            int displayPos = 0;
            for (int i = 0; i < digitCount && displayPos < sizeof(displayText)-1; i++) {
                if (i > 0 && i % 4 == 0 && displayPos < sizeof(displayText)-2) {
                    displayText[displayPos++] = ' ';
                }
                displayText[displayPos++] = digits[i];
            }
            displayText[displayPos] = '\0';
        } else {
            strcpy(displayText, field->placeholder);
        }
    }
    // Campos de fecha (MM y AA)
    else if (field == &app->fechaCaducidadFieldMes || field == &app->fechaCaducidadFieldAnio) {
        if (strlen(field->text) > 0) {
            // Limitar a 2 caracteres para MM/AA
            strncpy(displayText, field->text, 2);
            displayText[2] = '\0';
        } else {
            strcpy(displayText, field->placeholder);
        }
    }
    // Campo CVV
    else if (field == &app->cvvField) {
        if (strlen(field->text) > 0) {
            // Mostrar los números reales ingresados
            strncpy(displayText, field->text, sizeof(displayText)-1);
            displayText[sizeof(displayText)-1] = '\0';
        } else {
            // Mostrar "CVV" como placeholder cuando está vacío
            strcpy(displayText, "CVV");
        }
    }
    else {
        strncpy(displayText, sourceText, sizeof(displayText)-1);
        displayText[sizeof(displayText)-1] = '\0';
    }

    // Crear superficie de texto
    SDL_Surface* textSurface = TTF_RenderText_Blended(app->fonts.medium, displayText, field->textColor);
    if (!textSurface) {
        printf("No se pudo crear la superficie de texto: %s\n", TTF_GetError());
        return;
    }

    // Crear textura desde la superficie
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(app->renderer, textSurface);
    if (!textTexture) {
        printf("No se pudo crear la textura de texto: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    // Calcular posición del texto (centrado verticalmente)
    SDL_Rect textRect = field->rect;
    textRect.x += 5; // Pequeño margen izquierdo
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.y += (field->rect.h - textSurface->h) / 2;

    // Dibujar el campo de texto
    SDL_SetRenderDrawColor(app->renderer, 
                          field->active ? 50 : 30, 
                          field->active ? 50 : 30, 
                          field->active ? 50 : 30, 
                          255);
    SDL_RenderFillRect(app->renderer, &field->rect);

    // Borde del campo
    SDL_SetRenderDrawColor(app->renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(app->renderer, &field->rect);

    // Renderizar el texto
    SDL_RenderCopy(app->renderer, textTexture, NULL, &textRect);

    // Dibujar cursor (solo si el campo está activo y enfocado)
    if (field->active && (SDL_GetTicks() / 500) % 2 == 0) {
        SDL_Rect cursorRect = {
            textRect.x + (strlen(displayText) ? textRect.w : 0),
            textRect.y + 2,
            2,
            textRect.h - 4
        };
        
        // Guardar estado actual del renderer
        SDL_Color oldColor;
        SDL_GetRenderDrawColor(app->renderer, &oldColor.r, &oldColor.g, &oldColor.b, &oldColor.a);
        
        
        
        SDL_RenderFillRect(app->renderer, &cursorRect);
        
        // Restaurar color original
        SDL_SetRenderDrawColor(app->renderer, oldColor.r, oldColor.g, oldColor.b, oldColor.a);
    }

    // Liberar recursos
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
void updateTextTexture(App* app, SDL_Texture** texture, const char* text) {
    if (*texture) SDL_DestroyTexture(*texture);
    
    if (app->fonts.medium && text) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(app->fonts.medium, text, white);
        if (surface) {
            *texture = SDL_CreateTextureFromSurface(app->renderer, surface);
            SDL_FreeSurface(surface);
        }
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
    if (app->fonts.medium) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(app->fonts.medium, "Pago exitoso!", white);
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

void deactivateAllFields(App* app) {
    app->numeroTarjetaField.active = false;
    app->fechaCaducidadFieldMes.active = false;
    app->fechaCaducidadFieldAnio.active = false;
    app->cvvField.active = false;
}

void activateClickedField(App* app, SDL_Point p) {
    deactivateAllFields(app);  // <--- Desactivas todo primero

    if (SDL_PointInRect(&p, &app->numeroTarjetaField.rect)) {
        app->numeroTarjetaField.active = true;
        SDL_StartTextInput();
    } else if (SDL_PointInRect(&p, &app->fechaCaducidadFieldMes.rect)) {
        app->fechaCaducidadFieldMes.active = true;
        SDL_StartTextInput();
    } else if (SDL_PointInRect(&p, &app->fechaCaducidadFieldAnio.rect)) {
        app->fechaCaducidadFieldAnio.active = true;
        SDL_StartTextInput();
    } else if (SDL_PointInRect(&p, &app->cvvField.rect)) {
        app->cvvField.active = true;
        SDL_StartTextInput();
    } else {
        // Si clickeaste fuera de campos, para evitar input fantasma
        SDL_StopTextInput();
    }
}
void handleBackButton(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->backButtonRect)) {
        printf("Botón presionado: Retroceso\n");
        app->currentState = STATE_HOME;
        setupButtonsForState(app);
    }
}

void handleHomeState(App* app, SDL_Point p) {
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
}

void handlePlanGratuitoState(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->comprarButton.rect)) {
        printf("Botón presionado: Comprar plan gratuito\n");
        app->currentState = STATE_COMPRAR_PREMIUM;
        setupButtonsForState(app);
    }
}

void handleComprarPremiumState(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->comprarButton.rect)) {
        printf("Botón presionado: Comprar plan premium\n");

        // Extraer número de tarjeta
        int idx = 0;
        for (int i = 0; app->numeroTarjetaField.text[i] && idx < 16; i++) {
            if (isdigit(app->numeroTarjetaField.text[i])) {
                app->tarjetaNumber[idx++] = app->numeroTarjetaField.text[i];
            }
        }
        app->tarjetaNumber[idx] = '\0';

        // Fecha de caducidad
        if (strlen(app->fechaCaducidadFieldMes.text) >= 2 && strlen(app->fechaCaducidadFieldAnio.text) >= 2) {
            strncpy(app->caducidad, app->fechaCaducidadFieldMes.text, 2);
            app->caducidad[2] = '/';
            strncpy(app->caducidad + 3, app->fechaCaducidadFieldAnio.text, 2);
            app->caducidad[5] = '\0';
        } else {
            app->caducidad[0] = '\0';
        }

        // CVV
        strncpy(app->cvv, app->cvvField.text, 3);
        app->cvv[3] = '\0';

        if (strlen(app->tarjetaNumber) < 16 || strlen(app->fechaCaducidadFieldMes.text) < 2 ||
            strlen(app->fechaCaducidadFieldAnio.text) < 2 || strlen(app->cvvField.text) < 3) {
            printf("❌ Por favor complete todos los campos\n");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
                                     "Campos incompletos",
                                     "Por favor complete todos los campos requeridos.",
                                     app->window);
            return;
        }

        int resultadoValidacion = ValidarTarjeta(app->tarjetaNumber, app->caducidad, app->cvv, app->tipotarjeta);
        if (resultadoValidacion != 1) {
            app->intentosTarjeta++;
            printf("❌ Tarjeta inválida (intento %d de 3)\n", app->intentosTarjeta);

            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     "Error de pago",
                                     "Tarjeta inválida. Por favor verifique los datos.",
                                     app->window);

            app->cvvField.text[0] = '\0';
            app->cvv[0] = '\0';

            app->numeroTarjetaField.active = true;
            app->fechaCaducidadFieldMes.active = false;
            app->fechaCaducidadFieldAnio.active = false;
            app->cvvField.active = false;

            if (app->intentosTarjeta >= 3) {
                printf("⚠️ Demasiados intentos fallidos. Redirigiendo al plan gratuito.\n");

                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
                                         "Límite de intentos",
                                         "Demasiados intentos fallidos. Se ha asignado el plan gratuito.",
                                         app->window);

                app->numeroTarjetaField.text[0] = '\0';
                app->fechaCaducidadFieldMes.text[0] = '\0';
                app->fechaCaducidadFieldAnio.text[0] = '\0';
                app->tarjetaNumber[0] = '\0';
                app->caducidad[0] = '\0';

                app->currentState = STATE_PLAN_GRATUITO;
                app->planTipo = 0;
            }
        } else {
            char* nuevaFecha = FechaMasUnMes();
            if (nuevaFecha) {
                strcpy(app->fechaStr, nuevaFecha);
                free(nuevaFecha);
            }

            app->planTipo = 1;
            app->currentState = STATE_PLAN_PREMIUM;
            app->intentosTarjeta = 0;
        }

        setupButtonsForState(app);
    }
}

void handlePlanPremiumState(App* app, SDL_Point p) {
    if (SDL_PointInRect(&p, &app->pagarButton.rect) && !app->pagoRealizado) {
        printf("Botón presionado: Pagar plan premium\n");
        app->showPaymentSuccess = true;
        app->paymentSuccessTimer = SDL_GetTicks() + 2000;
        app->pagoRealizado = true;
    }
}

void handleTextInput(App* app, const SDL_Event* e) {
    if (app->currentState == STATE_COMPRAR_PREMIUM) {
        TextField* activeField = NULL;

        if (app->numeroTarjetaField.active) activeField = &app->numeroTarjetaField;
        else if (app->fechaCaducidadFieldMes.active) activeField = &app->fechaCaducidadFieldMes;
        else if (app->fechaCaducidadFieldAnio.active) activeField = &app->fechaCaducidadFieldAnio;
        else if (app->cvvField.active) activeField = &app->cvvField;

        if (activeField && strlen(activeField->text) < sizeof(activeField->text) - 1) {
            if ((activeField == &app->fechaCaducidadFieldMes ||
                 activeField == &app->fechaCaducidadFieldAnio ||
                 activeField == &app->cvvField) &&
                (e->text.text[0] < '0' || e->text.text[0] > '9')) {
                return;
            }

            if ((activeField == &app->fechaCaducidadFieldMes && strlen(activeField->text) >= 2) ||
                (activeField == &app->fechaCaducidadFieldAnio && strlen(activeField->text) >= 2) ||
                (activeField == &app->cvvField && strlen(activeField->text) >= 3)) {
                return;
            }

            strcat(activeField->text, e->text.text);
        }
    }
}

void handleBackspace(App* app) {
    if (app->currentState == STATE_COMPRAR_PREMIUM) {
        TextField* activeField = NULL;

        if (app->numeroTarjetaField.active) activeField = &app->numeroTarjetaField;
        else if (app->fechaCaducidadFieldMes.active) activeField = &app->fechaCaducidadFieldMes;
        else if (app->fechaCaducidadFieldAnio.active) activeField = &app->fechaCaducidadFieldAnio;
        else if (app->cvvField.active) activeField = &app->cvvField;

        if (activeField && strlen(activeField->text) > 0) {
            activeField->text[strlen(activeField->text) - 1] = '\0';
        }
    }
}

void handleTabNavigation(App* app) {
    TextField* fields[] = {
        &app->numeroTarjetaField,
        &app->fechaCaducidadFieldMes,
        &app->fechaCaducidadFieldAnio,
        &app->cvvField
    };

    int current = -1;
    for (int i = 0; i < 4; i++) {
        if (fields[i]->active) {
            current = i;
            break;
        }
    }

    if (current != -1) {
        fields[current]->active = false;
        fields[(current + 1) % 4]->active = true;
    }
}

//MOTOR DE BUSQUEDA
cancionNodo* buscarMotorDeBusqueda(cancionNodo* raiz, const char* nombre) {
    if (!raiz) return NULL;
    int cmp = strcmp(nombre, raiz->info->nombre);
    if (cmp == 0) return raiz;
    else if (cmp < 0) return buscarMotorDeBusqueda(raiz->izquierda, nombre);
    else return buscarMotorDeBusqueda(raiz->derecha, nombre);
}
albumLista* buscarAlbumPorNombreMT(Artista* raiz, const char* nombre) {
    if (!raiz) return NULL;

    // Revisar álbumes de este artista
    albumLista* actual = raiz->albumes;
    while (actual) {
        if (strcmp(actual->info->nombre, nombre) == 0) return actual;
        actual = actual->sig;
    }

    // Revisar en subárbol izquierdo y derecho
    albumLista* izq = buscarAlbumPorNombreMT(raiz->izq, nombre);
    if (izq) return izq;
    return buscarAlbumPorNombreMT(raiz->der, nombre);
}
Artista* buscarArtistaPorNombreMT(Artista* raiz, const char* nombre) {
    if (!raiz) return NULL;
    int cmp = strcmp(nombre, raiz->info->nombre);
    if (cmp == 0) return raiz;
    else if (cmp < 0) return buscarArtistaPorNombreMT(raiz->izq, nombre);
    else return buscarArtistaPorNombreMT(raiz->der, nombre);
}
playlist* buscarPlaylistPorNombre(playlist* lista, const char* nombre) {
    while (lista) {
        if (strcmp(lista->nombre, nombre) == 0) return lista;
        lista = lista->sig;
    }
    return NULL;
}
void motorBusqueda(App* app, const char* textoBuscado) {
    cancionNodo* cancion = buscarMotorDeBusqueda(app->raizCancion, textoBuscado);
    if (cancion) {
        printf("Encontrada cancion: %s\n", cancion->info->nombre);
        return;
    }

    albumLista* album = buscarAlbumPorNombreMT(app->raizArtistas, textoBuscado);
    if (album) {
        printf("Encontrado album: %s\n", album->info->nombre);
        return;
    }

    Artista* artista = buscarArtistaPorNombreMT(app->raizArtistas, textoBuscado);
    if (artista) {
        printf("Encontrado artista: %s\n", artista->info->nombre);
        return;
    }
    playlist* listaEncontrada = buscarPlaylistPorNombre(app->playlists, textoBuscado);
    if (listaEncontrada) {
        printf("Encontrada playlist: %s\n", listaEncontrada->nombre);
        return;
    }
    printf("No se encontro nada para: %s\n", textoBuscado);
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
    anuncioNodo* ad = app->inicioAnuncios;
    while (ad) {
        anuncioNodo* next = ad->sig;
        free(ad);
        ad = next;
    }
    app->inicioAnuncios = NULL;
    app->finAnuncios = NULL;

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
    if (app->backPlayer) SDL_DestroyTexture(app->backPlayer);
    if (app->miniMenutexture) SDL_DestroyTexture(app->miniMenutexture);
    if (app->backHome) SDL_DestroyTexture(app->backHome);
    if(app->AgregarMusicaAPlaylist) SDL_DestroyTexture(app->AgregarMusicaAPlaylist);
    if (app->PageSearch) SDL_DestroyTexture(app->PageSearch);
    if (app->planPremiumTexture) SDL_DestroyTexture(app->planPremiumTexture);
    if (app->planGratuitoTexture) SDL_DestroyTexture(app->planGratuitoTexture);
    if (app->backUser) SDL_DestroyTexture(app->backUser);
    if (app->comprarButton.texture) SDL_DestroyTexture(app->comprarButton.texture);
    if (app->pagarButton.texture) SDL_DestroyTexture(app->pagarButton.texture);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    if (app->emailTexture) SDL_DestroyTexture(app->emailTexture);
    if (app->nicknameTexture) SDL_DestroyTexture(app->nicknameTexture);
        
    // Liberar botones faltantes
    if (app->friendsButton.texture) SDL_DestroyTexture(app->friendsButton.texture);
    if (app->planButton.texture) SDL_DestroyTexture(app->planButton.texture);
    if (app->logoutButton.texture) SDL_DestroyTexture(app->logoutButton.texture);
    if (app->historyButton.texture) SDL_DestroyTexture(app->historyButton.texture);

    // Liberar texturas faltantes
    if (app->canciontexture) SDL_DestroyTexture(app->canciontexture);
    if (app->fechaTexture) SDL_DestroyTexture(app->fechaTexture);
    if (app->tempImage) SDL_DestroyTexture(app->tempImage);

    // ---------------------------
    // 7️⃣ Cerrar fuentes
    // ---------------------------
    if (app->fonts.small1) TTF_CloseFont(app->fonts.small1);
    if (app->fonts.medium) TTF_CloseFont(app->fonts.medium);
    if (app->fonts.large) TTF_CloseFont(app->fonts.large);

    // ---------------------------
    // 8️⃣ Destruir renderer y ventana
    // ---------------------------
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    app->renderer = NULL;
    app->window = NULL;

    if (app->currentMusic) Mix_FreeMusic(app->currentMusic);

    // 2️⃣
    if (app->searchTexture) SDL_DestroyTexture(app->searchTexture);
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