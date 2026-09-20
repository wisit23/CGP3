#version 330 core

out vec4 colour;

in vec4 vCol ;
in vec2 TextCoord;

uniform sampler2D texture_data;

void main()
{
    vec4 tex = texture(texture_data,TexCoord);
    vec4 tex_2 = texture(texture_data_2,TexCoord);
    colour = texture(texture_data,TexCoord); 
}