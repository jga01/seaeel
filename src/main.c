#include <stdio.h>
#include <math.h>

#define GLFW_INCLUDE_NONE
#include "../include/glad/gl.h"
#include "../include/GLFW/glfw3.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "../include/nuklear/nuklear.h"
#include "../include/nuklear/nuklear_glfw.h"

// #define MAX_VERTEX_BUFFER 512 * 1024
// #define MAX_ELEMENT_BUFFER 128 * 1024

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