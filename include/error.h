#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <string.h>

#include "GLFW/glfw3.h"
#include "glad/gl.h"

void APIENTRY seel_gl_debug_output(GLenum source,
                                   GLenum type,
                                   unsigned int id,
                                   GLenum severity,
                                   GLsizei length,
                                   const char *message,
                                   const void *userParam)
{
    /* ignore non-significant error/warning codes */
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    fprintf(stderr, "---------------\n");
    fprintf(stderr, "Debug message (%d): %s\n", id, message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        fprintf(stderr, "Source: API");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        fprintf(stderr, "Source: Window System");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        fprintf(stderr, "Source: Shader Compiler");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        fprintf(stderr, "Source: Third Party");
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        fprintf(stderr, "Source: Application");
        break;
    case GL_DEBUG_SOURCE_OTHER:
        fprintf(stderr, "Source: Other");
        break;
    }
    fprintf(stderr, "\n");

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        fprintf(stderr, "Type: Error");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        fprintf(stderr, "Type: Deprecated Behaviour");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        fprintf(stderr, "Type: Undefined Behaviour");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        fprintf(stderr, "Type: Portability");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        fprintf(stderr, "Type: Performance");
        break;
    case GL_DEBUG_TYPE_MARKER:
        fprintf(stderr, "Type: Marker");
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        fprintf(stderr, "Type: Push Group");
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        fprintf(stderr, "Type: Pop Group");
        break;
    case GL_DEBUG_TYPE_OTHER:
        fprintf(stderr, "Type: Other");
        break;
    }
    fprintf(stderr, "\n\n");

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        fprintf(stderr, "Severity: high");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        fprintf(stderr, "Severity: medium");
        break;
    case GL_DEBUG_SEVERITY_LOW:
        fprintf(stderr, "Severity: low");
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        fprintf(stderr, "Severity: notification");
        break;
    }
    fprintf(stderr, "\n\n");
}

void seel_glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

#endif /* ERROR_H */