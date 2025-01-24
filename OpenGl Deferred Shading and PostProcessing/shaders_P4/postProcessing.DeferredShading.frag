#version 420 core

out vec4 outColor;
in vec2 texCoord;

layout (binding = 0) uniform sampler2D colorTex;
layout (binding = 1) uniform sampler2D zTex; 
layout (binding = 2) uniform sampler2D normalTex;
layout (binding = 3) uniform sampler2D positionTex;
layout (binding = 4) uniform sampler2D emiTex;
layout (binding = 5) uniform sampler2D specTex;

//Uniforms
uniform float alpha;
uniform float focalDistance;
uniform float maxDistanceFactor;
uniform float mask[25];
uniform vec2 texIdx[25];

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alphaObj = 500.0;
vec3 Ke;

//Propiedades de la luz
vec3 Ia = vec3 (0.2);
vec3 Id = vec3 (1.0);
vec3 Is = vec3 (0.8);
vec3 lpos = vec3 (1.0); 
//Propiedades G buffer
vec3 fragPos;
vec3 normal; 
vec3 albedo; 
vec4 spec;
vec4 emissive;

vec3 shade()
{
	vec3 c = vec3(0.0);
	c += Ia * Ka;

	vec3 L = normalize (lpos - fragPos);
	vec3 diffuse = Id * Kd* max(0,dot (N,L));
	//c += clamp(diffuse, 0.0, 1.0);
	c+=diffuse;
	
	vec3 V = normalize (-fragPos);
	vec3 R = reflect (-L,N);
	vec3 H = normalize((L+V)*0.5);
	float factor = max (dot (H,N), 0.0);
	vec3 specular = Is*Ks*pow(factor,alphaObj);
	//c += clamp(specular, 0.0, 1.0);
	c+=specular;

	c+=Ke;
	
	return c;
}

void main()
{

	// Leer datos del G-buffer
    fragPos = texture(positionTex, texCoord).rgb;
    normal = normalize(texture(normalTex, texCoord).rgb);
    albedo = texture(colorTex, texCoord).rgb;
	spec = vec4(texture(specTex, texCoord).rgb,1.0);
	emissive = vec4(texture(emiTex, texCoord).rgb,1.0);

	Ka = albedo;
	Kd = albedo;
	Ke = emissive.rgb;
	Ks = spec.rgb;

	N = normal;
	
	outColor += vec4(shade(), alpha);  

	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0)); 

	float dof = abs(texture(zTex,texCoord).x - focalDistance) * maxDistanceFactor; 
	dof = clamp (dof, 0.0, 1.0);
	dof *= dof;

	vec4 color = vec4 (0.0);
	for (uint i = 0u; i < mask.length; i++)
	{
		vec2 iidx = texCoord + ts * texIdx[i]*dof;
		color += texture(colorTex, iidx,0.0) * mask[i];
	}
	//outColor += vec4(color.rgb, alpha);
}