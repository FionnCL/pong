#include "include/raylib.h"
#include <cstdio>
#include <math.h> 

const int screenWidth = 1280;
const int screenHeight = 720;
const Color screenColor = {5, 10, 5, 255};
const Color entityColor = {50, 245, 150, 255};

Sound paddleWAV;
Sound scoreWAV;
Sound wallWAV;

class Paddle {
    private:
        float posx, posy;

    public:
        Paddle(float posx, float posy){
            this->posx = posx;
            this->posy = posy;
        }

        void draw(float posy){ this->posy = posy; DrawRectangle(this->posx, posy, 25, 100, entityColor); }
        float getX(){ return this->posx; }
        float getY(){ return this->posy; }
};

int width = 25;
int height = 100;
float p1y = screenHeight/2 - height/2;
float p2y = screenHeight/2 - height/2;
float p1v = 0, p2v = 0;
int p1Score = 0;
int p2Score = 0;
Paddle p1(30, p1y);
Paddle p2(screenWidth - 55, p2y);

Vector2 ballpos = { (float)screenWidth/2, (float)screenHeight/2 };
int bRad = 10;
bool rightHeading = true;
float currYHeading = 0;
bool yCol1, yCol2, xCol1, xCol2, col = false;
float ballSpeedMultiplier = 0.25f;

bool waiting = true;
bool activeRound = false;

void resetRound(){
    rightHeading = !rightHeading; 
    ballpos = { (float)screenWidth/2, (float)screenHeight/2 };
    currYHeading = 0;
}

void collisionDetection(){
    // Collision Detection
    yCol1 = (ballpos.y + bRad >= p1.getY()) && (ballpos.y - bRad <= p1.getY() + height);
    yCol2 = (ballpos.y + bRad >= p2.getY()) && (ballpos.y - bRad <= p2.getY() + height);
    xCol1 = ballpos.x - bRad <= p1.getX() + width;
    xCol2 = ballpos.x + bRad >= p2.getX();
    col = (yCol1 && xCol1) || (yCol2 && xCol2);

    // Deals with ball velocities.
    if (col) {rightHeading = !rightHeading; PlaySound(paddleWAV); }
    if(yCol1 && xCol1) currYHeading += p1v * ballSpeedMultiplier;
    if(yCol2 && xCol2) currYHeading += p2v * ballSpeedMultiplier;
    ballpos.y += currYHeading;
    rightHeading ? ballpos.x += 2.0f : ballpos.x -= 2.0f;
}

void outOfBounds(){
    // Hits top/bottom of screen or player scores.
    if(ballpos.y + bRad >= screenHeight || ballpos.y - bRad <= 0) { currYHeading = -currYHeading; PlaySound(wallWAV); }
    if(ballpos.x >= screenWidth) { p1Score++; resetRound(); PlaySound(scoreWAV); }
    if(ballpos.x <= 0) { p2Score++; resetRound(); PlaySound(scoreWAV); }
}

void playerAndAIMovement(){
    // Key Bindings + ball y velocity.
    if(IsKeyDown(KEY_W) && (p2.getY() > 0)) p2v = -2.0f;
    else if(IsKeyDown(KEY_S) && (p2.getY() + height < screenHeight)) p2v = 2.0f;
    else if(IsKeyDown(KEY_ESCAPE)) { activeRound = false; }
    else p2v = 0.0f;
    p2y += p2v;

    // AI
    // Min functions used to get smoother movement.
    float paddleCentre = p1.getY() + 50;
    if(ballpos.y < paddleCentre){
        p1.getY() > 0 ? p1v = std::min(paddleCentre - ballpos.y, -2.0f) : p1v = 0.0f;
    } else {
        p1.getY() + 100 < screenHeight ? p1v = std::min(ballpos.y - paddleCentre, 2.0f) : p1v = 0.0f;
    }
    
    p1y += p1v;
}

void drawMenu(){
    if(IsKeyDown(KEY_X) || IsKeyDown(KEY_ESCAPE)) { waiting = false; }

    ClearBackground(screenColor);
    DrawText("Press X to Start!", screenWidth/2 - 50, screenHeight/2 - 8, 16, entityColor);
}

bool playOnce = true;
unsigned char transitionFunctionValue = 0;
int transitionFunctionInput = 1;
void drawTransition(){
    ClearBackground(screenColor);
    DrawRectangle(0 , 0, screenWidth, screenHeight, {255, 255, 255, transitionFunctionValue});

    transitionFunctionValue+=2;
    if(transitionFunctionValue == 0){ activeRound = true; }
}

// Vector2 leftx1y1 = { 255, 0 };
// Vector2 leftx2y2 = { 255, screenHeight};
// Vector2 rightx1y1 = { screenWidth - 255, 0 };
// Vector2 rightx2y2 = { screenWidth - 255, screenHeight };
void drawRound(){
    playerAndAIMovement();
    collisionDetection();
    outOfBounds();

    ClearBackground(screenColor);

    // DrawLineEx(rightx1y1, rightx2y2, 2, MAROON); 
    // DrawLineEx(leftx1y1, leftx2y2, 2, MAROON); 

    p1.draw(p1y);
    p2.draw(p2y);

    DrawCircleV(ballpos, bRad, entityColor);

    DrawText(TextFormat("%i | %i", p1Score, p2Score), screenWidth/2 - 16, 10, 16, entityColor);
}

const int monitorLineGap = 8;
const int monitorLineCount = screenHeight / monitorLineGap;
const int lineWidth = 2;
void drawMonitor(){
    float ys = 0;
    for(int i = 0; i <= monitorLineCount; i++){
        DrawLineEx({0, ys}, {screenWidth, ys}, lineWidth, {75, 210, 115, 10});
        ys += monitorLineGap;
    }
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Pong Frvr");
    SetTargetFPS(165);

    InitAudioDevice();

    // I feel like loading these here and declaring them globally is bad practice.
    paddleWAV = LoadSound("./resources/paddle.wav");
    scoreWAV = LoadSound("./resources/score.wav");
    wallWAV = LoadSound("./resources/wall.wav");

    while (!WindowShouldClose()) {
        BeginDrawing();
            if(waiting){ drawMenu(); }
            else if(activeRound){ drawRound(); }
            else { drawTransition(); }

            DrawText("ESC: Close Game", 10, 10, 8, entityColor);
            drawMonitor();

            // Illusion of monitor being convex.
            DrawCircleGradient(
                screenWidth/2, 
                screenHeight/2, 
                500, 
                {255, 255, 255, 5}, 
                {255, 255, 255, 0}
            );
            DrawCircleGradient(
                screenWidth/2, 
                screenHeight/2, 
                screenWidth, 
                {0, 0, 0, 0}, 
                {0, 0, 0, 200}
            );


        EndDrawing();
    }

    UnloadSound(paddleWAV);
    UnloadSound(scoreWAV);
    UnloadSound(wallWAV);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}