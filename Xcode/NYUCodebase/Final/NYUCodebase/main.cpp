//Vighneshan Moorthy

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "SheetSprite.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

SDL_Window* displayWindow;

class Entity
{
public:
    Entity () {};
    Entity(SheetSprite s, float x, float y) : sprite(s), x(x), y(y)
    {
        size = sprite.size;
    }
    float x;
    float y;
    float size;
    SheetSprite sprite;
};

GLuint LoadTexture(const char *filePath) {
    
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

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

bool checkCollision (Entity& a, Entity& b)
{
    if ((a.y - a.size/2) > (b.y + b.size/2))
    {
        return false;
    }
    if ((a.y + a.size/2) < (b.y - b.size/2))
    {
        return false;
    }
    if ((a.x - a.size/2) > (b.x + b.size/2))
    {
        return false;
    }
    if ((a.x + a.size/2) < (b.x - b.size/2))
    {
        return false;
    }
    return true;
}
int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Asteroid Belts", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    SDL_Event event;
    bool done = false;
    
    GLuint fonts = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint sheetSpriteTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
//level 1 vectors
    vector<Entity> meteorsLeft;
    vector<Entity> meteorsTop;
    vector<Entity> meteorsRight;
    vector<Entity> meteorsBottom;
//level 2 vectors
    vector<Entity> meteorsLeft2;
    vector<Entity> meteorsTop2;
    vector<Entity> meteorsRight2;
    vector<Entity> meteorsBottom2;
//level 3 vectors
    vector<Entity> meteorsLeft3;
    vector<Entity> meteorsTop3;
    vector<Entity> meteorsRight3;
    vector<Entity> meteorsBottom3;
    
    enum{START_SCREEN, LEVEL1_SCREEN, LEVEL2_SCREEN, LEVEL3_SCREEN, DEATH_SCREEN, LEVEL1WIN_SCREEN, LEVEL2WIN_SCREEN, LEVEL3WIN_SCREEN};
    int state = START_SCREEN;
    bool left = true;
    
    srand(time(0));
    
//from left
    int randInt1 = (rand() % 5) - 8;
    int randInt2 = (rand() % 5) - 8;
    int randInt3 = (rand() % 5) - 8;
    
//from top
    //x values
    int randInt4 = (rand() % 7) - 3;
    int randInt5 = (rand() % 7) - 3;
    int randInt6 = (rand() % 7) - 3;
    //y values
    int randInt7 = (rand() % 3) + 3;
    int randInt8 = (rand() % 3) + 3;
    int randInt9 = (rand() % 3) + 3;
    
//from right
    int randInt10 = (rand() % 6) + 3;
    int randInt11 = (rand() % 6) + 3;
    int randInt12 = (rand() % 6) + 3;
    
//from bottom
    //x values
    int randInt13 = (rand() % 7) - 3;
    int randInt14 = (rand() % 7) - 3;
    int randInt15 = (rand() % 7) - 3;
    //y values
    int randInt16 = (rand() % 3) - 5;
    int randInt17= (rand() % 3) - 5;
    int randInt18= (rand() % 3) - 5;
    
    Matrix projectionMatrix;
    Matrix viewMatrix;
    Matrix modelMatrix;
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    float lastFrameTicks = 0.0f;
    
    glUseProgram(program.programID);
    
    
//PLAYER AND ENEMIES
    SheetSprite player = SheetSprite(sheetSpriteTexture, 112.0f/1024.0f, 791.0f/1024.0f, 112.0f/1024.0f, 75.0f/1024.0f, 0.3f);
    Entity thePlayer(player, 0, 0);
    
    SheetSprite enemy1(sheetSpriteTexture, 224.0 / 1024.0, 496.0 / 1024.0, 103.0 / 1024.0, 84.0 / 1024.0, 0.3);
    Entity theEnemy1(enemy1, 0, 1.0);
    
    SheetSprite enemy2(sheetSpriteTexture, 224.0 / 1024.0, 832.0 / 1024.0, 99.0/ 1024.0, 75.0 / 1024.0, 0.3);
    Entity theEnemy2(enemy2, 0, -1.0);
    
    
//LEVEL 1 METEORS
//Meteors from the left
    SheetSprite meteor1 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor1(meteor1, randInt1, 1.5);
    
    SheetSprite meteor2 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor2(meteor2, randInt2, 0);
    
    SheetSprite meteor3 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor3(meteor3, randInt3, -1.5);
    
//Meteors from the top
    SheetSprite meteor4 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor4(meteor4, randInt4, randInt7);
    
    SheetSprite meteor5 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor5(meteor5, randInt5, randInt8);
    
    SheetSprite meteor6 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor6(meteor6, randInt6, randInt9);
    
//Meteors from the right
    SheetSprite meteor7 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor7(meteor7, randInt10, 1.5);
    
    SheetSprite meteor8 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor8(meteor8, randInt11, 0);
    
    SheetSprite meteor9 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor9(meteor9, randInt12, -1.5);
    
//Meteors from the Bottom
    SheetSprite meteor10 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor10(meteor10, randInt13, randInt16);
    
    SheetSprite meteor11 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor11(meteor11, randInt14, randInt17);
    
    SheetSprite meteor12 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor12(meteor12, randInt15, randInt18);
    
    
//Meteors from the left
    meteorsLeft.push_back(theMeteor1);
    meteorsLeft.push_back(theMeteor2);
    meteorsLeft.push_back(theMeteor3);
//Meteors from the top
    meteorsTop.push_back(theMeteor4);
    meteorsTop.push_back(theMeteor5);
    meteorsTop.push_back(theMeteor6);
//Meteors from the Right
    meteorsRight.push_back(theMeteor7);
    meteorsRight.push_back(theMeteor8);
    meteorsRight.push_back(theMeteor9);
//Meteors from the Bottom
    meteorsBottom.push_back(theMeteor10);
    meteorsBottom.push_back(theMeteor11);
    meteorsBottom.push_back(theMeteor12);
    
//LEVEL 2 METEORS
    //Meteors from the left
    SheetSprite meteor13 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor13(meteor13, randInt1, 1.5);
    
    SheetSprite meteor14 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor14(meteor14, randInt3, 0);
    
    SheetSprite meteor15 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor15(meteor15, randInt2, -1.5);
    
    //Meteors from the top
    SheetSprite meteor16 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor16(meteor16, randInt4, randInt7);
    
    SheetSprite meteor17 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor17(meteor17, randInt6, randInt8);
    
    SheetSprite meteor18 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor18(meteor18, randInt5, randInt9);
    
    //Meteors from the right
    SheetSprite meteor19 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor19(meteor19, randInt10, 1.5);
    
    SheetSprite meteor20 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor20(meteor20, randInt12, 0);
    
    SheetSprite meteor21 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor21(meteor21, randInt11, -1.5);
    
    //Meteors from the Bottom
    SheetSprite meteor22 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor22(meteor22, randInt13, randInt16);
    
    SheetSprite meteor23 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor23(meteor23, randInt15, randInt17);
    
    SheetSprite meteor24 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor24(meteor24, randInt14, randInt18);
    
    
    //Meteors from the left
    meteorsLeft2.push_back(theMeteor13);
    meteorsLeft2.push_back(theMeteor14);
    meteorsLeft2.push_back(theMeteor15);
    //Meteors from the top
    meteorsTop2.push_back(theMeteor16);
    meteorsTop2.push_back(theMeteor17);
    meteorsTop2.push_back(theMeteor18);
    //Meteors from the Right
    meteorsRight2.push_back(theMeteor19);
    meteorsRight2.push_back(theMeteor20);
    meteorsRight2.push_back(theMeteor21);
    //Meteors from the Bottom
    meteorsBottom2.push_back(theMeteor22);
    meteorsBottom2.push_back(theMeteor23);
    meteorsBottom2.push_back(theMeteor24);
    
//LEVEL 3 METEORS
    //Meteors from the left
    SheetSprite meteor25 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor25(meteor25, randInt2, 1.5);
    
    SheetSprite meteor26 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor26(meteor26, randInt1, 0);
    
    SheetSprite meteor27 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor27(meteor27, randInt3, -1.5);
    
    //Meteors from the top
    SheetSprite meteor28 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor28(meteor28, randInt5, randInt7);
    
    SheetSprite meteor29 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor29(meteor29, randInt4, randInt8);
    
    SheetSprite meteor30 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor30(meteor30, randInt6, randInt9);
    
    //Meteors from the right
    SheetSprite meteor31 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor31(meteor31, randInt11, 1.5);
    
    SheetSprite meteor32 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor32(meteor32, randInt10, 0);
    
    SheetSprite meteor33 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor33(meteor33, randInt12, -1.5);
    
    //Meteors from the Bottom
    SheetSprite meteor34 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor34(meteor34, randInt14, randInt16);
    
    SheetSprite meteor35 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor35(meteor35, randInt13, randInt17);
    
    SheetSprite meteor36 = SheetSprite(sheetSpriteTexture, 224.0f/1024.0f, 664.0f/1024.0f, 101.0f/1024.0f, 84.0f/1024.0f, 0.5f);
    Entity theMeteor36(meteor36, randInt15, randInt18);
    
    
    //Meteors from the left
    meteorsLeft3.push_back(theMeteor25);
    meteorsLeft3.push_back(theMeteor26);
    meteorsLeft3.push_back(theMeteor27);
    //Meteors from the top
    meteorsTop3.push_back(theMeteor28);
    meteorsTop3.push_back(theMeteor29);
    meteorsTop3.push_back(theMeteor30);
    //Meteors from the Right
    meteorsRight3.push_back(theMeteor31);
    meteorsRight3.push_back(theMeteor32);
    meteorsRight3.push_back(theMeteor33);
    //Meteors from the Bottom
    meteorsBottom3.push_back(theMeteor34);
    meteorsBottom3.push_back(theMeteor35);
    meteorsBottom3.push_back(theMeteor36);
    
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    Mix_Chunk *firstSound;
    firstSound = Mix_LoadWAV("first_sound.wav");
    Mix_Chunk *secondSound;
    secondSound = Mix_LoadWAV("second_sound.wav");
    Mix_Music *music;
    music = Mix_LoadMUS("music.mp3");
    
    
    

    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        int count = 0;
        int count2 = 0;
        int count3 = 0;
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, sheetSpriteTexture);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        switch(state)
        {
            case START_SCREEN:
                Mix_PlayMusic(music, -1);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.3, 1.5, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Welcome to Avoid the Meteors!", .5f, -.26f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 0, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Press arrow keys to move", .5f, -.27f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-3.3, -0.75, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Press spacebar to quit anytime", .5f, -.27f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, -1.5, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Press enter to start", .5f, -.27f);
                
                if (keys[SDL_SCANCODE_RETURN])
                {
                    lastFrameTicks = (float)SDL_GetTicks() / 1000.0f;
                    state = LEVEL1_SCREEN;
                }
                break;
                
            case DEATH_SCREEN:
                Mix_PlayMusic(music, -1);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 1, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "You died", .6f, -.26f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, -0.33, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Press Enter to close", .5f, -.27f);
                
                if (keys[SDL_SCANCODE_RETURN])
                {
                    done = true;
                }
                break;
                
            case LEVEL1WIN_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                Mix_PlayMusic(music, -1);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 1, 0);
                program.setModelMatrix(modelMatrix);
            
                DrawText(&program, fonts, "You completed level 1!", .5f, -.26f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, -0.33, 0);
                program.setModelMatrix(modelMatrix);
                
                DrawText(&program, fonts, "Press Enter to move on", .5f, -.27f);
                
                if (keys[SDL_SCANCODE_RETURN])
                {
                    state = LEVEL2_SCREEN;
                }
                break;
                
            case LEVEL2WIN_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                Mix_PlayMusic(music, -1);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 1, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "You completed level 2!", .5f, -.26f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, -0.33, 0);
                program.setModelMatrix(modelMatrix);
                
                DrawText(&program, fonts, "Press Enter to move on", .5f, -.27f);
                
                if (keys[SDL_SCANCODE_RETURN])
                {
                    state = LEVEL3_SCREEN;
                }
                break;
                
            case LEVEL3WIN_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                Mix_PlayMusic(music, -1);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, 1, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "You completed level 3!", .5f, -.26f);
                
                modelMatrix.identity();
                modelMatrix.Translate(-2.5, -0.33, 0);
                program.setModelMatrix(modelMatrix);
                DrawText(&program, fonts, "Press Enter to close", .5f, -.27f);
        
                if (keys[SDL_SCANCODE_RETURN])
                {
                    done = true;
                }
                break;
