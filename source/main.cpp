#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// ���ڳߴ�
#define GRID_SIZE 20
#define GRID_WIDTH 18
#define GRID_HEIGHT 27
#define WINDOW_WIDTH (GRID_WIDTH * GRID_SIZE + 200)

// ��ɫRGB
#define FLAT_RED RGB(231, 76, 60)
#define FLAT_BLUE RGB(52, 152, 219)
#define FLAT_GREEN RGB(46, 204, 113)
#define FLAT_YELLOW RGB(241, 196, 15)
#define FLAT_CYAN RGB(26, 188, 156)
#define FLAT_MAGENTA RGB(155, 89, 182)
#define FLAT_GRAY RGB(149, 165, 166)

#define INVISIBLE_BLOCK -1 // ����һ���������η��������ֵ

// ������Ϣ
typedef struct
{
    int x, y;        // ����
    int shape[4][4]; // ������״
    int color;       // ������ɫ
} Block;

int board[GRID_HEIGHT][GRID_WIDTH] = {0}; // ��ʼ������
int score = 0;                            // ������ʼ��Ϊ0
bool isPaused = false;                    // ��Ϸ�Ƿ���ͣ
bool gameOver = false;                    // ��Ϸ������־
Block nextBlock;                          // ��һ������
int currentDifficulty = 0;                // Ĭ��Ϊ��򵥵��Ѷ�
bool isInvisibleMode = false;             // ����ģʽ��

// ��ͬ��״����ɫ�ķ���...
int shapes[7][4][4] = {
    // I��
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // ����
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // T��
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0}},
    // S��
    {
        {0, 0, 0, 0},
        {0, 0, 1, 1},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // Z��
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 0}},
    // L��
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // ��L
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}}};
int colors[7] = {FLAT_RED, FLAT_BLUE, FLAT_GREEN, FLAT_YELLOW, FLAT_CYAN, FLAT_MAGENTA, FLAT_GRAY};

// ����
void initBlock(Block *block);                                                   // ��ʼ������
void drawBlock(const Block *block);                                             // ���Ʒ���
void rotateBlock(Block *block);                                                 // ��������ת
bool checkCollision(const Block *block, int dx, int dy);                        // �ж���ײ
void mergeBlock(Block *block);                                                  // �ϲ�����
void clearLines();                                                              // �����
void drawBoard();                                                               // ���ƽ���
int predictFallPosition(const Block *block);                                    // Ԥ�ⷽ������λ��
void drawNextBlockPreview();                                                    // ������һ������
void showStartMenu();                                                           // ��ʼ�˵�
int selectDifficulty();                                                         // �Ѷ�ѡ��˵�
void showMenu();                                                                // ���˵�
void startGame(int fallInterval);                                               // ��Ϸ������
void handleInput(Block *block, DWORD *lastTimeKeyPressed, DWORD *lastFallTime); // ����������
void updateGameState(Block *block, DWORD *lastFallTime, int fallInterval);      // ������Ϸ
void showGameOverScreen();                                                      // ��Ϸ�����˵�
void drawPauseMenu();                                                           // ��ͣ�˵�
void resetGameState();                                                          // ������Ϸ
bool askForReplay();                                                            // �Ƿ����¿�ʼ
void saveHighScore(int currentScore);                                           // ��߷�д���ļ�
int loadHighScore();                                                            // ������߷�
void playMusic(const char *fileName);                                           // ��������
void pauseMusic();                                                              // ��ͣ����
void resumeMusic();                                                             // ��������

int main()
{

    initgraph(WINDOW_WIDTH, GRID_HEIGHT * GRID_SIZE, 0);

    srand((unsigned)time(NULL));

    showMenu(); // ��ʾ�˵�����ʼ��Ϸ

    closegraph();

    return 0;
}

