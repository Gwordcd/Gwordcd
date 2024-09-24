#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 窗口尺寸
#define GRID_SIZE 20
#define GRID_WIDTH 18
#define GRID_HEIGHT 27
#define WINDOW_WIDTH (GRID_WIDTH * GRID_SIZE + 200)

// 颜色RGB
#define FLAT_RED RGB(231, 76, 60)
#define FLAT_BLUE RGB(52, 152, 219)
#define FLAT_GREEN RGB(46, 204, 113)
#define FLAT_YELLOW RGB(241, 196, 15)
#define FLAT_CYAN RGB(26, 188, 156)
#define FLAT_MAGENTA RGB(155, 89, 182)
#define FLAT_GRAY RGB(149, 165, 166)

#define INVISIBLE_BLOCK -1 // 定义一个用于隐形方块的特殊值

// 方块信息
typedef struct
{
    int x, y;        // 坐标
    int shape[4][4]; // 方块形状
    int color;       // 方块颜色
} Block;

int board[GRID_HEIGHT][GRID_WIDTH] = {0}; // 初始化画布
int score = 0;                            // 分数初始化为0
bool isPaused = false;                    // 游戏是否暂停
bool gameOver = false;                    // 游戏结束标志
Block nextBlock;                          // 下一个方块
int currentDifficulty = 0;                // 默认为最简单的难度
bool isInvisibleMode = false;             // 隐形模式关

// 不同形状和颜色的方块...
int shapes[7][4][4] = {
    // I形
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // 方形
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // T形
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0}},
    // S形
    {
        {0, 0, 0, 0},
        {0, 0, 1, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // Z形
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 0}},
    // L形
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // 反L
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}}};
int colors[7] = {FLAT_RED, FLAT_BLUE, FLAT_GREEN, FLAT_YELLOW, FLAT_CYAN, FLAT_MAGENTA, FLAT_GRAY};

// 声明
void initBlock(Block *block);                                                   // 初始化方块
void drawBlock(const Block *block);                                             // 绘制方块
void rotateBlock(Block *block);                                                 // 处理方块旋转
bool checkCollision(const Block *block, int dx, int dy);                        // 判断碰撞
void mergeBlock(Block *block);                                                  // 合并方块
void clearLines();                                                              // 清除行
void drawBoard();                                                               // 绘制界面
int predictFallPosition(const Block *block);                                    // 预测方块落下位置
void drawNextBlockPreview();                                                    // 绘制下一个方块
void showStartMenu();                                                           // 开始菜单
int selectDifficulty();                                                         // 难度选择菜单
void showMenu();                                                                // 主菜单
void startGame(int fallInterval);                                               // 游戏主界面
void handleInput(Block *block, DWORD *lastTimeKeyPressed, DWORD *lastFallTime); // 键盘鼠标操作
void updateGameState(Block *block, DWORD *lastFallTime, int fallInterval);      // 更新游戏
void showGameOverScreen();                                                      // 游戏结束菜单
void drawPauseMenu();                                                           // 暂停菜单
void resetGameState();                                                          // 重置游戏
bool askForReplay();                                                            // 是否重新开始
void saveHighScore(int currentScore);                                           // 最高分写入文件
int loadHighScore();                                                            // 加载最高分
void playMusic(const char *fileName);                                           // 播放音乐
void pauseMusic();                                                              // 暂停音乐
void resumeMusic();                                                             // 继续播放

int main()
{

    initgraph(WINDOW_WIDTH, GRID_HEIGHT * GRID_SIZE, 0);

    srand((unsigned)time(NULL));

    showMenu(); // 显示菜单并开始游戏

    closegraph();

    return 0;
}

// 播放音乐
void playMusic(const char *fileName)
{
    mciSendString("close all", NULL, 0, NULL); // 关闭所有音乐
    char command[256];
    sprintf(command, "open \"%s\" type mpegvideo alias music", fileName);
    mciSendString(command, NULL, 0, NULL);
    mciSendString("play music repeat", NULL, 0, NULL); // 循环播放音乐
}

// 暂停音乐
void pauseMusic()
{
    mciSendString("pause music", NULL, 0, NULL); // 暂停音乐
}

// 继续播放
void resumeMusic()
{
    mciSendString("resume music", NULL, 0, NULL); // 恢复音乐
}

