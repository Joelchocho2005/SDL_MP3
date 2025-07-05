// StateLogin.c
#include "StateLogin.h"
#include <string.h>
#include <stdio.h>

#define MAX_INPUT_LENGTH 256



// -------------------- VARIABLES GLOBALES -------------------- //

static SDL_Window *ventana = NULL;
static SDL_Renderer *renderizador = NULL;
static TTF_Font *fuente = NULL;

static SDL_Texture *fondoLoginTexture = NULL;
static SDL_Texture *fondoRegistroTexture = NULL;

static SDL_Cursor *arrowCursor = NULL;
static SDL_Cursor *handCursor  = NULL;

static bool mostrarPassword = false;
static bool mostrarMensajeError = false;
static char mensajeError[100] = "";

static EstadoLogin estadoLoginInterno = LOGIN_CONTINUAR;

static Uint32 tiempoCursor = 0;
static bool mostrarCursor = true;

static enum {
    PANTALLA_LOGIN,
    PANTALLA_REGISTRO
} pantallaActual = PANTALLA_LOGIN;

// Estructuras para campos de entrada y botones
typedef struct {
    SDL_Rect rect;
    char text[MAX_INPUT_LENGTH];
    char placeholder[50];
    bool active;
    SDL_Color textColor;
    SDL_Color bgColor;
    SDL_Color borderColor;
} InputField;

typedef struct {
    SDL_Rect rect;
    SDL_Color bgColor;
    SDL_Color borderColor;
    SDL_Color textColor;
    const char *label;
} Button;

// Campos de login
static InputField emailField, passField;
static Button mostrarContraBtn, iniciarSesionBtn, crearCuentaBtn;

// Campos de registro
static InputField nombreField, apellidoField, diaField, mesField, anioField;
static InputField nicknameField, paisField, correoField;
static InputField contraseniaField, contraseniaConfirmarField;
static Button crearCuentaFinalBtn, volverBtn;

// -------------------- FUNCIONES AUXILIARES -------------------- //

static SDL_Texture* cargarTextura(const char *ruta) {
    SDL_Surface *surface = IMG_Load(ruta);
    if (!surface) return NULL;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderizador, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button *btn) {

    SDL_SetRenderDrawColor(renderer, btn->bgColor.r, btn->bgColor.g, btn->bgColor.b, 255);
    if (btn->bgColor.a > 0) {
        SDL_SetRenderDrawColor(renderer, btn->bgColor.r, btn->bgColor.g, btn->bgColor.b, btn->bgColor.a);
        SDL_RenderFillRect(renderer, &btn->rect);
    }
    // borde del boton 
    // SDL_SetRenderDrawColor(renderer, btn->borderColor.r, btn->borderColor.g, btn->borderColor.b, 255);
    // SDL_RenderDrawRect(renderer, &btn->rect);

    // Renderizar el texto del botón
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, btn->label, btn->textColor);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect;
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;
        textRect.x = btn->rect.x + (btn->rect.w - textRect.w) / 2;
        textRect.y = btn->rect.y + (btn->rect.h - textRect.h) / 2;

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

static bool validarCamposRegistro() {
    // Validar campos vacíos
    if (
        strlen(nombreField.text) == 0 || strlen(apellidoField.text) == 0 ||
        strlen(diaField.text) == 0 || strlen(mesField.text) == 0 || strlen(anioField.text) == 0 ||
        strlen(nicknameField.text) == 0 || strlen(paisField.text) == 0 ||
        strlen(correoField.text) == 0 || strlen(contraseniaField.text) == 0 ||
        strlen(contraseniaConfirmarField.text) == 0
    ) {
        strcpy(mensajeError, "Faltan campos por llenar");
        mostrarMensajeError = true;
        return false;
    }

    // Validar fecha
    int dia = atoi(diaField.text);
    int mes = atoi(mesField.text);
    int anio = atoi(anioField.text);

    if (dia < 1 || dia > 31) {
        strcpy(mensajeError, "Dia invalido (1-31)");
        mostrarMensajeError = true;
        return false;
    }

    if (mes < 1 || mes > 12) {
        strcpy(mensajeError, "Mes invalido (1-12)");
        mostrarMensajeError = true;
        return false;
    }

    if (anio < 1900 || anio > 2025) {
        strcpy(mensajeError, "Anio invalido (1900-2025)");
        mostrarMensajeError = true;
        return false;
    }

    // Verificar contraseñas iguales
    if (strcmp(contraseniaField.text, contraseniaConfirmarField.text) != 0) {
        strcpy(mensajeError, "Las contrasenias no coinciden");
        mostrarMensajeError = true;
        return false;
    }

    return true; // Todo está bien
}

