#version	140


#define	MAX_LIGHTS	2


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

out		vec3		fragcol;

uniform	light		lights[MAX_LIGHTS];
uniform	vec3		ambient;
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
	mat3 tan_model;
	float d;
	
	n = normalize(pass_normal);
	t = normalize(pass_tangent);
	e = normalize(pass_toeye);
	
	b = cross(n, t);

	// matrix conversion from tangent-space to model-space
	tan_model[0] = t;
	tan_model[1] = b;
	tan_model[2] = n;

	// find new normal vector modified by normal map
	n = tan_model*(texture2D(tex_normal, pass_uv).rgb*2.f - 1.f);

	dif = vec3(0.f);
	spc = vec3(0.f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		l = normalize(pass_tolight[i]);
		d = dot(n, l);
		
		dif += lights[i].dif * max(d, 0.f);
		if (d > 0.f)
			spc += lights[i].spc * pow(max(dot(reflect(-l, n), e), 0.f), material.shn);
	}
	
	amb = ambient*material.amb;
	dif *= material.dif;
	spc *= material.spc;
	
	fragcol = amb + dif + spc;
}