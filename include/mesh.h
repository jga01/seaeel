#ifndef MESH_H
#define MESH_H

#include <stdio.h>

#include "cglm/cglm.h"
#include "texture.h"
#include "shader.h"
#include "bone.h"

typedef struct vertex
{
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
    vec3 tangent;
    vec3 bitangent;
    int m_bone_ids[MAX_BONE_INFLUENCE];
    float m_weights[MAX_BONE_INFLUENCE];
} vertex;

typedef struct mesh
{
    vertex *vertices;
    unsigned int *indices;
    texture *textures;
    unsigned int num_vertices;
    unsigned int num_indices;
    unsigned int num_textures;
    unsigned int VAO, VBO, EBO;
} mesh;

void mesh_setup(mesh *m)
{
    glGenVertexArrays(1, &m->VAO);
    glGenBuffers(1, &m->VBO);
    glGenBuffers(1, &m->EBO);

    glBindVertexArray(m->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m->VBO);
    glBufferData(GL_ARRAY_BUFFER, m->num_vertices * sizeof(vertex), &m->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->num_indices * sizeof(unsigned int),
                 &m->indices[0], GL_STATIC_DRAW);

    /* vertex positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
    /* vertex texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, tex_coords));
    /* vertex normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, normal));
    /* vertex tangent */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, tangent));
    /* vertex bitangent */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, bitangent));
    /* ids */
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(vertex), (void *)offsetof(vertex, m_bone_ids));
    /* weights */
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, m_weights));

    glBindVertexArray(0);
}

mesh mesh_init(vertex *v, unsigned int *i, texture *t, unsigned int num_vertices, unsigned int num_indices, unsigned int num_textures)
{
    mesh m = {0};
    m.vertices = v;
    m.indices = i;
    m.textures = t;
    m.num_vertices = num_vertices;
    m.num_indices = num_indices;
    m.num_textures = num_textures;

    mesh_setup(&m);
    return m;
}

void mesh_draw(mesh m, shader s)
{
    unsigned int diffuse_nr = 1;
    unsigned int specular_nr = 1;
    unsigned int normal_nr = 1;
    unsigned int height_nr = 1;
    unsigned int emissive_nr = 1;
    unsigned int ambient_nr = 1;
    for (unsigned int i = 0; i < m.num_textures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); /* activate proper texture unit before binding */
        /* retrieve texture number (the N in diffuse_textureN) */
        enum texture_type type = m.textures[i].type;
        unsigned int number = 0;
        if (type == DIFFUSE)
            number = diffuse_nr++;
        else if (type == SPECULAR)
            number = specular_nr++;
        else if (type == NORMAL)
            number = normal_nr++;
        else if (type == HEIGHT)
            number = height_nr++;
        else if (type == EMISSIVE)
            number = emissive_nr++;
        else if (type == AMBIENT)
            number = ambient_nr++;

        char uniform_name[32];
        sprintf(uniform_name, "material.%s%d", 
            type == DIFFUSE ? "diffuse" :
            type == SPECULAR ? "specular" :
            type == NORMAL ? "normal" :
            type == EMISSIVE ? "emissive" :
            type == AMBIENT ? "ambient" :
            "height", 
            number);

        shader_set_int(s, uniform_name, i);
        glBindTexture(GL_TEXTURE_2D, m.textures[i].id);
    }

    /* draw mesh */
    glBindVertexArray(m.VAO);
    glDrawElements(GL_TRIANGLES, m.num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    shader_set_int(s, "material.diffuse1", 0);
    shader_set_int(s, "material.specular1", 0);
    shader_set_int(s, "material.ambient1", 0);
    shader_set_int(s, "material.height1", 0);
    shader_set_int(s, "material.emissive1", 0);
    shader_set_int(s, "material.normal1", 0);

    glActiveTexture(GL_TEXTURE0);
}

#endif /* MESH_H*/