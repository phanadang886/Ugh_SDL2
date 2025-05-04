#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include "activities.h"
#include "def.h"
#include "menus.h"
#include "combine.h"
#include "Graphics.h"
#include "background.h"
int main(int argc, char* argv[]) {
    srand(time(0));
    Graphics graphics;
    graphics.init();

    //khởi tạo background
    Background backgrounds;
    backgrounds.MakeTexture(graphics);


    //khởi tạo nhân vật
    UIA uia;
    uia.DeclareUIAFrames(graphics);
    SDL_Texture *startUIA = graphics.loadTexture("frame_00_delay-1.69s.png");

    //khởi tạo chướng ngại vật
    Obstacle obstacle1;
    obstacle1.DeclareObstacle1(graphics);

    //khởi tạo đạn
    BulletManager bulletmanager;
    bulletmanager.DeclareBullets(graphics);

    //khởi tạo âm thanh
    Sound sounds;
    sounds.DeclareSound(graphics);
    //phát nhạc nền
    sounds.playBackgroundMusic(graphics);

    //khởi tạo font
    Font fonts;
    fonts.DeclareFont(graphics);

    //khởi tạo menu
    Menu menu(graphics, &fonts, &sounds);
    SDL_Texture *ughMenu = graphics.loadTexture("UghMenu.png");
    GameoverMenu gameoverMenu(graphics,&fonts);
    SDL_Texture *MenuOver = graphics.loadTexture("wooden_board_500x400.png");

    //jump+1
    FallingRectManager fallingrects;
    SDL_Texture* bubble = graphics.loadTexture("bubbleJump.png");

    bool gameOver = false;  //trạng thái thua
    bool quit = false;//thoát game
    bool gamestart = false;//game chưa bắt đầu
    int speed_count = 0;//đếm số vòng lặp để tăng speed
    int speed = 4;//tốc độ di chuyển chung của mọi thứ
    Uint32 start_time; //lưu thời gian bắt đầu
    int jump_count = 5;

    HighestScore scoreManager;
    int highScore = scoreManager.loadHighScore("highscore.txt");
    int scoreSave;//lưu thay biến score vì score đc khai báo trong if
    SDL_Event e;
    while (!quit) {

        while (SDL_PollEvent(&e) != 0) {

        //màn hình menu
        if(menu.startGame == false)
        {
        menu.handleEvents(e,quit,graphics);
            continue;
        }

        //màn hình game over
        if(gameOver){
        gameoverMenu.handleEvent(e,gameOver,gamestart,menu,graphics,uia,obstacle1,
         backgrounds,speed,speed_count, start_time,startUIA,bulletmanager,sounds,jump_count,fallingrects);
        }

        fallingrects.handleClick(e,jump_count);

        if (e.type == SDL_QUIT) {
            quit = true;
        }

        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_SPACE:
                if (!uia.isJumping && jump_count> 0) {  //chỉ nhảy khi đang chạm đất
                    sounds.playJumpSound(graphics);
                    uia.playerVelY = JUMP_FORCE;//động lực chính của jump
                    uia.isJumping = true;
                    jump_count--;
                }
                break;
            case SDLK_r:  //nhấn R để reset khi thua/bắt đầu game
                if(gamestart == false){
                    speed_count = 0;
                    speed = 4;
                    jump_count = 5;
                    gamestart = true;
                    sounds.playNormalSound(graphics);
                    start_time = SDL_GetTicks(); //lưu thời gian bắt đầu
                }
                if (gameOver) {
                    uia.playerX = SCREEN_WIDTH / 2.0f- PLAYER_HEIGHT/2;
                    uia.playerY = SCREEN_HEIGHT_CENTER - PLAYER_HEIGHT/2;
                    uia.playerVelY = 0.0f;
                    obstacle1.resetObstacles();
                    bulletmanager.resetBullets();
                    fallingrects.reset();
                    backgrounds.intergrationBck(graphics,speed);
                    SDL_Rect Uia = {(int)uia.playerX, (int)uia.playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
                    SDL_RenderCopy(graphics.renderer,startUIA, NULL, &Uia);
                    SDL_RenderPresent(graphics.renderer);
                    SDL_Delay(1000);
                    gameOver = false;
                    sounds.playNormalSound(graphics);
                    start_time = SDL_GetTicks(); //lưu thời gian bắt đầu
                    jump_count = 5;
                }
                break;
            }
        }
    }

        if(menu.startGame == false)
        {
            menu.render(graphics,ughMenu);
            SDL_Delay(1000 / 60);
            continue;
        }
        if(gamestart == false){
            speed = 0;
            uia.playerX = SCREEN_WIDTH / 2.0f- PLAYER_HEIGHT/2;
            uia.playerY = SCREEN_HEIGHT_CENTER - PLAYER_HEIGHT/2;
            uia.playerVelY = 0.0f;
            backgrounds.intergrationBck(graphics,speed);
            SDL_Rect Uia = {(int)uia.playerX, (int)uia.playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
            SDL_RenderCopy(graphics.renderer,startUIA, NULL, &Uia);
            Mix_HaltChannel(-1); // Dừng tất cả các kênh âm thanh

        }

        if (!gameOver && gamestart == true) {

            Uint32 current_time = SDL_GetTicks();
            int score = (current_time - start_time) / 100;
            scoreSave = score;
            speed_count++;
            if(speed_count>=480){
            speed+=2;
            speed_count =0;
            }


            //nạp background vào back buffer(render)
            backgrounds.intergrationBck(graphics,speed);


            //vẽ nhân vật
            uia.UIAShow(graphics);
            //di chuyển trái/phải,nhảy
            uia.Movement(speed);
            uia.Jump(speed);
            //chạm biên
            uia.TouchRedZone(gameOver,graphics,backgrounds,startUIA,speed);


            if(current_time - start_time >= 3000){
            //chướng ngại vật
            obstacle1.spawnObstacle(100-speed/2);
            obstacle1.updateObstacles(speed);
            obstacle1.checkCollisions(gameOver,uia.playerX,uia.playerY);
            obstacle1.removeOffscreenObstacles();
            obstacle1.renderObstacle1(graphics.renderer);

            //bullet
            bulletmanager.spawnBullet(300,speed);
            bulletmanager.updateBullets();
            bulletmanager.renderBullets(graphics);
            bulletmanager.checkCollisions(gameOver,uia.playerX,uia.playerY);
            bulletmanager.removeOffscreenBullets();

            //xử lí jump
            fallingrects.update();
            fallingrects.render(graphics, bubble);
            }
            //render fonts
            fonts.renderFontText(graphics,score,jump_count);
        }


        //thua, hiển thị thông báo
        if (gameOver) {
            Mix_HaltChannel(0);
            bulletmanager.resetBullets();
            fallingrects.reset();
             scoreManager.saveHighScore("highscore.txt",scoreSave, highScore);
            gameoverMenu.render(graphics,scoreSave,highScore,MenuOver);
            speed_count =0;
            speed = 4;
            jump_count = 5;
        }
        SDL_RenderPresent(graphics.renderer);
        if(speed >14){speed = 14;}
        SDL_Delay(1000 / 60);//FPS
    }
    fonts.DestroyFont();
    sounds.DestroySound();
    backgrounds.DestroyBck();
    uia.UIADestroy();
    graphics.quit();
    return 0;
}
