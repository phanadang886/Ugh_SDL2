#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "def.h"
#include "Graphics.h"
using namespace std;

//background trôi
struct ScrollingBackground {
    SDL_Texture* texture;
    int scrollingOffset = 0;
    int width, height;

    void setTexture(SDL_Texture* _texture) {
        texture = _texture;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }

    void scroll(int distance) {
        scrollingOffset -= distance;
        if( scrollingOffset < 0 ) { scrollingOffset = height; }
    }

};

//gộp background trôi
struct Background
{
    ScrollingBackground background;
    ScrollingBackground leftbck;
    ScrollingBackground rightbck;
    void MakeTexture(Graphics graphics)
    {
        background.setTexture(graphics.loadTexture("woodbackground.png"));
        leftbck.setTexture(graphics.loadTexture("deepwood.png"));
        rightbck.setTexture(graphics.loadTexture("deepwood.png"));
    }
    void renderTexture(SDL_Texture *texture, int x, int y,Graphics graphics)
    {
        SDL_Rect dest;

        dest.x = x;
        dest.y = y;
        SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

        SDL_RenderCopy(graphics.renderer, texture, NULL, &dest);
    }
    void renderA(Graphics graphics)
    {
        renderTexture(background.texture, 0, background.scrollingOffset,graphics);
        renderTexture(background.texture, 0, background.scrollingOffset - background.height,graphics);
    }
    void renderB(Graphics graphics)
    {
        renderTexture(leftbck.texture, 0, leftbck.scrollingOffset,graphics);
        renderTexture(leftbck.texture, 0, leftbck.scrollingOffset - leftbck.height,graphics);
        renderTexture(rightbck.texture, 600, rightbck.scrollingOffset,graphics);
        renderTexture(rightbck.texture, 600, rightbck.scrollingOffset - rightbck.height,graphics);
    }
    void intergrationBck(Graphics graphics,int speed)
    {
        background.scroll(speed/2);
        renderA(graphics);
        leftbck.scroll(speed);
        rightbck.scroll(speed);
        renderB(graphics);
    }
    void DestroyBck()
    {
            SDL_DestroyTexture( background.texture );
            SDL_DestroyTexture( leftbck.texture );
            SDL_DestroyTexture( rightbck.texture );
    }
};


#endif
