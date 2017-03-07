#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER };

using namespace std;

GLuint LoadTexture(const char *image_path, GLenum format) {
    SDL_Surface *surface = IMG_Load(image_path);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    SDL_FreeSurface(surface);
    
    return textureID;
}

void drText(int fontTexture, string text, float size, float spacing, float r, float g, float b, float a, float x, float y) {
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    glTranslatef(x, y, 0.0f);
    float texture_size = 1.0 / 16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    vector<float> colorData;
    for (unsigned int i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        
        colorData.insert(colorData.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
        vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) +
            (-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f
            * size });
        texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
            texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
    }
    glColorPointer(4, GL_FLOAT, 0, colorData.data());
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_QUADS, 0, text.size() * 4);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

class Bullet{
public:
    void draw();
    void update();
    float u;
    float v;
    float width;
    float height;
    float scale;
    
    float x;
    float y;
    float speed;
    
    GLuint textureID;
    
    bool visible;
    float timeAlive;
    
    Bullet(){};
    Bullet(GLuint textureID, float u, float v, float width, float height, float scale, float x, float y, bool visible)
    :textureID(textureID), u(u), v(v), width(width), height(height), scale(scale), x(x), y(y), speed(0.4f), visible(visible){};
};

class Entity{
public:
    void draw(float stretch = 1.0f);
    void render();
    
    GLuint textureID;
    
    float u;
    float v;
    float scale;
    
    float x;
    float y;
    
    float width;
    float height;
    
    float d_x;
    float d_y;
    
    float waittime;
    
    bool alive;
    bool shot;
    
    Bullet bullet;
    Entity(){};
    Entity(GLuint textureID, float u, float v, float width, float height, float scale)
    :textureID(textureID), u(u), v(v), width(width), height(height), scale(scale), alive(true), d_x(0.1f), d_y(0.04f), shot(false), waittime(-999.0f){};
};

