#include <GL/glut.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib,"winmm.lib")

#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

const int WIDTH = 600;
const int HEIGHT = 600;

const int ROWS = 20;
const int COLS = 20;

const int TILE = WIDTH / COLS;

int maze[ROWS][COLS];

int pacX = 1;
int pacY = 1;

int dirX = 0;
int dirY = 0;

int score = 0;
int lives = 3;

string playerName;

bool inMenu = true;
bool gameOver = false;

float mouthAngle = 30;
bool opening = true;

const double PI = 3.141592653589793;

// ================= GHOST =================

struct Ghost{
    int x, y;
    float r, g, b;
};

vector<Ghost> ghosts;

// ================= SOUND =================

void playBackgroundMusic(){

    mciSendString(
        "close bgm",
        NULL,
        0,
        NULL
    );

    mciSendString(
        "open \"bg.wav\" type waveaudio alias bgm",
        NULL,
        0,
        NULL
    );

    mciSendString(
        "play bgm repeat",
        NULL,
        0,
        NULL
    );
}


void stopBackgroundMusic(){

    mciSendString(
        "stop bgm",
        NULL,
        0,
        NULL
    );

    mciSendString(
        "close bgm",
        NULL,
        0,
        NULL
    );
}
void playEatSound(){

    PlaySound(
        TEXT("eat.wav"),
        NULL,
        SND_ASYNC | SND_FILENAME
    );
}

void playDeathSound(){

    PlaySound(
        TEXT("death.wav"),
        NULL,
        SND_FILENAME | SND_SYNC
    );
}

