#ifndef MODEL_H
#define MODEL_H

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "cglm/cglm.h"

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "bone.h"

typedef struct model
{
    texture *textures_loaded;
    mesh *meshes;
    unsigned int num_textures;
    unsigned int num_meshes;
    char directory[1024];
    bool gamma_correction;
    bone_info bone_info[100];
    unsigned int bone_counter;
} model;

void set_vertex_bone_data_to_default(vertex *v)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        v->m_bone_ids[i] = -1;
        v->m_weights[i] = 0.0f;
    }
}

void set_vertex_bone_data(vertex *v, int v_id, int bone_id, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) /* check if pre incrementing breaks anything */
    {
        if (v[v_id].m_bone_ids[i] < 0)
        {
            v[v_id].m_weights[i] = weight;
            v[v_id].m_bone_ids[i] = bone_id;
            break;
        }
    }
}

void ai_matrix_to_glm_mat4(const struct aiMatrix4x4 *from, mat4 to)
{
    /* Assimp matrices are row-major, while GLM expects column-major */
		to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
		to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
		to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
		to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;
}

void extract_bone_weight_for_vertices(vertex *vertices, struct aiMesh *mesh, model *model)
{
    for (unsigned int bone_index = 0; bone_index < mesh->mNumBones; ++bone_index)
    {
        struct aiBone *bone = mesh->mBones[bone_index];
        int bone_id = -1;

        // Check if the bone is already in the model's bone map
        char *bone_name = mesh->mBones[bone_index]->mName.data;
        for (int i = 0; i < model->bone_counter; ++i)
        {
            if (strcmp(bone_name, model->bone_info[i].name) == 0)
            {
                bone_id = model->bone_info[i].id;
                break;
            }
        }

        // If not found, create a new bone entry
        if (bone_id == -1)
        {
            bone_id = model->bone_counter;
            model->bone_info[model->bone_counter].id = bone_id;
            strcpy(model->bone_info[model->bone_counter].name, bone_name);
            ai_matrix_to_glm_mat4(&bone->mOffsetMatrix, model->bone_info[model->bone_counter].offset);
            model->bone_counter++;
        }

        // Set bone weights for each vertex affected by this bone
        for (unsigned int weight_index = 0; weight_index < bone->mNumWeights; ++weight_index)
        {
            struct aiVertexWeight weight = bone->mWeights[weight_index];
            int vertex_id = weight.mVertexId;
            float weight_value = weight.mWeight;

            if (vertex_id < mesh->mNumVertices)
            {
                set_vertex_bone_data(vertices, vertex_id, bone_id, weight_value);
            }
        }
    }
}

texture *load_material_textures(model *m, struct aiMaterial *mat, enum aiTextureType type, enum texture_type t, unsigned int *n_textures)
{
    unsigned int num_textures = aiGetMaterialTextureCount(mat, type);
    texture *textures = malloc(sizeof(texture) * num_textures);
    if (!textures)
    {
        fprintf(stderr, "Failed to allocate memory for material textures!\n");
        return NULL;
    }

    for (unsigned int i = 0; i < num_textures; i++)
    {
        struct aiString str;
        aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
        /* check if texture was loaded before and if so, continue to next iteration: skip loading a new texture */
        bool skip = false;
        for (unsigned int j = 0; j < m->num_textures; j++)
        {
            if (strcmp(m->textures_loaded[j].name, str.data) == 0)
            {
                textures[i] = m->textures_loaded[j];
                skip = true; /* a texture with the same filepath has already been loaded, continue to next one. (optimization) */
                break;
            }
        }
        if (!skip)
        { /* if texture hasn't been loaded already, load it */
            char path[1024];
            strcpy(path, m->directory);
            texture texture = texture_create(strcat(path, str.data), t);
            strcpy(texture.name, str.data);
            textures[i] = texture;
            m->textures_loaded = realloc(m->textures_loaded, sizeof(struct texture) * (m->num_textures + 1));
            m->textures_loaded[m->num_textures++] = texture; /* store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures. */
        }
    }

    *n_textures += num_textures;
    return textures;
}

