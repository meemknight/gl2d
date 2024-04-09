#version 330
precision highp float;

out vec4 color;
in vec2 v_positions;
uniform sampler2D u_sampler;

void main()
{
	ivec2 s = textureSize(u_sampler, 0).xy;

	vec2 texelSize = vec2(1.f) / s;

	vec3 rez = vec3(0.f);

	for(int i=-3; i<=3; i++)
		for(int j=-3; j<=3; j++)
		{
			rez += texture2D(u_sampler, 
				(v_positions + vec2(1.f))/2.f + texelSize*vec2(i,j)).rgb * (1.f / 49.f);
		}

	color = vec4(rez,1);
}
