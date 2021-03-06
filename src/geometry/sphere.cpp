/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/geometry/sphere.h"

namespace Saiga {

int Sphere::intersectAabb(const AABB &other){

    if(!intersectAabb2(other))
        return 0;

    for(int i=0;i<8;i++){
        if(!contains(other.cornerPoint(i)))
            return 1;
    }



    return 2;
}

void Sphere::getMinimumAabb(AABB &box){
    vec3 rad(r+1,r+1,r+1);
    box.min = pos-rad;
    box.max = pos+rad;
}

bool Sphere::intersectAabb2(const AABB &other){


    float s, d = 0;

    //find the square of the distance
    //from the sphere to the box
    for( long i=0 ; i<3 ; i++ )
    {

        if( pos[i] < other.min[i] )
        {



            s = pos[i] - other.min[i];
            d += s*s;

        }

        else if( pos[i] > other.max[i] )
        {

            s = pos[i] - other.max[i];
            d += s*s;

        }

    }
    return d <= r*r;

}

bool Sphere::contains(vec3 p){
    return glm::length(p-pos) < r;
}

bool Sphere::intersect(const Sphere &other)
{
    return glm::distance(other.pos,pos) < r+other.r;
}

std::ostream& operator<<(std::ostream& os, const Saiga::Sphere& s)
{
    std::cout<< "Sphere: " << s.pos << s.r;
    return os;
}

}