// 开始游戏菜单
void showStartMenu()
{
    playMusic("./assets/menu.mp3");

    settextcolor(WHITE);
    settextstyle(40, 0, "黑体");
    outtextxy(WINDOW_WIDTH / 2 - 100, GRID_HEIGHT * GRID_SIZE / 2 - 20, "开始游戏");
    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN && msg.x >= WINDOW_WIDTH / 2 - 100 && msg.x <= WINDOW_WIDTH / 2 + 100 &&
                msg.y >= GRID_HEIGHT * GRID_SIZE / 2 - 20 && msg.y <= GRID_HEIGHT * GRID_SIZE / 2 + 20)
            {
                cleardevice();
                break;
            }
        }
    }
}

// 难度选择菜单
int selectDifficulty()
{
    isInvisibleMode = false;
    settextcolor(WHITE);
    settextstyle(25, 0, "黑体");
    outtextxy(250, 100, "容易");
    outtextxy(250, 200, "中等");
    outtextxy(250, 300, "困难");
    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                if (msg.y >= 100 && msg.y <= 125)
                {
                    currentDifficulty = 0; // 容易
                    playMusic("./assets/easy.mp3");
                    return 0;
                }
                if (msg.y >= 200 && msg.y <= 225)
                {
                    currentDifficulty = 1; // 中等
                    playMusic("./assets/common.mp3");
                    return 1;
                }
                if (msg.y >= 300 && msg.y <= 325)
                {
                    currentDifficulty = 2; // 困难
                    playMusic("./assets/hard.mp3");
                    return 2;
                }
                if (msg.y >= 400 && msg.y <= 425)
                {
                    currentDifficulty = 3; // 隐形
                    playMusic("./assets/secret.mp3");
                    return 3;
                }
            }
        }
    }
}

// 初始化方块
void initBlock(Block *block)
{
    if (nextBlock.color == 0)
    { // 如果是游戏的开始，则随机生成下一个方块
        nextBlock.color = colors[rand() % 7];
        memcpy(nextBlock.shape, shapes[rand() % 7], sizeof(nextBlock.shape));
    }

    // 将当前方块设置为下一个方块
    memcpy(block->shape, nextBlock.shape, sizeof(block->shape));
    block->color = nextBlock.color;
    block->x = GRID_WIDTH / 2 - 2;
    block->y = 0;

    // 生成新的下一个方块
    nextBlock.color = colors[rand() % 7];
    memcpy(nextBlock.shape, shapes[rand() % 7], sizeof(nextBlock.shape));
}

// 绘制方块
void drawBlock(const Block *block)
{
    // 设置填充颜色并填充方块
    setfillcolor(block->color);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block->shape[i][j])
            {
                int x = (block->x + j) * GRID_SIZE;
                int y = (block->y + i) * GRID_SIZE;
                fillrectangle(x, y, x + GRID_SIZE - 1, y + GRID_SIZE - 1);

                // 设置边框颜色并绘制边框
                setcolor(WHITE); // 可以改为您选择的颜色
                setlinestyle(PS_SOLID, 2);
                rectangle(x, y, x + GRID_SIZE - 1, y + GRID_SIZE - 1);
            }
        }
    }
    int fallY = predictFallPosition(block);

    // 绘制预测落地位置的加粗白色轮廓
    setcolor(WHITE);           // 设置颜色为白色
    setlinestyle(PS_SOLID, 3); // 设置线条样式为实线，线宽为 3
    if (isInvisibleMode)
    {
        setlinestyle(PS_NULL, 1);
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block->shape[i][j])
            {
                int x = (block->x + j) * GRID_SIZE;
                int y = (fallY + i) * GRID_SIZE;
                rectangle(x, y, x + GRID_SIZE - 1, y + GRID_SIZE - 1);
            }
        }
    }
}

// 绘制下一个方块
void drawNextBlockPreview()
{
    int previewX = GRID_WIDTH * GRID_SIZE + 50; // 设置预览区域的 X 坐标
    int previewY = 50;                          // 设置预览区域的 Y 坐标

    // 绘制下一个方块
    settextcolor(WHITE);
    settextstyle(25, 0, "黑体");
    outtextxy(400, 20, "下一个方块：");
    setfillcolor(nextBlock.color);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (nextBlock.shape[i][j])
            {
                fillrectangle(
                    previewX + j * GRID_SIZE,
                    previewY + i * GRID_SIZE,
                    previewX + (j + 1) * GRID_SIZE - 1,
                    previewY + (i + 1) * GRID_SIZE - 1);
            }
        }
    }
}

