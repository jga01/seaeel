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

#include "../include/shader.h"
#include "../include/quad.h"
#include "../include/texture.h"
#include "../include/camera.h"
#include "../include/input.h"
#include "../include/light.h"
#include "../include/model.h"
#include "../include/animator.h"

#include "../include/cglm/cglm.h"

float delta_time = 0.0f;
float last_time = 0.0f;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

camera c;
float lastx = WINDOW_WIDTH / 2.0f;
float lasty = WINDOW_HEIGHT / 2.0f;
bool first_mouse = true;

bool release_cursor = false;
int debugMode = 0;

animator animator_vampire;
animator animator_mannequin;

void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void glfw_error_callback(int error, const char *description);

void process_input(GLFWwindow *window, float delta_time);

struct
{
    model *m;
    vec3 position;
    animation current_animation;
    animation animations[2];
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

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(glfw_error_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(0);

    triangle_init();
    quad_init();
    cube_init();

    shader colors = program_create("../shaders/colors.vs", "../shaders/colors.fs");
    shader lights = program_create("../shaders/lights.vs", "../shaders/lights.fs");

    texture_init_stb();
    texture container_diff = texture_create("../assets/container2.png", DIFFUSE);
    texture container_spec = texture_create("../assets/container2_specular.png", SPECULAR);

    model vampire = model_load("../assets/vampire/dancing_vampire.dae");
    animation dance_animation = animation_create("../assets/vampire/dancing_vampire.dae", &vampire);
    animator_create(&animator_vampire);

    model mannequin = model_load("../assets/mannequin/mannequin.dae");
    animation walking_animation = animation_create("../assets/mannequin/Walking.dae", &mannequin);
    animation idle_animation = animation_create("../assets/mannequin/Idle.dae", &mannequin);
    animator_create(&animator_mannequin);

    character.m = &mannequin;

    model arena = model_load("../assets/geonosis_arena/scene.gltf");

    struct nk_context *ctx;
    ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    };

    camera_init(&c);

    int frames = 0;
    float lastFrame = (float)glfwGetTime();

    /* Scene properties */
    struct nk_color clear_color = {51, 76, 76, 255};
    float cube_shininess = 32.0f;
    int enable_directional_light = true;
    int enable_point_lights = false;
    int enable_spot_light = true;

    int c_debug = 0;

    play_animation(&animator_vampire, &dance_animation, true);
    play_animation(&animator_mannequin, &idle_animation, true);

    character.current_animation = idle_animation;
    character.animations[0] = idle_animation;
    character.animations[1] = walking_animation;

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

