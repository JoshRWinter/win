#version 330 core

in vec3 fcolor;
in vec2 ftexcoord;
out vec4 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, ftexcoord) * vec4(fcolor.rgb, 1.0);
}
