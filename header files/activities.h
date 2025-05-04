#ifndef ACTIVITIES_H_INCLUDED
#define ACTIVITIES_H_INCLUDED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "def.h"
#include "Graphics.h"
#include "background.h"
using namespace std;

struct UIA
{
        //trạng thái nhân vật
    float playerX = SCREEN_WIDTH / 2.0f- PLAYER_HEIGHT/2;  //vị trí X ban đầu (giữa màn hình)
    float playerY = SCREEN_HEIGHT_CENTER - PLAYER_HEIGHT/2;  //vị trí Y ban đầu (chạm đất)
    float playerVelY = 0.0f;  //vận tốc theo trục Y
    bool isJumping = false;  //trạng thái nhảy

    SDL_Texture* frames[14]; // 14 frame của Uia Cat
    int num_frames = 14;
    int frame_index = 0;
    int frames_per_second = 60; // FPS của game
    int animation_speed = 20;  // Tốc độ hoạt ảnh: 20 FPS (1 frame hoạt ảnh mỗi 3 vòng lặp game)
    int frame_counter = 0;     // Đếm số vòng lặp game để chuyển frame

    void Movement(int speed)
    {
                //di chuyển trái/phải
        const Uint8* keyState = SDL_GetKeyboardState(NULL);
        if (keyState[SDL_SCANCODE_LEFT]||keyState[SDL_SCANCODE_A]) {
            playerX -= MOVE_SPEED+speed;
        }
        else if (keyState[SDL_SCANCODE_RIGHT]||keyState[SDL_SCANCODE_D]) {
            playerX += MOVE_SPEED;
        }

    }
    void Jump(int speed)
    {

        //trọng lực
        playerVelY += GRAVITY;//tốc độ nhảy lên giảm dần
        playerY += playerVelY;//cập nhật vào vị trí theo trục y thực tế

        //kiểm tra chạm đất
        if (playerY + PLAYER_HEIGHT > SCREEN_HEIGHT_CENTER+PLAYER_HEIGHT/2) {
            playerY = SCREEN_HEIGHT_CENTER-PLAYER_HEIGHT/2;
            playerVelY = 0.0f;
            isJumping = false;
        }
    }
    //chạm biên
    bool CheckRedZone(float playerX, float playerY) {

        if (playerX <= RED_ZONE_WIDTH || playerX + PLAYER_WIDTH >= SCREEN_WIDTH - RED_ZONE_WIDTH) {
            return true;
        }
        return false;
    }

    //load frame trực tiếp thành texture
    void DeclareUIAFrames(Graphics graphics)
    {
        for (int i = 0; i < num_frames; i++) {
            char path[32];
            sprintf(path, "frame_%dUIA.png", i);
            frames[i] = graphics.loadTexture(path);
        }
    }

    void TouchRedZone(bool &gameOver, Graphics graphics,Background backgrounds,SDL_Texture *startUIA,int speed)
    {
        if (CheckRedZone(playerX, playerY)) {
        gameOver = true;
            backgrounds.intergrationBck(graphics,speed);
            SDL_Rect Uia = {(int)playerX, (int)playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
            SDL_Rect srcrect = {20,4,166,190};
            SDL_RenderCopy(graphics.renderer,startUIA, &srcrect, &Uia);
        }
    }


    void UIAShow(Graphics graphics)
    {
        SDL_Rect dest = { (int)playerX, (int)playerY, PLAYER_WIDTH+10, PLAYER_HEIGHT+10};
        SDL_Rect srcrect = {2,5,185,190};
        frame_counter++;
        //cứ frame_counter đếm đc 3 lần thì frame_index sẽ chuyển ảnh sang ảnh tiếp theo
        if (frame_counter >= frames_per_second / animation_speed) {
            frame_index = (frame_index + 1) % num_frames;
            frame_counter = 0;
        }
        SDL_RenderCopy(graphics.renderer, frames[frame_index], &srcrect, &dest);
    }

    //giải phóng
    void UIADestroy()
    {
        for (int i = 0; i < num_frames; i++) {
        SDL_DestroyTexture(frames[i]);
        }
    }
};

//khởi tạo chướng ngại vật
struct Obstacle {
    SDL_Rect rect;
    vector<SDL_Rect> obstacles;
    int frame_counter1 = 0;
    SDL_Texture* obstacle1;

