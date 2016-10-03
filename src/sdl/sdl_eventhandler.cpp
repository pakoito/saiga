#include "saiga/sdl/sdl_eventhandler.h"

void SDL_EventHandler::update(){
    //Handle events on queue
    SDL_Event e;
    while( SDL_PollEvent( &e ) != 0 )
    {
        //User requests quit
        if( e.type == SDL_QUIT )
        {
            quit = true;
        }
        if (e.type == SDL_KEYDOWN)
        {
            keyPressed(e.key.keysym);
        }
        if (e.type == SDL_KEYUP)
        {
            keyReleased(e.key.keysym);
        }
        /* If the mouse is moving */
        if (e.type == SDL_MOUSEMOTION)
        {
            mouseMoved(e.motion.x,e.motion.y);
        }
        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            int key = e.button.button;
            mousePressed(key,e.button.x,e.button.y);
        }
        if (e.type == SDL_MOUSEBUTTONUP)
        {
            int key = e.button.button;
            mouseReleased(key,e.button.x,e.button.y);
        }
    }
}

void SDL_EventHandler::keyPressed(const SDL_Keysym &key){
   keyboard.setKeyState(key.scancode,1);
    for(SDL_KeyListener* listener : keyListener){
        listener->keyPressed(key);
    }
}

void SDL_EventHandler::keyReleased(const SDL_Keysym &key){
    keyboard.setKeyState(key.scancode,0);
    for(SDL_KeyListener* listener : keyListener){
        listener->keyReleased(key);
    }
}

void SDL_EventHandler::mouseMoved(int x, int y){
    mouse.setPosition(glm::ivec2(x,y));
    for(SDL_MouseListener* listener : mouseListener){
        listener->mouseMoved(x,y);
    }
}

void SDL_EventHandler::mousePressed(int key, int x, int y){
    mouse.setKeyState(key,1);
    for(SDL_MouseListener* listener : mouseListener){
        listener->mousePressed(key,x,y);
    }
}

void SDL_EventHandler::mouseReleased(int key, int x, int y){
    mouse.setKeyState(key,0);
    for(SDL_MouseListener* listener : mouseListener){
        listener->mouseReleased(key,x,y);
    }
}