// ��������
void playMusic(const char *fileName)
{
    mciSendString("close all", NULL, 0, NULL); // �ر���������
    char command[256];
    sprintf(command, "open \"%s\" type mpegvideo alias music", fileName);
    mciSendString(command, NULL, 0, NULL);
    mciSendString("play music repeat", NULL, 0, NULL); // ѭ����������
}

// ��ͣ����
void pauseMusic()
{
    mciSendString("pause music", NULL, 0, NULL); // ��ͣ����
}

// ��������
void resumeMusic()
{
    mciSendString("resume music", NULL, 0, NULL); // �ָ�����
}

// ��ʼ��Ϸ�˵�
void showStartMenu()
{
    playMusic("./assets/menu.mp3");

    settextcolor(WHITE);
    settextstyle(40, 0, "����");
    outtextxy(WINDOW_WIDTH / 2 - 100, GRID_HEIGHT * GRID_SIZE / 2 - 20, "��ʼ��Ϸ");
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

// �Ѷ�ѡ��˵�
int selectDifficulty()
{
    isInvisibleMode = false;
    settextcolor(WHITE);
    settextstyle(25, 0, "����");
    outtextxy(250, 100, "����");
    outtextxy(250, 200, "�е�");
    outtextxy(250, 300, "����");
    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                if (msg.y >= 100 && msg.y <= 125)
                {
                    currentDifficulty = 0; // ����
                    playMusic("./assets/easy.mp3");
                    return 0;
                }
                if (msg.y >= 200 && msg.y <= 225)
                {
                    currentDifficulty = 1; // �е�
                    playMusic("./assets/common.mp3");
                    return 1;
                }
                if (msg.y >= 300 && msg.y <= 325)
                {
                    currentDifficulty = 2; // ����
                    playMusic("./assets/hard.mp3");
                    return 2;
                }
                if (msg.y >= 400 && msg.y <= 425)
                {
                    currentDifficulty = 3; // ����
                    playMusic("./assets/secret.mp3");
                    return 3;
                }
            }
        }
    }
}

// ��ʼ������
void initBlock(Block *block)
{
    if (nextBlock.color == 0)
    { // �������Ϸ�Ŀ�ʼ�������������һ������
        nextBlock.color = colors[rand() % 7];
        memcpy(nextBlock.shape, shapes[rand() % 7], sizeof(nextBlock.shape));
    }

    // ����ǰ��������Ϊ��һ������
    memcpy(block->shape, nextBlock.shape, sizeof(block->shape));
    block->color = nextBlock.color;
    block->x = GRID_WIDTH / 2 - 2;
    block->y = 0;

    // �����µ���һ������
    nextBlock.color = colors[rand() % 7];
    memcpy(nextBlock.shape, shapes[rand() % 7], sizeof(nextBlock.shape));
}

// ���Ʒ���
void drawBlock(const Block *block)
{
    // ���������ɫ����䷽��
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

                // ���ñ߿���ɫ�����Ʊ߿�
                setcolor(WHITE); // ���Ը�Ϊ��ѡ�����ɫ
                setlinestyle(PS_SOLID, 2);
                rectangle(x, y, x + GRID_SIZE - 1, y + GRID_SIZE - 1);
            }
        }
    }
    int fallY = predictFallPosition(block);

    // ����Ԥ�����λ�õļӴְ�ɫ����
    setcolor(WHITE);           // ������ɫΪ��ɫ
    setlinestyle(PS_SOLID, 3); // ����������ʽΪʵ�ߣ��߿�Ϊ 3
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

