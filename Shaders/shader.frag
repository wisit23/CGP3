#version 330 core

out vec4 colour;
in vec4 vCol;
in vec2 TexCoord;
uniform sampler2D texture_data;
uniform sampler2D texture_data_2;
void main()
{
    vec4 tex = texture(texture_data, TexCoord);
    vec4 tex2 = texture(texture_data_2, TexCoord);
    // colour = mix(vCol, tex, tex.a);
    colour = mix(tex, tex2, tex2.a);
}