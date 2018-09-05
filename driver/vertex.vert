#version 330 core

layout (location = 0) in vec2 verts;
layout (location = 1) in vec2 position;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 texcoord;

uniform mat4 projection;
uniform float size;
out vec3 fcolor;
out vec2 ftexcoord;

void main()
{
	mat4 translate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, position.x + (size / 2.0), position.y + (size / 2.0), 0.0, 1.0);
	fcolor = color;
	ftexcoord = texcoord;
	gl_Position = projection * translate * vec4(verts.x * size, verts.y * size, 0.0, 1.0);
}
