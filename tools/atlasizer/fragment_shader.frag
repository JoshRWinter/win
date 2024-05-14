#version 330 core

uniform sampler2D tex;
uniform vec4 solidcolor;
uniform bool mode_solidcolor;

in vec2 ftexcoord;
out vec4 color;

void main()
{
    if (mode_solidcolor)
        color = solidcolor;
    else
        color = texture(tex, ftexcoord);
}