// ������һ������
void drawNextBlockPreview()
{
    int previewX = GRID_WIDTH * GRID_SIZE + 50; // ����Ԥ������� X ����
    int previewY = 50;                          // ����Ԥ������� Y ����

    // ������һ������
    settextcolor(WHITE);
    settextstyle(25, 0, "����");
    outtextxy(400, 20, "��һ�����飺");
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

// ���Ƽ�λ����
void drawControl()
{
    outtextxy(400, 250, "���̿���");
    outtextxy(400, 300, "A\\D\\S �ƶ�");
    outtextxy(400, 350, "W ��ת");
    outtextxy(400, 400, "�ո� ����");
    outtextxy(400, 450, "ESC ��ͣ");
}

// Ԥ�ⷽ������λ��
int predictFallPosition(const Block *block)
{
    int fallY = block->y;
    while (!checkCollision(block, 0, fallY - block->y + 1))
    {
        fallY++;
    }
    return fallY;
}

// ��������ת
void rotateBlock(Block *block)
{
    int temp[4][4];
    memcpy(temp, block->shape, sizeof(block->shape));

    // ��ת����
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            block->shape[j][3 - i] = temp[i][j];
        }
    }

    // ���Բ�ͬ��λ�õ�����ֱ���ҵ����ʵ�λ�û�ȷ����ת������
    int adjustments[5] = {0, -1, 1, -2, 2}; // ����ԭλ�á����ơ�����
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

    // ���û�гɹ���ת����ԭ��ԭ����״̬
    if (!rotated)
    {
        memcpy(block->shape, temp, sizeof(temp));
    }
}

// �ж���ײ
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

// �ϲ�����������
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

// �����У��ӷ�
void clearLines()
{
    for (int i = GRID_HEIGHT - 1; i >= 0; i--)
    {
        bool fullLine = true;
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (board[i][j] == 0)
            { // ���ĳ����ԪΪ��
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

// ���ƻ���
void drawBoard()
{
    // ���Ʒ���
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            if (board[i][j] != 0)
            { // �����Ԫ��Ϊ��
                setfillcolor(board[i][j]);
                fillrectangle(
                    j * GRID_SIZE,
                    i * GRID_SIZE,
                    (j + 1) * GRID_SIZE - 1,
                    (i + 1) * GRID_SIZE - 1);
            }
        }
    }

    // ����������
    setcolor(0x646464); // ������������ɫ
    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i <= GRID_HEIGHT; i++)
    {
        // ����ˮƽ��
        line(0, i * GRID_SIZE, GRID_WIDTH * GRID_SIZE, i * GRID_SIZE);
    }
    for (int j = 0; j <= GRID_WIDTH; j++)
    {
        // ���ƴ�ֱ��
        line(j * GRID_SIZE, 0, j * GRID_SIZE, GRID_HEIGHT * GRID_SIZE);
    }
}

// ������ͣ�˵�
void drawPauseMenu()
{
    // �����ı�����ɫ
    settextcolor(WHITE);
    settextstyle(20, 0, "Arial");
    setlinestyle(PS_SOLID, 3);

    // ���谴ť�Ĵ�С��λ��
    int btnWidth = 100, btnHeight = 30;
    int continueX = WINDOW_WIDTH / 2 - btnWidth / 2, continueY = 200;
    int restartX = WINDOW_WIDTH / 2 - btnWidth / 2, restartY = 250;

    // ���Ƽ�����Ϸ��ť
    outtextxy(continueX, continueY, "������Ϸ");
    rectangle(continueX, continueY, continueX + btnWidth, continueY + btnHeight);

    // �������¿�ʼ��ť
    outtextxy(restartX, restartY, "���¿�ʼ");
    rectangle(restartX, restartY, restartX + btnWidth, restartY + btnHeight);
}

// ������Ϸ
void resetGameState()
{
    // ������Ϸ��ͷ���
    memset(board, 0, sizeof(board));
    score = 0;

    // ������Ϸ������־
    gameOver = false;
    isPaused = false;

    // ������һ������
    nextBlock.color = 0; // �⽫����initBlock�е���һ����������
}

// ���ֲ˵�
void showMenu()
{

    cleardevice();
    // ��ʾ��ʼ�˵�����ȡ�Ѷ�ѡ��
    showStartMenu();
    int difficulty = selectDifficulty();

    // ������Ϸ״̬
    resetGameState();

    // ��ʼ��Ϸ
    startGame(difficulty);
}

