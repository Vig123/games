//
//  Vector.h
//  NYUCodebase
//
//  Created by Vighneshan Moorthy on 4/6/17.
//  Copyright © 2017 Ivan Safrin. All rights reserved.
//

#ifndef Vector_h
#define Vector_h
#include <math.h>

class Vector
{
public:
    Vector(){};
    Vector(float X, float Y) : x(X), y(Y), z(0){};
    float x, y, z;
    float theLength = sqrt((x*x) + (y*y));
    const float length() const;
    void normalize();
};


#endif /* Vector_h */
