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
// ================= SAVE SCORE =================

void saveScore(){

    ofstream file("leaderboard.txt", ios::app);

    file << playerName << " " << score << "\n";

    file.close();
}

// ================= SHOW LEADERBOARD =================

void showLeaderboard(){

    ifstream file("leaderboard.txt");

    string name;
    int sc;

    cout << "\n===== LEADERBOARD =====\n";

    while(file >> name >> sc){

        cout << name << " : " << sc << endl;
    }

    file.close();
}

// ================= MAZE =================

void generateMaze(){

    for(int i=0;i<ROWS;i++){

        for(int j=0;j<COLS;j++){

            if(i==0 || j==0 ||
               i==ROWS-1 ||
               j==COLS-1){

                maze[i][j] = 1;
            }

            else{

                if(i <= 3 && j <= 3){

                    maze[i][j] = 0;
                }

                else{

                    maze[i][j] =
                    (rand()%100 < 18)
                    ? 1 : 0;
                }
            }
        }
    }

    pacX = 1;
    pacY = 1;

    maze[1][1] = 0;
    maze[1][2] = 0;
    maze[2][1] = 0;
    maze[2][2] = 0;
}
// ================= DRAW MAZE =================

void drawMaze(){

    for(int i=0;i<ROWS;i++){

        for(int j=0;j<COLS;j++){

            // WALL
            if(maze[i][j] == 1){

                glColor3f(0,0,1);

                glRectf(
                    j*TILE,
                    i*TILE,
                    (j+1)*TILE,
                    (i+1)*TILE
                );
            }

            // FOOD
            else if(maze[i][j] == 0){

                glColor3f(1,1,1);

                glRectf(
                    j*TILE + TILE/2 - 2,
                    i*TILE + TILE/2 - 2,
                    j*TILE + TILE/2 + 2,
                    i*TILE + TILE/2 + 2
                );
            }
        }
    }
}
// ================= DRAW PACMAN =================

void drawPacman(){

    glColor3f(1,1,0);

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(
        pacX*TILE + TILE/2,
        pacY*TILE + TILE/2
    );

    for(int i=mouthAngle;
        i<=360-mouthAngle;
        i++){

        float rad = i * PI / 180;

        glVertex2f(
            pacX*TILE + TILE/2 +
            cos(rad)*TILE/2,

            pacY*TILE + TILE/2 +
            sin(rad)*TILE/2
        );
    }

    glEnd();

    // Eye
    glColor3f(0,0,0);

    glBegin(GL_POLYGON);

    for(int i=0;i<360;i++){

        float rad = i * PI / 180;

        glVertex2f(
            pacX*TILE + TILE/2 +
            5 + cos(rad)*2,

            pacY*TILE + TILE/2 -
            5 + sin(rad)*2
        );
    }

    glEnd();
}
// ================= DRAW GHOST =================

void drawGhost(Ghost &g){

    int gx = g.x*TILE + TILE/2;
    int gy = g.y*TILE + TILE/2;

    glColor3f(g.r,g.g,g.b);

    glBegin(GL_POLYGON);

    for(int i=0;i<180;i++){

        float rad = i * PI / 180;

        glVertex2f(
            gx + cos(rad)*TILE/2,
            gy + sin(rad)*TILE/2
        );
    }

    glVertex2f(gx-TILE/2,gy);
    glVertex2f(gx+TILE/2,gy);

    glEnd();

    // Eyes
    glColor3f(1,1,1);

    for(int k=0;k<2;k++){

        int offset =
            (k==0 ? -6 : 6);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++){

            float rad = i * PI / 180;

            glVertex2f(
                gx + offset +
                cos(rad)*4,

                gy - 5 +
                sin(rad)*4
            );
        }

        glEnd();

        glColor3f(0,0,0);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++){

            float rad = i * PI / 180;

            glVertex2f(
                gx + offset +
                cos(rad)*2,

                gy - 5 +
                sin(rad)*2
            );
        }

        glEnd();

        glColor3f(g.r,g.g,g.b);
    }
}

// ================= MOVE PACMAN =================

void movePacman(){

    int nx = pacX + dirX;
    int ny = pacY + dirY;

    if(maze[ny][nx] != 1){

        pacX = nx;
        pacY = ny;

        if(maze[ny][nx] == 0){

            maze[ny][nx] = -1;

            score += 10;

            playEatSound();
        }
    }
}
// ================= MOVE GHOSTS =================

void moveGhosts(){

    for(auto &g : ghosts){

        int dir = rand()%4;

        int dx[4] = {1,-1,0,0};
        int dy[4] = {0,0,1,-1};

        int nx = g.x + dx[dir];
        int ny = g.y + dy[dir];

        if(maze[ny][nx] != 1){

            g.x = nx;
            g.y = ny;
        }
    }
}

