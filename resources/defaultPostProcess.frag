#version 330
precision highp float;

out vec4 color;
in vec2 v_positions;

uniform sampler2D u_sampler;

void main()
{
    color = texture2D(u_sampler, v_positions).rgba;
}