void Bullet::draw(){
    if (visible){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glLoadIdentity();
        glTranslatef(x, y, 0.0f);
        
        GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
            width * scale, -height * scale, width * scale, height * scale };
        glVertexPointer(2, GL_FLOAT, 0, quad);
        glEnableClientState(GL_VERTEX_ARRAY);
        
        GLfloat quadUVs[] = { u, v, u, v + height, u + width, v + height, u + width, v };
        glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glDrawArrays(GL_QUADS, 0, 4);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

void Entity::draw(float stretch) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glLoadIdentity();
    glTranslatef(x, y, 0.0f);
    glScalef(stretch, 1.0f, 1.0f);
    
    GLfloat quad[] = { -width * scale, height * scale, -width * scale, -height * scale,
        width * scale, -height * scale, width * scale, height * scale };
    glVertexPointer(2, GL_FLOAT, 0, quad);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    GLfloat quadUVs[] = { u, v, u, v + height, u + width, v + height, u + width, v };
    glTexCoordPointer(2, GL_FLOAT, 0, quadUVs);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArrays(GL_QUADS, 0, 4);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void Entity::render(){
    if (alive == true){
        draw();
    }
}

class App{
public:
    App();
    ~App();
    
    void Init();
    bool UpdateAndRender();
    
    void Render();
    void Update(float elapsed);
private:
    const Uint8* keys;
    bool done;
    GameState state;
    float lastFrameTicks;
    SDL_Window* displayWindow;
    GLuint font;
    
    Entity enemy1[12];
    Entity enemy2[12];
    Entity enemy3[12];
    Entity enemy4[12];
    Entity livesicon[3];
    Entity barricade[4];
    int enemysize;
    
    void reset();
    
    void RenderMainMenu();
    void RenderGameLevel();
    void RenderGameOver();
    
    void UpdateGameLevel(float elapsed);
    
    bool bumprightwall();
    bool bumpleftwall();
    float delay;
    Entity user;
    
    bool userhit();
    bool eachuserhit(Entity enemy);
    float userhitdelay;
    bool enemyhit(Entity enemy);
    bool barricadehit(Entity barricade);
    bool eachbarricadehit(Entity& entity, Entity barricade);
    int score;
    int lives;
    bool bumpbarricade();
};

bool App::eachuserhit(Entity enemy){
    if (((enemy.bullet.x + enemy.bullet.width*enemy.bullet.scale<user.x - user.width*user.scale) || (enemy.bullet.x - enemy.bullet.width*enemy.bullet.scale>user.x + user.width*user.scale) ||
         (enemy.bullet.y + enemy.bullet.height*enemy.bullet.scale<user.y - user.height*user.scale) || (enemy.bullet.y - enemy.bullet.height*enemy.bullet.scale>user.y + user.height*user.scale)) &&
        ((enemy.bullet.x + enemy.bullet.width*enemy.bullet.scale<user.x - user.width*user.scale) || (enemy.bullet.x - enemy.bullet.width*enemy.bullet.scale>user.x + user.width*user.scale) ||
         (enemy.bullet.y + enemy.bullet.height*enemy.bullet.scale<user.y - user.height*user.scale) || (enemy.bullet.y - enemy.bullet.height*enemy.bullet.scale>user.y + user.height*user.scale))){
            return false;
        }
    return true;
}

bool App::eachbarricadehit(Entity& entity, Entity barricade){
    if (barricade.scale == 0){
        return false;
    }
    if (((entity.bullet.x + entity.bullet.width*entity.bullet.scale<barricade.x - barricade.width*barricade.scale*2.4f) || (entity.bullet.x - entity.bullet.width*entity.bullet.scale>barricade.x + barricade.width*barricade.scale*2.4f) ||
         (entity.bullet.y + entity.bullet.height*entity.bullet.scale<barricade.y - barricade.height*barricade.scale) || (entity.bullet.y - entity.bullet.height*entity.bullet.scale>barricade.y + barricade.height*barricade.scale)) &&
        ((entity.bullet.x + entity.bullet.width*entity.bullet.scale<barricade.x - barricade.width*barricade.scale*2.4f) || (entity.bullet.x - entity.bullet.width*entity.bullet.scale>barricade.x + barricade.width*barricade.scale*2.4f) ||
         (entity.bullet.y + entity.bullet.height*entity.bullet.scale<barricade.y - barricade.height*barricade.scale) || (entity.bullet.y - entity.bullet.height*entity.bullet.scale>barricade.y + barricade.height*barricade.scale))){
            return false;
        }
    if (entity.y == user.y){
        entity.bullet.y = 2.0f;
        return true;
    }
    entity.bullet.y = -2.0f;
    return true;
}

bool App::barricadehit(Entity barricade){
    bool hit = false;
    for (int i = 0; i < enemysize; i++){
        hit = hit || eachbarricadehit(enemy1[i], barricade) || eachbarricadehit(enemy2[i], barricade) ||
        eachbarricadehit(enemy3[i], barricade) || eachbarricadehit(enemy4[i], barricade);
        if (hit){
            return true;
        }
    }
    if (eachbarricadehit(user, barricade)){
        return true;
    }
    return false;
    
}

bool App::enemyhit(Entity enemy){
    if (((user.bullet.x + user.bullet.width*user.bullet.scale<enemy.x - enemy.width*enemy.scale) || (user.bullet.x - user.bullet.width*user.bullet.scale>enemy.x + enemy.width*enemy.scale) ||
         (user.bullet.y + user.bullet.height*user.bullet.scale<enemy.y - enemy.height*enemy.scale) || (user.bullet.y - user.bullet.height*user.bullet.scale>enemy.y + enemy.height*enemy.scale)) &&
        ((user.bullet.x + user.bullet.width*user.bullet.scale<enemy.x - enemy.width*enemy.scale) || (user.bullet.x - user.bullet.width*user.bullet.scale>enemy.x + enemy.width*enemy.scale) ||
         (user.bullet.y + user.bullet.height*user.bullet.scale<enemy.y - enemy.height*enemy.scale) || (user.bullet.y - user.bullet.height*user.bullet.scale>enemy.y + enemy.height*enemy.scale))){
            return false;
        }
    return true;
}

bool App::userhit(){
    bool hit = false;
    for (int i = 0; i < enemysize; i++){
        hit = hit || (eachuserhit(enemy1[i]) || eachuserhit(enemy2[i]) || eachuserhit(enemy3[i]) || eachuserhit(enemy4[i]));
        if (hit) {
            return true;
        }
    }
    return false;
}

bool App::bumpleftwall(){
    for (int i = 0; i < enemysize; i++){
        if (enemy1[i].alive || enemy2[i].alive || enemy3[i].alive || enemy4[i].alive){
            if (min(min(enemy1[i].x, enemy2[i].x), min(enemy3[i].x, enemy4[i].x)) - enemy2[i].width*enemy1[i].scale < -1.33f){
                return true;
            }
        }
    }
    return false;
}

bool App::bumprightwall(){
    for (int i = 0; i < enemysize; i++){
        if (enemy1[i].alive || enemy2[i].alive || enemy3[i].alive || enemy4[i].alive){
            if (max(max(enemy1[i].x, enemy2[i].x), max(enemy3[i].x, enemy4[i].x)) + enemy1[i].width*enemy1[i].scale > 1.33f){
                return true;
            }
        }
    }
    return false;
}

bool App::bumpbarricade(){
    for (int i = 0; i < enemysize; i++){
        if (enemy1[i].alive){
            if ((enemy1[i].y - enemy1[i].height*enemy1[i].scale) < -0.6f){
                return true;
            }
        }
        if (enemy2[i].alive){
            if ((enemy2[i].y - enemy2[i].height*enemy2[i].scale) < -0.6f){
                return true;
            }
        }
        if (enemy3[i].alive){
            if ((enemy3[i].y - enemy3[i].height*enemy3[i].scale) < -0.6f){
                return true;
            }
        }
        if (enemy4[i].alive){
            if ((enemy4[i].y - enemy4[i].height*enemy4[i].scale) < -0.6f){
                return true;
            }
        }
        
    }
    return false;
}

void App::reset(){
    score = 0;
    lives = 3;
    delay = 0.0f;
    
    GLuint SheetSpriteTexture = LoadTexture("sheet.png", GL_RGBA);
    fill_n(enemy1, enemysize, Entity(SheetSpriteTexture, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.7f));
    fill_n(enemy2, enemysize, Entity(SheetSpriteTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.7f));
    fill_n(enemy3, enemysize, Entity(SheetSpriteTexture, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.7f));
    fill_n(enemy4, enemysize, Entity(SheetSpriteTexture, 425.0f / 1024.0f, 384.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.7f));
    
    float xtemp = -1.15f;
    float ytemp = 0.6f;
    
    for (int i = 0; i < enemysize; i++){
        enemy1[i].x = xtemp;
        enemy1[i].y = ytemp;
        enemy1[i].bullet = Bullet(SheetSpriteTexture, 856.0f / 1024.0f, 812.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.6f, xtemp, ytemp, true);
        enemy2[i].x = xtemp;
        enemy2[i].y = ytemp - 0.18f;
        enemy2[i].bullet = Bullet(SheetSpriteTexture, 856.0f / 1024.0f, 812.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.6f, xtemp, ytemp - 0.18f, true);
        enemy3[i].x = xtemp;
        enemy3[i].y = ytemp - 0.36f;
        enemy3[i].bullet = Bullet(SheetSpriteTexture, 856.0f / 1024.0f, 812.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.6f, xtemp, ytemp - 0.36f, true);
        enemy4[i].x = xtemp;
        enemy4[i].y = ytemp - 0.54f;
        enemy4[i].bullet = Bullet(SheetSpriteTexture, 856.0f / 1024.0f, 812.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.6f, xtemp, ytemp - 0.54f, true);
        xtemp += 0.18f;
    }
    user = Entity(SheetSpriteTexture, 777.0f / 1024.0f, 712.0f / 1024.0f, 32.0f / 1024.0f, 26.0 / 1024.0f, 1.7f);
    for (int i = 0; i < lives; i++){
        livesicon[i] = Entity(SheetSpriteTexture, 777.0f / 1024.0f, 712.0f / 1024.0f, 32.0f / 1024.0f, 26.0 / 1024.0f, 1.1f);
        livesicon[i].x = -0.79f + i*0.09f;
        livesicon[i].y = 0.8f;
    }
    user.x = 0.0f;
    user.y = -0.8f;
    user.bullet = Bullet(SheetSpriteTexture, 855.0f / 1024.0f, 173.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.6f, user.x, user.y, false);
    
    xtemp = 0.3325f - 1.33f;
    for (int i = 0; i < 4; i++){
        barricade[i] = Entity(SheetSpriteTexture, 224.0f / 1024.0f, 748.0f / 1024.0f, 101.0f / 1024.0f, 84.0 / 1024.0f, 1.0f);
        barricade[i].x = xtemp + i*0.665f;
        barricade[i].y = user.y + 0.15f;
    }
}

App::App() {
    Init();
    
    keys = SDL_GetKeyboardState(NULL);
    
    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-1.33, 1.33, -1.0, 1.0, -1.0, 1.0);
    
    state = STATE_MAIN_MENU;
    
    done = false;
    lastFrameTicks = 0.0f;
    enemysize = 12;
    
    font = LoadTexture("letters.png", GL_RGBA);
    
    glMatrixMode(GL_MODELVIEW);
    
}

