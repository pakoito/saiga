/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include <saiga/util/joystick.h>
#include <saiga/config.h>

namespace Saiga {

struct SAIGA_GLOBAL glfw_Joystick {
private:
    static int joystickId;

public:


    static void update();

    static void enableFirstJoystick();

    static void joystick_callback(int joy, int event);


    static bool isEnabled(){return joystickId != -1;}

};

}