// ���봦��
void handleInput(Block *block, DWORD *lastTimeKeyPressed, DWORD *lastFallTime)
{
    DWORD currentTime = GetTickCount();

    // �����ͣ�� (ESC ��)
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        if (currentTime - lastTimeKeyPressed[5] > 300)
        { // ��ֹ���������ظ�����
            if (!isPaused)
            {
                pauseMusic(); // ��ͣ����
                isPaused = true;
            }
            else
            {
                resumeMusic(); // �ָ�����
                isPaused = false;
            }
            lastTimeKeyPressed[5] = currentTime; // ����ʱ���
        }
    }

    // �����Ϸ��ͣ
    if (isPaused)
    {
        if (MouseHit())
        {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_LBUTTONDOWN)
            {
                // ���谴ť�Ĵ�С��λ��
                int btnWidth = 100, btnHeight = 30;
                int continueX = WINDOW_WIDTH / 2 - btnWidth / 2, continueY = 200;
                int restartX = WINDOW_WIDTH / 2 - btnWidth / 2, restartY = 250;

                // ��������Ϸ��ť���
                if (m.x >= continueX && m.x <= continueX + btnWidth && m.y >= continueY && m.y <= continueY + btnHeight)
                {
                    resumeMusic(); // �ָ�����
                    isPaused = false;
                }
                // ������¿�ʼ��ť���
                else if (m.x >= restartX && m.x <= restartX + btnWidth && m.y >= restartY && m.y <= restartY + btnHeight)
                {
                    resetGameState();
                    initBlock(block); // ���÷���λ�ú���״
                    *lastFallTime = GetTickCount();
                    isPaused = false;

                    // ���ݵ�ǰ�ѶȲ�������
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
                    // �����ת������ײ������ת��ԭ״
                    rotateBlock(block);
                    rotateBlock(block);
                    rotateBlock(block);
                }
                lastTimeKeyPressed[0] = currentTime;
            }
        }

        // �����ƶ����� (A ��)
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

        // �����ƶ����� (D ��)
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

        // ���ٷ������� (S ��)
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

        // ����ֱ���䵽�ײ� (�ո��)
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

// ������Ϸ״̬
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
                gameOver = true; // ��Ϸ����
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

// ����
void startGame(int difficulty)
{
    int fallInterval;
    // ����ѡ����Ѷ�������Ϸ����
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

    // ����˫����
    BeginBatchDraw();

    Block block;
    initBlock(&block);

    DWORD lastFallTime = GetTickCount();
    DWORD lastTimeKeyPressed[5] = {0, 0, 0, 0, 0}; // ʱ������飬��Ӧ W, A, D, S, Space

    // ������Ϸ״̬
    score = 0;
    gameOver = false;
    isPaused = false;

    // ��Ϸ��ѭ��
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

// ѯ���Ƿ�����һ��
bool askForReplay()
{
    int result = MessageBox(GetHWnd(), "����һ����", "��Ϸ����", MB_YESNO | MB_ICONQUESTION);
    return result == IDYES;
}

// ��Ϸ�����˵�
void showGameOverScreen()
{
    mciSendString("open ./assets/die.mp3", NULL, 0, NULL);
    mciSendString("play ./assets/die.mp3 wait", NULL, 0, NULL);
    // ��ʾ��Ϸ������Ļ
    outtextxy(WINDOW_WIDTH / 2 - 75, GRID_HEIGHT * GRID_SIZE / 2, "Game Over");
    Sleep(2000);

    // ������߷�
    saveHighScore(score);

    // ѯ���û��Ƿ�����һ��
    if (askForReplay())
    {
        // ������Ϸ״̬����ʼ����Ϸ
        showMenu();
    }
    else
    {
        // �˳�����
        closegraph();
        exit(0);
    }
}

// ��߷ִ����ļ�
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

// ��ȡ��߷�
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
