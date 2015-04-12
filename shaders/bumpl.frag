#version	140


#define	MAX_LIGHTS		8

#define	ATT_LINEAR		0.1
#define	ATT_QUADRATIC	0.002


struct light
{
	vec3	pos;
	vec3	dif;
	vec3	spc;
};


in		vec3		pass_normal;
in		vec3		pass_tangent;
in		vec2		pass_uv;
in		vec3		pass_toeye;
in		vec3		pass_tolight[MAX_LIGHTS];

out		vec4		fragcol;

uniform int			num_lights;
uniform	light		lights[MAX_LIGHTS];
uniform	vec3		ambient;
uniform sampler2D	tex_diffuse;
uniform	sampler2D	tex_normal;
uniform	struct
{
	vec3	amb;
	vec3	dif;
	vec3	spc;
	float	shn;
}					material;


void main()
{
	vec3 amb, dif, spc;
	vec3 n, t, e, b, l;
	float d, dist, att;
	mat3 tan_model;
	
	n = normalize(pass_normal);
	t = normalize(pass_tangent);
	e = normalize(pass_toeye);
	
	b = cross(t, n);

	// matrix conversion from tangent-space to model-space
	tan_model[0] = b;
	tan_model[1] = t;
	tan_model[2] = n;

	// find new normal vector modified by normal map
	n = tan_model * (texture2D(tex_normal, pass_uv).rgb*2. - 1.);

	dif = texture2D(tex_diffuse, pass_uv).rgb;
	spc = vec3(0.f);
	for (int i = 0; i < num_lights; i++)
	{
		dist = length(pass_tolight[i]);
		l = pass_tolight[i] / dist;
		d = dot(n, l);

		att = 1. / (ATT_LINEAR * dist + ATT_QUADRATIC * dist*dist);

		dif += att * lights[i].dif * max(d, 0.f);
		if (d > 0.f)
			spc += att * lights[i].spc * pow(max(dot(reflect(-l, n), e), 0.f), material.shn);
	}
	
	amb = ambient*material.amb;
	dif *= material.dif;
	spc *= material.spc;
	
	fragcol = vec4(amb + dif + spc, 1.f);
}