// 绘制键位控制
void drawControl()
{
    outtextxy(400, 250, "键盘控制");
    outtextxy(400, 300, "A\\D\\S 移动");
    outtextxy(400, 350, "W 旋转");
    outtextxy(400, 400, "空格 下落");
    outtextxy(400, 450, "ESC 暂停");
}

// 预测方块落下位置
int predictFallPosition(const Block *block)
{
    int fallY = block->y;
    while (!checkCollision(block, 0, fallY - block->y + 1))
    {
        fallY++;
    }
    return fallY;
}

// 处理方块旋转
void rotateBlock(Block *block)
{
    int temp[4][4];
    memcpy(temp, block->shape, sizeof(block->shape));

    // 旋转方块
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            block->shape[j][3 - i] = temp[i][j];
        }
    }

    // 尝试不同的位置调整，直到找到合适的位置或确认旋转不可能
    int adjustments[5] = {0, -1, 1, -2, 2}; // 包括原位置、左移、右移
    bool rotated = false;
    for (int i = 0; i < 5; i++)
    {
        int dx = adjustments[i];
        if (!checkCollision(block, dx, 0))
        {
            block->x += dx;
            rotated = true;
            break;
        }
    }

    // 如果没有成功旋转，还原到原来的状态
    if (!rotated)
    {
        memcpy(block->shape, temp, sizeof(temp));
    }
}

// 判断碰撞
bool checkCollision(const Block *block, int dx, int dy)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block->shape[i][j])
            {
                int x = block->x + j + dx;
                int y = block->y + i + dy;
                if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || board[y][x] != 0)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// 合并方块至画布
void mergeBlock(Block *block)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block->shape[i][j])
            {
                if (isInvisibleMode)
                {
                    board[block->y + i][block->x + j] = INVISIBLE_BLOCK;
                }
                else
                {
                    board[block->y + i][block->x + j] = block->color;
                }
            }
        }
    }
}

// 消除行，加分
void clearLines()
{
    for (int i = GRID_HEIGHT - 1; i >= 0; i--)
    {
        bool fullLine = true;
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (board[i][j] == 0)
            { // 如果某个单元为空
                fullLine = false;
                break;
            }
        }

        if (fullLine)
        {
            for (int k = i; k > 0; k--)
            {
                for (int j = 0; j < GRID_WIDTH; j++)
                {
                    board[k][j] = board[k - 1][j];
                }
            }
            for (int j = 0; j < GRID_WIDTH; j++)
            {
                board[0][j] = 0;
            }
            score += 100;
            i++;
        }
    }
}

// 绘制画布
void drawBoard()
{
    // 绘制方块
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (board[i][j] != 0)
            { // 如果单元格不为空
                setfillcolor(board[i][j]);
                fillrectangle(
                    j * GRID_SIZE,
                    i * GRID_SIZE,
                    (j + 1) * GRID_SIZE - 1,
                    (i + 1) * GRID_SIZE - 1);
            }
        }
    }

    // 绘制网格线
    setcolor(0x646464); // 设置网格线颜色
    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i <= GRID_HEIGHT; i++)
    {
        // 绘制水平线
        line(0, i * GRID_SIZE, GRID_WIDTH * GRID_SIZE, i * GRID_SIZE);
    }
    for (int j = 0; j <= GRID_WIDTH; j++)
    {
        // 绘制垂直线
        line(j * GRID_SIZE, 0, j * GRID_SIZE, GRID_HEIGHT * GRID_SIZE);
    }
}