void App::Init() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
}

App::~App() {
    SDL_Quit();
}

void App::Render() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    glClear(GL_COLOR_BUFFER_BIT);
    switch (state) {
        case STATE_MAIN_MENU:
            RenderMainMenu();
            break;
        case STATE_GAME_LEVEL:
            RenderGameLevel();
            break;
        case STATE_GAME_OVER:
            RenderGameOver();
            break;
    }
    SDL_GL_SwapWindow(displayWindow);
}

void App::RenderMainMenu(){
    drText(font, "SPACE INVADERS", 0.3f, -0.15f, 1.0f, 1.0f, 1.0f, 1.0f, -0.97f, 0.3f);
    drText(font, "Press SPACE to play!", 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -0.93f, -0.6f);
    drText(font, "Use the left and right arrow keys to move.", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, -0.95f, 0.0f);
    drText(font, "Use the spacebar to shoot.", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, -0.95f, -0.1f);
    drText(font, "If you run out of lives, or the ships reach", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, -0.95f, -0.2f);
    drText(font, "your barricade, the game is over. :(", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, -0.95f, -0.3f);
}
void App::RenderGameLevel(){
    drText(font, "Score", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, 0.65f, 0.9f);
    drText(font, "Lives", 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, -0.8f, 0.9f);
    for (int i = 0; i < lives; i++){
        livesicon[i].draw();
    }
    drText(font, to_string(score), 0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, 0.65f, 0.8f);
    
    for (int i = 0; i < enemysize; i++){
        enemy1[i].render();
        enemy1[i].bullet.draw();
        enemy2[i].render();
        enemy2[i].bullet.draw();
        enemy3[i].render();
        enemy3[i].bullet.draw();
        enemy4[i].render();
        enemy4[i].bullet.draw();
    }
    
    user.render();
    user.bullet.draw();
    
    for (int i = 0; i < 4; i++){
        barricade[i].draw(2.4f);
    }
    
}
void App::RenderGameOver(){
    drText(font, "Your final score:", 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -0.8f, 0.45f);
    drText(font, to_string(score), 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -0.1f, 0.2f);
    
    drText(font, "Press SPACE to play again!", 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -1.25f, -0.2f);
    drText(font, "Press ENTER to go to the", 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -1.15f, -0.4f);
    drText(font, "main menu!", 0.2f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f, -0.45f, -0.55f);
}


