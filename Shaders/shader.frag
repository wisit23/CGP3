#version 330 core

out vec4 colour;
in vec4 vCol;

void main()
{
    colour = vCol; 
}