//LEVEL 1 CODE
            case LEVEL1_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                program.setViewMatrix(viewMatrix);
                program.setProjectionMatrix(projectionMatrix);
                
                // player movement
                //
                if(keys[SDL_SCANCODE_LEFT] && thePlayer.x > -3.0f)
                {
                    thePlayer.x -= 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_RIGHT] && thePlayer.x < 3.0f)
                {
                    thePlayer.x += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_UP] && thePlayer.y < 1.5f)
                {
                    thePlayer.y += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_DOWN] && thePlayer.y > -1.5f)
                {
                    thePlayer.y -= 1.8 * elapsed;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(thePlayer.x, thePlayer.y, 0.0f);
                program.setModelMatrix(modelMatrix);
                player.Draw(&program);
            
                //Meteor movement
                
                for(int i = 0; i < meteorsLeft.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsLeft[i].x += 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsLeft[i].x, meteorsLeft[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsLeft[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsLeft[i]) && state == LEVEL1_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsLeft[i].x > 5.0f)
                    {
                        count++;
                    }
                }
                for(int i = 0; i < meteorsTop.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsTop[i].y -= 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsTop[i].x, meteorsTop[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsTop[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsTop[i]) && state == LEVEL1_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsTop[i].y < -6.0)
                    {
                        count++;
                    }
                }
                for(int i = 0; i < meteorsRight.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsRight[i].x -= 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsRight[i].x, meteorsRight[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsRight[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsRight[i]) && state == LEVEL1_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsRight[i].x < -5.0f)
                    {
                        count++;
                    }
                }
                for(int i = 0; i < meteorsBottom.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsBottom[i].y += 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsBottom[i].x, meteorsBottom[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsBottom[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsBottom[i]) && state == LEVEL1_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsBottom[i].y > 6.0)
                    {
                        count++;
                    }
                }
                if (count == 12 && state == LEVEL1_SCREEN)
                {
                    thePlayer.x = 0;
                    thePlayer.y = 0;
                    state = LEVEL1WIN_SCREEN;
                }
                break;
//LEVEL 2 CODE
            case LEVEL2_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                program.setViewMatrix(viewMatrix);
                program.setProjectionMatrix(projectionMatrix);
                
                // player movement
                if(keys[SDL_SCANCODE_LEFT] && thePlayer.x > -3.0f)
                {
                    thePlayer.x -= 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_RIGHT] && thePlayer.x < 3.0f)
                {
                    thePlayer.x += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_UP] && thePlayer.y < 1.5f)
                {
                    thePlayer.y += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_DOWN] && thePlayer.y > -1.5f)
                {
                    thePlayer.y -= 1.8 * elapsed;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(thePlayer.x, thePlayer.y, 0.0f);
                program.setModelMatrix(modelMatrix);
                player.Draw(&program);
                
                //Meteor movement
                
                for(int i = 0; i < meteorsLeft2.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsLeft2[i].x += 1.6 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsLeft2[i].x, meteorsLeft2[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsLeft2[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsLeft2[i]) && state == LEVEL2_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsLeft2[i].x > 5.0f)
                    {
                        count2++;
                    }
                }
                for(int i = 0; i < meteorsTop2.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsTop2[i].y -= 1.6 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsTop2[i].x, meteorsTop2[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsTop2[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsTop2[i]) && state == LEVEL2_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsTop2[i].y < -6.0)
                    {
                        count2++;
                    }
                }
                for(int i = 0; i < meteorsRight2.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsRight2[i].x -= 1.6 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsRight2[i].x, meteorsRight2[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsRight2[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsRight2[i]) && state == LEVEL2_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsRight2[i].x < -5.0f)
                    {
                        count2++;
                    }
                }
                for(int i = 0; i < meteorsBottom2.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsBottom2[i].y += 1.6 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsBottom2[i].x, meteorsBottom2[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsBottom2[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsBottom2[i]) && state == LEVEL2_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsBottom2[i].y > 6.0)
                    {
                        count2++;
                    }
                }
                if (count2 == 12 && state == LEVEL2_SCREEN)
                {
                    thePlayer.x = 0;
                    thePlayer.y = 0;
                    state = LEVEL2WIN_SCREEN;
                }
                break;
                
//LEVEL 3 CODE
            case LEVEL3_SCREEN:
                if (keys[SDL_SCANCODE_SPACE])
                {
                    done = true;
                }
                program.setViewMatrix(viewMatrix);
                program.setProjectionMatrix(projectionMatrix);
                
                // player movement
                if(keys[SDL_SCANCODE_LEFT] && thePlayer.x > -3.0f)
                {
                    thePlayer.x -= 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_RIGHT] && thePlayer.x < 3.0f)
                {
                    thePlayer.x += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_UP] && thePlayer.y < 1.5f)
                {
                    thePlayer.y += 1.8 * elapsed;
                }
                else if(keys[SDL_SCANCODE_DOWN] && thePlayer.y > -1.5f)
                {
                    thePlayer.y -= 1.8 * elapsed;
                }
                
                modelMatrix.identity();
                modelMatrix.Translate(thePlayer.x, thePlayer.y, 0.0f);
                program.setModelMatrix(modelMatrix);
                player.Draw(&program);
                
                //Enemy movement
                program.setViewMatrix(viewMatrix);
                program.setProjectionMatrix(projectionMatrix);
                
                if(left == true)
                {
                    theEnemy1.x -= 2.0 * elapsed;
                    if(theEnemy1.x < -2.8)
                    {
                        left = false;
                    }
                }
                else if(left == false)
                {
                    theEnemy1.x += 2.0 * elapsed;
                    if(theEnemy1.x > 2.8)
                    {
                        left = true;
                    }
                }
                modelMatrix.identity();
                modelMatrix.Translate(theEnemy1.x, theEnemy1.y, 0.0f);
                program.setModelMatrix(modelMatrix);
                theEnemy1.sprite.Draw(&program);
                
                program.setViewMatrix(viewMatrix);
                program.setProjectionMatrix(projectionMatrix);
                
                if(left == true)
                {
                    theEnemy2.x -= 2.0 * elapsed;
                    if(theEnemy2.x < -2.8)
                    {
                        left = false;
                    }
                }
                else if(left == false)
                {
                    theEnemy2.x += 2.0 * elapsed;
                    if(theEnemy2.x > 2.8)
                    {
                        left = true;
                    }
                }
                modelMatrix.identity();
                modelMatrix.Translate(theEnemy2.x, theEnemy2.y, 0.0f);
                program.setModelMatrix(modelMatrix);
                theEnemy2.sprite.Draw(&program);
                
                if(thePlayer.y > 0.9 && thePlayer.x > theEnemy1.x && left == false)
                {
                    Mix_PlayChannel(-1, firstSound, 0);
                    theEnemy1.x += 2.3 * elapsed;
                }
                if(thePlayer.y > 0.9 && thePlayer.x < theEnemy1.x && left == true)
                {
                    Mix_PlayChannel(-1, firstSound, 0);
                    theEnemy1.x -= 2.3 * elapsed;
                }
                
                if(thePlayer.y < -0.9 && thePlayer.x > theEnemy2.x && left == false)
                {
                    Mix_PlayChannel(-1, firstSound, 0);
                    theEnemy2.x += 2.3 * elapsed;
                }
                if(thePlayer.y < -0.9 && thePlayer.x < theEnemy2.x && left == true)
                {
                    Mix_PlayChannel(-1, firstSound, 0);
                    theEnemy2.x -= 2.3 * elapsed;
                }
                
                if(checkCollision(thePlayer, theEnemy1) || checkCollision(thePlayer, theEnemy2))
                {
                    Mix_PlayChannel(-1, secondSound, 0);
                    state = DEATH_SCREEN;
                }
                
                //Meteor movement
                
                for(int i = 0; i < meteorsLeft3.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsLeft3[i].x += 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsLeft3[i].x, meteorsLeft3[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsLeft3[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsLeft3[i]) && state == LEVEL3_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsLeft3[i].x > 5.0f)
                    {
                        count3++;
                    }
                }
                for(int i = 0; i < meteorsTop3.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsTop3[i].y -= 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsTop3[i].x, meteorsTop3[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsTop3[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsTop3[i]) && state == LEVEL3_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsTop3[i].y < -6.0)
                    {
                        count3++;
                    }
                }
                for(int i = 0; i < meteorsRight3.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsRight3[i].x -= 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsRight3[i].x, meteorsRight3[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsRight3[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsRight3[i]) && state == LEVEL3_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsRight3[i].x < -5.0f)
                    {
                        count3++;
                    }
                }
                for(int i = 0; i < meteorsBottom3.size(); i++)
                {
                    program.setViewMatrix(viewMatrix);
                    program.setProjectionMatrix(projectionMatrix);
                    
                    meteorsBottom3[i].y += 0.9 * elapsed;
                    modelMatrix.identity();
                    modelMatrix.Translate(meteorsBottom3[i].x, meteorsBottom3[i].y, 0);
                    modelMatrix.Rotate(360 * lastFrameTicks * 3.14159265 / 180);
                    program.setModelMatrix(modelMatrix);
                    
                    meteorsBottom3[i].sprite.Draw(&program);
                    
                    if(checkCollision(thePlayer, meteorsBottom3[i]) && state == LEVEL3_SCREEN)
                    {
                        Mix_PlayChannel(-1, secondSound, 0);
                        state = DEATH_SCREEN;
                    }
                    if(meteorsBottom3[i].y > 6.0)
                    {
                        count3++;
                    }
                }
                if (count3 == 12 && state == LEVEL3_SCREEN)
                {
                    state = LEVEL3WIN_SCREEN;
                }
                break;
        }
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
