#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.5f
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define LEVEL_HEIGHT 24
#define LEVEL_WIDTH 70

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

unsigned char** levelData;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL)
    {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        //assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

class SheetSprite
{
public:
    SheetSprite() {};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size): textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
    void Draw(ShaderProgram *program)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        // draw our arrays
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    
};

float lerp(float v0, float v1, float t)
{
    return (1.0-t)*v0 + t*v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(-worldY / TILE_SIZE);
}

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_COIN};

class Entity
{
public:
    void Render(ShaderProgram *program)
    {
        Matrix modelMatrix;
        modelMatrix.Translate(x, y, 0);
        program->setModelMatrix(modelMatrix);
        sprite.Draw(program);
    }
    bool collidesWith(Entity entity);
    void Update(float elapsed)
    {
        collidedTop = false;
        collidedBottom = false;
        collidedLeft = false;
        collidedRight = false;
        velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
        velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);
        velocity_x += acceleration_x * elapsed;
        velocity_y += -10 * elapsed;
        x += velocity_x * elapsed;
        y += velocity_y * elapsed;
    }
    
    SheetSprite sprite;
    
    float x;
    float y;
    float width;
    float height;
    float velocity_x;
    float velocity_y;
    float acceleration_x;
    float acceleration_y;
    float friction_x;
    float friction_y;
    
    bool isStatic;
    EntityType entitytype;
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
};


bool readHeader(std::ifstream &stream, int& mapWidth, int& mapHeight, unsigned char**& levelData)
{
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while(getline(stream, line))
    {
        if(line == "")
        {
            break;
        }
        
        istringstream sStream(line);
        string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if(key == "width")
        {
            mapWidth = atoi(value.c_str());
        }
        else if(key == "height")
        {
            mapHeight = atoi(value.c_str());
        }
    }
    if(mapWidth == -1 || mapHeight == -1)
    {
        return false;
    }
    else
    {//allocate our map data
        levelData = new unsigned char*[mapHeight];
        for(int i = 0; i < mapHeight; ++i)
        {
            levelData[i] = new unsigned char[mapWidth];
        }
        return true;
    }
}

bool readLayerData(std::ifstream &stream, int& mapWidth, int& mapHeight, unsigned char**& levelData)
{
    string line;
    while(getline(stream, line))
    {
        if(line == "")
        {
            break;
        }
        istringstream sStream(line);
        string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data")
        {
            for(int y = 0; y < mapHeight; y++)
            {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                
                for(int x = 0; x < mapWidth; x++)
                {
                    getline(lineStream, tile, ',');
                    unsigned char val = (unsigned char)atoi(tile.c_str());
                    if(val > 0)
                    {
                        levelData[y][x] = val - 1;
                    }
                    else
                    {
                        levelData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}

Entity player;
Entity enemy;

void placeEntity(string type, float& placeX, float& placeY)
{
    if(type == "Start")
    {
        player.entitytype = ENTITY_PLAYER;
        player.x = placeX;
        player.y = placeY;
        player.isStatic = false;
    }
    else if(type == "Enemy")
    {
        enemy.entitytype = ENTITY_ENEMY;
        enemy.x = placeX;
        enemy.y = placeY;
        enemy.isStatic = false;
    }
}

bool readEntityData(std::ifstream &stream)
{
    string line;
    string type;
    
    while(getline(stream, line))
    {
        if(line == "")
        {
            break;
        }
        istringstream sStream(line);
        string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if(key == "type")
        {
            type = value;
        }
        else if(key == "location")
        {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            
            float placeX = atoi(xPosition.c_str()) * TILE_SIZE;
            float placeY = atoi(yPosition.c_str()) * -TILE_SIZE;
            
            placeEntity(type, placeX, placeY);
        }
    }
    return true;
}


/*void drawMap(ShaderProgram *program, GLuint tilesTexture) {
    vector<float> vertexData;
    vector<float> texCoordData;
    int count = 0;
    for (int y = 0; y < LEVEL_HEIGHT; y++)
    {
        for (int x = 0; x < LEVEL_WIDTH; x++)
        {
            if (levelData[y][x] != 0)
            {
                float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
                float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
                float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
                count += 1;
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v + (spriteHeight),
                    u + spriteWidth, v + (spriteHeight),
                    u, v,
                    u + spriteWidth, v + (spriteHeight),
                    u + spriteWidth, v
                });
            }
        }
    }
    
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, tilesTexture);
    glDrawArrays(GL_TRIANGLES, 0, count * 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}*/

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    ShaderProgram program = ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    GLuint tileTexture = LoadTexture("tiles1.png");
    SheetSprite tileSprite (tileTexture, 48.0f/256.0f, 48.0f/128.0f, 16.0f/256.0f, 16.0f/128.0f, 0.5f);
    
    //drawMap(&program, tileTexture);
    
    SDL_Event event;
    bool done = false;
    
    int mapWidth = 0;
    int mapHeight = 0;
    
    ifstream infile("myMap1.txt");
    string line;
    while (getline(infile, line))
    {
        if(line == "[header]")
        {
            readHeader(infile, mapWidth, mapHeight, levelData);
        }
        else if(line == "[layer]")
        {
            readLayerData(infile, mapWidth, mapHeight, levelData);
        }
        else if(line == "characters")
        {
            readEntityData(infile);
        }
    }
    
    float lastFrameTicks = 0.0f;
    while (!done) //Game loop
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
            else if (keys[SDL_SCANCODE_LEFT])
            {
                player.acceleration_x -= 0.1f;
            }
            else if (keys[SDL_SCANCODE_RIGHT])
            {
                player.acceleration_x += 0.1f;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        float fixedElapsed = elapsed;
        if(fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
        {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP )
        {
            fixedElapsed -= FIXED_TIMESTEP;
        }
        player.Update(FIXED_TIMESTEP);

    }
    
    SDL_Quit();
    return 0;
}
