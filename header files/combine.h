#ifndef COMBINE_H_INCLUDED
#define COMBINE_H_INCLUDED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include "activities.h"
#include "def.h"
#include "menus.h"
#include "Graphics.h"
#include "background.h"
using namespace std;
struct HighestScore{
    int loadHighScore(const string& filename) {
        ifstream file(filename);
        int score = 0;
        if (file.is_open()) {
            file >> score;
            file.close();
        }
        return score;
    }

    void saveHighScore(const string& filename, int currentScore, int &highScore) {
    if (currentScore > highScore) {
        highScore = currentScore;
        ofstream output("highscore.txt");
        if (output.is_open()) {
            output << highScore;
            output.close();
        }
    }
    }
};

#endif
