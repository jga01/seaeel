#version 460 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D billboardTexture;
uniform vec3 color;

void main()
{
    vec4 texColor = texture(billboardTexture, TexCoords) * vec4(color, 1.0);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}