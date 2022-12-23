#version 400
uniform sampler2D diffuse_tex;
uniform vec4 kd = vec4(1.0);
uniform float time;
uniform int draw_id;
uniform vec4 Lw = vec4(0.0, 10.0, 0.0, 1.0);

in vec2 tex_coord;
in vec4 normal;
in vec4 pos;

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	if(draw_id==0 || draw_id == 2)
	{
		//draw alpha blended quad
		fragcolor = kd;
	}
	else if(draw_id == 1)
	{
		//draw mesh
		vec4 amb = vec4(0.3, 0.3, 0.5, 1.0);
		vec4 k = vec4(1.0, 0.0, 1.0, 1.0);
		vec3 nw = normalize(normal.xyz);
		vec3 lw = normalize(Lw.xyz-pos.xyz);
		vec4 diff = k*max(0.0, dot(nw, lw));
		fragcolor = amb+diff;
	}
}


