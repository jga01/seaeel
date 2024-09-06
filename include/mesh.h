#ifndef MESH_H
#define MESH_H

#include <stdio.h>

#include "cglm/cglm.h"
#include "texture.h"
#include "shader.h"
#include "bone.h"

#define MAX_UNIFORM_NAME_LEN 32

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
    vec3 tangent;
    vec3 bitangent;
    int m_bone_ids[MAX_BONE_INFLUENCE];
    float m_weights[MAX_BONE_INFLUENCE];
};

struct Mesh
{
    struct Vertex *vertices;
    unsigned int *indices;
    struct Texture *textures;
    unsigned int num_vertices;
    unsigned int num_indices;
    unsigned int num_textures;
    unsigned int VAO, VBO, EBO;
};

void seel_mesh_setup(struct Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(struct Vertex), &mesh->vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(unsigned int),
                 &mesh->indices[0], GL_STATIC_DRAW);

    /* vertex positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)0);
    /* vertex texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, tex_coords));
    /* vertex normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, normal));
    /* vertex tangent */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, tangent));
    /* vertex bitangent */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, bitangent));
    /* ids */
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(struct Vertex), (void *)offsetof(struct Vertex, m_bone_ids));
    /* weights */
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, m_weights));

    glBindVertexArray(0);
}

struct Mesh seel_mesh_init(struct Vertex *vertices,
                           unsigned int *indices,
                           struct Texture *textures,
                           unsigned int num_vertices,
                           unsigned int num_indices,
                           unsigned int num_textures)
{
    struct Mesh mesh = {0};
    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.textures = textures;
    mesh.num_vertices = num_vertices;
    mesh.num_indices = num_indices;
    mesh.num_textures = num_textures;

    seel_mesh_setup(&mesh);
    return mesh;
}

void seel_mesh_draw(struct Mesh mesh, struct Shader shader)
{
    unsigned int diffuse_nr = 1;
    unsigned int specular_nr = 1;
    unsigned int normal_nr = 1;
    unsigned int height_nr = 1;
    unsigned int emissive_nr = 1;
    unsigned int ambient_nr = 1;

    unsigned int i;
    for (i = 0; i < mesh.num_textures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); /* activate proper texture unit before binding */
        /* retrieve texture number (the N in diffuse_textureN) */
        enum TextureType type = mesh.textures[i].type;
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

        char uniform_name[MAX_UNIFORM_NAME_LEN];
        sprintf(uniform_name, "material.%s%d",
                type == DIFFUSE ? "diffuse" : type == SPECULAR ? "specular"
                                          : type == NORMAL     ? "normal"
                                          : type == EMISSIVE   ? "emissive"
                                          : type == AMBIENT    ? "ambient"
                                                               : "height",
                number);

        seel_shader_set_int(shader, uniform_name, i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }

    /* draw mesh */
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    seel_shader_set_int(shader, "material.diffuse1", 0);
    seel_shader_set_int(shader, "material.specular1", 0);
    seel_shader_set_int(shader, "material.ambient1", 0);
    seel_shader_set_int(shader, "material.height1", 0);
    seel_shader_set_int(shader, "material.emissive1", 0);
    seel_shader_set_int(shader, "material.normal1", 0);

    glActiveTexture(GL_TEXTURE0);
}

#endif /* MESH_H*/