    void DeclareObstacle1(Graphics graphics)
    {
        obstacle1 = graphics.loadTexture("Obstacle1.png");
    }

    void spawnObstacle(int deltaTime) {
        frame_counter1++;
    if (frame_counter1 >= deltaTime) {
        int x =200 + rand() % (500-200+1); // 100 là chiều dài viên gạch
        SDL_Rect push = { x, 600, 100, 32 };
        obstacles.push_back(push); //tạo viên gạch
        frame_counter1 = 0;
        }
    }
    //gọi mỗi khung hình
    void updateObstacles(int speed) {
    for (SDL_Rect& obs : obstacles) {
        obs.y -= speed;
    }
}
    //phát hiện va chạm
    void checkCollisions(bool &gameover,float playerX,float playerY)
                         {
        SDL_Rect playerRect = {(int)playerX+5,(int)playerY+10,PLAYER_WIDTH,75};
        for (SDL_Rect& obs : obstacles) {
            if (SDL_HasIntersection(&playerRect, &obs)) {//hàm kiểm tra va chạm
                gameover = true;
                break;
            }
        }


    }
    //xóa obstacles
    void removeOffscreenObstacles() {

        for (int i = obstacles.size() - 1; i >= 0; i--) {
            //nếu chướng ngại vật ra khỏi màn hình (y < -25)
            if (obstacles[i].y < -25) {
                //xóa phần tử tại vị trí i
                obstacles.erase(obstacles.begin() + i);
            }
        }
    }

    //nạp vào back buffer
    void renderObstacle1(SDL_Renderer* renderer) {

    for (SDL_Rect& obs : obstacles) {
        SDL_RenderCopy(renderer, obstacle1, NULL, &obs);
                //vẽ khung viền màu xanh để kiểm tra
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &obs);
        }
    }
    void resetObstacles() {
        obstacles.clear();
    }
};


struct BulletManager {
    std::vector<SDL_Rect> bullets; // Danh sách các đạn
    int frame_counter1 = 0;
    int BULLET_WIDTH = 75;
    int BULLET_HEIGHT = 56;
    float BULLET_SPEED = 5.0f;
    std::vector<float> bulletSpeeds; // Tốc độ theo từng viên đạn (hướng đi)
    SDL_Texture* bulletTextures[8];
    int animFrame = 0;
    int frame_counter = 0;
    void DeclareBullets(Graphics graphics) {
    for (int i = 0; i < 8; ++i) {
        char filename[32];
        sprintf(filename, "bullet_%d.png", i); // Tên ảnh: bullet_0.png, bullet_1.png, ...
        bulletTextures[i] = graphics.loadTexture(filename);
    }
}

    void spawnBullet(int deltaTime, int speed) {
        frame_counter1++;
        if (frame_counter1 >= deltaTime) {
            frame_counter1 = 0;

            bool fromLeft = (rand() % 2 == 0);
            float xPos = fromLeft ? -BULLET_WIDTH : SCREEN_WIDTH;
            float yPos = SCREEN_HEIGHT / 2 + 100 + rand() % (SCREEN_HEIGHT / 2 - 100);

            SDL_Rect newBullet = {
                (int)xPos,
                (int)yPos,
                BULLET_WIDTH,
                BULLET_HEIGHT
            };
            bullets.push_back(newBullet);

            float directionSpeed = fromLeft ? speed : -speed;
            bulletSpeeds.push_back(directionSpeed);
        }

    }

    void updateBullets() {
        for (size_t i = 0; i < bullets.size(); ++i) {
            bullets[i].x += bulletSpeeds[i];
            bullets[i].y -= std::abs(bulletSpeeds[i]) / 2;
        }
    }

    void checkCollisions(bool &gameover, float playerX, float playerY) {
        SDL_Rect playerRect = {(int)playerX+15, (int)playerY+10, PLAYER_WIDTH-30, 70};
        for (SDL_Rect& bullet : bullets) {
            if (SDL_HasIntersection(&playerRect, &bullet)) {
                gameover = true;
                break;
            }
        }
    }

