#version 400
            
uniform mat4 PV;
uniform mat4 M;
uniform float time;
uniform int draw_id;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec4 normal;
out vec4 pos;

void main(void)
{
	vec4 p = vec4(pos_attrib, 1.0);
	vec4 n = vec4(normal_attrib, 0.0);
	if(draw_id==1 || draw_id==2)
	{
		p.z += 0.01*sin(15.0*p.x+10.0*time);
	}
	else
	{
		p.xy += 0.01*sin(5.0*p.x+2.0*time);
	}

	gl_Position = PV*M*p; 
	tex_coord = tex_coord_attrib;
	normal = M*n;
	pos = M*p;
}
