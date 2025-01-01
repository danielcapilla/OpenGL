#version 330 core

out vec4 outColor;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;

//Variables uniformes de la fuente de luz
//uniform vec3 Ia;
vec3 Ia = vec3(0.2);
vec3 Il = vec3(0.4);
vec3 pl = vec3(0); // Est� en el sistema de ref. de la c�mara.
vec3 D  = vec3(0,0,-1);	// Direcci�n en coord. del mundo en la que viene la luz direccional

//Variables del objeto
vec3 normal;

in vec3 po;
in vec3 no;
in vec3 color;
in vec2 tc;
in vec3 tan;
in mat3 TBN;

vec3 shade()
{
	vec3 c = vec3(0);

	//Amb
	c+= Ia * Ka;

	//Diff
	vec3 L = normalize(pl-po);
	vec3 N = normalize(normal);
	c += Il * Kd * max (0, dot (N,L));

	//Spec
	vec3 V = normalize(-po); 
	vec3 H = normalize((L+V)*0.5);
	c += Il * Ks * pow(max(0,dot(H,N)), n);

	//Emi
	c += Ke;

	return c;
}
vec3 directional_shade()
{
	vec3 c = vec3(0);

	// Ambiental
	c += Ia * Ka;

	// Difusa
	vec3 L = normalize(-D);
	vec3 N = normalize(normal);
	c += Il * Kd * max (0, dot (N,L));

	// Specular
	vec3 V = normalize(-po); 
	vec3 H = normalize((L+V)*0.5);
	c += Il * Ks * pow(max(0,dot(H,N)), n);

	// Emisiva
	// c += Ke;

	return c;
}


void main()
{
	Ka = texture(colorTex,tc).rgb;
	Kd = Ka;
	Ks = texture(specularTex,tc).rgb;
	n = 200;
	Ke = texture(emiTex, tc).rgb;
	//No se puede escribir en una variable variante
	//no = normalize(texture(normalTex, tc).xyz * 2.0 - 1.0);
	// Obtener la normal del mapa normal y transformarla con TBN
    vec3 nMap = normalize(texture(normalTex, tc).xyz * 2.0 - 1.0);
    normal = normalize(TBN * nMap);
	outColor += vec4(shade(), 1.0); 
	outColor += vec4(directional_shade(), 1.0); 
}
