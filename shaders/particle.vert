#version 460
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec4 aInstance; // xyz = position, w = size
layout (location = 3) in vec3 aColor;
layout (location = 4) in float aLife;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 vp;

void main() {
    TexCoords = aTexCoords;
    ParticleColor = vec4(aColor, aLife); // Use life as alpha
    
    // Billboard calculation
    vec3 pos = aPos * aInstance.w;
    pos += aInstance.xyz;
    gl_Position = vp * vec4(pos, 1.0);
}