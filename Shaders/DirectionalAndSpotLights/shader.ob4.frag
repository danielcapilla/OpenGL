#version 330 core

out vec4 outColor;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D emiTex;


// ----------------------------------------- //
//  Variables uniformes de la fuente de luz  //
// ----------------------------------------- //

// Luz focal en una dirección D, con ángulos Au (cutoff) y Ap (penumbra)

vec3 Ia  = vec3(0.2);
vec3 Il  = vec3(1);
vec3 pl  = vec3(0,0,0);		// Está en el sistema de ref. de la cámara.
vec3 D   = vec3(0,0,-1);		// Dirección en coord. del mundo en la que viene la luz direccional
float Au = radians(10.0);
float Ap = radians(5.0);
float m = 2;  // Exponente de atenuiación

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

// Esta función simplemente calcula L usando la variable uniforme D

vec3 shade()
{
	vec3 c = vec3(0);

	// Ambiental
	c += Ia * Ka;

	// Difusa
	vec3 L = normalize(pl - po);
	vec3 N = normalize(no);
	c += Il * Kd * max (0, dot (N,L));

	// Specular
	vec3 V = normalize(-po); 
	vec3 H = normalize((L+V)*0.5);
	c += Il * Ks * pow(max(0,dot(H,N)), n);

	// Emisiva
	c += Ke;

	return c;
}

// ----------------------------------------- //
//           Función de luz focal            //
// ----------------------------------------- //

//	Params:
//	  - pos_o: posición del objeto
//	  - pos_l: posición de la luz
//	  - dir_l: dirección del foco
//	  - theta_u: ángulo de cutoff (límite de la luz)
//	  - theta_p: ángulo de penumbra (donde la luz comienza a decaer)

float fdir(vec3 pos_o, vec3 pos_l, vec3 dir_l, float theta_u, float theta_p){
	float t = 0;

	vec3 L = normalize(pos_l - pos_o);
	t = (dot(-L, D) - cos(theta_u)) / (cos(theta_p) - cos(theta_u));
	t = clamp(t, 0.0, 1.0);
	t = pow(t,m);
	return t * t * (3 - 2 * t);
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

	// Aplicamos la función de luz focal a la intensidad de la luz
	Il = fdir(po, pl, D, Au, Ap) * Il;

	outColor = vec4(shade(), 1.0);   
}