// 绘制暂停菜单
void drawPauseMenu()
{
    // 设置文本和颜色
    settextcolor(WHITE);
    settextstyle(20, 0, "Arial");
    setlinestyle(PS_SOLID, 3);

    // 假设按钮的大小和位置
    int btnWidth = 100, btnHeight = 30;
    int continueX = WINDOW_WIDTH / 2 - btnWidth / 2, continueY = 200;
    int restartX = WINDOW_WIDTH / 2 - btnWidth / 2, restartY = 250;

    // 绘制继续游戏按钮
    outtextxy(continueX, continueY, "继续游戏");
    rectangle(continueX, continueY, continueX + btnWidth, continueY + btnHeight);

    // 绘制重新开始按钮
    outtextxy(restartX, restartY, "重新开始");
    rectangle(restartX, restartY, restartX + btnWidth, restartY + btnHeight);
}

// 重置游戏
void resetGameState()
{
    // 重置游戏板和分数
    memset(board, 0, sizeof(board));
    score = 0;

    // 重置游戏结束标志
    gameOver = false;
    isPaused = false;

    // 重置下一个方块
    nextBlock.color = 0; // 这将触发initBlock中的下一个方块生成
}

// 再现菜单
void showMenu()
{

    cleardevice();
    // 显示开始菜单并获取难度选择
    showStartMenu();
    int difficulty = selectDifficulty();

    // 重置游戏状态
    resetGameState();

    // 开始游戏
    startGame(difficulty);
}

