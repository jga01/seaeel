#include <stdio.h>
#include <math.h>

#define GLFW_INCLUDE_NONE
#include "../include/glad/gl.h"
#include "../include/GLFW/glfw3.h"

#include "../include/engine.h"
#include "../include/asset_manager.h"

int main(int argc, char **argv)
{
    struct Engine engine;
    seel_engine_init(&engine);

    while (!glfwWindowShouldClose(engine.window))
    {
        seel_engine_update(&engine);
    }

    seel_engine_cleanup(&engine);
    return 0;
}