    void removeOffscreenBullets() {
        for (int i = bullets.size() - 1; i >= 0; --i) {
            SDL_Rect& b = bullets[i];
            if (b.x + BULLET_WIDTH < 0 || b.x > SCREEN_WIDTH ||
                b.y + BULLET_HEIGHT < 0 || b.y > SCREEN_HEIGHT) {
                bullets.erase(bullets.begin() + i);
                bulletSpeeds.erase(bulletSpeeds.begin() + i);
            }
        }
    }

void renderBullets(Graphics graphics) {
    frame_counter++;
    if (frame_counter >= 3) {
    animFrame = (animFrame + 1) % 8;
    for (const SDL_Rect& bullet : bullets) {
        SDL_RenderCopy(graphics.renderer, bulletTextures[animFrame], NULL, &bullet);
    }
}

}

    void resetBullets() {
        bullets.clear();
        bulletSpeeds.clear();
    }
};

struct Sound {
    Mix_Music* backgroundMusic; //nhạc nền
    Mix_Chunk* normalSound;     //âm thanh bình thường
    Mix_Chunk* jumpSound;       //âm thanh nhảy
    int normalChannel = 0;
    int jumpChannel =1;
    int bgVolume = MIX_MAX_VOLUME;     // Âm lượng nhạc nền
    int effectVolume = MIX_MAX_VOLUME; // Âm lượng âm thanh hiệu ứng

    //khởi tạo
    void DeclareSound(Graphics graphics){
    backgroundMusic = graphics.loadMusic("merx-market-song-33936.mp3");
    normalSound = graphics.loadSound("Voicy_uia .wav");
    jumpSound = graphics.loadSound("woosh-230554.wav");
    }

        //phát nhạc nền (vòng lặp vô hạn)
    void playBackgroundMusic(Graphics graphics) {
        Mix_VolumeMusic(bgVolume); // Áp dụng âm lượng trước khi phát
        graphics.playBg(backgroundMusic);
    }

        //phát âm thanh bình thường (lặp lại)
    void playNormalSound(Graphics graphics) {
        Mix_VolumeChunk(normalSound,effectVolume); // Áp dụng âm lượng
        graphics.playChunk(normalSound,normalChannel,-1);
    }

    //dừng âm thanh bình thường và phát âm thanh nhảy
    void playJumpSound(Graphics graphics) {
        Mix_HaltChannel(normalChannel);
        Mix_VolumeChunk(jumpSound,effectVolume); // Áp dụng âm lượng
        graphics.playChunk(jumpSound,jumpChannel,0);
        playNormalSound(graphics);
    }

// Đặt âm lượng nhạc nền
    void setBackgroundVolume(int volume) {
        bgVolume = max(0, min(volume, MIX_MAX_VOLUME)); // Giới hạn 0-128
        Mix_VolumeMusic(bgVolume);
    }

    // Đặt âm lượng âm thanh hiệu ứng
    void setEffectVolume(int volume) {
        effectVolume = max(0, min(volume, MIX_MAX_VOLUME)); // Giới hạn 0-128
        Mix_VolumeChunk(normalSound, effectVolume);
        Mix_VolumeChunk(jumpSound, effectVolume);
    }

    //hủy sound
    void DestroySound(){
          if (normalSound != nullptr) {
            Mix_FreeChunk(normalSound);
        }
        if (jumpSound != nullptr) {
            Mix_FreeChunk(jumpSound);
        }
        if (backgroundMusic != nullptr) {
            Mix_FreeMusic(backgroundMusic);
        }
    }
};

struct Font {
    TTF_Font* font1;
    TTF_Font* font2;
    SDL_Color color = {255, 255, 255, 255};
    SDL_Texture* ScoreText;
    SDL_Texture* JumpText;
    void DeclareFont(Graphics graphics)
    {
        font1 = graphics.loadFont("SVN-Retron 2000.otf", 24);
        font2 = graphics.loadFont("SVN-Retron 2000.otf", 24);
    }