static void renderInputField(SDL_Renderer *renderer, TTF_Font *font, InputField *field, bool ocultarTexto, bool activo);


// -------------------- FUNCIONES DE LA INTERFAZ -------------------- //

bool inicializarLogin(SDL_Window **windowOut, SDL_Renderer **rendererOut) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    if (TTF_Init() != 0) return false;
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) return false;

    ventana = SDL_CreateWindow("Login System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1026, 652, 0);
    if (!ventana) return false;

    renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!renderizador) return false;

    fuente = TTF_OpenFont("src/assets/Inder-Regular.ttf", 15);
    if (!fuente) return false;

    *windowOut = ventana;
    *rendererOut = renderizador;

    SDL_StartTextInput();

    fondoLoginTexture = cargarTextura("imgs/Login.png");
    fondoRegistroTexture = cargarTextura("imgs/CrearCuenta.png");

    arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    handCursor  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_SetCursor(arrowCursor);

    // Input para correo (emailField)
        emailField.rect = (SDL_Rect){250, 359, 528, 30};
        strcpy(emailField.placeholder, "Ingresar Contrasenia");
        emailField.text[0] = '\0';
        emailField.active = false;
        emailField.textColor = (SDL_Color){255, 255, 255, 255};

        // Input para contraseña (passField)
        passField.rect = (SDL_Rect){250, 452, 528, 30};
        strcpy(passField.placeholder, "Ingresar Contrasenia");
        passField.text[0] = '\0';
        passField.active = false;
        passField.textColor = (SDL_Color){255, 255, 255, 255};


    // -------------------- CAMPOS DE REGISTRO (CREAR CUENTA) --------------------
        // Campo: Nombre
    nombreField.rect = (SDL_Rect){327, 164, 170, 24};
    strcpy(nombreField.placeholder, "Nombre");
    nombreField.text[0] = '\0';
    nombreField.active = false;
    nombreField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Apellido
    apellidoField.rect = (SDL_Rect){519, 164, 170, 24};
    strcpy(apellidoField.placeholder, "Apellido");
    apellidoField.text[0] = '\0';
    apellidoField.active = false;
    apellidoField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Dia 
    diaField.rect = (SDL_Rect){327, 223, 113, 24};
    strcpy(diaField.placeholder, "DD");
    diaField.text[0] = '\0';
    diaField.active = false;
    diaField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Mes
    mesField.rect = (SDL_Rect){452, 224, 113, 24};
    strcpy(mesField.placeholder, "MM");
    mesField.text[0] = '\0';
    mesField.active = false;
    mesField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Anio
    anioField.rect = (SDL_Rect){577, 223, 113, 24};
    strcpy(anioField.placeholder, "AA");
    anioField.text[0] = '\0';
    anioField.active = false;
    anioField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Nickname
    nicknameField.rect = (SDL_Rect){327, 283, 170, 24};
    strcpy(nicknameField.placeholder, "Nickname");
    nicknameField.text[0] = '\0';
    nicknameField.active = false;
    nicknameField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Pais
    paisField.rect = (SDL_Rect){328, 344, 170, 24};
    strcpy(paisField.placeholder, "Pais");
    paisField.text[0] = '\0';
    paisField.active = false;
    paisField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Correo
    correoField.rect = (SDL_Rect){327, 403, 363, 24};
    strcpy(correoField.placeholder, "Correo");
    correoField.text[0] = '\0';
    correoField.active = false;
    correoField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Contrasenia
    contraseniaField.rect = (SDL_Rect){327, 463, 363, 24};
    strcpy(contraseniaField.placeholder, "Contrasenia");
    contraseniaField.text[0] = '\0';
    contraseniaField.active = false;
    contraseniaField.textColor = (SDL_Color){255, 255, 255, 255};
        // Campo: Confirmar Contrasenia
    contraseniaConfirmarField.rect = (SDL_Rect){327, 522, 363, 24};
    strcpy(contraseniaConfirmarField.placeholder, "Confirmar Contrasenia");
    contraseniaConfirmarField.text[0] = '\0';
    contraseniaConfirmarField.active = false;
    contraseniaConfirmarField.textColor = (SDL_Color){255, 255, 255, 255};
        //Boton "Crear Cuenta del createAccount"
    crearCuentaFinalBtn.rect = (SDL_Rect){374, 590, 276, 48};
    crearCuentaFinalBtn.borderColor = (SDL_Color){250, 0, 0, 255};
    crearCuentaFinalBtn.label = "";

    volverBtn.rect = (SDL_Rect){40, 25, 24, 18};  // Ajusta según diseño
    volverBtn.borderColor = (SDL_Color){250, 0, 0, 255};  // Rojo para debug
    volverBtn.label = "";


    // Botón "Mostrar"
    mostrarContraBtn.rect = (SDL_Rect){739, 487, 20, 20}; //mostrarContraBtn.rect = (SDL_Rect){635, 248, 60, 30};
    mostrarContraBtn.bgColor = (SDL_Color){0, 0, 0, 0};
    mostrarContraBtn.borderColor = (SDL_Color){250, 0, 0, 255}; // borde rojo para el debug
    mostrarContraBtn.label = "";

    // Botón "Iniciar sesión"
    iniciarSesionBtn.rect = (SDL_Rect){375, 540, 275, 45};
    iniciarSesionBtn.borderColor = (SDL_Color){250, 0, 0, 255};
    iniciarSesionBtn.label = "";

    // Botón "Crear cuenta"
    crearCuentaBtn.rect = (SDL_Rect){433, 600, 160, 35};
    crearCuentaBtn.borderColor = (SDL_Color){250, 0, 0, 255};
    crearCuentaBtn.label = "";

    return true;
}

