#version 420 core

out vec4 outColor;
in vec2 texCoord;

layout (binding = 0) uniform sampler2D colorTex;
layout (binding = 1) uniform sampler2D zTex; 

//Uniforms
uniform float alpha;
uniform float focalDistance;
uniform float maxDistanceFactor;
uniform float mask[25];
uniform vec2 texIdx[25];





void main()
{
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
	outColor = vec4(color.rgb, alpha);
}