#version 330 core

out vec4 outColor;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D emiTex;


// ----------------------------------------- //
//  Variables uniformes de la fuente de luz  //
// ----------------------------------------- //

//uniform vec3 Ia;
vec3 Ia = vec3(0.2);
vec3 Il = vec3(1);
vec3 pl = vec3(0); // Está en el sistema de ref. de la cámara.

// ----------------------------------------- //
//        Variables uniformes de fdist       //
// ----------------------------------------- //

float c1 = 0.5;	// Cte para que denominador no sea 0
float c2 = 0.3;	// Factor de decay lineal
float c3 = 0.1;	// Factor de decay cuadrático

// ----------------------------------------- //
//            Variables del objeto           //
// ----------------------------------------- //

vec3 Ka; //Uniforme || Attrib || Tex
vec3 Kd; //Uniforme || Attrib || Tex
vec3 Ks; //Uniforme || Attrib || Tex
float n; //Uniforme || Attrib || Tex
vec3 Ke;

in vec3 po;
in vec3 no;
in vec3 color;
in vec2 tc;

// ----------------------------------------- //
//          Función de Blinn-Phong           //
// ----------------------------------------- //

vec3 shade()
{
	vec3 c = vec3(0);

	// Ambiental
	c += Ia * Ka;

	// Difusa
	vec3 L = normalize(pl-po);
	vec3 N = normalize(no);
	c += Il * Kd * max (0, dot (N,L));

	// Specular
	vec3 V = normalize(-po); 
	vec3 H = normalize((L+V)*0.5);
	c += Il * Ks * pow(max(0,dot(H,N)), n);

	// Emisiva
	// c += Ke;

	return c;
}


// ----------------------------------------- //
//      Función de atenuación lumínica       //
// ----------------------------------------- //

// Interesante: https://learnwebgl.brown37.net/09_lights/lights_attenuation.html

float fdist(){
	float d = length(po - pl);

	return min(1/(c1 + c2 * d + c3 * pow(d,2)), 1);
}

// ----------------------------------------- //
//                    Main                   //
// ----------------------------------------- //

void main()
{

	// Coeficiente de luz ambiental
	Ka = texture(colorTex,tc).rgb;

	// Coeficiente de luz difusa
	Kd = Ka;

	// Coeficiente de luz especular
	Ks = texture(specularTex,tc).rgb;

	// Brillo para la luz especular
	n = 200;

	// Coeficiente de luz emisiva
	Ke = texture(emiTex, tc).rgb;

	// Actualizamos Il utilizando fdist
	Il = fdist() * Il;

	outColor = vec4(shade(), 1.0);   
}
