#version 330 core

out vec4 colour;

in vec2 TextCoord;

uniform sampler2D texture_data;

void main()
{
    colour = texture(texture_data, TextCoord);
}