void manejarEventosLogin(SDL_Event *event, bool *corriendo) {
    if (event->type == SDL_QUIT) {
        *corriendo = false;
        estadoLoginInterno = LOGIN_SALIR;
        return;
    }

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x = event->button.x, y = event->button.y;
        SDL_Point mouse = {x, y};

        // Desactivar todos los campos
        emailField.active = passField.active = false;
        nombreField.active = apellidoField.active = diaField.active = false;
        mesField.active = anioField.active = nicknameField.active = false;
        paisField.active = correoField.active = false;
        contraseniaField.active = contraseniaConfirmarField.active = false;

        if (pantallaActual == PANTALLA_LOGIN) {
            emailField.active = SDL_PointInRect(&mouse, &emailField.rect);
            passField.active = SDL_PointInRect(&mouse, &passField.rect);

            if (SDL_PointInRect(&mouse, &mostrarContraBtn.rect)) {
                mostrarPassword = !mostrarPassword;
            } else if (SDL_PointInRect(&mouse, &iniciarSesionBtn.rect)) {
                mostrarMensajeError = false;
                if (strlen(emailField.text) == 0) {
                    strcpy(mensajeError, "Falta correo");
                    mostrarMensajeError = true;
                } else if (strlen(passField.text) == 0) {
                    strcpy(mensajeError, "Falta contrasenia");
                    mostrarMensajeError = true;
                } else {
                    printf("Email: %s\n", emailField.text);
                    printf("Password: %s\n", passField.text);
                }
            } else if (SDL_PointInRect(&mouse, &crearCuentaBtn.rect)) {
                pantallaActual = PANTALLA_REGISTRO;
                emailField.text[0] = passField.text[0] = '\0';
                mostrarMensajeError = false;
                mensajeError[0] = '\0';
            }
        } else if (pantallaActual == PANTALLA_REGISTRO) {
            if (SDL_PointInRect(&mouse, &volverBtn.rect)) {
                pantallaActual = PANTALLA_LOGIN;
                mostrarMensajeError = false;
                mensajeError[0] = '\0';
            } else if (SDL_PointInRect(&mouse, &crearCuentaFinalBtn.rect)) {
                mostrarMensajeError = false;
                
                if (!validarCamposRegistro()) {
                    // Ya se configuró el mensaje de error adentro de la función
                    return;
                }

                // Si pasó la validación
                    printf("Nombre: %s\n", nombreField.text);
                    printf("Apellido: %s\n", apellidoField.text);
                    printf("Fecha de nacimiento: %s/%s/%s\n", diaField.text, mesField.text, anioField.text);
                    printf("Nickname: %s\n", nicknameField.text);
                    printf("Pais: %s\n", paisField.text);
                    printf("Correo: %s\n", correoField.text);
                    printf("Contrasenia: %s\n", contraseniaField.text);
                    printf("Confirmar: %s\n", contraseniaConfirmarField.text);
                
            } else {
                nombreField.active = SDL_PointInRect(&mouse, &nombreField.rect);
                apellidoField.active = SDL_PointInRect(&mouse, &apellidoField.rect);
                diaField.active = SDL_PointInRect(&mouse, &diaField.rect);
                mesField.active = SDL_PointInRect(&mouse, &mesField.rect);
                anioField.active = SDL_PointInRect(&mouse, &anioField.rect);
                nicknameField.active = SDL_PointInRect(&mouse, &nicknameField.rect);
                paisField.active = SDL_PointInRect(&mouse, &paisField.rect);
                correoField.active = SDL_PointInRect(&mouse, &correoField.rect);
                contraseniaField.active = SDL_PointInRect(&mouse, &contraseniaField.rect);
                contraseniaConfirmarField.active = SDL_PointInRect(&mouse, &contraseniaConfirmarField.rect);
            }
        }
    } else if (event->type == SDL_MOUSEMOTION) {
            SDL_Point mouse = { event->motion.x, event->motion.y };

            bool hovering = false;

            if (pantallaActual == PANTALLA_LOGIN) {
                if (SDL_PointInRect(&mouse, &mostrarContraBtn.rect) ||
                    SDL_PointInRect(&mouse, &iniciarSesionBtn.rect) ||
                    SDL_PointInRect(&mouse, &crearCuentaBtn.rect) ||
                    SDL_PointInRect(&mouse, &emailField.rect) ||
                    SDL_PointInRect(&mouse, &passField.rect)) {
                    hovering = true;
                }
            } else if (pantallaActual == PANTALLA_REGISTRO) {
                if (SDL_PointInRect(&mouse, &crearCuentaFinalBtn.rect) ||
                    SDL_PointInRect(&mouse, &volverBtn.rect) ||
                    SDL_PointInRect(&mouse, &nombreField.rect) ||
                    SDL_PointInRect(&mouse, &apellidoField.rect) ||
                    SDL_PointInRect(&mouse, &diaField.rect) ||
                    SDL_PointInRect(&mouse, &mesField.rect) ||
                    SDL_PointInRect(&mouse, &anioField.rect) ||
                    SDL_PointInRect(&mouse, &nicknameField.rect) ||
                    SDL_PointInRect(&mouse, &paisField.rect) ||
                    SDL_PointInRect(&mouse, &correoField.rect) ||
                    SDL_PointInRect(&mouse, &contraseniaField.rect) ||
                    SDL_PointInRect(&mouse, &contraseniaConfirmarField.rect)) {
                    hovering = true;
                }
            }

            SDL_SetCursor(hovering ? handCursor : arrowCursor);
    }


    else if (event->type == SDL_TEXTINPUT) {
    InputField *active = NULL;

    // Buscar campo activo
    if (pantallaActual == PANTALLA_LOGIN) {
        if (emailField.active) active = &emailField;
        else if (passField.active) active = &passField;
    } else {
        if (nombreField.active) active = &nombreField;
        else if (apellidoField.active) active = &apellidoField;
        else if (diaField.active) active = &diaField;
        else if (mesField.active) active = &mesField;
        else if (anioField.active) active = &anioField;
        else if (nicknameField.active) active = &nicknameField;
        else if (paisField.active) active = &paisField;
        else if (correoField.active) active = &correoField;
        else if (contraseniaField.active) active = &contraseniaField;
        else if (contraseniaConfirmarField.active) active = &contraseniaConfirmarField;
    }

    if (active && strlen(active->text) + strlen(event->text.text) < MAX_INPUT_LENGTH - 1) {
        // Validar campos numéricos
        if ((active == &diaField || active == &mesField || active == &anioField) &&
            (event->text.text[0] < '0' || event->text.text[0] > '9')) return;

        if ((active == &diaField && strlen(active->text) >= 2) ||
            (active == &mesField && strlen(active->text) >= 2) ||
            (active == &anioField && strlen(active->text) >= 4)) return;

        // Verificar que quepa visualmente
        char textoTemp[MAX_INPUT_LENGTH];
        strcpy(textoTemp, active->text);
        strcat(textoTemp, event->text.text);

        int anchoTexto = 0, altoTexto = 0;
        if (TTF_SizeText(fuente, textoTemp, &anchoTexto, &altoTexto) == 0) {
            if (anchoTexto + 5 < active->rect.w) { // margen lateral
                strcat(active->text, event->text.text);
            }
        }
    }
}


    else if (event->type == SDL_KEYDOWN) {
        InputField *active = NULL;
        if (pantallaActual == PANTALLA_LOGIN) {
            active = emailField.active ? &emailField : (passField.active ? &passField : NULL);
        } else {
            if (nombreField.active) active = &nombreField;
            else if (apellidoField.active) active = &apellidoField;
            else if (diaField.active) active = &diaField;
            else if (mesField.active) active = &mesField;
            else if (anioField.active) active = &anioField;
            else if (nicknameField.active) active = &nicknameField;
            else if (paisField.active) active = &paisField;
            else if (correoField.active) active = &correoField;
            else if (contraseniaField.active) active = &contraseniaField;
            else if (contraseniaConfirmarField.active) active = &contraseniaConfirmarField;
        }
        if (active) {
            if (event->key.keysym.sym == SDLK_BACKSPACE && strlen(active->text) > 0) {
                active->text[strlen(active->text) - 1] = '\0';
            } else if (event->key.keysym.sym == SDLK_TAB) {
                 if (pantallaActual == PANTALLA_LOGIN) {
                    if (emailField.active) {
                        emailField.active = false;
                        passField.active = true;
                    } else {
                        emailField.active = true;
                        passField.active = false;
                    }
                }else {
                    // Lista ordenada de campos en registro
                    InputField* campos[] = {
                        &nombreField, &apellidoField, &diaField, &mesField, &anioField,
                        &nicknameField, &paisField, &correoField,
                        &contraseniaField, &contraseniaConfirmarField
                    };

                    int total = sizeof(campos) / sizeof(campos[0]);
                    for (int i = 0; i < total; i++) {
                        if (campos[i]->active) {
                            campos[i]->active = false;
                            campos[(i + 1) % total]->active = true;  // ciclo
                            break;
                        }
                    }
                }
            }
        }
    }
}
    // Aquí irá todo el contenido de SDL_MOUSEBUTTONDOWN, SDL_TEXTINPUT y SDL_KEYDOWN
    // adaptado a nuestras variables estáticas


