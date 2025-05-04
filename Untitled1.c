#include <SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 900;      // Chiều rộng màn hình (3 lần chiều cao để giữ tỷ lệ 3:1)
const int SCREEN_HEIGHT = 500;     // Chiều cao màn hình
const int CHARACTER_SIZE = 50;     // Kích thước nhân vật (hình vuông)
const float GRAVITY = 0.5f;        // Gia tốc trọng lực
const float JUMP_FORCE = -10.0f;   // Lực nhảy lên
const float MOVE_SPEED = 5.0f;     // Tốc độ di chuyển trái/phải

int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Không thể khởi tạo SDL! Lỗi: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Tạo cửa sổ
    SDL_Window* window = SDL_CreateWindow("GameDauTien", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Không thể tạo cửa sổ! Lỗi: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Không thể tạo renderer! Lỗi: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Thiết lập vị trí và vận tốc của nhân vật
    float charX = SCREEN_WIDTH / 2.0f - CHARACTER_SIZE / 2.0f;  // Vị trí X ban đầu (giữa phần xanh)
    float charY = SCREEN_HEIGHT / 2.0f - CHARACTER_SIZE / 2.0f; // Vị trí Y ban đầu (giữa theo chiều dọc)
    float velocityY = 0.0f;  // Vận tốc theo trục Y
    float velocityX = 0.0f;  // Vận tốc theo trục X

    // Trạng thái của nhân vật
    bool isJumping = false;  // Kiểm tra đang nhảy hay không
    bool movingLeft = false; // Kiểm tra di chuyển trái
    bool movingRight = false;// Kiểm tra di chuyển phải

    // Offset của camera để giữ nhân vật ở giữa theo chiều dọc
    float cameraOffsetY = 0.0f;

    // Định nghĩa khu vực di chuyển (phần xanh giữa)
    const int GREEN_START_X = SCREEN_WIDTH / 3;    // Bắt đầu phần xanh (1/3 chiều rộng)
    const int GREEN_END_X = 2 * SCREEN_WIDTH / 3;  // Kết thúc phần xanh (2/3 chiều rộng)

    // Vòng lặp chính
    SDL_Event event;
    bool quit = false;
    while (!quit) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        movingLeft = true;
                        break;
                    case SDLK_RIGHT:
                        movingRight = true;
                        break;
                    case SDLK_SPACE:
                        if (!isJumping) {
                            velocityY = JUMP_FORCE; // Nhảy lên
                            isJumping = true;
                        }
                        break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        movingLeft = false;
                        break;
                    case SDLK_RIGHT:
                        movingRight = false;
                        break;
                }
            }
        }

        // Cập nhật vận tốc di chuyển trái/phải
        velocityX = 0.0f;
        if (movingLeft) {
            velocityX = -MOVE_SPEED;
        }
        if (movingRight) {
            velocityX = MOVE_SPEED;
        }

        // Giới hạn di chuyển trong phần xanh (GREEN_START_X đến GREEN_END_X)
        charX += velocityX;
        if (charX < GREEN_START_X) {
            charX = GREEN_START_X;
        } else if (charX > GREEN_END_X - CHARACTER_SIZE) {
            charX = GREEN_END_X - CHARACTER_SIZE;
        }

        // Áp dụng trọng lực
        velocityY += GRAVITY;

        // Cập nhật vị trí nhân vật
        charY += velocityY;

        // Giới hạn nhân vật không rơi xuống dưới đáy màn hình
        if (charY > SCREEN_HEIGHT - CHARACTER_SIZE) {
            charY = SCREEN_HEIGHT - CHARACTER_SIZE;
            velocityY = 0.0f;
            isJumping = false;
        }

        // Điều chỉnh camera để nhân vật luôn ở giữa theo chiều dọc
        cameraOffsetY = charY - (SCREEN_HEIGHT / 2.0f - CHARACTER_SIZE / 2.0f);

        // Xóa màn hình
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Màu nền đen (cho dễ nhìn)
        SDL_RenderClear(renderer);

        // Vẽ background (3 phần: đỏ - xanh - đỏ)
        // Phần đỏ bên trái
        SDL_Rect redLeft = { 0, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ
        SDL_RenderFillRect(renderer, &redLeft);

        // Phần xanh giữa
        SDL_Rect greenMiddle = { SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Màu xanh lá
        SDL_RenderFillRect(renderer, &greenMiddle);

        // Phần đỏ bên phải
        SDL_Rect redRight = { 2 * SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ
        SDL_RenderFillRect(renderer, &redRight);

        // Vẽ nhân vật (hình vuông) với vị trí đã điều chỉnh theo camera
        SDL_Rect charRect = { static_cast<int>(charX), static_cast<int>(charY - cameraOffsetY),
         CHARACTER_SIZE, CHARACTER_SIZE };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Màu trắng cho nhân vật
        SDL_RenderFillRect(renderer, &charRect);

        // Hiển thị lên màn hình
        SDL_RenderPresent(renderer);

        // Giới hạn FPS (~60 FPS)
        SDL_Delay(16);
    }

    // Giải phóng tài nguyên
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
