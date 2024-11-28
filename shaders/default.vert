#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in ivec4 boneIds; // Bone IDs affecting this vertex
layout (location = 6) in vec4 weights;  // Corresponding bone weights

out vec2 TexCoord;  // Texture coordinates
out vec3 Normal;    // Transformed normal vector
out vec3 FragPos;   // Position of fragment in world space

// Animation parameters
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 boneMatrices[MAX_BONES];  // Bone transformation matrices
uniform bool animate;                 // Toggle animation on/off

// Matrices for non-animated transformations
uniform mat4 model;        // Model matrix
uniform mat4 view;         // View matrix
uniform mat4 projection;   // Projection matrix
uniform mat4 normalMatrix; // Model-view normal transformation matrix

void main()
{
    vec4 updatedPosition = vec4(0.0f);
    vec3 updatedNormal = vec3(0.0f);

    if (animate)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (weights[i] > 0.0 && boneIds[i] < MAX_BONES)
            {
                vec4 bonePosition = boneMatrices[boneIds[i]] * vec4(aPos, 1.0);
                updatedPosition += bonePosition * weights[i];

                mat3 boneNormalMatrix = mat3(boneMatrices[boneIds[i]]);
                updatedNormal += weights[i] * normalize(boneNormalMatrix * aNormal);
            }
        }
    }
    else
    {
        updatedPosition = vec4(aPos, 1.0f);
        updatedNormal = aNormal;
    }

    TexCoord = aTexCoord;
    FragPos = vec3(model * updatedPosition);
    Normal = normalize(mat3(normalMatrix) * updatedNormal);
    
    gl_Position = projection * view * model * updatedPosition;
}