void renderizarLogin() {
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    SDL_Texture *fondo = (pantallaActual == PANTALLA_LOGIN) ? fondoLoginTexture : fondoRegistroTexture;
    if (fondo) {
        SDL_Rect dest = { 0, 0, 1026, 652 };
        SDL_RenderCopy(renderizador, fondo, NULL, &dest);
    }

    if (SDL_GetTicks() - tiempoCursor > 500) {
    mostrarCursor = !mostrarCursor;
    tiempoCursor = SDL_GetTicks();
    }

    // Aquí dibujarías los campos y botones
    if (pantallaActual == PANTALLA_LOGIN) {
        renderInputField(renderizador, fuente, &emailField, false, emailField.active);
        renderInputField(renderizador, fuente, &passField, !mostrarPassword, passField.active);

        // Actualizar color de fondo según estado
    mostrarContraBtn.bgColor = mostrarPassword
    ? (SDL_Color){65, 95, 135, 255}  // fondo azul oscuro si está visible
    : (SDL_Color){200, 200, 200, 255};  // fondo gris claro si está oculto


        renderButton(renderizador, fuente, &mostrarContraBtn);
        renderButton(renderizador, fuente, &iniciarSesionBtn);
        renderButton(renderizador, fuente, &crearCuentaBtn);
    }

    if (pantallaActual == PANTALLA_REGISTRO) {
        renderInputField(renderizador, fuente, &nombreField, false, nombreField.active);
        renderInputField(renderizador, fuente, &apellidoField, false, apellidoField.active);
        renderInputField(renderizador, fuente, &diaField, false, diaField.active);
        renderInputField(renderizador, fuente, &mesField, false, mesField.active);
        renderInputField(renderizador, fuente, &anioField, false, anioField.active);
        renderInputField(renderizador, fuente, &nicknameField, false, nicknameField.active);
        renderInputField(renderizador, fuente, &paisField, false, paisField.active);
        renderInputField(renderizador, fuente, &correoField, false, correoField.active);
        renderInputField(renderizador, fuente, &contraseniaField, true, contraseniaField.active);
        renderInputField(renderizador, fuente, &contraseniaConfirmarField, true, contraseniaConfirmarField.active);

        renderButton(renderizador, fuente, &crearCuentaFinalBtn);
        renderButton(renderizador, fuente, &volverBtn);

    }

    // Mostrar error si aplica
    if (mostrarMensajeError) {
        SDL_Color rojo = {255, 0, 0, 255};
        SDL_Surface *errSurface = TTF_RenderText_Blended(fuente, mensajeError, rojo);
        SDL_Texture *errTexture = SDL_CreateTextureFromSurface(renderizador, errSurface);
        SDL_Rect errRect = { 50, 600, errSurface->w, errSurface->h };
        SDL_RenderCopy(renderizador, errTexture, NULL, &errRect);
        SDL_FreeSurface(errSurface);
        SDL_DestroyTexture(errTexture);
    }

    SDL_RenderPresent(renderizador);
}

