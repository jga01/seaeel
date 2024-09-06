#ifndef TEXT_H
#define TEXT_H

#include <stdio.h>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "cglm/cglm.h"
#include "glad/gl.h"
#include "shader.h"

#define DEFAULT_FONT "../assets/fonts/arial.ttf"

struct Character
{
    unsigned int texture_id;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
};

#define CHARACTERS_SIZE 128
struct Character characters[CHARACTERS_SIZE];

int seel_freetype_init(void);
int seel_generate_characters(FT_Face face);
void seel_render_text(struct Shader shader, const char *text, float x, float y, float scale, vec3 color, int width, int height);

int seel_freetype_init(void)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "Failed to initialize FreeType!\n");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, DEFAULT_FONT, 0, &face))
    {
        fprintf(stderr, "Failed to load font!\n");
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    seel_generate_characters(face);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void seel_render_text(struct Shader shader, const char *text, float x, float y, float scale, vec3 color, int width, int height)
{
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    glm_ortho(0.0f, width, 0.0f, height, 0.0f, 1.0f, projection);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    seel_shader_use(shader);
    seel_shader_set_vec3(shader, "textColor", (vec3){color[0], color[1], color[2]});
    seel_shader_set_mat4(shader, "projection", &projection[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    const char *ptr = text;
    unsigned int text_size = strlen(text);

    unsigned int i;
    for (i = 0; i < text_size; i++)
    {
        unsigned char c = *(ptr++);

        struct Character ch = characters[c];

        float xpos = x + ch.bearing[0] * scale;
        float ypos = y - (ch.size[1] - ch.bearing[1]) * scale;

        float w = ch.size[0] * scale;
        float h = ch.size[1] * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

int seel_generate_characters(FT_Face face)
{
    unsigned char c;
    for (c = 0; c < CHARACTERS_SIZE; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            fprintf(stderr, "Failed to load glyph!\n");
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        struct Character ch = {
            .texture_id = texture,
            .size = {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            .bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top},
            .advance = face->glyph->advance.x};

        characters[c] = ch;
    }
}

#endif /* TEXT_H*/