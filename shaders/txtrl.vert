#version	140


#define	MAX_LIGHTS	2


struct light
{
	vec3	pos;
	vec3	dif;
	vec3	spc;
};


in		vec3		vertpos;
in		vec3		vertnor;
in		vec2		verttex;

out		vec3		pass_normal;
out		vec2		pass_uv;
out		vec3		pass_toeye;
out		vec3		pass_tolight[MAX_LIGHTS];

uniform	mat4		transform;
uniform	vec3		eyepos;
uniform	light		lights[MAX_LIGHTS];


void main()
{
	int i;
	
	pass_normal = vertnor;
	pass_uv = verttex;

	pass_toeye = eyepos - vertpos;
	
	for (i = 0; i < MAX_LIGHTS; i++)
		pass_tolight[i] = lights[i].pos - vertpos;
	
	gl_Position = transform*vec4(vertpos, 1.f);
}