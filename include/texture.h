#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad/gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "error.h"

#define MAX_TEXTURE_NAME_LEN 1024

enum TextureType
{
    DIFFUSE,
    SPECULAR,
    NORMAL,
    HEIGHT,
    EMISSIVE,
    AMBIENT,
};

struct Texture
{
    unsigned int id;
    enum TextureType type;
    char name[MAX_TEXTURE_NAME_LEN];
};

void seel_texture_init_stb(void)
{
    stbi_set_flip_vertically_on_load(true);
}

struct Texture seel_texture_create(const char *path, enum TextureType type)
{
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
        fprintf(stderr, "Failed to load texture %s!\n", path);
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return (struct Texture){.id = tex, .type = type};
}

void seel_texture_bind(struct Texture t)
{
    glBindTexture(GL_TEXTURE_2D, t.id);
}

void seel_texture_unbind(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* TEXTURE_H */