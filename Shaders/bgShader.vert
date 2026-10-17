#version 330 core

layout (location = 0) in vec3 pos;

out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightView;
uniform mat4 lightProjection;

void main()
{
    FragPosLightSpace = lightProjection * lightView * model * vec4(pos, 1.0f);
                    //clip space    //view space    //world space    //local space
    gl_Position = projection * view * model * vec4(pos, 1.0f);
}