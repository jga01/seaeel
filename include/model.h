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

#define MAX_DIRECTORY_LEN 1024
#define MAX_BONES 100

struct Model
{
    struct Texture *textures_loaded;
    struct Mesh *meshes;
    unsigned int num_textures;
    unsigned int num_meshes;
    char name[256];
    char directory[MAX_DIRECTORY_LEN];
    struct BoneInfo bone_info[MAX_BONES];
    unsigned int bone_counter;
    bool animated;
};

void seel_set_vertex_bone_data_to_default(struct Vertex *vertex)
{
    unsigned int i;
    for (i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex->m_bone_ids[i] = -1;
        vertex->m_weights[i] = 0.0f;
    }
}

void seel_set_vertex_bone_data(struct Vertex *vertex, int vertex_id, int bone_id, float weight)
{
    unsigned int i;
    for (i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (vertex[vertex_id].m_bone_ids[i] < 0)
        {
            vertex[vertex_id].m_weights[i] = weight;
            vertex[vertex_id].m_bone_ids[i] = bone_id;
            break;
        }
    }
}

void seel_ai_matrix_to_glm_mat4(const struct aiMatrix4x4 *from, mat4 to)
{
    to[0][0] = from->a1;
    to[1][0] = from->a2;
    to[2][0] = from->a3;
    to[3][0] = from->a4;
    to[0][1] = from->b1;
    to[1][1] = from->b2;
    to[2][1] = from->b3;
    to[3][1] = from->b4;
    to[0][2] = from->c1;
    to[1][2] = from->c2;
    to[2][2] = from->c3;
    to[3][2] = from->c4;
    to[0][3] = from->d1;
    to[1][3] = from->d2;
    to[2][3] = from->d3;
    to[3][3] = from->d4;
}

void seel_extract_bone_weight_for_vertices(struct Vertex *vertices, struct aiMesh *mesh, struct Model *model)
{
    unsigned int bone_index;
    for (bone_index = 0; bone_index < mesh->mNumBones; bone_index++)
    {
        struct aiBone *bone = mesh->mBones[bone_index];
        int bone_id = -1;

        /* Check if the bone is already in the model's bone map */
        char *bone_name = mesh->mBones[bone_index]->mName.data;
        unsigned int i;
        for (i = 0; i < model->bone_counter; i++)
        {
            if (strcmp(bone_name, model->bone_info[i].name) == 0)
            {
                bone_id = model->bone_info[i].id;
                break;
            }
        }

        /* If not found, create a new bone entry */
        if (bone_id == -1)
        {
            bone_id = model->bone_counter;
            model->bone_info[model->bone_counter].id = bone_id;
            strcpy(model->bone_info[model->bone_counter].name, bone_name);
            seel_ai_matrix_to_glm_mat4(&bone->mOffsetMatrix, model->bone_info[model->bone_counter].offset);
            model->bone_counter++;
        }

        // Set bone weights for each vertex affected by this bone
        unsigned int weight_index;
        for (weight_index = 0; weight_index < bone->mNumWeights; weight_index++)
        {
            struct aiVertexWeight weight = bone->mWeights[weight_index];
            int vertex_id = weight.mVertexId;
            float weight_value = weight.mWeight;

            if (vertex_id < mesh->mNumVertices)
            {
                seel_set_vertex_bone_data(vertices, vertex_id, bone_id, weight_value);
            }
        }
    }
}

struct Texture *seel_load_material_textures(struct Model *m, struct aiMaterial *mat, enum aiTextureType type, enum TextureType tex_type, unsigned int *n_textures)
{
    unsigned int num_textures = aiGetMaterialTextureCount(mat, type);
    struct Texture *textures = malloc(sizeof(struct Texture) * num_textures);
    if (!textures)
    {
        fprintf(stderr, "Failed to allocate memory for material textures!\n");
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < num_textures; i++)
    {
        struct aiString str;
        aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);
        /* check if texture was loaded before and if so, continue to next iteration: skip loading a new texture */
        bool skip = false;
        unsigned int j;
        for (j = 0; j < m->num_textures; j++)
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
            char path[MAX_DIRECTORY_LEN];
            strcpy(path, m->directory);
            struct Texture texture = seel_texture_create(strcat(path, str.data), tex_type);
            strcpy(texture.name, str.data);
            textures[i] = texture;
            m->textures_loaded = realloc(m->textures_loaded, sizeof(struct Texture) * (m->num_textures + 1));
            m->textures_loaded[m->num_textures++] = texture; /* store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures. */
        }
    }

    *n_textures += num_textures;
    return textures;
}

struct Mesh seel_process_mesh(struct Model *model, struct aiMesh *mesh, const struct aiScene *scene)
{
    struct Vertex *vertices = malloc(sizeof(struct Vertex) * mesh->mNumVertices);
    unsigned int *indices = NULL;
    struct Texture *textures = NULL;

