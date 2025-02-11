#version 420 core

in vec3 inPos;	
in vec3 inColor;
in vec3 inNormal;
in vec2 inTexCoord;


uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;
//No sacamos todav�a el color
//out vec3 color;
out vec3 pos;
out vec3 norm;
out vec2 texCoord;

void main()
{
	//color = inColor;
	texCoord = inTexCoord;
	norm = (normal * vec4(inNormal, 0.0)).xyz;
	pos = (modelView * vec4(inPos, 1.0)).xyz;
	
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}
