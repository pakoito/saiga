/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/rendering/lighting/deferred_lighting.h"
#include "saiga/util/inputcontroller.h"

#include "saiga/rendering/deferred_renderer.h"
#include "saiga/util/error.h"

#include "saiga/rendering/lighting/directional_light.h"
#include "saiga/rendering/lighting/point_light.h"
#include "saiga/rendering/lighting/spot_light.h"
#include "saiga/rendering/lighting/box_light.h"

#include "saiga/geometry/triangle_mesh_generator.h"
#include "saiga/opengl/texture/cube_texture.h"

#include "saiga/opengl/shader/shaderLoader.h"
#include "saiga/rendering/renderer.h"
#include "saiga/imgui/imgui.h"
#include "saiga/util/tostring.h"

namespace Saiga {

DeferredLighting::DeferredLighting(GBuffer &framebuffer):gbuffer(framebuffer){
    createLightMeshes();
    shadowCameraBuffer.createGLBuffer(nullptr,sizeof(CameraDataGLSL),GL_DYNAMIC_DRAW);
}

DeferredLighting::~DeferredLighting(){
}

void DeferredLighting::loadShaders(const DeferredLightingShaderNames &names)
{
    int shadowSamplesX = glm::round( glm::sqrt( (float) shadowSamples) );
    //    cout << "DeferredLighting shadowSamplesX=" << shadowSamplesX << endl;


    ShaderPart::ShaderCodeInjections shadowInjection;
    shadowInjection.emplace_back(GL_FRAGMENT_SHADER,
                                 "#define SHADOWS",1);
    shadowInjection.emplace_back(GL_FRAGMENT_SHADER,
                                 "#define SHADOW_SAMPLES_X " + std::to_string(shadowSamplesX),2);

    spotLightShader         = ShaderLoader::instance()->load<SpotLightShader>(names.spotLightShader);
    spotLightShadowShader   = ShaderLoader::instance()->load<SpotLightShader>(names.spotLightShader,shadowInjection);

    pointLightShader = ShaderLoader::instance()->load<PointLightShader>(names.pointLightShader);
    pointLightShadowShader = ShaderLoader::instance()->load<PointLightShader>(names.pointLightShader,shadowInjection);

    directionalLightShader = ShaderLoader::instance()->load<DirectionalLightShader>(names.directionalLightShader);
    directionalLightShadowShader = ShaderLoader::instance()->load<DirectionalLightShader>(names.directionalLightShader,shadowInjection);

    boxLightShader = ShaderLoader::instance()->load<BoxLightShader>(names.boxLightShader);
    boxLightShadowShader = ShaderLoader::instance()->load<BoxLightShader>(names.boxLightShader,shadowInjection);

    debugShader = ShaderLoader::instance()->load<MVPColorShader>(names.debugShader);
    stencilShader = ShaderLoader::instance()->load<MVPShader>(names.stencilShader);

    //    blitDepthShader = ShaderLoader::instance()->load<MVPTextureShader>("post_processing/blitDepth.glsl");
    lightAccumulationShader = ShaderLoader::instance()->load<LightAccumulationShader>(names.lightAccumulationShader);
}

void DeferredLighting::init(int _width, int _height, bool _useTimers){
    this->width=_width;this->height=_height;
    useTimers = _useTimers;

    if (useTimers) {
        timers2.resize(5);
        for (int i = 0; i < 5; ++i) {
            timers2[i].create();

        }
        timerStrings.resize(5);
        timerStrings[0] = "Init";
        timerStrings[1] = "Point Lights";
        timerStrings[2] = "Spot Lights";
        timerStrings[3] = "Box Lights";
        timerStrings[4] = "Directional Lights";
    }

    lightAccumulationBuffer.create();

    //    std::shared_ptr<Texture> depth_stencil = new Texture();
    //    depth_stencil->createEmptyTexture(width,height,GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8,GL_UNSIGNED_INT_24_8);
    //    lightAccumulationBuffer.attachTextureDepthStencil( framebuffer_texture_t(depth_stencil) );

    //NOTE: Use the same depth-stencil buffer as the gbuffer. I hope this works on every hardware :).
    lightAccumulationBuffer.attachTextureDepthStencil(gbuffer.getTextureDepth());

    lightAccumulationTexture = std::make_shared<Texture>();
    lightAccumulationTexture->createEmptyTexture(_width,_height,GL_RGBA,GL_RGBA16,GL_UNSIGNED_SHORT);
    lightAccumulationBuffer.attachTexture( framebuffer_texture_t(lightAccumulationTexture) );
    lightAccumulationBuffer.drawToAll();
    lightAccumulationBuffer.check();
    lightAccumulationBuffer.unbind();
}

void DeferredLighting::resize(int _width, int _height)
{
    this->width=_width;this->height=_height;
    lightAccumulationBuffer.resize(_width,_height);
}

void DeferredLighting::cullLights(Camera *cam){

    visibleLights = directionalLights.size();

    //cull lights that are not visible
    for(auto &light : spotLights){
        if(light->isActive()){
            light->calculateCamera();
            light->shadowCamera.recalculatePlanes();
            visibleLights += (light->cullLight(cam))? 0 : 1;
        }
    }

    for(auto &light : boxLights){
        if(light->isActive()){
            light->calculateCamera();
            light->shadowCamera.recalculatePlanes();
            visibleLights += (light->cullLight(cam))? 0 : 1;
        }
    }


    for(auto &light : pointLights){
        if(light->isActive()){
            visibleLights += (light->cullLight(cam))? 0 : 1;
        }
    }
}

void DeferredLighting::printTimings()
{
    if (!useTimers)
        return;
    for(int i = 0 ;i < 5 ;++i){
        cout<<"\t "<< getTime(i)<<"ms "<<timerStrings[i]<<endl;
    }
}


void DeferredLighting::initRender()
{
    totalLights = 0;
    renderedDepthmaps = 0;
    totalLights = directionalLights.size() + spotLights.size() + pointLights.size()  + boxLights.size();
}

void DeferredLighting::renderDepthMaps(Program *renderer){
    // When GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, or GL_POLYGON_OFFSET_POINT is enabled,
    // each fragment's depth value will be offset after it is interpolated from the depth values of the appropriate vertices.
    // The value of the offset is factor×DZ+r×units, where DZ is a measurement of the change in depth relative to the screen area of the polygon,
    // and r is the smallest value that is guaranteed to produce a resolvable offset for a given implementation.
    // The offset is added before the depth test is performed and before the value is written into the depth buffer.
    glEnable(GL_POLYGON_OFFSET_FILL);

    float shadowMult = backFaceShadows ? -1 : 1;
    glPolygonOffset(shadowMult*shadowOffsetFactor,shadowMult*shadowOffsetUnits);
    if(backFaceShadows)
        glCullFace(GL_FRONT);
    else
        glCullFace(GL_BACK);


    shadowCameraBuffer.bind(CAMERA_DATA_BINDING_POINT);
    DepthFunction depthFunc = [&](Camera* cam) -> void{
        renderedDepthmaps++;
        renderer->renderDepth(cam);
    };
    for(auto &light : directionalLights){
        light->renderShadowmap(depthFunc,shadowCameraBuffer);
    }
    for(auto &light : boxLights){
        light->renderShadowmap(depthFunc,shadowCameraBuffer);
    }
    for(auto &light : spotLights){
        light->renderShadowmap(depthFunc,shadowCameraBuffer);
    }
    for(auto &light : pointLights){
        light->renderShadowmap(depthFunc,shadowCameraBuffer);
    }
    glCullFace(GL_BACK);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void DeferredLighting::render(Camera* cam){
    //    gbuffer.blitDepth(lightAccumulationBuffer.getId());

    startTimer(0);


    //viewport is maybe different after shadow map rendering
    glViewport(0,0,width,height);



    //    glClear( GL_STENCIL_BUFFER_BIT );
    //    glClear( GL_COLOR_BUFFER_BIT );

    //    glDepthMask(GL_FALSE);
    //    glDisable(GL_DEPTH_TEST);


    lightAccumulationBuffer.bind();

    //    glClearColor(0,0,0,0);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear( GL_COLOR_BUFFER_BIT );

    //    blitGbufferDepthToAccumulationBuffer();






    //deferred lighting uses additive blending of the lights.
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    //never overwrite current depthbuffer
    glDepthMask(GL_FALSE);

    //all light volumes are using stencil culling
    glEnable(GL_STENCIL_TEST);

    //use depth test for all light volumes
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //    glClearStencil(0x0);
    //    glClear(GL_STENCIL_BUFFER_BIT);
    currentStencilId = 1;
    stopTimer(0);

    assert_no_glerror();
    startTimer(1);
    for(auto& l : pointLights){
        renderLightVolume< std::shared_ptr<PointLight> ,std::shared_ptr<PointLightShader>>(pointLightMesh,l,cam,pointLightShader,pointLightShadowShader);
    }
    stopTimer(1);

    startTimer(2);
    for(auto& l : spotLights){
        renderLightVolume< std::shared_ptr<SpotLight> ,std::shared_ptr<SpotLightShader>>(spotLightMesh,l,cam,spotLightShader,spotLightShadowShader);
    }
    stopTimer(2);

    startTimer(3);
    for(auto& l : boxLights){
        renderLightVolume< std::shared_ptr<BoxLight> ,std::shared_ptr<BoxLightShader>>(boxLightMesh,l,cam,boxLightShader,boxLightShadowShader);
    }
    stopTimer(3);
    assert_no_glerror();

    //reset depth test to default value
    glDepthFunc(GL_LESS);




    //use default culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_DEPTH_TEST);

    startTimer(4);
    glStencilFunc(GL_NOTEQUAL, 0xFF, 0xFF);
    //    glStencilFunc(GL_EQUAL, 0x0, 0xFF);
    //    glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);
    renderDirectionalLights(cam,false);
    renderDirectionalLights(cam,true);
    stopTimer(4);

    glDisable(GL_STENCIL_TEST);
    assert_no_glerror();
    //reset state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if(drawDebug){
        glDepthMask(GL_TRUE);
        renderDebug(cam);
        glDepthMask(GL_FALSE);
    }


    assert_no_glerror();

}



void DeferredLighting::setupStencilPass(){
    //don't write color in stencil pass
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

    //render only front faces
    glCullFace(GL_BACK);

    //default depth test
    glDepthFunc(GL_LEQUAL);

    //set stencil to 'id' if depth test fails
    //all 'failed' pixels are now marked in the stencil buffer with the id
    glStencilFunc(GL_ALWAYS, currentStencilId, 0xFF);
    //    glStencilFunc(GL_LEQUAL, currentStencilId, 0xFF);
    glStencilOp(GL_KEEP,GL_REPLACE,GL_KEEP);
}
void DeferredLighting::setupLightPass(){
    //write color in the light pass
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    //render only back faces
    glCullFace(GL_FRONT);

    //reversed depth test: it passes if the light volume is behind an object
    glDepthFunc(GL_GEQUAL);

    //discard all pixels that are marked with 'id' from the previous pass
    glStencilFunc(GL_NOTEQUAL, currentStencilId, 0xFF);
    //    glStencilFunc(GL_NEVER, currentStencilId, 0xFF);
    //    glStencilFunc(GL_GREATER, currentStencilId, 0xFF);
    //    glStencilFunc(GL_EQUAL, 0x0, 0xFF);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);

    //-> the reverse depth test + the stencil test make now sure that the current pixel is in the light volume
    //this also works, when the camera is inside the volume, but fails when the far plane is intersecting the volume


    //increase stencil id, so the next light will write a different value to the stencil buffer.
    //with this trick the expensive clear can be saved after each light
    currentStencilId++;
    SAIGA_ASSERT(currentStencilId<256);
}



void DeferredLighting::renderDirectionalLights(Camera *cam,bool shadow){

    std::shared_ptr<DirectionalLightShader>  shader = (shadow)?directionalLightShadowShader:directionalLightShader;

    shader->bind();
    shader->DeferredShader::uploadFramebuffer(&gbuffer);
    shader->uploadScreenSize(vec2(width,height));
    shader->uploadSsaoTexture(ssaoTexture);

    directionalLightMesh.bind();
    for(auto &obj : directionalLights){
        bool render = (shadow&&obj->shouldCalculateShadowMap()) || (!shadow && obj->shouldRender() && !obj->hasShadows());
        if(render){
            obj->bindUniforms(*shader,cam);
            directionalLightMesh.draw();
        }
    }
    directionalLightMesh.unbind();
    shader->unbind();
}


void DeferredLighting::renderDebug(Camera *cam){

    debugShader->bind();

    // ======================= Pointlights ===================

    pointLightMesh.bind();
    //center
    for(auto &obj : pointLights){
        mat4 sm = glm::scale(obj->model,vec3(0.05));
        vec4 color = obj->colorDiffuse;
        if(!obj->isActive()||!obj->isVisible()){
            //render as black if light is turned off
            color = vec4(0);
        }
        debugShader->uploadModel(sm);
        debugShader->uploadColor(color);
        pointLightMesh.draw();
    }

    //render outline
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    for(auto &obj : pointLights){
        //        if(obj->isSelected()){
        debugShader->uploadModel(obj->model);
        debugShader->uploadColor(obj->colorDiffuse);
        pointLightMesh.draw();
        //        }
    }
    pointLightMesh.unbind();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


    //==================== Spotlights ==================

    spotLightMesh.bind();
    //center
    for(auto &obj : spotLights){
        mat4 sm = glm::scale(obj->model,vec3(0.05));
        vec4 color = obj->colorDiffuse;
        if(!obj->isActive()||!obj->isVisible()){
            //render as black if light is turned off
            color = vec4(0);
        }
        debugShader->uploadModel(sm);
        debugShader->uploadColor(color);
        spotLightMesh.draw();
    }

    //render outline
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    for(auto &obj : spotLights){
        //        if(obj->isSelected()){
        debugShader->uploadModel(obj->model);
        debugShader->uploadColor(obj->colorDiffuse);
        spotLightMesh.draw();
        //        }
    }
    spotLightMesh.unbind();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


    //==================== Box lights ====================

    boxLightMesh.bind();
    //center
    for(auto &obj : boxLights){
        mat4 sm = glm::scale(obj->model,vec3(0.05));
        vec4 color = obj->colorDiffuse;
        if(!obj->isActive()||!obj->isVisible()){
            //render as black if light is turned off
            color = vec4(0);
        }
        debugShader->uploadModel(sm);
        debugShader->uploadColor(color);
        boxLightMesh.draw();
    }

    //render outline
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    for(auto &obj : boxLights){
        //        if(obj->isSelected()){
        debugShader->uploadModel(obj->model);
        debugShader->uploadColor(obj->colorDiffuse);
        boxLightMesh.draw();
        //        }
    }
    boxLightMesh.unbind();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    debugShader->unbind();

}

void DeferredLighting::blitGbufferDepthToAccumulationBuffer()
{
    //    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(GL_ALWAYS);
    //    blitDepthShader->bind();
    //    blitDepthShader->uploadTexture(gbuffer.getTextureDepth().get());
    //    directionalLightMesh.bindAndDraw();
    //    blitDepthShader->unbind();
    //    glDepthFunc(GL_LESS);




    //    glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer.getId());
    //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightAccumulationBuffer.getId());
    //    glBlitFramebuffer(0, 0, gbuffer.getTextureDepth()->getWidth(), gbuffer.getTextureDepth()->getHeight(), 0, 0, gbuffer.getTextureDepth()->getWidth(), gbuffer.getTextureDepth()->getHeight(),GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    //    glClearColor(0,0,0,0);
    //    glClear( GL_COLOR_BUFFER_BIT );
}

void DeferredLighting::setShader(std::shared_ptr<SpotLightShader>  spotLightShader, std::shared_ptr<SpotLightShader>  spotLightShadowShader){
    this->spotLightShader = spotLightShader;
    this->spotLightShadowShader = spotLightShadowShader;
}

void DeferredLighting::setShader(std::shared_ptr<PointLightShader>  pointLightShader, std::shared_ptr<PointLightShader>  pointLightShadowShader){
    this->pointLightShader = pointLightShader;
    this->pointLightShadowShader = pointLightShadowShader;
}

void DeferredLighting::setShader(std::shared_ptr<DirectionalLightShader>  directionalLightShader, std::shared_ptr<DirectionalLightShader>  directionalLightShadowShader){
    this->directionalLightShader = directionalLightShader;
    this->directionalLightShadowShader = directionalLightShadowShader;
}

void DeferredLighting::setShader(std::shared_ptr<BoxLightShader>  boxLightShader, std::shared_ptr<BoxLightShader>  boxLightShadowShader)
{
    this->boxLightShader = boxLightShader;
    this->boxLightShadowShader = boxLightShadowShader;
}

void DeferredLighting::setDebugShader(std::shared_ptr<MVPColorShader>  shader){
    this->debugShader = shader;
}

void DeferredLighting::setStencilShader(std::shared_ptr<MVPShader> stencilShader){
    this->stencilShader = stencilShader;
}





void DeferredLighting::createLightMeshes(){

    auto qb = TriangleMeshGenerator::createFullScreenQuadMesh();
    qb->createBuffers(directionalLightMesh);



    //the create mesh returns a sphere with outer radius of 1
    //but here we want the inner radius to be 1
    //we estimate the required outer radius with apothem of regular polygons
    float n = 4.9;
    float r = 1.0f / glm::cos(glm::pi<float>() / n);
    //    cout << "point light radius " << r << endl;
    Sphere s(vec3(0),r);
    auto sb = TriangleMeshGenerator::createMesh(s,1);
    sb->createBuffers(pointLightMesh);


    Cone c(vec3(0),vec3(0,1,0),1.0f,1.0f);
    auto cb = TriangleMeshGenerator::createMesh(c,10);
    cb->createBuffers(spotLightMesh);

    AABB box(vec3(-1),vec3(1));
    auto bb = TriangleMeshGenerator::createMesh(box);
    bb->createBuffers(boxLightMesh);
}

std::shared_ptr<DirectionalLight> DeferredLighting::createDirectionalLight(){
    std::shared_ptr<DirectionalLight> l =  std::make_shared<DirectionalLight>();
    directionalLights.push_back(l);
    return l;
}

std::shared_ptr<PointLight> DeferredLighting::createPointLight(){
    std::shared_ptr<PointLight> l =  std::make_shared<PointLight>();
    pointLights.push_back(l);
    return l;
}

std::shared_ptr<SpotLight> DeferredLighting::createSpotLight(){
    std::shared_ptr<SpotLight> l =  std::make_shared<SpotLight>();
    spotLights.push_back(l);
    return l;
}

std::shared_ptr<BoxLight> DeferredLighting::createBoxLight(){
    std::shared_ptr<BoxLight> l =  std::make_shared<BoxLight>();
    boxLights.push_back(l);
    return l;
}

void DeferredLighting::removeLight(std::shared_ptr<DirectionalLight> l)
{
    directionalLights.erase(std::find(directionalLights.begin(),directionalLights.end(),l));
}

void DeferredLighting::removeLight(std::shared_ptr<PointLight> l)
{
    pointLights.erase(std::find(pointLights.begin(),pointLights.end(),l));
}

void DeferredLighting::removeLight(std::shared_ptr<SpotLight> l)
{
    spotLights.erase(std::find(spotLights.begin(),spotLights.end(),l));
}

void DeferredLighting::removeLight(std::shared_ptr<BoxLight> l)
{
    boxLights.erase(std::find(boxLights.begin(),boxLights.end(),l));

}


template<typename T>
static void imGuiLightBox(int id, const std::string &name, T& lights){
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::PushID(id);
    if(ImGui::CollapsingHeader(name.c_str())){
        int i = 0;
        for(auto &light : lights){
            ImGui::PushID(i);
            if(ImGui::CollapsingHeader(to_string(i).c_str())){
                light->renderImGui();
            }
            i++;
            ImGui::PopID();
        }
    }
    ImGui::PopID();
}

void DeferredLighting::renderImGui(bool *p_open)
{
    int w = 340;
    int h = 240;
    ImGui::SetNextWindowPos(ImVec2(680, height - h), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w,h), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("DeferredLighting",p_open);

    ImGui::Text("resolution: %dx%d",width,height);
    ImGui::Text("visibleLights/totalLights: %d/%d",visibleLights,totalLights);
    ImGui::Text("renderedDepthmaps: %d",renderedDepthmaps);
    ImGui::Text("shadowSamples: %d",shadowSamples);
    ImGui::ColorEdit4("clearColor ",&clearColor[0]);
    ImGui::Checkbox("drawDebug",&drawDebug);
    ImGui::Checkbox("useTimers",&useTimers);

    ImGui::Text("Render Time (without shadow map computation)");
    for(int i = 0 ;i < 5 ;++i){
        ImGui::Text("  %f ms %s",getTime(i),timerStrings[i].c_str());
    }
    ImGui::Checkbox("backFaceShadows",&backFaceShadows);
    ImGui::InputFloat("shadowOffsetFactor",&shadowOffsetFactor,0.1,1);
    ImGui::InputFloat("shadowOffsetUnits",&shadowOffsetUnits,0.1,1);
    imGuiLightBox(0,"Directional Lights",directionalLights);
    imGuiLightBox(1,"Spot Lights",spotLights);
    imGuiLightBox(2,"Point Lights",pointLights);
    imGuiLightBox(3,"Box Lights",boxLights);

    ImGui::End();

}

}
