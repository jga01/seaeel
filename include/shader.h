#ifndef SHADER_H
#define SHADER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "GLFW/glfw3.h"

#define MAX_INFO_LEN 512

enum ShaderType
{
    FRAGMENT_SHADER = 0x8B30,
    VERTEX_SHADER = 0x8B31
};

struct Shader
{
    unsigned int id;
};

static char *
seel_shader_read_file(const char *path)
{
    FILE *file;
    long file_size;
    char *buffer;
    size_t result;

    file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "Could not load shader file!\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    buffer = (char *)malloc(sizeof(char) * file_size + 1);
    if (!buffer)
    {
        fprintf(stderr, "Could not allocate memory for shader buffer!\n");
        fclose(file);
        return NULL;
    }

    result = fread(buffer, 1, file_size, file);
    if (result != file_size)
    {
        fprintf(stderr, "Failed to read file!\n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

static unsigned int seel_shader_make(const char *shader_path, int type)
{
    unsigned int shader;
    char *shader_source = seel_shader_read_file(shader_path);

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    int success;
    char info[MAX_INFO_LEN];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, MAX_INFO_LEN, NULL, info);
        fprintf(stderr, "Error in compiling %s shader!\n\n%s\n",
                type == VERTEX_SHADER ? "vertex" : "fragment",
                info);
        free(shader_source);
        return -1;
    }

    free(shader_source);
    return shader;
}

struct Shader seel_shader_create(const char *vs_path, const char *fs_path)
{
    unsigned int program;
    program = glCreateProgram();

    unsigned int vertex_shader = seel_shader_make(vs_path, VERTEX_SHADER);
    unsigned int fragment_shader = seel_shader_make(fs_path, FRAGMENT_SHADER);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success;
    char info[MAX_INFO_LEN];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, MAX_INFO_LEN, NULL, info);
        fprintf(stderr, "Error in creating shader program!\n\n, %s\n", info);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return (struct Shader){-1};
    }

    return (struct Shader){program};
}

void seel_shader_use(struct Shader *s)
{
    glUseProgram(s->id);
}

void seel_shader_delete(struct Shader *s)
{
    glDeleteProgram(s->id);
}

void seel_shader_set_int(struct Shader *s, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(s->id, name), value);
}

void seel_shader_set_float(struct Shader *s, const char *name, float value)
{
    glUniform1f(glGetUniformLocation(s->id, name), value);
}

void seel_shader_set_vec3(struct Shader *s, const char *name, float *value)
{
    glUniform3fv(glGetUniformLocation(s->id, name), 1, value);
}

void seel_shader_set_mat4(struct Shader *s, const char *name, float *value)
{
    glUniformMatrix4fv(glGetUniformLocation(s->id, name), 1, GL_FALSE, value);
}

#endif /* SHADER_H */