    unsigned int i;
    for (i = 0; i < mesh->mNumVertices; i++)
    {
        struct Vertex vertex;
        seel_set_vertex_bone_data_to_default(&vertex);

        vec3 vector;
        vector[0] = mesh->mVertices[i].x;
        vector[1] = mesh->mVertices[i].y;
        vector[2] = mesh->mVertices[i].z;
        glm_vec3_copy(vector, vertex.position);

        if (mesh->mNormals != NULL && mesh->mNumVertices > 0)
        {
            vector[0] = mesh->mNormals[i].x;
            vector[1] = mesh->mNormals[i].y;
            vector[2] = mesh->mNormals[i].z;
            glm_vec3_copy(vector, vertex.normal);
        }

        if (mesh->mTextureCoords[0])
        {
            /* texture coordinates */
            vec2 vec;
            vec[0] = mesh->mTextureCoords[0][i].x;
            vec[1] = mesh->mTextureCoords[0][i].y;
            glm_vec2_copy(vec, vertex.tex_coords);

            /* tangent */
            if (mesh->mTangents)
            {
                vector[0] = mesh->mTangents[i].x;
                vector[1] = mesh->mTangents[i].y;
                vector[2] = mesh->mTangents[i].z;

                glm_vec3_copy(vector, vertex.tangent);
            }
            else
                glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, vertex.tangent);

            /* bitangent */
            if (mesh->mBitangents)
            {
                vector[0] = mesh->mBitangents[i].x;
                vector[1] = mesh->mBitangents[i].y;
                vector[2] = mesh->mBitangents[i].z;
                glm_vec3_copy(vector, vertex.bitangent);
            }
            else
                glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, vertex.bitangent);
        }
        else
        {
            glm_vec2_copy(vertex.tex_coords, (vec2){0.0f, 0.0f});
        }
        /* process vertex positions, normals and texture coordinates */
        vertices[i] = vertex;
    }

    unsigned int num_indices = 0;
    for (i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        num_indices += face.mNumIndices;
    }

    indices = malloc(sizeof(unsigned int) * num_indices);

    unsigned int i_counter = 0;
    for (i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        unsigned int j;
        for (j = 0; j < face.mNumIndices; j++)
            indices[i_counter++] = face.mIndices[j];
    }

    struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    unsigned int num_textures = 0;

    unsigned int before = num_textures;
    struct Texture *diffuseMaps = seel_load_material_textures(model, material, aiTextureType_DIFFUSE, DIFFUSE, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], diffuseMaps, sizeof(struct Texture) * (num_textures - before));
    free(diffuseMaps);

    /* 2. specular maps */
    before = num_textures;
    struct Texture *specularMaps = seel_load_material_textures(model, material, aiTextureType_SPECULAR, SPECULAR, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], specularMaps, sizeof(struct Texture) * (num_textures - before));
    free(specularMaps);

    /* 3. normal maps */
    before = num_textures;
    struct Texture *normalMaps = seel_load_material_textures(model, material, aiTextureType_NORMALS, NORMAL, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], normalMaps, sizeof(struct Texture) * (num_textures - before));
    free(normalMaps);

    /* 4. height maps */
    before = num_textures;
    struct Texture *heightMaps = seel_load_material_textures(model, material, aiTextureType_HEIGHT, HEIGHT, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], heightMaps, sizeof(struct Texture) * (num_textures - before));
    free(heightMaps);

    /* 5. emissive maps */
    before = num_textures;
    struct Texture *emissiveMaps = seel_load_material_textures(model, material, aiTextureType_EMISSIVE, EMISSIVE, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], emissiveMaps, sizeof(struct Texture) * (num_textures - before));
    free(emissiveMaps);

    /* 6. ambient maps */
    before = num_textures;
    struct Texture *ambientMaps = seel_load_material_textures(model, material, aiTextureType_AMBIENT, AMBIENT, &num_textures);
    textures = realloc(textures, sizeof(struct Texture) * num_textures);
    memcpy(&textures[before], ambientMaps, sizeof(struct Texture) * (num_textures - before));
    free(ambientMaps);

    seel_extract_bone_weight_for_vertices(vertices, mesh, model);

    return seel_mesh_init(vertices, indices, textures, mesh->mNumVertices, num_indices, num_textures);
}

void seel_process_node(struct Model *model, struct aiNode *node, const struct aiScene *scene)
{
    /* process all the node's meshes (if any) */
    unsigned int i;
    for (i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes = realloc(model->meshes, sizeof(struct Mesh) * (model->num_meshes + 1));
        model->meshes[model->num_meshes++] = seel_process_mesh(model, mesh, scene);
    }

    /* then do the same for each of its children */
    for (i = 0; i < node->mNumChildren; i++)
    {
        seel_process_node(model, node->mChildren[i], scene);
    }
}

struct Model seel_model_load(const char *path)
{
    struct Model m = {0};

    const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "Failed in loading model!\n\n%s\n", aiGetErrorString());
        return m;
    }

    if (scene->mAnimations && scene->mNumAnimations)
        m.animated = true;

    const char *name = strrchr(path, '/');
    unsigned int size = name - path;
    strncpy(m.directory, path, size + 1);
    strncpy(m.name, name + 1, strlen(name + 1));

    seel_process_node(&m, scene->mRootNode, scene);

    aiReleaseImport(scene);

    return m;
}

void seel_model_draw(struct Model *model, struct Shader *shader)
{
    unsigned int i;
    for (i = 0; i < model->num_meshes; i++)
        seel_mesh_draw(model->meshes[i], shader);
}

#endif /* MODEL_H */