void App::Update(float elapsed) {
    
    
    if (state == STATE_GAME_LEVEL){
        UpdateGameLevel(elapsed);
    }
    
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && state == STATE_GAME_LEVEL){
                if (user.bullet.visible == false){
                    user.bullet.x = user.x;
                    user.bullet.y = user.y;
                    user.bullet.visible = true;
                }
            }
            if (state == STATE_GAME_OVER){
                if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                    state = STATE_MAIN_MENU;
                }
                else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    state = STATE_GAME_LEVEL;
                    reset();
                }
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && state == STATE_MAIN_MENU){
                state = STATE_GAME_LEVEL;
                reset();
            }
            
            
        }
    }
}

void App::UpdateGameLevel(float elapsed){
    if (lives == 0 || score == 1200 || bumpbarricade()){
        state = STATE_GAME_OVER;
        return;
    }
    
    for (int i = 0; i < 4; i++){
        if (barricadehit(barricade[i])){
            if ((barricade[i].scale) < 0.0f){
                barricade[i].scale = 0.0f;
            }
            else {
                barricade[i].scale -= 0.04f;
            }
        }
    }
    
    delay -= elapsed;
    
    if (user.bullet.visible == true) {
        user.bullet.y += elapsed * 2.0f;
    }
    
    if (user.bullet.y > 1.2f) {
        user.bullet.visible = false;
    }
    
    if (userhit()){
        for (int i = 0; i < enemysize; i++){
            enemy1[i].bullet.y = -2.0f;
            enemy1[i].bullet.x = enemy1[i].x;
            enemy1[i].shot = false;
            enemy2[i].bullet.y = -2.0f;
            enemy2[i].bullet.x = enemy2[i].x;
            enemy2[i].shot = false;
            enemy3[i].bullet.y = -2.0f;
            enemy3[i].bullet.x = enemy3[i].x;
            enemy3[i].shot = false;
            enemy4[i].bullet.y = -2.0f;
            enemy4[i].bullet.x = enemy4[i].x;
            enemy4[i].shot = false;
            
        }
        user.x = 0.0f;
        lives--;
    }
    
    if (delay < 0.0f) {
        for (int i = 0; i < enemysize; i++){
            
            if (bumprightwall() || bumpleftwall()){
                enemy1[i].d_x = -(enemy1[i].d_x)*1.15f;
                enemy2[i].d_x = -(enemy2[i].d_x)*1.15f;
                enemy3[i].d_x = -(enemy3[i].d_x)*1.15f;
                enemy4[i].d_x = -(enemy4[i].d_x)*1.15f;
                enemy1[i].y -= enemy1[i].d_y;
                enemy2[i].y -= enemy2[i].d_y;
                enemy3[i].y -= enemy3[i].d_y;
                enemy4[i].y -= enemy4[i].d_y;
                if (!enemy1[i].shot && enemy1[i].alive){
                    enemy1[i].bullet.y = enemy1[i].y;
                }
                if (!enemy2[i].shot && enemy2[i].alive){
                    enemy2[i].bullet.y = enemy2[i].y;
                }
                if (!enemy3[i].shot && enemy3[i].alive){
                    enemy3[i].bullet.y = enemy3[i].y;
                }
                if (!enemy4[i].shot && enemy4[i].alive){
                    enemy4[i].bullet.y = enemy4[i].y;
                }
            }
        }
        delay = 0.05f;
    }
    
    for (int i = 0; i < enemysize; i++){
        if (enemyhit(enemy1[i]) && enemy1[i].alive){
            score += 40;
            enemy1[i].x = 0.0f;
            enemy1[i].alive = false;
            if (!enemy1[i].shot){
                enemy1[i].bullet.visible = false;
            }
            user.bullet.y = 10.0f;
            user.bullet.visible = false;
            break;
        }
        else if (enemyhit(enemy2[i]) && enemy2[i].alive){
            score += 30;
            enemy2[i].x = 0.0f;
            enemy2[i].alive = false;
            if (!enemy2[i].shot){
                enemy2[i].bullet.visible = false;
            }
            user.bullet.y = 10.0f;
            user.bullet.visible = false;
            break;
        }
        else if (enemyhit(enemy3[i]) && enemy3[i].alive){
            score += 20;
            enemy3[i].x = 0.0f;
            enemy3[i].alive = false;
            if (!enemy3[i].shot){
                enemy3[i].bullet.visible = false;
            }
            user.bullet.y = 10.0f;
            user.bullet.visible = false;
            break;
        }
        else if (enemyhit(enemy4[i]) && enemy4[i].alive){
            score += 10;
            enemy4[i].x = 0.0f;
            enemy4[i].alive = false;
            if (!enemy4[i].shot){
                enemy4[i].bullet.visible = false;
            }
            user.bullet.y = 10.0f;
            user.bullet.visible = false;
            break;
        }
    }
    
    for (int i = 0; i < enemysize; i++){
        
        if (!enemy1[i].alive && enemy1[i].bullet.y < -2.0f){}
        else {
            if (enemy1[i].shot){
                enemy1[i].bullet.y -= enemy1[i].bullet.speed*elapsed;
            }
            else if (enemy1[i].alive) {
                enemy1[i].bullet.x += enemy1[i].d_x*elapsed;
            }
            else if (enemy1[i].alive && !enemy1[i].shot){
                enemy1[i].bullet.y = -2.0f;
            }
        }
        
        if (!enemy2[i].alive && enemy2[i].bullet.y < -2.0f){}
        else {
            if (enemy2[i].shot){
                enemy2[i].bullet.y -= enemy2[i].bullet.speed*elapsed;
            }
            else if (enemy2[i].alive){
                enemy2[i].bullet.x += enemy2[i].d_x*elapsed;
            }
            else if (enemy2[i].alive && !enemy2[i].shot){
                enemy2[i].bullet.y = -2.0f;
            }
        }
        
        if (!enemy3[i].alive && enemy3[i].bullet.y < -2.0f){}
        else {
            if (enemy3[i].shot){
                enemy3[i].bullet.y -= enemy3[i].bullet.speed*elapsed;
            }
            else if (enemy3[i].alive){
                enemy3[i].bullet.x += enemy3[i].d_x*elapsed;
            }
            else if (enemy3[i].alive && !enemy3[i].shot){
                enemy3[i].bullet.y = -2.0f;
            }
        }
        
        if (!enemy1[i].alive && enemy1[i].bullet.y < -2.0f){}
        else {
            if (enemy4[i].shot){
                enemy4[i].bullet.y -= enemy4[i].bullet.speed*elapsed;
            }
            else if (enemy4[i].alive){
                enemy4[i].bullet.x += enemy4[i].d_x*elapsed;
            }
            else if (enemy4[i].alive && !enemy4[i].shot){
                enemy4[i].bullet.y = -2.0f;
            }
        }
        if (enemy1[i].alive){
            enemy1[i].waittime -= elapsed;
            enemy1[i].x += enemy1[i].d_x*elapsed;
            if (enemy1[i].waittime < -900.0f) {
                enemy1[i].waittime = rand() % 12 + 1.0f;
            }
            else if (enemy1[i].waittime < 0.0f){
                enemy1[i].shot = true;
                enemy1[i].waittime = rand() % 12 + 4.0f;
            }
            if (enemy1[i].bullet.y < -1.5f) {
                enemy1[i].bullet.x = enemy1[i].x;
                enemy1[i].bullet.y = enemy1[i].y;
                enemy1[i].shot = false;
            }
        }
        if (enemy2[i].alive){
            enemy2[i].waittime -= elapsed;
            enemy2[i].x += enemy2[i].d_x*elapsed;
            if (enemy2[i].waittime < -900.0f) {
                enemy2[i].waittime = rand() % 12 + 1.0f;
            }
            else if (enemy2[i].waittime < 0.0f){
                enemy2[i].shot = true;
                enemy2[i].waittime = rand() % 12 + 4.0f;
            }
            if (enemy2[i].bullet.y < -1.5f) {
                enemy2[i].bullet.x = enemy2[i].x;
                enemy2[i].bullet.y = enemy2[i].y;
                enemy2[i].shot = false;
            }
        }
        if (enemy3[i].alive){
            enemy3[i].waittime -= elapsed;
            enemy3[i].x += enemy3[i].d_x*elapsed;
            if (enemy3[i].waittime < -900.0f) {
                enemy3[i].waittime = rand() % 12 + 1.0f;
            }
            else if (enemy3[i].waittime < 0.0f){
                enemy3[i].shot = true;
                enemy3[i].waittime = rand() % 12 + 4.0f;
            }
            if (enemy3[i].bullet.y < -1.5f) {
                enemy3[i].bullet.x = enemy3[i].x;
                enemy3[i].bullet.y = enemy3[i].y;
                enemy3[i].shot = false;
            }
        }
        if (enemy4[i].alive){
            enemy4[i].waittime -= elapsed;
            
            enemy4[i].x += enemy4[i].d_x*elapsed;
            if (enemy4[i].waittime < -900.0f) {
                enemy4[i].waittime = rand() % 12 + 1.0f;
            }
            else if (enemy4[i].waittime < 0.0f){
                enemy4[i].shot = true;
                enemy4[i].waittime = rand() % 12 + 4.0f;
            }
            if (enemy4[i].bullet.y < -1.5f) {
                enemy4[i].bullet.x = enemy4[i].x;
                enemy4[i].bullet.y = enemy4[i].y;
                enemy4[i].shot = false;
            }
        }
    }
    if (keys[SDL_SCANCODE_LEFT]){
        if (user.x - user.width*user.scale > -1.33){
            user.x -= 0.75f * elapsed;
        }
    }
    else if (keys[SDL_SCANCODE_RIGHT]){
        if (user.x + user.width*user.scale < 1.33){
            user.x += 0.75f * elapsed;
        }
    }
}

bool App::UpdateAndRender() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    Update(elapsed);
    Render();
    return done;
}

SDL_Window* displayWindow; 
