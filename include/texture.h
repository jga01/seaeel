#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad/gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

enum texture_type
{
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT,
    EMISSIVE,
    AMBIENT,
};

typedef struct texture
{
    unsigned int id;
    enum texture_type type;
    char name[1024];
} texture;

void texture_init_stb(void)
{
    stbi_set_flip_vertically_on_load(true);
}

texture texture_create(const char *path, enum texture_type t)
{
    // printf("%s\n", path);

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);
    if (data)
    {
        unsigned int is_alpha = channels >= 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, is_alpha, width, height, 0, is_alpha, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        fprintf(stderr, "Failed to load texture!\n");
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return (texture){.id = tex, .type = t};
}

void texture_bind(texture t)
{
    glBindTexture(GL_TEXTURE_2D, t.id);
}

void texture_unbind(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* TEXTURE_H */