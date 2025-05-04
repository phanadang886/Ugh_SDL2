#ifndef MENUS_H_INCLUDED
#define MENUS_H_INCLUDED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "activities.h"
#include "def.h"
#include "combine.h"
#include "Graphics.h"
using namespace std;
struct Button {
    vector<SDL_Texture*> textures; // Danh sách 5 texture cho các trạng thái
    int x, y;                           // Vị trí của nút
    int width, height;                  // Kích thước của nút
    enum State { NORMAL = 0, HOVER = 1, CLICK_START = 2, CLICK_MID = 3, CLICK_END = 4 };
    State currentState;                 // Trạng thái hiện tại
    bool isClicked;                     // Đang được click
    Uint32 clickTime;                   // Thời gian bắt đầu click (để điều khiển hoạt ảnh)
    string baseName;
    Button(string base, int x_, int y_, Graphics graphics)
        : x(x_), y(y_), currentState(NORMAL), isClicked(false), clickTime(0),baseName(base){
        //tải 5 texture cho nút (ví dụ: play01.png, play02.png, ..., play05.png)
        for (int i = 1; i <= 5; i++) {
            string fileName = baseName + to_string(i) + ".png";
            SDL_Texture* texture = graphics.loadTexture(fileName.c_str());
            textures.push_back(texture);
        }

        //lấy kích thước từ texture đầu tiên (trạng thái NORMAL)
        if (textures[0]) {
            SDL_QueryTexture(textures[0], nullptr, nullptr, &width, &height);
        } else {
            width = 0;
            height = 0;
        }
    }

    //kiểm tra xem chuột có hover lên nút không
    bool isMouseHovered(int mouseX, int mouseY) {
        return mouseX >= x && mouseX <= x + width &&
               mouseY >= y && mouseY <= y + height;
    }

    //kiểm tra xem nút có được click không
    bool isClickedEvent(int mouseX, int mouseY,bool mousedown) {
        if(mousedown) return isMouseHovered(mouseX, mouseY);
        else return false;
    }

    void update(int mouseX, int mouseY, bool mouseDown) {
        bool hovered = isMouseHovered(mouseX, mouseY);

        //bắt đầu hoạt ảnh click khi nhấn chuột
        if (mouseDown && hovered) {
            if (!isClicked) {
                isClicked = true;
                clickTime = SDL_GetTicks();
                currentState = CLICK_START; // Bắt đầu hoạt ảnh click
            }
        }

        //điều khiển hoạt ảnh click
        if (isClicked) {
            Uint32 elapsed = SDL_GetTicks() - clickTime;
            if (elapsed < 100) {
                currentState = CLICK_START; // back03.png
            } else if (elapsed < 200) {
                currentState = CLICK_MID;   // back04.png
            } else {
                currentState = CLICK_END;   // back05.png
                //khi hoạt ảnh hoàn tất (sau 200ms), đặt isClicked về false
                if (elapsed >= 200) {
                    isClicked = false;
                }
            }
        }
         else {
            currentState = hovered ? HOVER : NORMAL; // back01.png hoặc back02.png
        }
    }

    //vẽ nút
    void render(Graphics graphics) {
        graphics.renderTexture(textures[currentState],x,y);
    }
};

struct VolumeSlider {
    SDL_Rect track;        // Thanh nền của slider (hình chữ nhật)
    SDL_Rect handle;       // Nút trượt (hình chữ nhật nhỏ hơn)
    int minValue = 0;      // Giá trị tối thiểu (âm lượng 0)
    int maxValue = MIX_MAX_VOLUME; // Giá trị tối đa (âm lượng 128)
    int currentValue = MIX_MAX_VOLUME; // Giá trị hiện tại (âm lượng hiện tại)
    bool isDragging = false; // Trạng thái kéo thả
    int x, y, width, height; // Vị trí và kích thước của thanh slider
    bool isBackground;       // Xác định đây là thanh cho âm thanh nền hay chunk
    VolumeSlider(int x_, int y_, int w_, int h_,bool isBackground_) : x
    (x_), y(y_), width(w_), height(h_),isBackground(isBackground_) {
        // Khởi tạo thanh nền
        track = {x, y, width, height};
        // Khởi tạo nút trượt
        handle = {x, y, 10, height}; // Nút trượt có chiều rộng 10, cao bằng thanh
        // Đặt vị trí ban đầu của nút trượt dựa trên currentValue
        updateHandlePosition();
    }

    // Cập nhật vị trí nút trượt dựa trên giá trị âm lượng
    void updateHandlePosition() {
        float ratio = static_cast<float>(currentValue - minValue) / (maxValue - minValue);
        handle.x = x + static_cast<int>(ratio * (width - handle.w));
    }


