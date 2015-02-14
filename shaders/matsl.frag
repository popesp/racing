#version	140


#define	MAX_LIGHTS	2


struct light
{
	vec3	pos;
	vec3	dif;
	vec3	spc;
};


in		vec3		pass_normal;
in		vec3		pass_toeye;
in		vec3		pass_tolight[MAX_LIGHTS];

out		vec3		fragcol;

uniform	light		lights[MAX_LIGHTS];
uniform	vec3		ambient;
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
	vec3 l, n, e;
	float d;
	
	n = normalize(pass_normal);
	e = normalize(pass_toeye);
	
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