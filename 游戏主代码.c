#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// 定义屏幕宽高和方块大小
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BLOCK_SIZE 15

// 蛇的结构
typedef struct {
    int x, y; // 蛇的头部位置
} Point;

typedef struct {
    Point body[100]; // 蛇的身体，最多100个方块
    int length;      // 蛇的长度
    int dx, dy;      // 蛇的移动方向
} Snake;

// 食物的结构
typedef struct {
    Point position; // 食物位置
} Food;

// 障碍物的结构
#define MAX_OBSTACLES 20
Point obstacles[MAX_OBSTACLES];
int obstacleCount=20;


// 游戏状态和得分
int running = 1;
int score = 0;

// 全局变量
Snake snake;
Food food;
SDL_Renderer* renderer;
Mix_Music* backgroundMusic;
Mix_Chunk* eatSound;
SDL_Window* window;

// 判断食物生成位置是否与蛇的身体或障碍物重叠
int isValidFoodPosition(int x, int y) {
    // 判断是否与蛇的身体重叠
    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == x && snake.body[i].y == y) {
            return 0; // 重叠，返回无效
        }
    }

    // 判断是否与障碍物重叠
    for (int i = 0; i < obstacleCount; i++) {
        if (obstacles[i].x == x && obstacles[i].y == y) {
            return 0; // 重叠，返回无效
        }
    }

    return 1; // 没有重叠，返回有效
}

// 判断食物生成位置是否在蛇头周围的5个单位内
int isNearSnakeHead(int x, int y) {
    int headX = snake.body[0].x;
    int headY = snake.body[0].y;
    int radius = 5; // 半径5个单位

    // 判断生成点是否在蛇头周围的5个单位内
    return (abs(x - headX) <= radius && abs(y - headY) <= radius);
}

// 随机生成有效的食物位置，避开蛇的身体、障碍物和蛇头周围5个单位的区域
void generateFood() {
    int x, y;
    do {
        x = rand() % (SCREEN_WIDTH / BLOCK_SIZE);
        y = rand() % (SCREEN_HEIGHT / BLOCK_SIZE);
    } while (!isValidFoodPosition(x, y) || isNearSnakeHead(x, y)); // 只要不满足条件就重新生成
    food.position.x = x;
    food.position.y = y;
}

// 随机生成障碍物的位置，避开蛇的身体、食物和蛇头周围5个单位的区域
void generateObstacles() {
    for (int i = 0; i < obstacleCount; i++) {
        int x, y;
        do {
            x = rand() % (SCREEN_WIDTH / BLOCK_SIZE);
            y = rand() % (SCREEN_HEIGHT / BLOCK_SIZE);
        } while (!isValidFoodPosition(x, y) || isNearSnakeHead(x, y)); // 避开蛇的身体、食物和蛇头周围15个单位的区域
        obstacles[i].x = x;
        obstacles[i].y = y;
    }
}

// 初始化游戏
void initGame() {
    // 初始化蛇的身体
    snake.length = 2;
    snake.body[0].x = 10;  // 蛇头初始位置
    snake.body[0].y = 10;
    snake.body[1].x = 9;
    snake.body[1].y = 10;
    snake.body[2].x = 8;
    snake.body[2].y = 10;
    snake.dx = 1;  // 初始向右
    snake.dy = 0;

    // 随机生成食物
    srand((unsigned int)time(NULL));
    generateFood();

    // 随机生成障碍物
    generateObstacles();

    score = 0;  // 初始化得分
}

