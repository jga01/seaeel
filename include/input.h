#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include "GLFW/glfw3.h"

#include "camera.h"

#define MAX_KEYS 1024

struct Input
{
    bool keys[MAX_KEYS];
    float last_x;
    float last_y;
    bool first_mouse;
    bool capture_mouse;
    bool invert_y_axis;
    float mouse_sensitivity;
    struct Camera *camera;
};

void seel_input_init(struct Input *input, GLFWwindow *window, struct Camera *camera);
void seel_input_process(struct Input *input, GLFWwindow *window, float delta_time);
void seel_input_set_key(struct Input *input, int key, bool value);
void seel_input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void seel_input_mouse_callback(GLFWwindow *window, double xpos, double ypos);
void seel_input_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void seel_input_init(struct Input *input, GLFWwindow *window, struct Camera *camera)
{
    input->last_x = 0.0f;
    input->last_y = 0.0f;
    input->first_mouse = true;
    input->capture_mouse = true;
    input->camera = camera;

    unsigned int i;
    for (i = 0; i < MAX_KEYS; i++)
    {
        input->keys[i] = false;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, input);

    glfwSetKeyCallback(window, seel_input_key_callback);
    glfwSetCursorPosCallback(window, seel_input_mouse_callback);
    glfwSetScrollCallback(window, seel_input_scroll_callback);
}

void seel_input_process(struct Input *input, GLFWwindow *window, float delta_time)
{
    if (input->keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    if (input->keys[GLFW_KEY_R])
    {
        input->capture_mouse = !input->capture_mouse;
        glfwSetInputMode(window, GLFW_CURSOR, input->capture_mouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    if (input->keys[GLFW_KEY_W])
        seel_camera_process_keyboard(input->camera, FORWARD, delta_time);
    if (input->keys[GLFW_KEY_S])
        seel_camera_process_keyboard(input->camera, BACKWARD, delta_time);
    if (input->keys[GLFW_KEY_A])
        seel_camera_process_keyboard(input->camera, LEFT, delta_time);
    if (input->keys[GLFW_KEY_D])
        seel_camera_process_keyboard(input->camera, RIGHT, delta_time);
}

void input_set_key(struct Input *input, int key, bool value)
{
    if (key >= 0 && key < 1024)
    {
        input->keys[key] = value;
    }
}

void seel_input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    struct Input *input = (struct Input *)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS)
        input_set_key(input, key, true);
    else if (action == GLFW_RELEASE)
        input_set_key(input, key, false);
}

void seel_input_mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    struct Input *input = (struct Input *)glfwGetWindowUserPointer(window);

    if (input->first_mouse)
    {
        input->last_x = (float)xpos;
        input->last_y = (float)ypos;
        input->first_mouse = false;
    }

    float xoffset = (float)xpos - input->last_x;
    float yoffset = input->last_y - (float)ypos;
    input->last_x = (float)xpos;
    input->last_y = (float)ypos;

    if (input->capture_mouse)
        seel_camera_process_mouse_movement(input->camera, xoffset, yoffset, true);
}

void seel_input_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    struct Input *input = (struct Input *)glfwGetWindowUserPointer(window);
    seel_camera_process_mouse_scroll(input->camera, (float)yoffset);
}

#endif /* INPUT_H */