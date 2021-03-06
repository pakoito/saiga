/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include <saiga/config.h>
#include <saiga/util/glm.h>

#include <vector>
#include "saiga/geometry/triangle.h"

namespace Saiga{

class SAIGA_GLOBAL AABB
{
public:
    vec3 min = vec3(0);
    vec3 max = vec3(0);

public:

    AABB();
    AABB(const vec3 &min, const vec3 &max);
    ~AABB();


    //returns the axis with the maximum extend
    int maxDimension();

    void makeNegative();
    void growBox(const vec3 &v);
    void growBox(const AABB &v);

    void transform(const mat4 &trafo);
    void translate(const vec3 &v);
    void scale(const vec3 &s);
    float height(){ return max.y-min.y;}
    void ensureValidity();

    int intersect(const AABB &other);
    bool intersectBool(const AABB &other);
    bool intersectTouching(const AABB &other); //returns true if boxes are touching

    bool intersectBool(const AABB &other, int side);
    int touching(const AABB &other);

    vec3 getHalfExtends();



    void getMinimumAabb(AABB &box){ box = *this;}

    vec3 cornerPoint(int i) const;

    vec3 getPosition() const;
    void setPosition(const vec3 &v);

    bool contains(const vec3 &p);

    //A list the 12 triangles (2 for each face)
    std::vector<Triangle> toTriangles();


    SAIGA_GLOBAL friend std::ostream& operator<<(std::ostream& os, const AABB& dt);
};

#include "saiga/geometry/aabb.inl"

}
