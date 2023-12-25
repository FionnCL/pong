#include "include/raylib.h"
#include <cstdio>
#include <math.h> 

class Paddle {
    private:
        float posx, posy;

    public:
        Paddle(float posx, float posy){
            this->posx = posx;
            this->posy = posy;
        }

        void draw(float posy){ this->posy = posy; DrawRectangle(this->posx, posy, 25, 100, WHITE); }
        float getX(){ return this->posx; }
        float getY(){ return this->posy; }
};

const int screenWidth = 1280;
const int screenHeight = 720;

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
    if (col) rightHeading = !rightHeading;
    if(yCol1 && xCol1) currYHeading += p1v * ballSpeedMultiplier;
    if(yCol2 && xCol2) currYHeading += p2v * ballSpeedMultiplier;
    ballpos.y += currYHeading;
    rightHeading ? ballpos.x += 2.0f : ballpos.x -= 2.0f;
}

void outOfBounds(){
    // Hits top/bottom of screen or player scores.
    if(ballpos.y + bRad >= screenHeight || ballpos.y - bRad <= 0) currYHeading = -currYHeading;
    if(ballpos.x >= screenWidth) { p1Score++; resetRound(); }
    if(ballpos.x <= 0) { p2Score++; resetRound(); }
}

void playerAndAIMovement(){
    // Key Bindings + ball y velocity.
    if(IsKeyDown(KEY_W) && (p2.getY() > 0)) p2v = -2.0f;
    else if(IsKeyDown(KEY_S) && (p2.getY() + height < screenHeight)) p2v = 2.0f;
    else if(IsKeyDown(KEY_ESCAPE)) { activeRound = false; }
    else p2v = 0.0f;
    p2y += p2v;

    // AI
    ballpos.y < p1.getY() + 50 ? p1v = -2.0f : p1v = 2.0f;
    p1y += p1v;
}

void drawMenu(){
    if(IsKeyDown(KEY_X) || IsKeyDown(KEY_ESCAPE)) { waiting = false; }

    BeginDrawing();

        ClearBackground(BLACK);
        DrawText("Press X to Start!", screenWidth/2 - 50, screenHeight/2 - 8, 16, WHITE);

    EndDrawing();
}

Sound transitionSound = LoadSound("./resources/bass.wav");
bool playOnce = true;
unsigned char transitionFunctionValue = 0;
int transitionFunctionInput = 1;
void drawTransition(){
    PlaySound(transitionSound);

    BeginDrawing();

        ClearBackground(BLACK);
        DrawRectangle(0 , 0, screenWidth, screenHeight, {255, 255, 255, transitionFunctionValue});
        printf("%i\n", transitionFunctionValue);

    EndDrawing();

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

    BeginDrawing();

        ClearBackground(BLACK);

        // DrawLineEx(rightx1y1, rightx2y2, 2, MAROON); 
        // DrawLineEx(leftx1y1, leftx2y2, 2, MAROON); 

        p1.draw(p1y);
        p2.draw(p2y);

        DrawCircleV(ballpos, bRad, WHITE);

        DrawText(TextFormat("%i | %i", p1Score, p2Score), screenWidth/2 - 16, 10, 16, WHITE);

    EndDrawing();
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Pong Frvr");
    SetTargetFPS(165);

    InitAudioDevice();
    Music roundMusic = LoadMusicStream("./resources/round-background.wav");
    PlayMusicStream(roundMusic);

    while (!WindowShouldClose()) {
        if(waiting){ drawMenu(); }
        else if(activeRound){ drawRound();  UpdateMusicStream(roundMusic);}
        else { drawTransition(); }

    }

    UnloadMusicStream(roundMusic);
    UnloadSound(transitionSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}