mesh process_mesh(model *mod, struct aiMesh *m, const struct aiScene *scene)
{
    vertex *vertices = malloc(sizeof(vertex) * m->mNumVertices);
    unsigned int *indices = NULL;
    texture *textures = NULL;

    unsigned int i;
    for (i = 0; i < m->mNumVertices; i++)
    {
        vertex vtx;
        set_vertex_bone_data_to_default(&vtx);

        vec3 vector;
        vector[0] = m->mVertices[i].x;
        vector[1] = m->mVertices[i].y;
        vector[2] = m->mVertices[i].z;
        glm_vec3_copy(vector, vtx.position);

        if (m->mNormals != NULL && m->mNumVertices > 0)
        {
            vector[0] = m->mNormals[i].x;
            vector[1] = m->mNormals[i].y;
            vector[2] = m->mNormals[i].z;
            glm_vec3_copy(vector, vtx.normal);
        }

        if (m->mTextureCoords[0])
        {
            /* texture coordinates */
            vec2 vec;
            vec[0] = m->mTextureCoords[0][i].x;
            vec[1] = m->mTextureCoords[0][i].y;
            glm_vec2_copy(vec, vtx.tex_coords);

            /* tangent */
            if (m->mTangents)
            {
                vector[0] = m->mTangents[i].x;
                vector[1] = m->mTangents[i].y;
                vector[2] = m->mTangents[i].z;

                glm_vec3_copy(vector, vtx.tangent);
            }
            else
                glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, vtx.tangent);

            /* bitangent */
            if (m->mBitangents)
            {
                vector[0] = m->mBitangents[i].x;
                vector[1] = m->mBitangents[i].y;
                vector[2] = m->mBitangents[i].z;
                glm_vec3_copy(vector, vtx.bitangent);
            }
            else
                glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, vtx.bitangent);
        }
        else
        {
            glm_vec2_copy(vtx.tex_coords, (vec2){0.0f, 0.0f});
        }
        /* process vertex positions, normals and texture coordinates */
        vertices[i] = vtx;
    }

    unsigned int j, num_indices = 0;
    for (i = 0; i < m->mNumFaces; i++)
    {
        struct aiFace face = m->mFaces[i];
        num_indices += face.mNumIndices;
    }

    indices = malloc(sizeof(unsigned int) * num_indices);

    unsigned int i_counter = 0;
    for (i = 0; i < m->mNumFaces; i++)
    {
        struct aiFace face = m->mFaces[i];
        for (j = 0; j < face.mNumIndices; j++)
            indices[i_counter++] = face.mIndices[j];
    }

    struct aiMaterial *material = scene->mMaterials[m->mMaterialIndex];

    unsigned int num_textures = 0;

    unsigned int before = num_textures;
    texture *diffuseMaps = load_material_textures(mod, material, aiTextureType_DIFFUSE, DIFFUSE, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], diffuseMaps, sizeof(texture) * (num_textures - before));
    free(diffuseMaps);

    /* 2. specular maps */
    before = num_textures;
    texture *specularMaps = load_material_textures(mod, material, aiTextureType_SPECULAR, SPECULAR, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], specularMaps, sizeof(texture) * (num_textures - before));
    free(specularMaps);

    /* 3. normal maps */
    before = num_textures;
    texture *normalMaps = load_material_textures(mod, material, aiTextureType_NORMALS, NORMAL, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], normalMaps, sizeof(texture) * (num_textures - before));
    free(normalMaps);

    /* 4. height maps */
    before = num_textures;
    texture *heightMaps = load_material_textures(mod, material, aiTextureType_HEIGHT, HEIGHT, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], heightMaps, sizeof(texture) * (num_textures - before));
    free(heightMaps);

    /* 5. emissive maps */
    before = num_textures;
    texture *emissiveMaps = load_material_textures(mod, material, aiTextureType_EMISSIVE, EMISSIVE, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], emissiveMaps, sizeof(texture) * (num_textures - before));
    free(emissiveMaps);

    /* 6. ambient maps */
    before = num_textures;
    texture *ambientMaps = load_material_textures(mod, material, aiTextureType_AMBIENT, AMBIENT, &num_textures);
    textures = realloc(textures, sizeof(texture) * num_textures);
    memcpy(&textures[before], ambientMaps, sizeof(texture) * (num_textures - before));
    free(ambientMaps);

    extract_bone_weight_for_vertices(vertices, m, mod);

    return mesh_init(vertices, indices, textures, m->mNumVertices, num_indices, num_textures);
}

void process_node(model *m, struct aiNode *node, const struct aiScene *scene)
{
    /* process all the node's meshes (if any) */
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        m->meshes = realloc(m->meshes, sizeof(struct mesh) * (m->num_meshes + 1));
        m->meshes[m->num_meshes++] = process_mesh(m, mesh, scene);
    }

    /* then do the same for each of its children */
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(m, node->mChildren[i], scene);
    }
}

model model_load(char *path)
{
    model m = {0};

    const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "Failed in loading model!\n\n%s\n", aiGetErrorString());
        return m;
    }

    char *name = strrchr(path, '/');
    unsigned int size = name - path;
    strncpy(m.directory, path, size + 1);

    process_node(&m, scene->mRootNode, scene);

    return m;
}

void model_draw(model m, shader s)
{
    unsigned int i;
    for (i = 0; i < m.num_meshes; i++)
        mesh_draw(m.meshes[i], s);
}

#endif /* MODEL_H */