    void renderFontText(Graphics graphics,int score,int jump_count)
    {
        string score_text = "Score: " + to_string(score);
        ScoreText = graphics.renderText(score_text.c_str(), font1, color);
        graphics.renderTexture(ScoreText, 15, 7);

        string jumptime = "JUMPS X" +to_string(jump_count);
        JumpText = graphics.renderText(jumptime.c_str(), font1, color);
        graphics.renderTexture(JumpText, 15, 37);
    }
    void renderText (const char* text,int x, int y,Graphics graphics)
    {
        SDL_Texture* menuTexts = graphics.renderText(text, font1, color);
        graphics.renderTexture(menuTexts,x,y);
    }
    void DestroyFont()
    {
        SDL_DestroyTexture(ScoreText);
        TTF_CloseFont(font1);
        TTF_CloseFont(font2);
    }
};

struct FallingRect {
    int x, y;           //vị trí hình chữ nhật (góc trên trái)
    int width, height;  //kích thước hình chữ nhật
    int speed;          //tốc độ rơi (số pixel mỗi frame)
    bool isAlive;       //trạng thái: còn tồn tại không

    //constructor khởi tạo một hình chữ nhật
    FallingRect(int _x, int _y, int _w, int _h, int _speed)
        : x(_x), y(_y), width(_w), height(_h), speed(_speed), isAlive(true) {}

    //cập nhật vị trí mỗi frame
    void update() {
        if(isAlive == true){
            y += speed; //dịch chuyển xuống dưới
            if (y > SCREEN_HEIGHT) {
                isAlive = false; //nếu vượt quá màn hình thì "biến mất"
            }
        }
    }

    //vẽ hình chữ nhật lên màn hình
    void render(Graphics graphics,SDL_Texture* bubble) {
        if (!isAlive) return;

        else {
        SDL_Rect rect = {x, y, width, height};
        SDL_RenderCopy(graphics.renderer, bubble, NULL, &rect);
        }
    }

    //kiểm tra xem người chơi có click trúng không
    bool isClicked(int mouseX, int mouseY, bool mouseDown, int &jumping_time) {
        if( mouseDown && isAlive &&
               mouseX >= x && mouseX <= x + width &&
               mouseY >= y && mouseY <= y + height)
               {
                   jumping_time+=1;
                   isAlive = false;
                   return true;
               }
        return false;
    }
};

struct FallingRectManager {
    vector<FallingRect> fallingRects; // Danh sách các hình chữ nhật rơi
    Uint32 lastSpawnTime = 0;              // Lưu thời gian spawn cuối cùng
    Uint32 spawnInterval = 2000;           // Thời gian giữa các lần spawn (ms)

    //cập nhật tất cả các hình chữ nhật
    void update() {
        //tạo mới nếu đủ thời gian
        Uint32 now = SDL_GetTicks();
        if (now - lastSpawnTime >= spawnInterval) {
            int x = rand() % (SCREEN_WIDTH - 50);
            fallingRects.emplace_back(x, 0, 40, 40, 2); // Spawn mới
            lastSpawnTime = now;
        }

        //cập nhật và loại bỏ hình chữ nhật đã hết
        for (FallingRect& rect : fallingRects)
            rect.update();

        // Xóa từng hình chữ nhật không còn tồn tại
        for (size_t i = 0; i < fallingRects.size(); ) {
            if (!fallingRects[i].isAlive) {
                // Xóa phần tử tại vị trí i
                fallingRects.erase(fallingRects.begin() + i);
            }
            else {
                ++i; //chỉ tăng i nếu không xóa
            }
        }
    }
    //vẽ tất cả các hình chữ nhật
    void render(Graphics graphics,SDL_Texture* bubble) {
        for (auto& rect : fallingRects)
            rect.render(graphics,bubble);
    }
    //kiểm tra va chạm chuột và cộng jumpCount nếu trúng
    void handleClick(SDL_Event& e,int& jumpCount) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool mouseDown = (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT);//e.button.button là mã nút chuột
        for (auto& rect : fallingRects) {
            if (rect.isClicked(mouseX, mouseY, mouseDown,jumpCount)) {
                rect.isAlive = false;
            }
        }
    }

    //reset danh sách
    void reset() {
        fallingRects.clear();
        lastSpawnTime = SDL_GetTicks();
    }
};

#endif
