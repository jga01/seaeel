#ifndef QUAD_H
#define QUAD_H

#include "glad/gl.h"

unsigned int triangle_vbo, triangle_vao;
unsigned int quad_vbo, quad_ebo, quad_vao;
unsigned int cube_vbo, cube_vao;

float triangle[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f};

float quad[] = {
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

float cube[] = {
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f};

void triangle_init(void)
{

    glGenBuffers(1, &triangle_vbo);
    glGenVertexArrays(1, &triangle_vao);

    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void quad_init(void)
{
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glGenBuffers(1, &quad_vbo);
    glGenBuffers(1, &quad_ebo);
    glGenVertexArrays(1, &quad_vao);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void cube_init(void)
{
    glGenBuffers(1, &cube_vbo);
    glGenVertexArrays(1, &cube_vao);

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void triangle_draw(void)
{
    glBindVertexArray(triangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void quad_draw(void)
{
    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void cube_draw(void)
{
    glBindVertexArray(cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void triangle_cleanup(void)
{
    glDeleteVertexArrays(1, &triangle_vao);
    glDeleteBuffers(1, &triangle_vbo);
}

void quad_cleanup(void)
{
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
}

void cube_cleanup(void)
{
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);
}

#endif /* QUAD_H */