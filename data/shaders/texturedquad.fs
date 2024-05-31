#version 330 core

out vec4 FragColor;

uniform vec4 color; 

in vec2 TexCoord;

in vec4 outColor;

uniform sampler2D texture1; // This should match the texture unit you bind the texture to


void main()
{
  FragColor  = texture(texture1, TexCoord);
}