#version 130
precision highp float;

out vec4 color;
in vec2 v_positions; //the positions are in NDC [-1, 1], for a post process shader they will just be the entire screen
in vec2 v_texture; //the positions are in texture space, for a post process shader they will just be the entire texture
in vec4 v_color; //for a post process shader the color is just 1,1,1,1


uniform sampler2D u_sampler;

void main()
{
	//both ways work the same for a post processing shader
	//color = texture2D(u_sampler, (v_positions + vec2(1.f))/2.f).rgba;
	color = texture2D(u_sampler, v_texture).rgba;
}