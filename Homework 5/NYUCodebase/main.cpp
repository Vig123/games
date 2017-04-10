#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "Vector.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
using namespace std;

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

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

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p >= 0) {
        return false;
    }
    
    float penetrationMin1 = e1Max - e2Min;
    float penetrationMin2 = e2Max - e1Min;
    
    float penetrationAmount = penetrationMin1;
    if(penetrationMin2 < penetrationAmount) {
        penetrationAmount = penetrationMin2;
    }
    
    penetration.x = normalX * penetrationAmount;
    penetration.y = normalY * penetrationAmount;
    
    return true;
}

bool penetrationSort(const Vector &p1, const Vector &p2)
{
    return p1.length() < p2.length();
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration)
{
    std::vector<Vector> penetrations;
    for(int i=0; i < e1Points.size(); i++)
    {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1)
        {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        if(!result)
        {
            return false;
        }
        penetrations.push_back(penetration);
    }
    for(int i=0; i < e2Points.size(); i++)
    {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1)
        {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        }
        else
        {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        Vector penetration;
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
        
        if(!result)
        {
            return false;
        }
        penetrations.push_back(penetration);
    }
    
    std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
    penetration = penetrations[0];
    
    Vector e1Center;
    for(int i=0; i < e1Points.size(); i++)
    {
        e1Center.x += e1Points[i].x;
        e1Center.y += e1Points[i].y;
    }
    e1Center.x /= (float)e1Points.size();
    e1Center.y /= (float)e1Points.size();
    
    Vector e2Center;
    for(int i=0; i < e2Points.size(); i++)
    {
        e2Center.x += e2Points[i].x;
        e2Center.y += e2Points[i].y;
    }
    e2Center.x /= (float)e2Points.size();
    e2Center.y /= (float)e2Points.size();
    
    Vector ba;
    ba.x = e1Center.x - e2Center.x;
    ba.y = e1Center.y - e2Center.y;
    
    if((penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f)
    {
        penetration.x *= -1.0f;
        penetration.y *= -1.0f;
    }
    
    return true;
}

class Entity
{
public:
    Entity(){};
    Entity(Matrix matrix, float x, float y, Vector scale, float rotation): matrix(matrix), x(x), y(y), scale(scale), rotation(rotation), velocity_x(0), velocity_y(0), acceleration_x(0), acceleration_y(0), friction_x(0), friction_y(0){};
    float lerp(float v0, float v1, float t)
    {
        return (1.0 - t)*v0 + t*v1;
    }
    void Update(float elapsed)
    {
        velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
        velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);
        velocity_x += acceleration_x * elapsed;
        velocity_y += acceleration_y * elapsed;
        x += velocity_x * elapsed;
        y += velocity_y * elapsed;
    }
    
    void Draw(ShaderProgram& program, float vertices[])
    {
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    }
    
    void toWorldCoord(std::vector<Vector> &worldCoords)
    {
        worldCoords.resize(vertices.size());
        for (int i = 0; i < vertices.size(); i++)
        {
            float x = vertices[i].x;
            float y = vertices[i].y;
            worldCoords[i].x = matrix.m[0][0] * x + matrix.m[1][0] * y + matrix.m[3][0];
            worldCoords[i].y = matrix.m[0][1] * x + matrix.m[1][1] * y + matrix.m[3][1];
        }
    }
    void addVertices(Vector vector)
    {
        vertices.push_back(vector);
    }
    Matrix matrix;
    Vector scale;
    float x;
    float y;
    float width;
    float height;
    float velocity_x = 0;
    float velocity_y = 0;
    float acceleration_x = 0;
    float acceleration_y = 0;
    float friction_x = 0;
    float friction_y = 0;
    float rotation;
    
    //SheetSprite sprite;
    std::vector<Vector> vertices;
    
   // bool isStatic;
    
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false ;
};

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    bool done = false;
    
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Matrix projectionMatrix;
    Matrix viewMatrix;
    Matrix modelMatrix;
    
    Vector penetration;
    
    projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    float lastFrameTicks = 0.0f;
    float vertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,0.5f};
    
    Entity one(modelMatrix, 0, 0, Vector(1,1), 0);
    Entity two(modelMatrix, 0, 0, Vector(1,1), 0);
    Entity three(modelMatrix, 0, 0, Vector(1,1), 0);
    
    one.addVertices(Vector(0.5, 0.5));
    one.addVertices(Vector(-0.5, 0.5));
    one.addVertices(Vector(-0.5, -0.5));
    one.addVertices(Vector(0.5, -0.5));
    
    two.addVertices(Vector(0.5, 0.5));
    two.addVertices(Vector(-0.5, 0.5));
    two.addVertices(Vector(-0.5, -0.5));
    two.addVertices(Vector(0.5, -0.5));
    
    three.addVertices(Vector(0.5, 0.5));
    three.addVertices(Vector(-0.5, 0.5));
    three.addVertices(Vector(-0.5, -0.5));
    three.addVertices(Vector(0.5, -0.5));
    
    glUseProgram(program.programID);
    program.setViewMatrix(viewMatrix);
    program.setProjectionMatrix(projectionMatrix);
    program.setModelMatrix(modelMatrix);
    

    
    while (!done)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
        }
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        float fixedElapsed = elapsed;
        if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS)
        {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP)
        {
            fixedElapsed -= FIXED_TIMESTEP;
            one.Update(FIXED_TIMESTEP);
            two.Update(FIXED_TIMESTEP);
            three.Update(FIXED_TIMESTEP);
        }
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setModelMatrix(modelMatrix);
        modelMatrix.identity();
        modelMatrix.Translate(0.0f, 0.0f, 0.0f);
        modelMatrix.Rotate(one.rotation);
        one.Draw(program, vertices);
        
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setModelMatrix(modelMatrix);
        modelMatrix.identity();
        modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
        two.Draw(program, vertices);
        
        program.setViewMatrix(viewMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setModelMatrix(modelMatrix);
        modelMatrix.identity();
        modelMatrix.Translate(3.0f, 0.0f, 0.0f);
        three.Draw(program, vertices);
        
        if(one.collidedTop == false && one.y < 2.0)
        {
            one.y += 1.0 * elapsed;
        }
        
        if(one.collidedBottom == false && one.y > -2.0)
        {
            one.y -= 1.0 * elapsed;
        }
        if(one.collidedLeft == false && one.y < 3.55)
        {
            one.x += 1.0 * elapsed;
        }
        if(one.collidedRight == false && one.y > -3.55)
        {
            one.x -= 1.0 * elapsed;
        }
        
        
        if(two.collidedTop == false && two.y < 2.0)
        {
            two.y += 1.0 * elapsed;
        }
        
        if(two.collidedBottom == false && two.y > -2.0)
        {
            two.y -= 1.0 * elapsed;
        }
        if(two.collidedLeft == false && two.y < 3.55)
        {
            two.x += 1.0 * elapsed;
        }
        if(two.collidedRight == false && two.y > -3.55)
        {
            two.x -= 1.0 * elapsed;
        }
        
        if(three.collidedTop == false && three.y < 2.0)
        {
            three.y += 1.0 * elapsed;
        }
        
        if(three.collidedBottom == false && three.y > -2.0)
        {
            one.y -= 1.0 * elapsed;
        }
        if(three.collidedLeft == false && three.y < 3.55)
        {
            three.x += 1.0 * elapsed;
        }
        if(three.collidedRight == false && three.y > -3.55)
        {
            three.x -= 1.0 * elapsed;
        }
        
        for (int i = 0; i <= 3; i++)
        {
            one.vertices[i] = one.matrix * one.vertices[i];
            two.vertices[i] = two.matrix * two.vertices[i];
            three.vertices[i] = three.matrix * three.vertices[i];
        }
        
        if (checkSATCollision(one.vertices, two.vertices, penetration)) {
            one.x += penetration.x *1.0f;
            one.y += penetration.y *1.0f;
            
            two.x -= penetration.x *1.0f;
            two.y -= penetration.y*1.0f;
            
            one.velocity_x = -one.velocity_x;
            one.velocity_y = -one.velocity_y;
            
            two.velocity_x = -two.velocity_x;
            two.velocity_y = -two.velocity_y;
        }
        if (checkSATCollision(one.vertices, three.vertices, penetration)) {
            one.x += penetration.x*1.0f;
            one.y += penetration.y*1.0f;
            
            three.x -= penetration.x*1.0f;
            three.y -= penetration.y*1.0f;
            
            one.velocity_x = -one.velocity_x;
            one.velocity_y = -one.velocity_y;
            
            three.velocity_x = -three.velocity_x;
            three.velocity_y = -three.velocity_y;
        }
        if (checkSATCollision(two.vertices, three.vertices, penetration))
        {
            two.x += penetration.x *1.0f;
            two.y -= penetration.y*1.0f;
            
            three.x -= penetration.x *1.0f;
            three.y += penetration.y*1.0f;
            
            three.velocity_x = -three.velocity_x;
            three.velocity_y = -three.velocity_y;
            
            two.velocity_x = -two.velocity_x;
            two.velocity_y = -two.velocity_y;
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
