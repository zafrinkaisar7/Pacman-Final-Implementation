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
// Restart Button
        glColor3f(0,1,0);

        glRectf(
            WIDTH/3,
            HEIGHT/2 + 80,
            WIDTH/3 + 180,
            HEIGHT/2 + 130
        );

        glColor3f(0,0,0);

        string btn =
            "PLAY AGAIN";

        glRasterPos2f(
            WIDTH/3 + 30,
            HEIGHT/2 + 110
        );

        for(char c : btn){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }
    }

    // GAME OVER
    else if(gameOver){

        glColor3f(1,0,0);

        string over =
            "GAME OVER";

        glRasterPos2f(
            WIDTH/3,
            HEIGHT/3
        );

        for(char c : over){

            glutBitmapCharacter(
                GLUT_BITMAP_TIMES_ROMAN_24,
                c
            );
        }

        glColor3f(1,1,1);

        string player =
            "Player: " +
            playerName;

        glRasterPos2f(
            WIDTH/3,
            HEIGHT/2
        );

        for(char c : player){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }

        string sc =
            "Final Score: " +
            to_string(score);

        glRasterPos2f(
            WIDTH/3,
            HEIGHT/2 + 40
        );

        for(char c : sc){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }

        // Restart Button
        glColor3f(0,1,0);

        glRectf(
            WIDTH/3,
            HEIGHT/2 + 80,
            WIDTH/3 + 180,
            HEIGHT/2 + 130
        );

        glColor3f(0,0,0);

        string btn =
            "START AGAIN";

        glRasterPos2f(
            WIDTH/3 + 30,
            HEIGHT/2 + 110
        );

        for(char c : btn){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }
    }

    // GAME
    else{  //actual gameplay

        drawMaze();

        drawPacman();

        for(auto &g : ghosts){

            drawGhost(g);
        }

        string hud =
            "Score: " +
            to_string(score) +

            " Lives: " +
            to_string(lives);

        glColor3f(1,1,1);

        glRasterPos2f(
            10,
            HEIGHT-20
        );

        for(char c : hud){

            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                c
            );
        }
    }

    glutSwapBuffers();
}
// ================= TIMER =================

void timer(int = 0){

    if(!inMenu &&
       !gameOver &&
       !gameWon){

        movePacman();

        moveGhosts();             //Update cycle of game

        checkCollision();


        // Mouth Animation for open close mouth
        if(opening)
            mouthAngle += 1;
        else
            mouthAngle -= 1;

        if(mouthAngle > 35)
            opening = false;

        if(mouthAngle < 10)
            opening = true;
    }

    glutPostRedisplay();

    glutTimerFunc(
        180,
        timer,                    //ghost speed inc dec reverse
        0
    );
}
// ================= KEYBOARD =================

void keyboard(
    int key,
    int,
    int){

    if(key == GLUT_KEY_LEFT){

        dirX = -1;
        dirY = 0;
    }

    if(key == GLUT_KEY_RIGHT){

        dirX = 1;
        dirY = 0;
    }

    if(key == GLUT_KEY_UP){

        dirX = 0;
        dirY = -1;
    }

    if(key == GLUT_KEY_DOWN){

        dirX = 0;
        dirY = 1;
    }
}

void normalKey(
    unsigned char key,  //glut keyboard callback func
    int,
    int){

    if(key == 13 &&   //13 is ASCII val of enter, 32 space, 27 esc
       inMenu){       // will check menu state

        inMenu = false;

        playBackgroundMusic();
    }
}
// ================= MOUSE =================

void mouse(
    int button,
    int state,
    int x,
    int y){

    if(button ==
       GLUT_LEFT_BUTTON &&

       state ==
       GLUT_DOWN){

        if((gameOver || gameWon) &&

           x >= WIDTH/3 &&
           x <= WIDTH/3 + 180 &&  //button boundary check

           y >= HEIGHT/2 + 80 &&
           y <= HEIGHT/2 + 130){

            restartGame();
        }
    }
}

// ================= MAIN =================

int main(
    int argc,
    char** argv){

    srand(time(0));

    cout <<
    "Enter Player Name: ";

    cin >> playerName;

    generateMaze();

    ghosts.push_back({
        10,10,1,0,0
    });

    maze[10][10] = -1;

    ghosts.push_back({
        15,15,0,1,1
    });

    maze[15][15] = -1;

    ghosts.push_back({
        5,15,1,0,1
    });

    /* adding more ghosts here
    ghosts.push_back({8,8,0,1,0});*/

    maze[5][15] = -1;

    glutInit(
        &argc,  //glut initialisation
        argv
    );

    glutInitDisplayMode(
        GLUT_DOUBLE |         //double buffering (avoid screen flickering)
        GLUT_RGB
    );

    glutInitWindowSize(
        WIDTH,
        HEIGHT
    );

    glutCreateWindow(
        "Pacman Ultimate"
    );

    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();

    gluOrtho2D(
        0,
        WIDTH,  // 2D coordinate setup
        HEIGHT,
        0
    );

    glutDisplayFunc(
        display
    );

    glutSpecialFunc(
        keyboard
    );

    glutKeyboardFunc(
        normalKey
    );