    // Chuyển đổi từ vị trí nút trượt sang giá trị âm lượng
    void updateValueFromHandle() {
        float ratio1 = static_cast<float>(handle.x - x) / (width - handle.w);
        currentValue = minValue + static_cast<int>(ratio1 * (maxValue - minValue));
    }
    // Cập nhật trạng thái slider khi người dùng tương tác
    void update(int mouseX, int mouseY, Sound* sound) {
        Uint32 mouseState = SDL_GetMouseState(NULL, NULL); // Lấy trạng thái chuột hiện tại
        bool mouseDown = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) == 1; // Kiểm tra chuột trái
        // Kiểm tra xem chuột có nằm trong khu vực nút trượt không
        bool mouseOverHandle = (mouseX >= handle.x && mouseX <= handle.x + handle.w &&
                                mouseY >= handle.y && mouseY <= handle.y + handle.h);

        // Nếu nhấn chuột và chuột ở trên nút trượt → Bắt đầu kéo
        if (mouseDown &&mouseOverHandle ) {
            isDragging = true;
        }

        // Nếu thả chuột → Dừng kéo
        if (!mouseDown) {
            isDragging = false;
        }

        // Nếu đang kéo, cập nhật vị trí nút trượt và âm lượng
        if (isDragging) {
            // Giới hạn vị trí nút trượt trong thanh nền
            handle.x = max(x, min(mouseX - handle.w / 2, x + width - handle.w));
//            if(mouseX < x) handle.x = x;
//            else if(mouseX > x + width - handle.w ) handle.x=x + width - handle.w;
//            else handle.x = mouseX- handle.w / 2;
            // Tính giá trị âm lượng dựa trên vị trí nút trượt
            updateValueFromHandle();
            // Cập nhật âm lượng trong Sound
        if (isBackground) {
                sound->setBackgroundVolume(currentValue); // Điều chỉnh âm lượng nền
            } else {
                sound->setEffectVolume(currentValue);     // Điều chỉnh âm lượng chunk
            }
        }
    }

    // Vẽ thanh slider
    void render(SDL_Renderer* renderer) {
        // Vẽ thanh nền (màu xám)
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderFillRect(renderer, &track);
        // Vẽ nút trượt (màu trắng)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &handle);
    }
};

struct Menu {
    Font* font;
    Sound* sound;
    std::vector<Button> buttons;
    bool showInstructions = false;
    bool showSoundSettings = false; // Thêm biến để kiểm soát màn hình điều chỉnh âm thanh
    bool startGame = false;
    Button backButton;
    VolumeSlider volumeBackground; // Thanh kéo cho âm thanh nền
    VolumeSlider volumeChunk;      // Thanh kéo cho âm thanh chunk

    Menu(Graphics graphics, Font* font_, Sound* sound_)
        : font(font_), sound(sound_),
          backButton("back", 300, 350, graphics),
          volumeBackground(410, 194, 100, 20,true), // Thanh kéo âm thanh nền
          volumeChunk(410, 265, 100, 20,false)       // Thanh kéo âm thanh chunk
    {
        buttons.emplace_back("play", 300, 100,graphics);
        buttons.emplace_back("information", 300, 200,graphics);
        buttons.emplace_back("sound", 300, 300,graphics); // Thay "selectlevel" bằng "sound"
    }

    void handleEvents(SDL_Event& e, bool& quit, Graphics graphics) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool mouseDown = (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT);//e.button.button là mã nút chuột

     // QUIT
    if (e.type == SDL_QUIT) {
        quit = true;
        return;
    }

    //MÀN HÌNH CHÍNH (MENU)
    if (!showInstructions && !showSoundSettings && !startGame) {
        // update + render các nút menu
        for (auto& btn : buttons) {
            btn.update(mouseX, mouseY, mouseDown);
        }
        // chỉ bắt click nếu mouseDown = true
        if (mouseDown) {
            if(buttons[0].isClickedEvent(mouseX, mouseY, mouseDown))
            {
                SDL_Delay(200);
                startGame = true;
            }
            else if (buttons[1].isClickedEvent(mouseX, mouseY, mouseDown)) showInstructions = true;
            else if (buttons[2].isClickedEvent(mouseX, mouseY, mouseDown)) showSoundSettings = true;
        }
    }
    //MÀN HÌNH HƯỚNG DẪN
    else if (showInstructions) {
        backButton.update(mouseX, mouseY, mouseDown);
        if (mouseDown && backButton.isClickedEvent(mouseX, mouseY, mouseDown)) {
            showInstructions = false;
        }
    }
    //MÀN HÌNH ÂM THANH
    else if (showSoundSettings) {
        // update và render thanh volume
        volumeBackground.update(mouseX, mouseY, sound);
        volumeChunk.update(mouseX, mouseY, sound);

        backButton.update(mouseX, mouseY, mouseDown);
        backButton.render(graphics);
        if (mouseDown && backButton.isClickedEvent(mouseX, mouseY, mouseDown)) {
            showSoundSettings = false;
        }
    }
    //MÀN HÌNH CHƠI GAME
    else if (startGame) {

        backButton.update(mouseX, mouseY, mouseDown);
        if (backButton.isClickedEvent(mouseX, mouseY, mouseDown)) {
            startGame = false;
        }
    }
}

    void render(Graphics graphics,SDL_Texture *ughMenu) {
        //SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
        //SDL_RenderClear(graphics.renderer);
        graphics.renderTexture(ughMenu, 0, 0);

        if (showInstructions) {
            font->renderText("Instructions", 320 - 50, 80,graphics);
            font->renderText("Use arrow keys to move", 320 - 100, 160,graphics);
            font->renderText("Press space to jump", 320 - 80, 220,graphics);
            font->renderText("Click on the bubble to get +1 jump", 180, 280,graphics);
            backButton.render(graphics);
        }
        else if (showSoundSettings) {
            font->renderText("Sound Settings", 270, 50,graphics);
            font->renderText("Background Volume:", 130, 180,graphics);
            volumeBackground.render(graphics.renderer);
            font->renderText("Chunk Volume:", 130, 250,graphics);
            volumeChunk.render(graphics.renderer);
            backButton.render(graphics);
        }
        else {
            font->renderText("Game Menu", 300, 50,graphics);
            for (auto& button : buttons) {
                button.render(graphics);
            }
        }

        SDL_RenderPresent(graphics.renderer);
    }
};

struct GameoverMenu
{
    Font *font;
    Button backToMenuButton;   // Giả định class Button đã được định nghĩa
    Button replayButton;

    GameoverMenu(Graphics graphics,Font *font_)
        : font(font_),
        backToMenuButton("home", 328, 177, graphics),
        replayButton("replay", 328, 264, graphics) {}

void handleEvent(SDL_Event& e, bool& gameover, bool& gamestart, Menu& menu, Graphics graphics, UIA& uia, Obstacle& obstacle1,
Background& backgrounds, int& speed, int& speed_count, Uint32& start_time, SDL_Texture* startUIA,BulletManager bulletmanager,
Sound sounds, int jump_count,FallingRectManager fallingrects) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool mouseDown = (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT);
        backToMenuButton.update(mouseX, mouseY, mouseDown);
        replayButton.update(mouseX, mouseY, mouseDown);
        if (mouseDown) {
            if (backToMenuButton.isClickedEvent(mouseX, mouseY, mouseDown)) {
                // Reset trạng thái để quay lại menu và chuẩn bị chơi lại từ đầu
                menu.startGame = false;
                gameover = false;
                gamestart = false;

                // Reset nhân vật
                uia.playerX = SCREEN_WIDTH / 2.0f - PLAYER_HEIGHT / 2;
                uia.playerY = SCREEN_HEIGHT_CENTER - PLAYER_HEIGHT / 2;
                uia.playerVelY = 0.0f;
                uia.isJumping = false; // Đảm bảo nhân vật không ở trạng thái nhảy

                // Reset chướng ngại vật
                obstacle1.resetObstacles();
                bulletmanager.resetBullets();

                // Reset background
                backgrounds.intergrationBck(graphics, 0); // Đặt tốc độ về 0 để background dừng

                // Reset tốc độ và điểm số
                speed = 4;
                speed_count = 0;
                start_time = SDL_GetTicks(); // Reset thời gian để điểm số bắt đầu từ 0

                fallingrects.reset();
                jump_count = 5;
            }
            else if (replayButton.isClickedEvent(mouseX, mouseY, mouseDown)) {
                uia.playerX = SCREEN_WIDTH / 2.0f - PLAYER_HEIGHT / 2;
                uia.playerY = SCREEN_HEIGHT_CENTER - PLAYER_HEIGHT / 2;
                uia.playerVelY = 0.0f;
                uia.isJumping = false; // Đảm bảo nhân vật không ở trạng thái nhảy
                obstacle1.resetObstacles();
                bulletmanager.resetBullets();
                backgrounds.intergrationBck(graphics, speed);
                SDL_Rect Uia = {(int)uia.playerX, (int)uia.playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
                SDL_RenderCopy(graphics.renderer, startUIA, NULL, &Uia);
                SDL_RenderPresent(graphics.renderer);
                SDL_Delay(1000);
                sounds.playNormalSound(graphics);
                gameover = false;
                gamestart = true;
                speed = 4;
                speed_count = 0;
                start_time = SDL_GetTicks();
                fallingrects.reset();
                jump_count = 5;
            }
        }
    }
    void render(Graphics graphics,int score, int highestscore,SDL_Texture *MenuOver)
    {
        SDL_Rect gameOverRect = { 150, 150, 500, 400 };
        SDL_RenderCopy(graphics.renderer,MenuOver,NULL,&gameOverRect);
        font->renderText("Game Over!", 330, 50,graphics);
        backToMenuButton.render(graphics);
        replayButton.render(graphics);
        string Score = "Your Score: " + to_string(score);
        string Highestscore ="Your Record: "+ to_string(highestscore);
        font->renderText(Score.c_str(), 300, 340,graphics);
        font->renderText(Highestscore.c_str(), 295, 410,graphics);
        font->renderText("OR PRESS R TO REPLAY", 260,470,graphics);
    }
};
#endif