static void renderInputField(SDL_Renderer *renderer, TTF_Font *font, InputField *field, bool ocultarTexto, bool activo) {
    // Determinar el texto a mostrar (oculto o no)
    char buffer[MAX_INPUT_LENGTH];
    if (ocultarTexto) {
        memset(buffer, '*', strlen(field->text));
        buffer[strlen(field->text)] = '\0';
    } else {
        strcpy(buffer, field->text);
    }

    const char *textoMostrar = (strlen(buffer) > 0) ? buffer : field->placeholder;
    SDL_Color colorTexto = (strlen(buffer) > 0) ? field->textColor : (SDL_Color){255, 255, 255, 255};

    // Renderizar texto
    SDL_Surface *surfaceTexto = TTF_RenderText_Blended(font, textoMostrar, colorTexto);
    if (!surfaceTexto) return;

    SDL_Texture *textura = SDL_CreateTextureFromSurface(renderer, surfaceTexto);
    SDL_Rect destino = {
        field->rect.x + 5,
        field->rect.y + (field->rect.h - surfaceTexto->h) / 2 ,
        surfaceTexto->w,
        surfaceTexto->h
    };

    SDL_RenderCopy(renderer, textura, NULL, &destino);

    SDL_FreeSurface(surfaceTexto);
    SDL_DestroyTexture(textura);

    // Dibujar solo una línea inferior si está activo
   
        /*if (activo && mostrarCursor) {
            // Dibujar cursor estilo '|'
            int cursorX = destino.x + destino.w + 2;
            int cursorY1 = destino.y;
            int cursorY2 = destino.y + destino.h;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // blanco
            SDL_RenderDrawLine(renderer, cursorX, cursorY1, cursorX, cursorY2);
        }*/
        // Si está activo, dibujar línea inferior o borde opcional
        /*if (activo) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Línea roja
            SDL_RenderDrawLine(renderer, field->rect.x, field->rect.y + field->rect.h,
                                        field->rect.x + field->rect.w, field->rect.y + field->rect.h);
        }*/

    if (activo && mostrarCursor) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // blanco
        SDL_RenderDrawLine(renderer, field->rect.x, field->rect.y + field->rect.h,
                                       field->rect.x + field->rect.w, field->rect.y + field->rect.h);
    }
}




       
void destruirLogin() {
    SDL_StopTextInput();
    if (fondoLoginTexture) SDL_DestroyTexture(fondoLoginTexture);
    if (fondoRegistroTexture) SDL_DestroyTexture(fondoRegistroTexture);
    if (fuente) TTF_CloseFont(fuente);
    if (renderizador) SDL_DestroyRenderer(renderizador);
    if (ventana) SDL_DestroyWindow(ventana);
    if (arrowCursor) SDL_FreeCursor(arrowCursor);
    if (handCursor) SDL_FreeCursor(handCursor);

    fuente = NULL;
    renderizador = NULL;
    ventana = NULL;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    estadoLoginInterno = LOGIN_CONTINUAR;
}

void cambiarPantallaLogin() {
    pantallaActual = PANTALLA_LOGIN;
}

void cambiarPantallaRegistro() {
    pantallaActual = PANTALLA_REGISTRO;
}

EstadoLogin obtenerEstadoLogin() {
    return estadoLoginInterno;
}