// 绘制矩形（用于绘制蛇、食物和障碍物）
void drawRectangle(int x, int y, int size, SDL_Color color) {
    SDL_Rect rect = { x, y, size, size };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

// 绘制蛇
void drawSnake() {
    SDL_Color snakeColor = { 0, 255, 0, 255 }; // 蛇的颜色（绿色）
    for (int i = 0; i < snake.length; i++) {
        drawRectangle(snake.body[i].x * BLOCK_SIZE, snake.body[i].y * BLOCK_SIZE, BLOCK_SIZE, snakeColor);
    }
}

// 绘制食物
void drawFood() {
    SDL_Color foodColor = { 255, 0, 0, 255 }; // 食物的颜色（红色）
    drawRectangle(food.position.x * BLOCK_SIZE, food.position.y * BLOCK_SIZE, BLOCK_SIZE, foodColor);
}

// 绘制障碍物
void drawObstacles() {
    SDL_Color obstacleColor = { 128, 128, 128, 255 }; // 障碍物的颜色（灰色）
    for (int i = 0; i < obstacleCount; i++) {
        drawRectangle(obstacles[i].x * BLOCK_SIZE, obstacles[i].y * BLOCK_SIZE, BLOCK_SIZE, obstacleColor);
    }
}

// 更新蛇的位置
void update() {
    // 更新蛇的身体
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    snake.body[0].x += snake.dx;
    snake.body[0].y += snake.dy;

    // 检查是否吃到食物
    if (snake.body[0].x == food.position.x && snake.body[0].y == food.position.y) {
        snake.length++;
        score += 10;
        generateFood(); // 生成新的食物

        // 播放吃食物音效
        Mix_PlayChannel(-1, eatSound, 0);
    }

    // 检查是否撞墙
    if (snake.body[0].x < 0 || snake.body[0].x >= SCREEN_WIDTH / BLOCK_SIZE ||
        snake.body[0].y < 0 || snake.body[0].y >= SCREEN_HEIGHT / BLOCK_SIZE) {
        running = 0;
    }

    // 检查是否撞到自己
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y) {
            running = 0;
        }
    }

    // 检查是否撞到障碍物
    for (int i = 0; i < obstacleCount; i++) {
        if (snake.body[0].x == obstacles[i].x && snake.body[0].y == obstacles[i].y) {
            running = 0;
        }
    }
}

// 处理用户输入
void handleInput(SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        running = 0;
    }
    else if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
        case SDLK_UP:
            if (snake.dy == 0) {
                snake.dx = 0;
                snake.dy = -1;
            }
            break;
        case SDLK_DOWN:
            if (snake.dy == 0) {
                snake.dx = 0;
                snake.dy = 1;
            }
            break;
        case SDLK_LEFT:
            if (snake.dx == 0) {
                snake.dx = -1;
                snake.dy = 0;
            }
            break;
        case SDLK_RIGHT:
            if (snake.dx == 0) {
                snake.dx = 1;
                snake.dy = 0;
            }
            break;
        case SDLK_ESCAPE:
            running = 0; // 按 ESC 键退出
            break;
        }
    }
}

// 初始化 SDL、加载纹理和音效
void initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // 初始化 SDL 视频和音频子系统
    IMG_Init(IMG_INIT_PNG); // 初始化 SDL_image
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // 初始化音频系统

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // 创建渲染器

    // 加载背景音乐和吃食物音效
    backgroundMusic = Mix_LoadMUS("background.mp3");
    eatSound = Mix_LoadWAV("eat.wav");

    // 播放背景音乐
    Mix_PlayMusic(backgroundMusic, -1);
}

// 清理资源
void cleanUp() {
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(eatSound);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();
    Mix_Quit();
}

// 游戏主循环
int main(int argc, char* argv[]) {
    initSDL();   // 初始化 SDL
    initGame();  // 初始化游戏

    SDL_Event event;
    while (running) {
        // 处理输入事件
        while (SDL_PollEvent(&event)) {
            handleInput(&event);
        }

        // 更新游戏状态
        update();

        // 清屏
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 设置背景色为黑色
        SDL_RenderClear(renderer);

        // 绘制游戏元素
        drawSnake();
        drawFood();
        drawObstacles();

        // 更新屏幕
        SDL_RenderPresent(renderer);

        // 控制游戏帧率
        SDL_Delay(100);
    }

    // 打印得分并清理资源
    printf("Game Over! Your score: %d\n", score);
    cleanUp();
    return 0;
}

