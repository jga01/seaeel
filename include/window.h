#ifndef WINDOW_H
#define WINDOW_H

#include <stdio.h>
#include <stdbool.h>

#include "GLFW/glfw3.h"

GLFWwindow *seel_create_window(int width, int height, const char *title)
{
    GLFWwindow *window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to load GLAD!\n");
        glfwTerminate();
        return NULL;
    }

    glfwSwapInterval(0);

    return window;
}

void seel_destroy_window(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

#endif /* WINDOW_H */