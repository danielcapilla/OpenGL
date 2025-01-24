#version 420 core

out vec4 outColor;
in vec2 texCoord;

layout (binding = 0) uniform sampler2D colorTex;

void main()
{
	outColor = vec4(textureLod(colorTex, texCoord, 0).xyz, 0.6);
	//outColor = vec4(texCoord, vec2(1.0));
}