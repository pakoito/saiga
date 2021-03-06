/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/window/window.h"
#include "saiga/rendering/deferred_renderer.h"
#include "saiga/opengl/shader/shaderLoader.h"
#include "saiga/opengl/texture/textureLoader.h"

#include "saiga/rendering/deferred_renderer.h"
#include "saiga/rendering/renderer.h"

#include "saiga/util/tostring.h"
#include "saiga/util/error.h"
#include "saiga/framework.h"
#include "saiga/imgui/imgui.h"

#include <cstring>
#include <vector>
#include <ctime>
#include <thread>

namespace Saiga {

using std::cout;
using std::endl;


void WindowParameters::setMode(bool fullscreen, bool borderLess)
{
    if(fullscreen){
        mode = (borderLess) ? Mode::borderLessFullscreen : Mode::fullscreen;
    }else{
        mode = (borderLess) ? Mode::borderLessWindowed : Mode::windowed;
    }
}

OpenGLWindow::OpenGLWindow(WindowParameters _windowParameters)
    :windowParameters(_windowParameters),
      updateTimer(0.97f),interpolationTimer(0.97f),renderCPUTimer(0.97f),swapBuffersTimer(0.97f),fpsTimer(50),upsTimer(50){

}

OpenGLWindow::~OpenGLWindow(){
    delete renderer;
}

void OpenGLWindow::close(){
    cout<<"Window: close"<<endl;
    running = false;
}

void OpenGLWindow::renderImGui(bool *p_open)
{
    p_open = &showImgui;

    int w = 340;
    int h = 240;
    ImGui::SetNextWindowPos(ImVec2(0, getHeight() - h), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w,h), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("OpenGLWindow",&showImgui);



    float ut = std::chrono::duration<double, std::milli>(updateTimer.getTime()).count();
    float ft = renderer->getUnsmoothedTimeMS(Deferred_Renderer::DeferredTimings::TOTAL);


    float avUt = 0, avFt = 0;
    for(int i = 0 ;i < numGraphValues; ++i){
        avUt +=   imUpdateTimes[i];
        avFt +=   imRenderTimes[i];
    }
    avUt /= numGraphValues;
    avFt /= numGraphValues;

    imUpdateTimes[imCurrentIndex] = ut;
    imRenderTimes[imCurrentIndex] = ft;
    imCurrentIndex = (imCurrentIndex+1) % numGraphValues;


    ImGui::Text("Update Time: %fms Ups: %f",ut, 1000.0f / upsTimer.getTimeMS());
    ImGui::PlotLines("Update Time", imUpdateTimes, numGraphValues, imCurrentIndex, ("avg "+Saiga::to_string(avUt)).c_str(), 0,20, ImVec2(0,80));
    ImGui::Text("Render Time: %fms Fps: %f",ft, 1000.0f / fpsTimer.getTimeMS());
    ImGui::PlotLines("Render Time", imRenderTimes, numGraphValues, imCurrentIndex, ("avg "+Saiga::to_string(avFt)).c_str(), 0,20, ImVec2(0,80));


    ImGui::Text("Running: %d",running);
    ImGui::Text("numUpdates: %d",numUpdates);
    ImGui::Text("numFrames: %d",numFrames);

    std::chrono::duration<double, std::milli> dt = gameTime.dt;
    ImGui::Text("Timestep: %fms",dt.count());

    std::chrono::duration<double, std::milli> delay = gameLoopDelay;
    ImGui::Text("Delay: %fms",delay.count());

    float scale = gameTime.getTimeScale();
    ImGui::SliderFloat("Time Scale",&scale,0,5);
    gameTime.setTimeScale(scale);


    ImGui::Checkbox("showRendererImgui",&showRendererImgui);
    ImGui::Checkbox("showImguiDemo",&showImguiDemo);

    ImGui::End();

    if(showRendererImgui){
        renderer->renderImGui(&showRendererImgui);
    }


    if(showImguiDemo){
        ImGui::SetNextWindowPos(ImVec2(340, 0), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&showImguiDemo);
    }
}


bool OpenGLWindow::init(const RenderingParameters& params){
    initSaiga();

    //init window and opengl context
    if(!initWindow()){
        std::cerr<<"Failed to initialize Window!"<<std::endl;
        return false;
    }


    //inits opengl (loads functions)
    initOpenGL();
    assert_no_glerror();


    if(!initInput()){
        std::cerr<<"Failed to initialize Input!"<<std::endl;
        return false;
    }


    //this somehow doesn't work in 32 bit windows


    //in older glew versions the last parameter of the function is void* instead of const void*
#if defined(GLEW_VERSION_4_5) || defined(SAIGA_USE_GLBINDING)

    //this somehow doesn't work on windows 32 bit
#if !defined _WIN32 || defined _WIN64
    glDebugMessageCallback(Error::DebugLogConst,NULL);
#endif

#else
    glDebugMessageCallback(Error::DebugLog,NULL);
#endif

    cout<<">> Window inputs initialized!"<<endl;
    assert_no_glerror();



    initDeferredRendering(params);
    assert_no_glerror();
    return true;
}

void OpenGLWindow::initDeferredRendering(const RenderingParameters &params)
{

    renderer = new Deferred_Renderer(getWidth(),getHeight(),params);


    std::shared_ptr<PostProcessingShader>  pps = ShaderLoader::instance()->load<PostProcessingShader>("post_processing/post_processing.glsl"); //this shader does nothing
    std::vector<std::shared_ptr<PostProcessingShader> > defaultEffects;
    defaultEffects.push_back(pps);

    renderer->postProcessor.setPostProcessingEffects(defaultEffects);

    renderer->lighting.setRenderDebug( false);

    renderer->currentCamera = &this->currentCamera;

}

void OpenGLWindow::resize(int width, int height)
{
    this->windowParameters.width = width;
    this->windowParameters.height = height;
    renderer->resize(width,height);
}


void OpenGLWindow::readToExistingImage(Image &out)
{
    //read data from default framebuffer and restore currently bound fb.
    GLint fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //    glReadPixels(0,0,out.width,out.height,GL_RGB,GL_UNSIGNED_BYTE,out.getRawData());
    glReadPixels(0,0,out.width,out.height,out.Format().getGlFormat(),out.Format().getGlType(),out.getRawData());


    glBindFramebuffer(GL_FRAMEBUFFER, fb);
}


void OpenGLWindow::readToImage(Image& out){
    int w = renderer->windowWidth;
    int h = renderer->windowHeight;

    out.width = w;
    out.height = h;
    out.Format() = ImageFormat(3,8,ImageElementFormat::UnsignedNormalized);
    out.create();

    readToExistingImage(out);
}


void OpenGLWindow::screenshot(const std::string &file)
{
    //    cout<<"Window::screenshot "<<file<<endl;

    Image img;
    readToImage(img);


    TextureLoader::instance()->saveImage(file,img);
}

void OpenGLWindow::screenshotRender(const std::string &file)
{
    //    cout<<"Window::screenshotRender "<<file<<endl;
    int w = renderer->width;
    int h = renderer->height;

    Image img;
    img.width = w;
    img.height = h;
    img.Format() = ImageFormat(3,8,ImageElementFormat::UnsignedNormalized);
    img.create();

    auto tex = getRenderer()->postProcessor.getCurrentTexture();
    tex->bind();
    glGetTexImage(tex->getTarget(),0,GL_RGB,GL_UNSIGNED_BYTE,img.getRawData());
    tex->unbind();

    TextureLoader::instance()->saveImage(file,img);
}

std::string OpenGLWindow::getTimeString()
{
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    std::string str;
    str =     std::to_string(now->tm_year + 1900) + '-'
            + std::to_string(now->tm_mon + 1) + '-'
            + std::to_string(now->tm_mday) + '_'
            + std::to_string(now->tm_hour) + '-'
            + std::to_string(now->tm_min) + '-'
            + std::to_string(now->tm_sec);

    ;

    return str;
}

void OpenGLWindow::setProgram(Program *program)
{
    renderer->renderer = program;
}

Ray OpenGLWindow::createPixelRay(const vec2 &pixel) const
{
    vec4 p = vec4(2*pixel.x/getWidth()-1.f,1.f-(2*pixel.y/getHeight()),0,1.f);
    p = glm::inverse(OpenGLWindow::currentCamera->proj)*p;
    p /= p.w;

    mat4 inverseView = glm::inverse(OpenGLWindow::currentCamera->view);
    vec3 ray_world =vec3(inverseView*p);
    vec3 origin = vec3(inverseView[3]);
    return Ray(glm::normalize(ray_world-origin),origin);
}

Ray OpenGLWindow::createPixelRay(const vec2 &pixel, const vec2& resolution, const mat4& inverseProj) const
{
    vec4 p = vec4(2*pixel.x/resolution.x-1.f,1.f-(2*pixel.y/resolution.y),0,1.f);
    p = inverseProj*p;
    p /= p.w;

    mat4 inverseView = glm::inverse(OpenGLWindow::currentCamera->view);
    vec3 ray_world =vec3(inverseView*p);
    vec3 origin = vec3(inverseView[3]);
    return Ray(glm::normalize(ray_world-origin),origin);
}

vec3 OpenGLWindow::screenToWorld(const vec2 &pixel) const
{
    vec4 p = vec4(2*pixel.x/getWidth()-1.f,1.f-(2*pixel.y/getHeight()),0,1.f);
    p = glm::inverse(OpenGLWindow::currentCamera->proj)*p;
    p /= p.w;

    mat4 inverseView = glm::inverse(OpenGLWindow::currentCamera->view);
    vec3 ray_world =vec3(inverseView*p);
    return ray_world;
}


vec3 OpenGLWindow::screenToWorld(const vec2 &pixel, const vec2& resolution, const mat4& inverseProj) const
{
    vec4 p = vec4(2*pixel.x/resolution.x-1.f,1.f-(2*pixel.y/resolution.y),0,1.f);
    p = inverseProj*p;
    p /= p.w;

    mat4 inverseView = glm::inverse(OpenGLWindow::currentCamera->view);
    vec3 ray_world =vec3(inverseView*p);
    return ray_world;
}



vec2 OpenGLWindow::projectToScreen(const vec3 &pos) const
{
    vec4 r = OpenGLWindow::currentCamera->proj * OpenGLWindow::currentCamera->view * vec4(pos,1);
    r /= r.w;

    vec2 pixel;
    pixel.x = (r.x +1.f)*getWidth() *0.5f;
    pixel.y = -(r.y - 1.f) * getHeight() * 0.5f;

    return pixel;
}

void OpenGLWindow::update(float dt)
{
    updateTimer.start();
    endParallelUpdate();
    renderer->renderer->update(dt);
    startParallelUpdate(dt);
    updateTimer.stop();

    numUpdates++;

    upsTimer.stop();
    upsTimer.start();
}




void OpenGLWindow::startParallelUpdate(float dt)
{

    if(parallelUpdate){
        semStartUpdate.notify();
    }else{
        parallelUpdateCaller(dt);
    }
}

void OpenGLWindow::endParallelUpdate()
{
    if(parallelUpdate)
        semFinishUpdate.wait();
}

void OpenGLWindow::parallelUpdateThread(float dt)
{
    semFinishUpdate.notify();
    semStartUpdate.wait();
    while(running){
        parallelUpdateCaller(dt);
        semFinishUpdate.notify();
        semStartUpdate.wait();
    }
}

void OpenGLWindow::parallelUpdateCaller(float dt)
{
    renderer->renderer->parallelUpdate(dt);
}

void OpenGLWindow::render(float dt, float interpolation)
{
    interpolationTimer.start();
    renderer->renderer->interpolate(dt,interpolation);
    interpolationTimer.stop();

    renderCPUTimer.start();
    renderer->render_intern();
    renderCPUTimer.stop();

    numFrames++;

    swapBuffersTimer.start();
    swapBuffers();
    swapBuffersTimer.stop();

    fpsTimer.stop();
    fpsTimer.start();
}



void OpenGLWindow::sleep(tick_t ticks)
{
    if(ticks > tick_t(0)){
        std::this_thread::sleep_for(ticks);
    }
}




void OpenGLWindow::startMainLoop(int updatesPerSecond, int framesPerSecond, float mainLoopInfoTime, int maxFrameSkip, bool _parallelUpdate, bool catchUp, bool _printInfoMsg)
{
    parallelUpdate = _parallelUpdate;
    printInfoMsg = _printInfoMsg;
    gameTime.printInfoMsg = printInfoMsg;
    running = true;

    cout << "> Starting the main loop..." << endl;
    cout << "> updatesPerSecond=" << updatesPerSecond << " framesPerSecond=" << framesPerSecond << " maxFrameSkip=" << maxFrameSkip << endl;


    if(updatesPerSecond <= 0)
        updatesPerSecond = gameTime.base.count();
    if(framesPerSecond <= 0)
        framesPerSecond = gameTime.base.count();


    float updateDT = 1.0f / updatesPerSecond;
    //    float framesDT = 1.0f / framesPerSecond;

    tick_t ticksPerUpdate = gameTime.base / updatesPerSecond;
    tick_t ticksPerFrame = gameTime.base / framesPerSecond;

    tick_t ticksPerInfo = std::chrono::duration_cast<tick_t>(gameTime.base * mainLoopInfoTime);

    tick_t ticksPerScreenshot = std::chrono::duration_cast<tick_t>(gameTime.base * 5.0f);

    if(windowParameters.debugScreenshotTime < 0)
        ticksPerScreenshot = std::chrono::duration_cast<tick_t>(std::chrono::hours(100000));

    gameTime.init(ticksPerUpdate,ticksPerFrame);


    tick_t nextInfoTick = gameTime.getTime();
    tick_t nextScreenshotTick = gameTime.getTime() + ticksPerScreenshot;

    if(!catchUp){
        gameTime.maxGameLoopDelay = std::chrono::duration_cast<tick_t>(std::chrono::milliseconds(1000));
    }


    if(parallelUpdate){
        updateThread = std::thread(&OpenGLWindow::parallelUpdateThread,this,updateDT);
    }


    while(true){
        checkEvents();

        if(shouldClose()){
            break;
        }

        //With this loop we are able to skip frames if the system can't keep up.
        for(int i = 0; i <= maxFrameSkip && gameTime.shouldUpdate(); ++i){
            update(updateDT);
        }

        if(gameTime.shouldRender()){
            render(updateDT,gameTime.interpolation);
        }

        if(printInfoMsg && gameTime.getTime() > nextInfoTick){
            auto gt = std::chrono::duration_cast<std::chrono::seconds>(gameTime.getTime());
            cout << "> Time: " << gt.count() << "s  Total number of updates/frames: " << numUpdates << "/" << numFrames << "  UPS/FPS: " << (1000.0f/upsTimer.getTimeMS()) << "/" << (1000.0f/fpsTimer.getTimeMS()) << endl;
            nextInfoTick += ticksPerInfo;
        }

        if(gameTime.getTime() > nextScreenshotTick){
            string file = windowParameters.debugScreenshotPath+getTimeString()+".png";
            this->screenshot(file);
            nextScreenshotTick += ticksPerScreenshot;
        }

        //sleep until the next interesting event
        sleep(gameTime.getSleepTime());
        assert_no_glerror_end_frame();
    }
    running = false;

    if(parallelUpdate){
        //cleanup the update thread
        cout << "Finished main loop. Exiting update thread." << endl;
        endParallelUpdate();
        semStartUpdate.notify();
        updateThread.join();
    }

    auto gt = std::chrono::duration_cast<std::chrono::seconds>(gameTime.getTime());
    cout << "> Main loop finished in " << gt.count() << "s  Total number of updates/frames: " << numUpdates << "/" << numFrames  << endl;
}

}
