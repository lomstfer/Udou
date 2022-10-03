#include <raylib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <fstream>

#define WINW 160
#define WINH 90
#define SCRW 1920
#define SCRH 1080
#define SCALE float(SCRW)/WINW

#define Log(x) std::cout << x << std::endl;

#define BATTERY_GREEN {47, 224, 29, 255}
#define BATTERY_YELLOW {230, 185, 23, 255}
#define BATTERY_RED {230, 64, 23, 255}

float clamp(float num, float min, float max)
{
    if (num < min)
        return min;
    if (num > max)
        return max;
    return num;
}

int main() 
{
    InitWindow(SCRW, SCRH, "");
    InitAudioDevice();
    SetWindowState(FLAG_FULLSCREEN_MODE);
    HideCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    Image icon = LoadImage("assets/icon.png");
    SetWindowIcon(icon);

    Texture2D cursorTexture = LoadTexture("assets/cursor.png");

    Font font = LoadFont("assets/Silkscreen-Regular.ttf");

    Texture2D alphaTexture = LoadRenderTexture(WINW, WINH).texture;
    Image alphaTextureImg = LoadImageFromTexture(alphaTexture);

    RenderTexture2D renderTarg = LoadRenderTexture(SCRW, SCRH);

    Sound flickSound = LoadSound("assets/flick.wav");
    Sound udouAngrySound = LoadSound("assets/udou_angry.wav");
    Sound udouWatSound = LoadSound("assets/udou_wat.wav");
    Sound udouDieSound = LoadSound("assets/udou_die.wav");
    Sound bzzSound = LoadSound("assets/bzz.wav");
    Sound clickSound = LoadSound("assets/click.wav");

    bool showControls = false;

    Vector2 mousePos;

    int highscore = 0;
    std::ifstream dataFileIn("data.udou");
    std::string dataFileText;
    std::getline(dataFileIn, dataFileText);
    if (dataFileText.length() > 0)
        highscore = std::stoi(dataFileText);

    std::ofstream dataFileOut("data.udou");

    enum gameStates {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER,
    };

    int gameState = MENU;

    Texture2D _texture = LoadTexture("assets/udou.png");
    Texture2D treeTextures[3] = {LoadTexture("assets/tree0.png"), LoadTexture("assets/tree1.png"), LoadTexture("assets/tree2.png")};


    RESTART:
    bool died = false;

    Vector2 _position = {WINW/2, WINH/2};
    Vector2 _velocity;
    int _maxVelocity = 10;
    float _rotation = 0;

    Color batteryColor = BATTERY_GREEN;

    const int numOfTrees = 130;
    int treesX[numOfTrees];
    int treesY[numOfTrees];
    int treesTree[numOfTrees];
    for (int i = 0; i < numOfTrees; i++) 
    {
        treesX[i] = rand() % WINW;
        treesY[i] = rand() % WINH;
        treesTree[i] = rand() % 3;
    }

    const int foodsSize = 10;
    int foodsX[foodsSize];
    int foodsY[foodsSize];
    for (int i = 0; i < foodsSize; i++) 
    {
        foodsX[i] = rand() % (WINW-50) + 25;
        foodsY[i] = rand() % (WINH-30) + 15;
    }

    const int enmsSize = 5;
    float enmsX[enmsSize];
    float enmsY[enmsSize];
    for (int i = 0; i < enmsSize; i++) 
    {
        enmsX[i] = rand() % WINW;
        enmsY[i] = rand() % WINH;
    }

    bool lightOn = true;
    bool prevlightOn;
    float lightSize = 1.f;
    float timer = 10;
    float timerSet = timer;

    float battery = 10.9f;

    float dt;
    bool gameOn = true;
while (gameOn)
{
dt = GetFrameTime();
if (WindowShouldClose())
    gameOn = false;

mousePos = GetMousePosition();

mousePos.x /= SCALE;
mousePos.x -= 0.5f;
mousePos.y /= SCALE;

float distMouse = sqrt(pow(mousePos.x - _position.x, 2) + pow(mousePos.y - _position.y, 2));
float distMouseX = mousePos.x - _position.x;
float distMouseY = mousePos.y - _position.y;

prevlightOn = lightOn;

switch (gameState)
{
case PLAYING:

if ((int)battery > highscore) {
    highscore = battery;
}

if (IsKeyPressed(KEY_ESCAPE)) {
    gameState = PAUSED;
    PlaySound(clickSound);
}

if (distMouse > 10) {
    SetMousePosition((_position.x + (distMouseX * 0.9f)) * SCALE, (_position.y + (distMouseY * 0.9f)) * SCALE);
}

_rotation = atan2(distMouseY, distMouseX) / PI * 180 + 90;

_velocity.x = (mousePos.x - _position.x);
_velocity.y = (mousePos.y - _position.y);

if (_velocity.x > _maxVelocity)
    _velocity.x = _maxVelocity;
if (_velocity.x < -_maxVelocity)
    _velocity.x = -_maxVelocity;
if (_velocity.y > _maxVelocity)
    _velocity.y = _maxVelocity;
if (_velocity.y < -_maxVelocity)
    _velocity.y = -_maxVelocity;

_position.x += _velocity.x * dt * 1.f;
_position.y += _velocity.y * dt * 1.f;

if (lightOn)
    timer -= dt;
if (timer <= 0) {
    timer = timerSet;
    lightOn = false;
}

if ((int)battery >= 1 && lightOn)
    battery -= dt / 2.f;

if (IsKeyPressed(KEY_SPACE) && (int)battery >= 1) {
    lightOn = true;
}

if ((int)battery <= 0) {
    lightOn = false;
    battery = 0;
    if (prevlightOn)
        PlaySound(udouAngrySound);
}

if (lightOn)
    batteryColor = BATTERY_GREEN;
if ((int)battery <= 0)
    batteryColor = BATTERY_RED;
else if (!lightOn)
    batteryColor = BATTERY_YELLOW;

// the light (matph + very slow cpu rendering)
if (lightOn) {
    lightSize += dt * 10.f;
}
if (!lightOn) {
    lightSize -= dt;
}
lightSize = clamp(lightSize, 0.f,1.f);
for (int x = 0; x < WINW; x++)
{
    for (int y = 0; y < WINH; y++)
    {
        float d = sqrt(pow(float(x) - _position.x, 2) + pow(float(y) - _position.y, 2));
        if (d < 32.f * lightSize)
            ImageDrawPixel(&alphaTextureImg, x, y, {5,5,5,(unsigned char)(clamp(d*10.f+1000.f*(1.f - lightSize),0,255-lightSize*60))});
        else
            ImageDrawPixel(&alphaTextureImg, x, y, {5,5,5,(unsigned char)(int)(255-lightSize*60)});
    }
}

if (prevlightOn != lightOn) {
    PlaySound(flickSound);
    if (!lightOn && rand() % 2 == 0 && (int)battery > 0)
        PlaySound(udouWatSound);
}
    

UnloadTexture(alphaTexture);
alphaTexture = LoadTextureFromImage(alphaTextureImg);

BeginTextureMode(renderTarg);
    ClearBackground({42, 71, 39, 255});

    DrawTexturePro(_texture, {0,0,(float)_texture.width,(float)_texture.height}, {_position.x*SCALE, _position.y*SCALE, _texture.width*SCALE, _texture.height*SCALE}, {_texture.width/2*SCALE, _texture.height/2*SCALE}, _rotation, WHITE);
    
    for (int i = 0; i < numOfTrees; i++)
    {
        DrawTextureEx(treeTextures[treesTree[i]], {treesX[i]*SCALE, treesY[i]*SCALE}, 0, SCALE, WHITE);
    }
    for (int i = 0; i < foodsSize; i++)
    {
        //DrawRectangle(foodsX[i]*SCALE, foodsY[i]*SCALE, 1*SCALE, 1*SCALE, GREEN);
        DrawTextEx(font, "+", {foodsX[i]*SCALE, foodsY[i]*SCALE}, 37, 0, BATTERY_GREEN);
        if (foodsX[i] < _position.x + _texture.width/2 &&
            foodsX[i] + 1 > _position.x - _texture.width/2 && 
            foodsY[i] < _position.y + _texture.height/2 &&
            foodsY[i] + 1 > _position.y - _texture.height/2) {
                foodsX[i] = rand() % (WINW-50) + 25;
                foodsY[i] = rand() % (WINH-30) + 15;
                battery += 1;
                PlaySound(bzzSound);
        }
    }
    for (int i = 0; i < enmsSize; i++)
    {
        DrawRectangle(enmsX[i]*SCALE, enmsY[i]*SCALE, 2*SCALE, 2*SCALE, {99, 94, 90, 255});
        
        float distP = sqrt(pow(enmsX[i] - _position.x, 2) + pow(enmsY[i] - _position.y, 2));
        if (distP < 16) {
            enmsX[i] += (_position.x - enmsX[i]) * pow(5.f/(distP),2) * dt;
            enmsY[i] += (_position.y - enmsY[i]) * pow(5.f/(distP),2) * dt;
        }

        if (distP < 5) {
                enmsX[i] = rand() % WINW;
                enmsY[i] = rand() % WINH;
                battery = 0;
                died = true;
                gameState = MENU;
                PlaySound(udouDieSound);
        }
    }

    // draw alpha texture on top
    DrawTexturePro(alphaTexture, {0,0,(float)alphaTexture.width,(float)alphaTexture.height}, {0,0,SCRW,SCRH}, {0,0}, 0, WHITE);
    if (!lightOn)
        DrawTexturePro(_texture, {0,0,(float)_texture.width,(float)_texture.height}, {_position.x*SCALE, _position.y*SCALE, _texture.width*SCALE, _texture.height*SCALE}, {_texture.width/2*SCALE, _texture.height/2*SCALE}, _rotation, {60,60,60,255});

EndTextureMode();

BeginDrawing();
    DrawTexturePro(renderTarg.texture, {0,0,(float)renderTarg.texture.width,(float)-renderTarg.texture.height}, {0,0,SCRW,SCRH}, {0,0}, 0, {255,255,255,255});
    DrawTextEx(font, std::to_string(int(battery)).c_str(), {_position.x+100, _position.y}, 50, 0, batteryColor);
    DrawTextEx(font, "||||||||||||||||||||||||||||||||||||||||||                 :  ", {_position.x+100 - 50, _position.y}, 50, -15, batteryColor);
    DrawRectangle(GetMousePosition().x, GetMousePosition().y, SCALE, SCALE, {100,100,100,100});
EndDrawing();
    break;


case PAUSED:

if (IsKeyPressed(KEY_ESCAPE)) {
    gameState = MENU;
    PlaySound(clickSound);
}
    
if (IsKeyPressed(KEY_SPACE)) {
    gameState = PLAYING;
    PlaySound(clickSound);
}

BeginDrawing();
    DrawTexturePro(renderTarg.texture, {0,0,(float)renderTarg.texture.width,(float)-renderTarg.texture.height}, {0,0,SCRW,SCRH}, {0,0}, 0, {100,100,100,255});
    DrawTextEx(font, "PAUSED", {100 - 50, _position.y}, 100, 0, batteryColor);
    DrawTextEx(font, "[SPACE] CONTINUE", {100, _position.y + 150}, 50, 0, batteryColor);
    DrawTextEx(font, "[ESCAPE] EXIT TO MENU", {100, _position.y + 100}, 50, 0, batteryColor);
    DrawTextEx(font, ("HIGHSCORE: " + std::to_string(highscore)).c_str(), {100, _position.y + 300}, 50, 0, batteryColor);
EndDrawing();
    break;

case MENU:
if (IsKeyPressed(KEY_ESCAPE)) {
    gameOn = false;
    PlaySound(clickSound);
}
if (IsKeyPressed(KEY_SPACE)) {
    gameState = PLAYING;
    PlaySound(clickSound);
    goto RESTART;
    
}
if (IsKeyPressed(KEY_C)) {
    PlaySound(clickSound);
    showControls = !showControls;
}

BeginDrawing();
    ClearBackground({0,0,0,0});
    DrawTexturePro(renderTarg.texture, {0,0,(float)renderTarg.texture.width,(float)-renderTarg.texture.height}, {0,0,SCRW,SCRH}, {0,0}, 0, {100,100,100,255});
    DrawTextEx(font, "MENU", {100 - 50, _position.y}, 100, 0, BATTERY_GREEN);
    DrawTextEx(font, "[SPACE] START", {100, _position.y + 100}, 50, 0, BATTERY_GREEN);    
    DrawTextEx(font, "[C] SHOW CONTROLS", {100, _position.y + 150}, 50, 0, BATTERY_GREEN);
    DrawTextEx(font, "[ESCAPE] EXIT GAME", {100, _position.y + 200}, 50, 0, BATTERY_GREEN);
    DrawTextEx(font, ("HIGHSCORE: " + std::to_string(highscore)).c_str(), {100, _position.y + 300}, 50, 0, BATTERY_GREEN);

    if (showControls) {
        DrawTextEx(font, "Move with the mouse", {100, _position.y + SCRH-300}, 40, 0, {199, 199, 199, 255});
        DrawTextEx(font, "Flick your light on with [SPACE]", {100, _position.y + SCRH-250}, 40, 0, {199, 199, 199, 255});
        DrawTextEx(font, "(if you have enough energy)", {102, _position.y + SCRH-200}, 30, 0, {199, 199, 199, 255});
    }
    

    if (died)
        DrawTextEx(font, "YOu dieD", {clamp(_position.x * SCALE - 200, 500, SCRW - 500), clamp(_position.y * SCALE, 350, SCRH - 350)}, 100, 0, BATTERY_RED);
EndDrawing();
    break;

default:
    break;
}

}
    UnloadImage(alphaTextureImg);
    dataFileOut << (int)highscore;
    dataFileOut.close();
    dataFileIn.close();
    CloseAudioDevice();
    CloseWindow();
}