//
//  SheetSprite.hpp
//  NYUCodebase
//
//  Created by Vighneshan Moorthy on 4/25/17.
//  Copyright © 2017 Ivan Safrin. All rights reserved.
//

#ifndef SheetSprite_hpp
#define SheetSprite_hpp

#include <stdio.h>
#include "ShaderProgram.h"

class SheetSprite {
public:
    SheetSprite(){};
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size) : textureID (textureID), u (u), v(v), width (width), height (height), size (size){};
    
    void Draw(ShaderProgram* program);
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};


#endif /* SheetSprite_h */
