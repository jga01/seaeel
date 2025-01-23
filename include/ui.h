#ifndef UI_H
#define UI_H

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
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_glfw.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

struct UIManager
{
    struct nk_context *ctx;
};

void seel_ui_init(struct UIManager *manager, GLFWwindow *window);
void seel_ui_begin_frame();
void seel_ui_end_frame();
void seel_ui_cleanup();

void seel_ui_init(struct UIManager *manager, GLFWwindow *window)
{
    manager->ctx = 0;
    manager->ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
}

void seel_ui_begin_frame()
{
    nk_glfw3_new_frame();
}

void seel_ui_end_frame()
{
    nk_glfw3_render(NK_ANTI_ALIASING_ON);
}

void seel_ui_cleanup()
{
    nk_glfw3_shutdown();
}

#endif /* UI_H */