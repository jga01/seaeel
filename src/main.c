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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

camera c;
float lastx = WINDOW_WIDTH / 2.0f;
float lasty = WINDOW_HEIGHT / 2.0f;
bool first_mouse = true;

bool release_cursor = false;

int debugMode = 0;

void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void process_input(GLFWwindow *window);

int main(int argc, char **argv)
{
    GLFWwindow *window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

    model backpack = model_load("../assets/backpack/backpack.obj");
    model nanosuit = model_load("../assets/nanosuit/nanosuit.obj");
    model planet = model_load("../assets/planet/planet.obj");
    model cyborg = model_load("../assets/cyborg/cyborg.obj");
    model vampire = model_load("../assets/vampire/dancing_vampire.dae");
    model steve = model_load("../assets/steve/steve.obj");
    model banjoo = model_load("../assets/banjoo/scene.gltf");

    animation dance_animation = animation_create("../assets/vampire/dancing_vampire.dae", &vampire);
    animator animator;
    animator_create(&animator, &dance_animation);

    vec3 cubePositions[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}};

    point_light pointLights[] = {
        {.position = {0.7f, 0.2f, 2.0f},
         .ambient = {0.05f, 0.05f, 0.05f},
         .diffuse = {0.8f, 0.8f, 0.8f},
         .specular = {1.0f, 1.0f, 1.0f},
         .constant = 1.0f,
         .linear = 0.09f,
         .quadratic = 0.032f},
        {.position = {2.3f, -3.3f, -4.0f},
         .ambient = {0.05f, 0.05f, 0.05f},
         .diffuse = {0.8f, 0.8f, 0.8f},
         .specular = {1.0f, 1.0f, 1.0f},
         .constant = 1.0f,
         .linear = 0.09f,
         .quadratic = 0.032f},
        {.position = {4.0f, 2.0f, -12.0f},
         .ambient = {0.05f, 0.05f, 0.05f},
         .diffuse = {0.8f, 0.8f, 0.8f},
         .specular = {1.0f, 1.0f, 1.0f},
         .constant = 1.0f,
         .linear = 0.09f,
         .quadratic = 0.032f},
        {.position = {0.0f, 0.0f, -3.0f},
         .ambient = {0.05f, 0.05f, 0.05f},
         .diffuse = {0.8f, 0.8f, 0.8f},
         .specular = {1.0f, 1.0f, 1.0f},
         .constant = 1.0f,
         .linear = 0.09f,
         .quadratic = 0.032f}};

    struct nk_context *ctx;
    ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&atlas);
        nk_glfw3_font_stash_end();
    };

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    camera_init(&c);

    int frames = 0;
    float lastFrame = (float)glfwGetTime();

    /* Scene properties */
    struct nk_color clear_color = {51, 76, 76, 255};
    float cube_shininess = 32.0f;
    int enable_directional_light = true;
    int enable_point_lights = true;
    int enable_spot_light = true;

    int c_debug = 0;

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

        update_animation(&animator, delta_time);

        // if (c_debug < 2)
        // {
        //     for (int i = 0; i < 100; i++)
        //         glm_mat4_print(animator.final_bone_matrices[i], stdout);
        //         printf("END\n\n\n\n");
        //     c_debug++;
        // }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        glViewport(0, 0, width, height);

        process_input(window);

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

        /* Point Lights */
        int i;
        char uniformName[64];
        for (i = 0; i < 4; i++)
        {
            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].position", i);
            shader_set_vec3(colors, uniformName, pointLights[i].position);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].ambient", i);
            shader_set_vec3(colors, uniformName, pointLights[i].ambient);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].diffuse", i);
            shader_set_vec3(colors, uniformName, pointLights[i].diffuse);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].specular", i);
            shader_set_vec3(colors, uniformName, pointLights[i].specular);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].constant", i);
            shader_set_float(colors, uniformName, pointLights[i].constant);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].linear", i);
            shader_set_float(colors, uniformName, pointLights[i].linear);

            snprintf(uniformName, sizeof(uniformName), "pointLights[%d].quadratic", i);
            shader_set_float(colors, uniformName, pointLights[i].quadratic);
        }

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
        camera_get_view_matrix(&c, view);
        shader_set_mat4(colors, "view", &view[0][0]);

        /* Rendering cubes */
        glActiveTexture(GL_TEXTURE0);
        texture_bind(container_diff);
        glActiveTexture(GL_TEXTURE1);
        texture_bind(container_spec);
        glBindVertexArray(cube_vao);
        for (unsigned int i = 0; i < 10; i++)
        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, cubePositions[i]);
            if (i % 3)
                glm_rotate(model, (float)glfwGetTime() * glm_rad(50.0f), (vec3){1.0f, 0.3f, 0.5f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 3.0f});
            glm_scale(model, (vec3){0.25f, 0.25f, 0.25f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(backpack, colors);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 6.0f});
            glm_scale(model, (vec3){0.25f, 0.25f, 0.25f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(nanosuit, colors);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 9.0f});
            glm_scale(model, (vec3){0.25f, 0.25f, 0.25f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(planet, colors);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 12.0f});
            glm_scale(model, (vec3){0.25f, 0.25f, 0.25f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(cyborg, colors);
        }

        mat4 *transforms = animator.final_bone_matrices;

        for (int i = 0; i < animator.num_matrices; ++i)
        {
            char location[64];
            sprintf(location, "finalBoneMatrices[%d]", i);
            mat4 t;
            glm_mat4_copy(transforms[i], t);
            shader_set_mat4(colors, location, &t[0][0]);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 1.0f});
            glm_scale(model, (vec3){0.005f, 0.005f, 0.005f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);
            
            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(vampire, colors);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 15.0f});
            glm_scale(model, (vec3){0.001f, 0.001f, 0.001f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(steve, colors);
        }

        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            glm_translate(model, (vec3){0.0f, 0.0f, 18.0f});
            glm_scale(model, (vec3){1.0f, 1.0f, 1.0f});
            glm_rotate(model, glfwGetTime() / 10.0f, (vec3){0.0f, 1.0f, 0.0f});

            mat4 normalMatrix = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_inv(model, normalMatrix);
            glm_mat4_transpose(normalMatrix);
            shader_set_mat4(colors, "normalMatrix", &normalMatrix[0][0]);

            shader_set_mat4(colors, "model", &model[0][0]);
            model_draw(banjoo, colors);
        }

        /* Rendering lights */
        {
            shader_use(lights);
            shader_set_mat4(lights, "projection", &proj[0][0]);
            shader_set_mat4(lights, "view", &view[0][0]);
            glBindVertexArray(lightVAO);
            for (unsigned int i = 0; i < 4; i++)
            {
                mat4 model = GLM_MAT4_IDENTITY_INIT;
                glm_translate(model, pointLights[i].position);
                glm_scale(model, (vec3){0.2f, 0.2f, 0.2f});
                shader_set_mat4(lights, "model", &model[0][0]);
                shader_set_vec3(lights, "lightColor", (vec3){1.0f, 1.0f, 1.0f});
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
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

    nk_glfw3_shutdown();
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

void process_input(GLFWwindow *window)
{
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
        camera_process_keyboard(&c, FORWARD, delta_time);
    }
    if (keys[GLFW_KEY_S])
    {
        camera_process_keyboard(&c, BACKWARD, delta_time);
    }
    if (keys[GLFW_KEY_A])
    {
        camera_process_keyboard(&c, LEFT, delta_time);
    }
    if (keys[GLFW_KEY_D])
    {
        camera_process_keyboard(&c, RIGHT, delta_time);
    }
}