#version 330 core

uniform bool use_texture;
uniform sampler2D tex;

uniform bool use_color;
uniform vec4 color;

in vec2 ftexcoord;
out vec4 frag;

void main()
{
    if (use_color && use_texture)
        frag = texture(tex, ftexcoord) + color;
    else if (use_texture)
        frag = texture(tex, ftexcoord);
    else
        frag = color;

    if (frag.r > 1.0) frag.r = 1.0;
    if (frag.g > 1.0) frag.g = 1.0;
    if (frag.b > 1.0) frag.b = 1.0;
    if (frag.a > 1.0) frag.a = 1.0;
}

