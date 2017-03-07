//Vighneshan Moorthy
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include "ShaderProgram.h"
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "vector"
using namespace std;
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path){
    SDL_Surface *surface = IMG_Load(image_path);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
    return textureID;
}

class SheetSprite
{
public:
    SheetSprite(ShaderProgram* program,unsigned int textureID, float u, float v, float width, float height, float size):program(program),textureID(textureID),u(u),v(v),width(width),height(height),size(size) {}
    void Draw();
    ShaderProgram* program;
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};

void SheetSprite::Draw()
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] =
    {
        u, v + height,
        u + width, v,
        u, v,
        u + width, v,
        u, v + height,
        u + width, v + height
    };
    float aspect = width / height;
    float vertices[] =
    {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, -0.5f * size
    };
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

class Entity
{
public:
    Entity(float x, float y, SheetSprite textureID) :x(x), y(y), textureID(textureID){}
    void Draw(){ textureID.Draw(); }
    float x;
    float y;
    float rotation;
    SheetSprite textureID;
    bool dead = false;
    bool collide(Entity entity)
    {
        if (!((y  - .1) >= (entity.y + .1) || (y+ .1) <= (entity.y - .1) || (x - .1) >= (entity.x + .1) || (x + .1) <= (entity.x - .1)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for (int i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(),{
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    float speed = 0.1;
    int enemyNum = 24;
    int done = 0;
    float lastFrameTicks = 0.0f;
    float playerX = 0;
    int canShoot = 1;
    int enemyCanShoot = 1;
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    GLuint spriteSheet = LoadTexture("sheet.png");
    GLuint letters = LoadTexture("letters.png");
    
    SheetSprite playerSprite(&program, spriteSheet, 211.0 / 1024.0, 941.0 / 1024.0, 99.0 / 1024.0, 75.0 / 1024.0, 0.4);
    SheetSprite playerbullet(&program, spriteSheet, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2);
    SheetSprite enemybullet(&program, spriteSheet, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2);
    SheetSprite enemyRow1(&program, spriteSheet, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
    SheetSprite enemyRow2(&program, spriteSheet, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.25);
    SheetSprite enemyRow3(&program, spriteSheet, 144.0f / 1024.0f, 156.0f / 1024.0f, 103.0f / 1024.0f, 84.0f / 1024.0f, 0.25);


    Entity playerBullet(50, 50, playerbullet);
    Entity enemyBullet(-50, -50, enemybullet);
    
    vector<Entity> enemies;
    for (int i = 0; i < 24; i++)
    {
        SheetSprite enemySprite = enemyRow1;
        if (i > 7 && i < 16){ enemySprite = enemyRow2; }
        else if (i > 15){ enemySprite = enemyRow3; }
        Entity enemy(-3.2 + ((i % 8) / 2.0f), 1.8 - (i / 8) / 3.0f, enemySprite);
        enemies.push_back(enemy);
    }
    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    enum{START_SCREEN, GAME_SCREEN};
    
    int state = START_SCREEN;
    while (done == 0) {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = 1;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        switch (state)
        {
            case START_SCREEN:
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 1, 0);
                program.setModelMatrix(modelMatrix);
                
                glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
                glEnableVertexAttribArray(program.positionAttribute);
                
                glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
                glEnableVertexAttribArray(program.texCoordAttribute);
                DrawText(&program, letters, "Space Invaders", .6f, -.26f);
                modelMatrix.identity();
                
                modelMatrix.Translate(-2.5, -0.33, 0);
                program.setModelMatrix(modelMatrix);
                
                DrawText(&program, letters, "Press Enter to start", .5f, -.27f);
                
                glDisableVertexAttribArray(program.positionAttribute);
                glDisableVertexAttribArray(program.texCoordAttribute);
                if (keys[SDL_SCANCODE_RETURN])
                {
                    lastFrameTicks = (float)SDL_GetTicks() / 1000.0f;
                    state = GAME_SCREEN;
                }
                break;
                
            case GAME_SCREEN:
                float ticks = (float)SDL_GetTicks() / 1000.0f ;
                float elapsed = ticks - lastFrameTicks;
                lastFrameTicks = ticks;
                
                if (keys[SDL_SCANCODE_RIGHT])
                {
                    if (playerX < 3.3)
                    {
                        playerX += elapsed * 1.2;
                    }
                }
                if (keys[SDL_SCANCODE_LEFT])
                {
                    if (playerX > -3.3)
                    {
                        playerX -= elapsed * 1.2;
                    }
                }
                if (keys[SDL_SCANCODE_SPACE])
                {
                    if (canShoot == 1)
                    {
                        canShoot = 0;
                        playerBullet.x = playerX;
                        playerBullet.y = -1.8;
                    }
                    
                }
                modelMatrix.identity();
                modelMatrix.Translate(playerX, -1.8, 0);
                program.setModelMatrix(modelMatrix);
                playerSprite.Draw();
                
                for (int j = 0; j < 24; j++)
                {
                    if (!enemies[j].dead)
                    {
                        modelMatrix.identity();
                        enemies[j].x += elapsed * speed;
                        if (enemies[j].x > 3.3)
                        {
                            speed *= -1;
                            for (int k = 0; k < enemies.size(); k++)
                            {
                                enemies[k].y -= .2;
                            }
                        }
                        else if (enemies[j].x < -3.3)
                        {
                            speed *= -1;
                            for (int k = 0; k < enemies.size(); k++)
                            {
                                enemies[k].y -= .2;
                            }
                        }
                        if (!((enemies[j].y - .1) >= (-1.8 + .1) || (enemies[j].y + .1) <= (-1.8 - .1) || (enemies[j].x - .1) >= (playerX + .1) || (enemies[j].x + .1) <= (playerX - .1)))
                        {
                            done = 1;
                            break;
                        }
                        if (rand() % 50 == 1 && enemyCanShoot== 1)
                        {
                            enemyCanShoot = 0;
                            enemyBullet.x = enemies[j].x;
                            enemyBullet.y = enemies[j].y;
                        }
                        modelMatrix.Translate(enemies[j].x, enemies[j].y, 0);
                        program.setModelMatrix(modelMatrix);
                        enemies[j].Draw();
                    }
                }
        
                playerBullet.y += elapsed * 2.7;
                if (playerBullet.y > 2)
                {
                    canShoot = 1;
                }
                for (int j = 0; j < enemies.size(); j++)
                {
                    if (playerBullet.collide(enemies[j]) && enemies[j].dead == false)
                    {
                        playerBullet.x = 50;
                        playerBullet.y = 50;
                        enemies[j].dead = true;
                        enemyNum--;
                        if (speed > 0){ speed += 0.02; }
                        else{ speed -= 0.02; }
                        canShoot = 1;
                    }
                }

                if (enemyNum == 0)
                {
                    done = 1;
                    break;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(playerBullet.x, playerBullet.y , 0);
                program.setModelMatrix(modelMatrix);
                playerBullet.Draw();
                
                enemyBullet.y -= elapsed * 2.7;
                if (enemyBullet.y < -2)
                {
                    enemyBullet.y =  -50;
                    enemyCanShoot = 1;
                }
                if (!((enemyBullet.y - .1) >= (-1.8 + .1) || (enemyBullet.y + .1) <= (-1.8 - .1) || (enemyBullet.x - .1) >= (playerX + .1) || (enemyBullet.x + .1) <= (playerX - .1)))
                {
                    done = 1;
                    break;
                }
                modelMatrix.identity();
                modelMatrix.Translate(enemyBullet.x, enemyBullet.y, 0);
                program.setModelMatrix(modelMatrix);
                enemyBullet.Draw();

                break;
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
