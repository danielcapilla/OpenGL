#version 330 core
in vec3 inPos;	
in vec3 inNormal;
in vec3 inColor;
in vec2 inTexCoord;	
in vec3 inTangent;	

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;			// Matriz normal (inverso transpuesto de modelView)

out vec3 po;
out vec3 no;
out vec3 color;
out vec2 tc;
out mat3 TBN; // Pasar TBN al fragment shader

void main()
{
    // Coordenadas de textura y color
	tc = inTexCoord;
	color = inColor;

    // Transformar tangente, bitangente y normal al espacio de la cámara
	vec3 T = normalize((modelView * vec4(inTangent, 0.0)).xyz);
	vec3 N = normalize((normal * vec4(inNormal, 0.0)).xyz);
	vec3 B = normalize(cross(N, T)); // Bitangente calculada como producto cruz

    // Construir matriz TBN en el espacio de la cámara
	TBN = mat3(T, B, N);

    // Posición del vértice en el espacio de la cámara
	po = (modelView * vec4(inPos, 1.0)).xyz;

    // Salida al espacio de la pantalla
	gl_Position = modelViewProj * vec4(inPos, 1.0);
}
