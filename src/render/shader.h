#ifndef	SHADER
#define	SHADER


#define	SHADER_VERTEX	0
#define	SHADER_FRAGMENT	1


unsigned shader_create(const char*, unsigned char);
unsigned shader_program(unsigned, unsigned);
unsigned shader_link(unsigned);
void shader_delete(unsigned);


#endif