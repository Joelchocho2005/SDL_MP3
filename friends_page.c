#include "friends_page.h"
#include <stdio.h>

static FriendsPage page;

void Friends_Init(SDL_Renderer* renderer, const char** friendsList, int count) {
    // Cargar recursos
    page.background = IMG_LoadTexture(renderer, "imgs/Friends_page.png");
    page.friendIcon = IMG_LoadTexture(renderer, "imgs/Friend_icon.png");
    page.font = TTF_OpenFont("src/assets/Inder.ttf", 20);
    
    // Configurar lista de amigos
    page.totalFriends = (count > MAX_FRIENDS) ? MAX_FRIENDS : count;
    for(int i = 0; i < page.totalFriends; i++) {
        page.friends[i] = friendsList[i];
    }
    page.currentPage = 0;
    
    // Calcular posiciones
    for(int i = 0; i < FRIENDS_PER_PAGE; i++) {
        page.friendRects[i] = (SDL_Rect){80, 210 + (i * 70), 600, 50};
    }
    
    // Botones
    page.backButton = (SDL_Rect){30, 10, 40, 40};
    page.nextButton = (SDL_Rect){700, 210 + (FRIENDS_PER_PAGE * 70), 150, 30};
}

void Friends_Render(SDL_Renderer* renderer) {
    // Fondo
    SDL_RenderCopy(renderer, page.background, NULL, NULL);
    
    SDL_Color white = {255, 255, 255, 255};
    int startIdx = page.currentPage * FRIENDS_PER_PAGE;
    int endIdx = startIdx + FRIENDS_PER_PAGE;
    
    // Renderizar amigos visibles
    for(int i = startIdx; i < endIdx && i < page.totalFriends; i++) {
        int pos = i % FRIENDS_PER_PAGE;
        
        // Ícono
        SDL_Rect iconRect = {80, 210 + (pos * 70), 50, 50};
        SDL_RenderCopy(renderer, page.friendIcon, NULL, &iconRect);
        
        // Nombre
        SDL_Surface* surf = TTF_RenderText_Solid(page.font, page.friends[i], white);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect textRect = {iconRect.x + 70, iconRect.y + 15, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &textRect);
        
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
        
        page.friendRects[pos] = (SDL_Rect){80, 210 + (pos * 70), 600, 50};
    }
    
    // Botón "Siguiente" (si hay más amigos)
    if(page.totalFriends > endIdx) {
        SDL_SetRenderDrawColor(renderer, 58, 58, 59, 255);
        SDL_RenderFillRect(renderer, &page.nextButton);
        
        SDL_Surface* nextSurf = TTF_RenderText_Solid(page.font, "Siguiente", white);
        SDL_Texture* nextTex = SDL_CreateTextureFromSurface(renderer, nextSurf);
        SDL_RenderCopy(renderer, nextTex, NULL, &page.nextButton);
        
        SDL_FreeSurface(nextSurf);
        SDL_DestroyTexture(nextTex);
    }
}

void Friends_HandleEvent(SDL_Event* event, bool* running) {
    if(event->type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Botón Atrás
        if(SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &page.backButton)) {
            *running = false;
            return;
        }
        
        // Botón Siguiente
        if(SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &page.nextButton)) {
            page.currentPage++;
            if((page.currentPage * FRIENDS_PER_PAGE) >= page.totalFriends) {
                page.currentPage = 0;
            }
            return;
        }
        
        // Click en amigos
        for(int i = 0; i < FRIENDS_PER_PAGE; i++) {
            int friendIdx = (page.currentPage * FRIENDS_PER_PAGE) + i;
            if(friendIdx < page.totalFriends && 
               SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &page.friendRects[i])) {
                printf("Click en amigo: %s\n", page.friends[friendIdx]);
                break;
            }
        }
    }
    else if(event->type == SDL_QUIT) {
        *running = false;
    }
}

void Friends_Destroy() {
    if(page.background) SDL_DestroyTexture(page.background);
    if(page.friendIcon) SDL_DestroyTexture(page.friendIcon);
    if(page.font) TTF_CloseFont(page.font);
}
