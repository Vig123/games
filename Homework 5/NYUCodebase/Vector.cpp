//
//  Vector.cpp
//  NYUCodebase
//
//  Created by Vighneshan Moorthy on 4/6/17.
//  Copyright © 2017 Ivan Safrin. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include "Vector.h"

const float Vector::length() const
{
    return theLength;
}
void Vector::normalize()
{
    
    x /= length();
    y /= length();
    z /= length();
    
}



