#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture;

void main()
{
    vec4 sampled = texture(textTexture, TexCoord) ;
    FragColor = sampled;
}