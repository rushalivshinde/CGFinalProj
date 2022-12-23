#version 400
uniform sampler2D diffuse_tex[11];
uniform vec4 kd;
uniform float time;
uniform int draw_id;
uniform int texCheck;
uniform vec4 Lw = vec4(0.0, 10.0, 0.0, 1.0);
uniform float texScale;
uniform int selectedTex;
uniform float ki = 2.0f;
uniform float ka = 0.1f;
uniform float ke = 0.5f;


uniform float di; // distance of light source from the object

in vec2 tex_coord;
in vec4 normal;
in vec4 pos;

out vec4 fragcolor; //the output color for this fragment    


void main(void)
{   
	vec4 tex_map = texture(diffuse_tex[selectedTex], 2*texScale*tex_coord);

	vec4 depth = vec4(1)-vec4(vec3(pow(gl_FragCoord.z,20.0)),1.0);//scene depth value
	
	vec4 k = vec4(0.5, 0.5, 0.5, 1.0);
	vec3 nw = normalize(normal.xyz);
	vec3 lw = normalize(Lw.xyz-pos.xyz);
	vec4 Id = k*max(0.0, dot(nw, lw));
	vec4 Ie = vec4(0,0,1,1)*pow(max(0.0, dot(nw, lw)),2);
	vec4 Ia = ka*(tex_map);

	if(draw_id==0 || draw_id == 2)
	{
		//draw alpha blended quad
		fragcolor = kd * depth * texture(diffuse_tex[selectedTex], 0.1*texScale*tex_coord);
	}
	else if(draw_id == 1)
	{
		//draw mesh
		
		if(texCheck == 1)
		{
			float t0 = 0.0;
			float t1 = 1-texture(diffuse_tex[selectedTex], 0.5*texScale*tex_coord).x;
			Id = vec4((vec3(smoothstep(t0, t1, pow(Id.b,ki)))),1);
			Ie = vec4(pow(Ie.b,ke));
			fragcolor = 1.4*depth*(Ia +(Ie * Id));
			//fragcolor = vec4(Is.b);
		}
		else
		{
			fragcolor = depth*Id;
		}
	}
	
}