// 输入处理
void handleInput(Block *block, DWORD *lastTimeKeyPressed, DWORD *lastFallTime)
{
    DWORD currentTime = GetTickCount();

    // 检查暂停键 (ESC 键)
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        if (currentTime - lastTimeKeyPressed[5] > 300)
        { // 防止按键过快重复触发
            if (!isPaused)
            {
                pauseMusic(); // 暂停音乐
                isPaused = true;
            }
            else
            {
                resumeMusic(); // 恢复音乐
                isPaused = false;
            }
            lastTimeKeyPressed[5] = currentTime; // 更新时间戳
        }
    }

    // 如果游戏暂停
    if (isPaused)
    {
        if (MouseHit())
        {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_LBUTTONDOWN)
            {
                // 假设按钮的大小和位置
                int btnWidth = 100, btnHeight = 30;
                int continueX = WINDOW_WIDTH / 2 - btnWidth / 2, continueY = 200;
                int restartX = WINDOW_WIDTH / 2 - btnWidth / 2, restartY = 250;

                // 检测继续游戏按钮点击
                if (m.x >= continueX && m.x <= continueX + btnWidth && m.y >= continueY && m.y <= continueY + btnHeight)
                {
                    resumeMusic(); // 恢复音乐
                    isPaused = false;
                }
                // 检测重新开始按钮点击
                else if (m.x >= restartX && m.x <= restartX + btnWidth && m.y >= restartY && m.y <= restartY + btnHeight)
                {
                    resetGameState();
                    initBlock(block); // 重置方块位置和形状
                    *lastFallTime = GetTickCount();
                    isPaused = false;

                    // 根据当前难度播放音乐
                    switch (currentDifficulty)
                    {
                    case 0:
                        playMusic("./assets/easy.mp3");
                        break;
                    case 1:
                        playMusic("./assets/common.mp3");
                        break;
                    case 2:
                        playMusic("./assets/hard.mp3");
                        break;
                    case 3:
                        playMusic("./assets/secret.mp3");
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if (GetAsyncKeyState('W') & 0x8000)
        {
            if (currentTime - lastTimeKeyPressed[0] > 100)
            {
                rotateBlock(block);
                if (checkCollision(block, 0, 0))
                {
                    // 如果旋转后发生碰撞，则旋转回原状
                    rotateBlock(block);
                    rotateBlock(block);
                    rotateBlock(block);
                }
                lastTimeKeyPressed[0] = currentTime;
            }
        }

        // 向左移动方块 (A 键)
        if (GetAsyncKeyState('A') & 0x8000)
        {
            if (currentTime - lastTimeKeyPressed[1] > 100)
            {
                if (!checkCollision(block, -1, 0))
                {
                    block->x--;
                    lastTimeKeyPressed[1] = currentTime;
                }
            }
        }

        // 向右移动方块 (D 键)
        if (GetAsyncKeyState('D') & 0x8000)
        {
            if (currentTime - lastTimeKeyPressed[2] > 100)
            {
                if (!checkCollision(block, 1, 0))
                {
                    block->x++;
                    lastTimeKeyPressed[2] = currentTime;
                }
            }
        }

        // 加速方块下落 (S 键)
        if (GetAsyncKeyState('S') & 0x8000)
        {
            if (currentTime - lastTimeKeyPressed[3] > 100)
            {
                if (!checkCollision(block, 0, 1))
                {
                    block->y++;
                    lastTimeKeyPressed[3] = currentTime;
                }
            }
        }

        // 方块直接落到底部 (空格键)
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            if (currentTime - lastTimeKeyPressed[4] > 200)
            {
                while (!checkCollision(block, 0, 1))
                {
                    block->y++;
                }
                lastTimeKeyPressed[4] = currentTime;
            }
        }
    }
}

// 更新游戏状态
void updateGameState(Block *block, DWORD *lastFallTime, int fallInterval)
{
    DWORD currentTime = GetTickCount();
    if (currentTime - *lastFallTime > fallInterval)
    {
        if (!checkCollision(block, 0, 1))
        {
            block->y++;
        }
        else
        {
            mergeBlock(block);
            clearLines();
            initBlock(block);
            if (checkCollision(block, 0, 0))
            {
                gameOver = true; // 游戏结束
            }
        }
        *lastFallTime = currentTime;
    }

    char scoreStr[20];
    sprintf(scoreStr, "Score: %d", score);
    outtextxy(5, 5, scoreStr);

    char maxStr[20];
    sprintf(maxStr, "Max:%d", loadHighScore());
    outtextxy(5, 25, maxStr);

    drawNextBlockPreview();
    drawControl();
}

// 开玩
void startGame(int difficulty)
{
    int fallInterval;
    // 根据选择的难度设置游戏参数
    switch (difficulty)
    {
    case 0:
        fallInterval = 400;
        isInvisibleMode = false;
        break;
    case 1:
        fallInterval = 200;
        isInvisibleMode = false;
        break;
    case 2:
        fallInterval = 100;
        isInvisibleMode = false;
        break;
    case 3:
        fallInterval = 100;
        isInvisibleMode = true;
        break;
    default:
        break;
    }

    // 开启双缓冲
    BeginBatchDraw();

    Block block;
    initBlock(&block);

    DWORD lastFallTime = GetTickCount();
    DWORD lastTimeKeyPressed[5] = {0, 0, 0, 0, 0}; // 时间戳数组，对应 W, A, D, S, Space

    // 重置游戏状态
    score = 0;
    gameOver = false;
    isPaused = false;

    // 游戏主循环
    while (!gameOver)
    {
        if (isPaused)
        {
            drawPauseMenu();
        }
        else
        {
            cleardevice();
            drawBoard();
            drawBlock(&block);
            updateGameState(&block, &lastFallTime, fallInterval);
        }

        handleInput(&block, lastTimeKeyPressed, &lastFallTime);
        FlushBatchDraw();

        Sleep(10);
    }
    EndBatchDraw();
    showGameOverScreen();
}

// 询问是否再来一局
bool askForReplay()
{
    int result = MessageBox(GetHWnd(), "再来一局吗？", "游戏结束", MB_YESNO | MB_ICONQUESTION);
    return result == IDYES;
}

// 游戏结束菜单
void showGameOverScreen()
{
    mciSendString("open ./assets/die.mp3", NULL, 0, NULL);
    mciSendString("play ./assets/die.mp3 wait", NULL, 0, NULL);
    // 显示游戏结束屏幕
    outtextxy(WINDOW_WIDTH / 2 - 75, GRID_HEIGHT * GRID_SIZE / 2, "Game Over");
    Sleep(2000);

    // 保存最高分
    saveHighScore(score);

    // 询问用户是否再来一局
    if (askForReplay())
    {
        // 重置游戏状态并开始新游戏
        showMenu();
    }
    else
    {
        // 退出程序
        closegraph();
        exit(0);
    }
}

// 最高分存入文件
void saveHighScore(int currentScore)
{
    int highScore = 0;
    FILE *file = fopen("highscore.dat", "r");
    if (file)
    {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }

    if (currentScore > highScore)
    {
        file = fopen("highscore.dat", "w");
        if (file)
        {
            fprintf(file, "%d", currentScore);
            fclose(file);
        }
    }
}

// 读取最高分
int loadHighScore()
{
    int highScore = 0;
    FILE *file = fopen("highscore.dat", "r");
    if (file)
    {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
    return highScore;
}
