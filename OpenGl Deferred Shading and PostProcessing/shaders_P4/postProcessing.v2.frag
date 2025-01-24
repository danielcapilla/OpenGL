#version 420 core

out vec4 outColor;
in vec2 texCoord;

layout (binding = 0) uniform sampler2D colorTex;
layout (binding = 1) uniform sampler2D zTex; //Texture Unit = 0

#define MASK_SIZE 25u
const float maskFactor = float (1.0/65.0);
const vec2 texIdx[MASK_SIZE] = vec2[](
	vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
	vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
	vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
	vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
	vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));
const float mask[MASK_SIZE] = float[](
	1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor,
	2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor,
	3.0*maskFactor, 4.0*maskFactor, 5.0*maskFactor,4.0*maskFactor, 3.0*maskFactor,
	2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor,
	1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor);

/* Deteccion de bordes
const float maskEdge[MASK_SIZE] = float[](
	float (0.0*maskFactor), float (-1.0*maskFactor), float (0.0*maskFactor),
	float (-1.0*maskFactor), float (4.0*maskFactor), float (-1.0*maskFactor),
	float (0.0*maskFactor), float (-1.0*maskFactor), float (0.0*maskFactor)); 
*/

const float focalDistance = -25.0; //Negativo porque la camara mira en el eje de las Zs negativas
const float maxDistanceFactor = 1.0/5.0;

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0)); //Uno entre el tamano de textura para saber distancia al texel de al lado

	float dof = abs(texture(zTex,texCoord).x - focalDistance) * maxDistanceFactor; //Usamos .x porque al ser un float, se almacena en la posicion x de la textura
	dof = clamp (dof, 0.0, 1.0);
	dof *= dof;

	vec4 color = vec4 (0.0);
	for (uint i = 0u; i < MASK_SIZE; i++)
	{
		vec2 iidx = texCoord + ts * texIdx[i]*dof;
		color += texture(colorTex, iidx,0.0) * mask[i];
	}
	outColor = color;
}