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

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "../include/error.h"
#include "../include/shader.h"
#include "../include/texture.h"
#include "../include/camera.h"
#include "../include/input.h"
#include "../include/light.h"
#include "../include/model.h"
#include "../include/animator.h"
#include "../include/text.h"

#include "../include/cglm/cglm.h"

float delta_time = 0.0f;
float last_time = 0.0f;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct Camera c;
float lastx = WINDOW_WIDTH / 2.0f;
float lasty = WINDOW_HEIGHT / 2.0f;
bool first_mouse = true;

bool release_cursor = false;
int debugMode = 0;

struct Animator animator_vampire;
struct Animator animator_vampire;

void seel_mouse_callback(GLFWwindow *window, double xpos, double ypos);
void seel_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void seel_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void glfw_error_callback(int error, const char *description);

void seel_process_input(GLFWwindow *window, float delta_time);

struct
{
    struct Model *m;
    vec3 position;
    struct Animation current_animation;
    struct Animation animations[3];
} character = {0};

int main(int argc, char **argv)
{
    GLFWwindow *window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World!", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to load GLAD!\n");
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, seel_mouse_callback);
    glfwSetScrollCallback(window, seel_scroll_callback);
    glfwSetKeyCallback(window, seel_key_callback);
    glfwSetErrorCallback(glfw_error_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(0);

    struct nk_context *ctx;

    c = seel_camera_create();
    seel_texture_init_stb();
    seel_freetype_init();
    // ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    // {
    //     struct nk_font_atlas *atlas;
    //     nk_glfw3_font_stash_begin(&atlas);
    //     nk_glfw3_font_stash_end();
    // };

    struct Shader dft = seel_shader_create("../shaders/default.vert", "../shaders/default.frag");
    struct Shader lights = seel_shader_create("../shaders/lights.vert", "../shaders/lights.frag");
    struct Shader text = seel_shader_create("../shaders/text.vert", "../shaders/text.frag");

    struct Model vampire = seel_model_load("../assets/models/vampire/dancing_vampire.dae");
    struct Animation dancing_animation = seel_animation_create("../assets/models/vampire/dancing_vampire.dae", &vampire);
    seel_animator_create(&animator_vampire);
    character.m = &vampire;

    int frames = 0;
    float lastFrame = (float)glfwGetTime();

    /* Scene properties */
    struct nk_color clear_color = {55, 81, 55, 255};
    float cube_shininess = 32.0f;
    int enable_directional_light = true;
    int enable_point_lights = false;
    int enable_spot_light = true;

    seel_play_animation(&animator_vampire, &dancing_animation, true);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        delta_time = currentFrame - last_time;
        last_time = currentFrame;
        frames++;
        float framerate;

        if (currentFrame - lastFrame >= 1.0)
        {
            framerate = frames / (currentFrame - lastFrame);

            lastFrame = currentFrame;
            frames = 0;
        }

        seel_update_animation(&animator_vampire, delta_time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        glViewport(0, 0, width, height);

        seel_process_input(window, delta_time);

        /* Rendering my stuff */
        glClearColor(clear_color.r / 255.0f, clear_color.g / 255.0f, clear_color.b / 255.0f, clear_color.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(seel_gl_debug_output, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float tex_start_y = (float)height - 10.0f;
        seel_render_text(text, "Seaeel Engine 0.1v", 10.0f, tex_start_y - 25.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, width, height);
        char fps_text[32];
        sprintf(fps_text, "Framerate: %.f", framerate);
        seel_render_text(text, fps_text, 10.0f, tex_start_y - 50.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, width, height);

        seel_shader_use(dft);
        seel_shader_set_vec3(dft, "viewPos", c.position);
        seel_shader_set_float(dft, "material.shininess", cube_shininess);
        seel_shader_set_int(dft, "material.diffuse1", 0);
        seel_shader_set_int(dft, "material.specular1", 1);

        seel_shader_set_int(dft, "enableDirectionalLight", enable_directional_light);
        seel_shader_set_int(dft, "enablePointLights", enable_point_lights);
        seel_shader_set_int(dft, "enableSpotLight", enable_spot_light);
        seel_shader_set_int(dft, "debugLighting", debugMode);

        /* Directional Light */
        seel_shader_set_vec3(dft, "dirLight.direction", (vec3){-0.2f, -1.0f, -0.3f});
        seel_shader_set_vec3(dft, "dirLight.ambient", (vec3){clear_color.r / 255.0f / 10.0f, clear_color.g / 255.0f / 10.0f, clear_color.b / 255.0f / 10.0f});
        seel_shader_set_vec3(dft, "dirLight.diffuse", (vec3){clear_color.r / 255.0f * 0.8f, clear_color.g / 255.0f * 0.8f, clear_color.b / 255.0f * 0.8f});
        seel_shader_set_vec3(dft, "dirLight.specular", (vec3){clear_color.r / 255.0f, clear_color.g / 255.0f, clear_color.b / 255.0f});

        /* Spotlight */
        seel_shader_set_vec3(dft, "spotLight.position", c.position);
        seel_shader_set_vec3(dft, "spotLight.direction", c.front);
        seel_shader_set_vec3(dft, "spotLight.ambient", (vec3){0.0f, 0.0f, 0.0f});
        seel_shader_set_vec3(dft, "spotLight.diffuse", (vec3){1.0f, 1.0f, 1.0f});
        seel_shader_set_vec3(dft, "spotLight.specular", (vec3){1.0f, 1.0f, 1.0f});
        seel_shader_set_float(dft, "spotLight.constant", 1.0f);
        seel_shader_set_float(dft, "spotLight.linear", 0.09f);
        seel_shader_set_float(dft, "spotLight.quadratic", 0.032f);
        seel_shader_set_float(dft, "spotLight.cutOff", cosf(glm_rad(12.5f)));
        seel_shader_set_float(dft, "spotLight.outerCutOff", cosf(glm_rad(15.0f)));

        mat4 proj = GLM_MAT4_IDENTITY_INIT;
        glm_perspective(glm_rad(c.zoom), (float)width / (float)height, c.near_clip, c.far_clip, proj);
        seel_shader_set_mat4(dft, "projection", &proj[0][0]);

        mat4 view = GLM_MAT4_IDENTITY_INIT;
        seel_camera_get_view_matrix(&c, view);
        seel_shader_set_mat4(dft, "view", &view[0][0]);

        mat4 *transforms = NULL;

        /* vampire */
        transforms = animator_vampire.final_bone_matrices;

        for (int i = 0; i < animator_vampire.num_matrices; i++)
        {
            char location[64] = {0};
            sprintf(location, "finalBonesMatrices[%d]", i);
            mat4 t;
            glm_mat4_copy(transforms[i], t);
            seel_shader_set_mat4(dft, location, &t[0][0]);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            // glm_translate(model, character.position);
            glm_scale(model, (vec3){0.01f, 0.01f, 0.01f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            seel_shader_set_mat4(dft, "normalMatrix", &normalMatrix[0][0]);

            seel_shader_set_int(dft, "animate", 1);
            seel_shader_set_mat4(dft, "model", &model[0][0]);
            seel_model_draw(vampire, dft);
        }
        /* End of rendering my stuff */

        /* Rendering Nuklear stuff */
        // nk_glfw3_new_frame();
        // nk_glfw3_render(NK_ANTI_ALIASING_ON);
        /* End of rendering Nuklear stuff */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    seel_shader_delete(dft);
    seel_shader_delete(lights);
    seel_shader_delete(text);

    // nk_glfw3_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void seel_mouse_callback(GLFWwindow *window, double xposin, double yposin)
{
    float xpos = (float)xposin;
    float ypos = (float)yposin;

    if (first_mouse)
    {
        lastx = xpos;
        lasty = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - lastx;
    float yoffset = lasty - ypos;
    lastx = xpos;
    lasty = ypos;

    if (!release_cursor)
        seel_camera_process_mouse_movement(&c, (float)xoffset, (float)yoffset, true);
}

void seel_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void seel_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    seel_camera_process_mouse_scroll(&c, (float)yoffset);
}

void seel_process_input(GLFWwindow *window, float delta_time)
{
    bool idle = true;

    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    if (keys[GLFW_KEY_R])
    {
        release_cursor = !release_cursor;
        glfwSetInputMode(window, GLFW_CURSOR, release_cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    if (keys[GLFW_KEY_TAB])
    {
        debugMode = (debugMode + 1) % 4;
    }

    if (keys[GLFW_KEY_W])
    {
        seel_camera_process_keyboard(&c, FORWARD, delta_time);
        idle = false;
    }
    if (keys[GLFW_KEY_S])
    {
        character.position[2] -= 0.75f * delta_time;
        seel_camera_process_keyboard(&c, BACKWARD, delta_time);
        idle = false;
    }
    if (keys[GLFW_KEY_A])
    {
        seel_camera_process_keyboard(&c, LEFT, delta_time);
    }
    if (keys[GLFW_KEY_D])
    {
        seel_camera_process_keyboard(&c, RIGHT, delta_time);
    }

    if (idle)
    {
    }
}

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}