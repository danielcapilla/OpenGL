#version 420 core

layout (location = 1) out vec4 outColor;
layout (location = 3) out vec4 outPos;
layout (location = 2) out vec4 outNormal;
layout (location = 0) out float outDepth;
layout (location = 4) out vec4 outEmission;
layout (location = 5) out vec4 outSpecular;

//Primera pasada, no color
//in vec3 color;
uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D specularTex;

in vec3 pos;
in vec3 norm;
in vec2 texCoord;


void main()
{
	outPos = vec4(pos,1);
	outNormal = normalize(vec4(norm,1));

    vec3 color = texture(colorTex, texCoord).rgb;

    vec3 emission = texture(emiTex, texCoord).rgb;

	outColor.rgb = color;;
	outEmission = vec4(emission,1.0);
	outSpecular = texture(specularTex, texCoord);
	outColor.a = 1.0;
	outDepth = pos.z;
}


