#version 460 core
in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 FragColor;

uniform sampler2D sprite;

void main() {
    vec4 texColor = texture(sprite, TexCoords);
    FragColor = texColor * ParticleColor;
}