        update_animation(&animator_vampire, delta_time);
        update_animation(&animator_mannequin, delta_time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        glViewport(0, 0, width, height);

        process_input(window, delta_time);

        /* Rendering my stuff */
        glClearColor(clear_color.r / 255.0f, clear_color.g / 255.0f, clear_color.b / 255.0f, clear_color.a / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shader_use(colors);
        shader_set_vec3(colors, "viewPos", c.position);
        shader_set_float(colors, "material.shininess", cube_shininess);
        shader_set_int(colors, "material.diffuse1", 0);
        shader_set_int(colors, "material.specular1", 1);

        shader_set_int(colors, "enableDirectionalLight", enable_directional_light);
        shader_set_int(colors, "enablePointLights", enable_point_lights);
        shader_set_int(colors, "enableSpotLight", enable_spot_light);
        shader_set_int(colors, "debugLighting", debugMode);

        /* Directional Light */
        shader_set_vec3(colors, "dirLight.direction", (vec3){-0.2f, -1.0f, -0.3f});
        shader_set_vec3(colors, "dirLight.ambient", (vec3){clear_color.r / 255.0f / 10.0f, clear_color.g / 255.0f / 10.0f, clear_color.b / 255.0f / 10.0f});
        shader_set_vec3(colors, "dirLight.diffuse", (vec3){clear_color.r / 255.0f * 0.8f, clear_color.g / 255.0f * 0.8f, clear_color.b / 255.0f * 0.8f});
        shader_set_vec3(colors, "dirLight.specular", (vec3){clear_color.r / 255.0f, clear_color.g / 255.0f, clear_color.b / 255.0f});

        /* Spotlight */
        shader_set_vec3(colors, "spotLight.position", c.position);
        shader_set_vec3(colors, "spotLight.direction", c.front);
        shader_set_vec3(colors, "spotLight.ambient", (vec3){0.0f, 0.0f, 0.0f});
        shader_set_vec3(colors, "spotLight.diffuse", (vec3){1.0f, 1.0f, 1.0f});
        shader_set_vec3(colors, "spotLight.specular", (vec3){1.0f, 1.0f, 1.0f});
        shader_set_float(colors, "spotLight.constant", 1.0f);
        shader_set_float(colors, "spotLight.linear", 0.09f);
        shader_set_float(colors, "spotLight.quadratic", 0.032f);
        shader_set_float(colors, "spotLight.cutOff", cosf(glm_rad(12.5f)));
        shader_set_float(colors, "spotLight.outerCutOff", cosf(glm_rad(15.0f)));

        mat4 proj = GLM_MAT4_IDENTITY_INIT;
        glm_perspective(glm_rad(c.zoom), (float)width / (float)height, c.near_clip, c.far_clip, proj);
        shader_set_mat4(colors, "projection", &proj[0][0]);

        mat4 view = GLM_MAT4_IDENTITY_INIT;
        // camera_get_view_matrix(&c, view);
        vec3 target;
        glm_vec3_add(character.position, (vec3){0.0f, 1.5f, -2.0f}, c.position);
        glm_vec3_copy(character.position, target);
        glm_lookat(c.position, target, c.up, view);
        shader_set_mat4(colors, "view", &view[0][0]);

        // /* Arena */
        // {
        //     mat4 model = GLM_MAT4_IDENTITY_INIT;
        //     glm_scale(model, (vec3){0.01f, 0.01f, 0.01f});

        //     mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
        //     glm_mat4_inv(model, normalMatrix);
        //     glm_mat4_transpose(normalMatrix);
        //     shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

        //     shader_set_int(colors, "animate", 0);
        //     shader_set_mat4(colors, "model", &model[0][0]);
        //     model_draw(arena, colors);
        // }

        /* Vampire */
        mat4 *transforms = animator_vampire.final_bone_matrices;

        for (int i = 0; i < animator_vampire.num_matrices; ++i)
        {
            char location[64] = {0};
            sprintf(location, "finalBonesMatrices[%d]", i);
            mat4 t;
            glm_mat4_copy(transforms[i], t);
            shader_set_mat4(colors, location, &t[0][0]);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 1.0f});
            glm_scale(model, (vec3){0.005f, 0.005f, 0.005f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_int(colors, "animate", 1);
            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(vampire, colors);
        }

        /* Mannequin */
        transforms = animator_mannequin.final_bone_matrices;

        for (int i = 0; i < animator_mannequin.num_matrices; ++i)
        {
            char location[64] = {0};
            sprintf(location, "finalBonesMatrices[%d]", i);
            mat4 t;
            glm_mat4_copy(transforms[i], t);
            shader_set_mat4(colors, location, &t[0][0]);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, character.position);
            glm_scale(model, (vec3){0.005f, 0.005f, 0.005f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_int(colors, "animate", 1);
            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(mannequin, colors);
        }
        /* End of rendering my stuff */

        /* Rendering Nuklear stuff */
        nk_glfw3_new_frame();

        if (nk_begin(ctx, "Controls", nk_rect(50, 50, 230, 250),
                     NK_WINDOW_BORDER |
                         NK_WINDOW_MOVABLE |
                         NK_WINDOW_TITLE |
                         NK_WINDOW_MINIMIZABLE |
                         NK_WINDOW_SCALABLE))
        {
            nk_layout_row_dynamic(ctx, 0, 1);
            nk_labelf(ctx, NK_TEXT_CENTERED, "XYZ: %.1f %.1f %.1f FPS: %f", c.position[0], c.position[1], c.position[2], framerate);
            if (nk_tree_push(ctx, NK_TREE_TAB, "Scene", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 0, 2);
                nk_label(ctx, "Clear Color", NK_TEXT_CENTERED);
                if (nk_combo_begin_color(ctx, clear_color, nk_vec2(200, 200)))
                {
                    float ratios[] = {0.15f, 0.85f};
                    nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratios);
                    nk_label(ctx, "R:", NK_TEXT_LEFT);
                    clear_color.r = (nk_byte)nk_slide_float(ctx, 0, clear_color.r, 255, 5);
                    nk_label(ctx, "G:", NK_TEXT_LEFT);
                    clear_color.g = (nk_byte)nk_slide_int(ctx, 0, clear_color.g, 255, 5);
                    nk_label(ctx, "B:", NK_TEXT_LEFT);
                    clear_color.b = (nk_byte)nk_slide_int(ctx, 0, clear_color.b, 255, 5);
                    nk_label(ctx, "A:", NK_TEXT_LEFT);
                    clear_color.a = (nk_byte)nk_slide_int(ctx, 0, clear_color.a, 255, 5);
                    nk_combo_end(ctx);
                }
                nk_labelf(ctx, NK_TEXT_CENTERED, "Debug Mode: %d", debugMode);
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_TAB, "Cube", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 0, 2);
                nk_label(ctx, "Cube shininess", NK_TEXT_CENTERED);
                nk_slider_float(ctx, 2.0f, &cube_shininess, 256.0f, 1.0f);
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_TAB, "Directional Light", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_checkbox_label(ctx, "Enable Directional Light", &enable_directional_light);
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_TAB, "Point Lights", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_checkbox_label(ctx, "Enable Point Light", &enable_point_lights);
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_TAB, "Spot Light", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_checkbox_label(ctx, "Enable Spot Light", &enable_spot_light);
                nk_tree_pop(ctx);
            }
        }
        nk_end(ctx);

        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        /* End of rendering Nuklear stuff */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shader_delete(colors);
    shader_delete(lights);
    triangle_cleanup();
    quad_cleanup();
    cube_cleanup();

    // nk_glfw3_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow *window, double xposin, double yposin)
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
        camera_process_mouse_movement(&c, (float)xoffset, (float)yoffset, true);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera_process_mouse_scroll(&c, (float)yoffset);
}

void process_input(GLFWwindow *window, float delta_time)
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
        character.position[2] += 0.75f * delta_time;
        c.position[2] += 0.75f * delta_time;
        play_animation(&animator_mannequin, &character.animations[1], true);
        idle = false;
    }
    if (keys[GLFW_KEY_S])
    {
        character.position[2] -= 0.75f * delta_time;
        c.position[2] -= 0.75f * delta_time;
        play_animation(&animator_mannequin, &character.animations[1], true);
        idle = false;
    }
    if (keys[GLFW_KEY_A])
    {
        camera_process_keyboard(&c, LEFT, delta_time);
    }
    if (keys[GLFW_KEY_D])
    {
        camera_process_keyboard(&c, RIGHT, delta_time);
    }

    if (idle) {
		play_animation(&animator_mannequin, &character.animations[0], true);
	}
}

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}