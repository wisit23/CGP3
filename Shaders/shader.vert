#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTexCoord;

out vec4 vCol ; 
out vec2 TextCoord;


uniform mat4 model ;
uniform mat4 view ;
uniform mat4 projection ;

void main()
{
    gl_Position =  projection * view * model * vec4(0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0);
    vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);
    TextCooord = aTextCooord; 
}