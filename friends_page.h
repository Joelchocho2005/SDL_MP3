#ifndef FRIENDS_PAGE_H
#define FRIENDS_PAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_FRIENDS 100
#define FRIENDS_PER_PAGE 6

typedef struct {
    SDL_Texture* background;
    SDL_Texture* friendIcon;
    TTF_Font* font;
    
    const char* friends[MAX_FRIENDS];
    int totalFriends;
    int currentPage;
    
    SDL_Rect friendRects[MAX_FRIENDS]; // Áreas clickeables
    SDL_Rect backButton;
    SDL_Rect nextButton;
} FriendsPage;

void Friends_Init(SDL_Renderer* renderer, const char** friendsList, int count);
void Friends_Render(SDL_Renderer* renderer);
void Friends_HandleEvent(SDL_Event* event, bool* running);
void Friends_Destroy();

#endif
