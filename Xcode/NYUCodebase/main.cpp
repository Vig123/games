#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char* filePath)
{
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        std::cout << "Unable to load image.\n";
        assert(false);
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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint ballTexture = LoadTexture(RESOURCE_FOLDER"ball.jpg");
    GLuint bar1Texture = LoadTexture(RESOURCE_FOLDER"bar1.jpg");
    GLuint bar2Texture = LoadTexture(RESOURCE_FOLDER"bar2.jpg");
    
    float ballx = 0.0f;
    float bally = 0.0f;
    float bar1 = 0.0f;
    float bar2 = 0.0f;
    int dir_y = 1;
    int dir_x = 1;

    float bar1Top;
    float bar1Bottom;
    float bar1Left;
    float bar1Right;
    
    float bar2Top;
    float bar2Bottom;
    float bar2Left;
    float bar2Right;

    
    Matrix projectionMatrix;
    Matrix viewMatrix;
    Matrix modelMatrix;
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    float lastFrameTicks = 0.0f;
    
    glUseProgram(program.programID);
    
    
    SDL_Event event;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    bool done = false;
    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                //Controls for left paddle
                if (keys[SDL_SCANCODE_W])
                {
                    if (bar1 <= 1.7)
                    {
                        bar1 += 0.1;
                    }
                }
                if (keys[SDL_SCANCODE_S])
                {
                    if (bar1 >= -1.7)
                    {
                        bar1 += -0.1;
                    }
                }
                if (keys[SDL_SCANCODE_UP])
                {
                    if (bar2 <= 1.7)
                    {
                        bar2 += 0.1;
                    }
                }
                if (keys[SDL_SCANCODE_DOWN])
                {
                    if (bar2 >= -1.7)
                    {
                        bar2 += -0.1;
                    }
                }
            }
        }
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
    
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        
        // Code for left paddle
        glBindTexture(GL_TEXTURE_2D, bar1Texture);
        
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        
        modelMatrix.identity();
        modelMatrix.Translate(-3.5f, 0.0f, 0.0f);
        program.setModelMatrix(modelMatrix);
        
        float bar1Vertices[] = {-0.1f, -0.3f+bar1, 0.1f, -0.3f+bar1, 0.1f, 0.3f+bar1, -0.1f, -0.3f+bar1, 0.1f, 0.3f+bar1, -0.1f,0.3f+bar1};
        
        bar1Top += 0.3f + bar1;
        bar1Bottom += -0.7f + bar1;
        bar1Left = -0.1;
        bar1Right = 1;
        
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bar1Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float bar1TexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, bar1TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        // Code for right paddle
        glBindTexture(GL_TEXTURE_2D, bar2Texture);
        
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        
        modelMatrix.identity();
        modelMatrix.Translate(3.5f, 0.0f, 0.0f);
        program.setModelMatrix(modelMatrix);
        
        float bar2Vertices[] = {-0.1f, -0.3f+bar2, 0.1f, -0.3f+bar2, 0.1f, 0.3f+bar2, -0.1f, -0.3f+bar2, 0.1f, 0.3f+bar2, -0.1f,0.3f+bar2};
        
        bar2Top += 0.3 + bar2;
        bar2Bottom += -0.7f + bar2;
        bar2Left = -0.1;
        bar2Right = 1;
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bar2Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float bar2TexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, bar2TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //ball movement
        if(dir_y == 1)
        {
            bally += 2.0f * elapsed;
            if(bally > 6.2f)
            {
                dir_y = 0;
            }
        }
        else if(dir_y == 0)
        {
            bally -= 2.0f * elapsed;
            if (bally < -6.2f)
            {
                dir_y = 1;
            }
        }
        
        if (dir_x == 1)
        {
            ballx += 2.0f * elapsed;
            if(ballx >= 10.9f && bally<bar2Top && bally>bar2Bottom)
            {
                dir_x = 0;
            }
        }
        
        else if(dir_x == 0)
        {
            ballx -= 2.0f * elapsed;
            if(ballx <= -10.9f && bally<bar1Top && bally>bar1Bottom)
            {
                dir_x = 1;
            }
        }
        
        //Changes screen color if someone wins
        if (ballx >= 12.5 || ballx <= -12.5)
        {
            glClearColor(0.4f, 0.8f, 0.8f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        
        
        // Code for ball
        glBindTexture(GL_TEXTURE_2D, ballTexture);
        
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        
        modelMatrix.identity();
        modelMatrix.Translate(0.0f, 0.0f, 0.0f);
        modelMatrix.Scale(0.3, 0.3, 0.0);
        program.setModelMatrix(modelMatrix);
        
        float ballVertices[] = { -0.5f+ballx, -0.5f+bally, 0.5f+ballx, -0.5f+bally, 0.5f+ballx, 0.5f+bally,-0.5f+ballx, -0.5f+bally, 0.5f+ballx, 0.5f+bally, -0.5f+ballx, 0.5f+bally};
       // float ballVertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,-0.5f, 0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float ballTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, ballTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
       
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
