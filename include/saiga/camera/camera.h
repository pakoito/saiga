/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include <saiga/util/glm.h>
#include <saiga/rendering/object3d.h>
#include <saiga/geometry/sphere.h>
#include <saiga/geometry/plane.h>
#include <saiga/opengl/uniformBuffer.h>

namespace Saiga {

using std::string;




class SAIGA_GLOBAL Camera : public Object3D{
public:
    std::string name;

    //    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 viewProj;


    float zNear,  zFar;
    float nw,nh,fw,fh; //dimensions of near and far plane

    vec3 vertices[8];
    Plane planes[6]; //for exact frustum culling
    Sphere boundingSphere; //for fast frustum culling

    Camera();
    virtual ~Camera(){}



    void setView(const mat4 &v);
    void setView(const vec3 &eye,const vec3 &center,const vec3 &up);


    void setProj(const mat4 &p){proj=p;recalculateMatrices();}
    //    virtual void setProj( double fovy, double aspect, double zNear, double zFar){}
    //    virtual void setProj( float left, float right,float bottom,float top,float near,  float far){}

    void updateFromModel();
    void recalculateMatrices(){viewProj = proj * view;}
    virtual void recalculatePlanes() = 0;



    enum IntersectionResult{
        OUTSIDE = 0,
        INSIDE,
        INTERSECT
    };

    //culling stuff
    IntersectionResult pointInFrustum(const vec3 &p);
    IntersectionResult sphereInFrustum(const Sphere &s);

    IntersectionResult pointInSphereFrustum(const vec3 &p);
    IntersectionResult sphereInSphereFrustum(const Sphere &s);

    /**
     * Return the intervall (min,max) when all vertices of the frustum are
     * projected to the axis 'd'. To dedect an overlap in intervalls the axis
     * does not have to be normalized.
     *
     * @brief projectedIntervall
     * @param d
     * @return
     */
    vec2 projectedIntervall(const vec3 &d);

    /**
     * Returns the side of the plane on which the frustum is.
     * +1 on the positive side
     * -1 on the negative side
     * 0 the plane is intersecting the frustum
     *
     * @brief sideOfPlane
     * @param plane
     * @return
     */
    int sideOfPlane(const Plane &plane);

    /**
     * Exact frustum-frustum intersection with the Separating Axes Theorem (SAT).
     * This test is expensive, so it should be only used when important.
     *
     * Number of Operations:
     * 6+6=12  sideOfPlane(const Plane &plane), for testing the faces of the frustum.
     * 6*6*2=72  projectedIntervall(const vec3 &d), for testing all cross product of pairs of non parallel edges
     *
     * http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
     * @brief intersectSAT
     * @param other
     * @return
     */

    bool intersectSAT(Camera* other);


    /**
     * Returns unique edges of the frustum.
     * A frustum has 6 unique edges ( non parallel edges).
     * @brief getEdge
     * @param i has to be in range (0 ... 5)
     * @return
     */

    std::pair<vec3,vec3> getEdge(int i);

    vec3 projectToViewSpace(vec3 worldPosition){
        return vec3(view * vec4(worldPosition,1));
    }

    vec3 projectToNDC(vec3 worldPosition){
        vec4 p = (viewProj * vec4(worldPosition,1));
        p /= p.w;
        return vec3(p);
    }

    vec2 projectToScreenSpace(vec3 worldPosition, int w, int h){
        vec3 p = projectToNDC(worldPosition);
        vec2 ip = vec2(p);
        ip = ip * 0.5f + vec2(0.5f);
        ip *= vec2(w,h);
        return ip;
    }

    vec3 inverseprojectToWorldSpace(vec2 ip, float depth, int w, int h){
        ip /= vec2(w,h);
        ip = (ip - vec2(0.5f)) * 2.0f;
        vec3 p = vec3(ip,depth);
        vec4 wp = inverse(viewProj) * vec4(p,1);
        wp /= wp.w;
        return vec3(wp);
    }

private:
    friend std::ostream& operator<<(std::ostream& os, const Camera& ca);
};

//========================= PerspectiveCamera =========================

class SAIGA_GLOBAL PerspectiveCamera : public Camera{
public:
    float fovy,  aspect;
    float tang;
    PerspectiveCamera(){}
    void setProj(float fovy, float aspect, float zNear, float zFar);
    friend std::ostream& operator<<(std::ostream& os, const PerspectiveCamera& ca);

    virtual void recalculatePlanes();
};

//========================= OrthographicCamera =========================

class SAIGA_GLOBAL OrthographicCamera : public Camera{
public:
    float left,right,bottom,top;
    OrthographicCamera(){}
    void setProj( float left, float right,float bottom,float top,float near,  float far);
    void setProj( AABB bb);

    friend std::ostream& operator<<(std::ostream& os, const OrthographicCamera& ca);


    virtual void recalculatePlanes() ;

};


/**
 * Equivalent to the uniform block defined in camera.glsl
 * This makes uploading the camera parameters more efficient, because they can be shared in multiple shaders and
 * be upload at once.
 */
struct CameraDataGLSL{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 camera_position;

    CameraDataGLSL(){}
    CameraDataGLSL(Camera* cam){
        view = cam->view;
        proj = cam->proj;
        viewProj = proj * view;
        camera_position = cam->position;
    }
};

}
