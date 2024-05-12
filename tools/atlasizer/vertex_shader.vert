#version 330 core

layout (location = 0) in vec2 vert;
layout (location = 1) in vec2 texcoord;

out vec2 ftexcoord;

uniform mat4 mvp;

void main()
{
    ftexcoord = texcoord;
    gl_Position = mvp * vec4(vert.xy, 0.0, 1.0);
}
