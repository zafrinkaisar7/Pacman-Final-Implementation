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
bool gameWon = false;

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

    int temp[ROWS][COLS] = {

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},

        {1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,0,1},

        {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},

        {1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1},

        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},

        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1},

        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1},

        {1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1},

        {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},

        {1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1},

        {1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1},

        {1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},

        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},

        {1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1},

        {1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},

        {1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1},

        {1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1},

        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    for(int i=0;i<ROWS;i++){

        for(int j=0;j<COLS;j++){

            maze[i][j] = temp[i][j];
        }
    }

    pacX = 1;
    pacY = 1;
}

// ================= CHECK WIN =================

bool allFoodEaten(){

    for(int i=0;i<ROWS;i++){

        for(int j=0;j<COLS;j++){

            if(maze[i][j] == 0){

                return false;
            }
        }
    }

    return true;
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

    if(nx >= 0 && nx < COLS &&
       ny >= 0 && ny < ROWS &&
       maze[ny][nx] != 1){

        pacX = nx;
        pacY = ny;

        if(maze[ny][nx] == 0){

            maze[ny][nx] = -1;

            score += 10;

            playEatSound();

            if(allFoodEaten()){

                stopBackgroundMusic();

                saveScore();

                showLeaderboard();

                gameWon = true;
            }
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

        if(nx >= 0 && nx < COLS &&
           ny >= 0 && ny < ROWS &&
           maze[ny][nx] != 1){

            g.x = nx;
            g.y = ny;
        }
    }
}
/ ================= COLLISION =================

void checkCollision(){

    for(auto &g : ghosts){

        if(pacX == g.x &&  // same tile == collision (col detect)
           pacY == g.y){

            playDeathSound();

            lives--;  // life incr decr

            pacX = 1;
            pacY = 1;

            dirX = 0;
            dirY = 0;

            if(lives <= 0){

                stopBackgroundMusic();

                saveScore();

                showLeaderboard();

                gameOver = true;
            }
        }
    }
}
// ================= RESTART =================

void restartGame(){

    score = 0;

    lives = 3;

    gameOver = false;

    gameWon = false;

    inMenu = false;

    pacX = 1;
    pacY = 1;

    dirX = 0;
    dirY = 0;

    generateMaze();

    ghosts.clear();

    ghosts.push_back({10,10,1,0,0});
    maze[10][10] = -1;

    ghosts.push_back({15,15,0,1,1});                // reset all the variables and start again
    maze[15][15] = -1;

    ghosts.push_back({5,15,1,0,1});
    maze[5][15] = -1;

    playBackgroundMusic();

    glutPostRedisplay();
}
// ================= MENU =================

void drawMenu(){

    glColor3f(0,1,0);

    string title = "PACMAN GAME";

    glRasterPos2f(WIDTH/6, HEIGHT/6);  // glRasterPos2f is text position

    for(char c : title){

        glutBitmapCharacter(    //character draw
            GLUT_BITMAP_TIMES_ROMAN_24,
            c
        );
    }

    glColor3f(1,1,1);

    string msg =
        "PRESS ENTER TO START";

    glRasterPos2f(WIDTH/3, HEIGHT/2);

    for(char c : msg){

        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            c
        );
    }
}


// ================= DISPLAY =================

void display(){   // renders all

    glClear(GL_COLOR_BUFFER_BIT);  // clear screen

    // MENU
    if(inMenu){

        drawMenu();
    }

    // WIN SCREEN
    else if(gameWon){

        glColor3f(0,1,0);

        string win =
            "YOU WIN THE GAME!";

        glRasterPos2f(
            WIDTH/3,
            HEIGHT/3
        );

        for(char c : win){

            glutBitmapCharacter(
                GLUT_BITMAP_TIMES_ROMAN_24,
                c
            );
        }

        glColor3f(1,1,1);

        string sc =
            "Final Score: " +
            to_string(score);

        glRasterPos2f(
            WIDTH/3,
            HEIGHT/2
        );

        for(char c : sc){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }

