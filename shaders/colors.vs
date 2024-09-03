#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

void main()
{
    vec4 updatedPosition = vec4(0.0f);
    vec3 updatedNormal = vec3(0.0f);

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        // Current bone-weight pair is non-existing
        if(boneIds[i] == -1) 
            continue;

        // Ignore all bones over count MAX_BONES
        if(boneIds[i] >= MAX_BONES) 
        {
            updatedPosition = vec4(aPos,1.0f);
            break;
        }
        // Set pos
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
        updatedPosition += localPosition * weights[i];
        // Set normal
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
        updatedNormal += localNormal * weights[i];
    }
	
	TexCoord = aTexCoord;
    // FragPos = vec3(model * vec4(aPos, 1.0f));
    // Normal = normalize(mat3(normalMatrix) * aNormal);
    // gl_Position = projection * view * vec4(FragPos, 1.0f);

    FragPos = vec3(model * vec4(vec3(updatedPosition), 1.0f));
    Normal = normalize(updatedNormal * mat3(normalMatrix));
    gl_Position =  projection * view * model * updatedPosition;
}