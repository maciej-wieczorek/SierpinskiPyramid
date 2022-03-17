#version 330 core
layout (location = 0) in vec4 aPos;   // the position variable has attribute position 0


uniform mat4 view;
uniform mat4 projection;

// uniform vec4 color=vec4(1,1,1,1);
// uniform vec4 lightDir=vec4(0,0,1,0);
// out vec4 Color;

void main()
{
    gl_Position = projection * view * aPos;
}