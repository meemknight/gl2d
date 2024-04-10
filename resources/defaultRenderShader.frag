#version 130
precision highp float;

out vec4 color;
in vec2 v_positions; //the positions are in NDC [-1, 1], not used here
in vec2 v_texture; //the positions are in texture space
in vec4 v_color; //the color sent by the user, can be different per vertex.

uniform sampler2D u_sampler;

void main()
{
    color = texture2D(u_sampler, v_texture) * v_color;
}