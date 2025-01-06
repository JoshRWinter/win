#version 330 core

in vec3 fcolor;
in vec2 ftexcoord;
out vec4 color;

uniform sampler2D tex;

void main()
{
	vec4 smpl = texture(tex, ftexcoord);
	color = vec4(((smpl.rgb / smpl.a) * fcolor.rgb) * smpl.a, smpl.a);
}
