/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/rendering/lighting/light.h"
#include "saiga/camera/camera.h"

namespace Saiga {


class SAIGA_GLOBAL AttenuatedLightShader : public LightShader{
public:
    GLint location_attenuation;

    virtual void checkUniforms();
    virtual void uploadA(vec3 &attenuation, float cutoffRadius);
};

namespace AttenuationPresets{
static const vec3 NONE = vec3(1,0,0); //Cutoff = 1

static const vec3 LinearWeak = vec3(1,0.5,0); //Cutoff = 0.666667
static const vec3 Linear = vec3(1,1,0); //Cutoff = 0.5
static const vec3 LinearStrong = vec3(1,4,0); //Cutoff = 0.2


static const vec3 QuadraticWeak = vec3(1,0.5,0.5); //Cutoff = 0.5
static const vec3 Quadratic = vec3(1,1,1); //Cutoff = 0.333333
static const vec3 QuadraticStrong = vec3(1,2,4); //Cutoff = 0.142857
}


class SAIGA_GLOBAL AttenuatedLight : public Light
{
    friend class DeferredLighting;
protected:
    /**
     * Quadratic attenuation of the form:
     * I = i/(a*x*x+b*x+c)
     *
     * Note: The attenuation is independent of the radius.
     * x = d / r, where d is the distance to the light
     *
     * This normalized attenuation makes it easy to scale lights without having to change the attenuation
     *
     */

    vec3 attenuation = AttenuationPresets::Quadratic;


    /**
     * Distance after which the light intensity is clamped to 0.
     * The shadow volumes should be constructed so that they closely contain
     * all points up to the cutoffradius.
     */
    float cutoffRadius;

public:


    AttenuatedLight();
    virtual ~AttenuatedLight(){}
    AttenuatedLight& operator=(const AttenuatedLight& light);

    /**
     * Use a simple linear attenuation, so that a=0 and c=1
     * Note: The attenuation isn't really linear because it is inverted: 1/(b*r)
     * Drop is the relative intensity lost over the distance
     */
    void setLinearAttenuation(float drop);


    /**
     * Calculates the radius required of this light, so that 'cutoffPercentage' of the original intensity
     * reaches the border.
     *
     * This solves:
     * 1/(a*r*r+b*r+c) = h, where h is the cutoffPercentage
     *
     */

    float calculateRadius(float cutoffPercentage);
    float calculateRadiusAbsolute(float cutoff);

    virtual void bindUniforms(std::shared_ptr<AttenuatedLightShader> shader, Camera *cam);


    float getRadius() const;
    virtual void setRadius(float value);

    vec3 getAttenuation() const;
    float getAttenuation(float r);
    void setAttenuation(const vec3 &value);

    void createShadowMap(int resX, int resY);

    void bindFace(int face);
    void calculateCamera(int face);


    bool cullLight(Camera *cam);
};

}
