#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//Ejercicio 1
//Motion Blur
float alpha = 0.9f;
//Ejercicio 2
//DOF
float focalDistance = -25.0f;
float maxDistanceFactor = 1.0f / 5.0f;
//Ejercicio 4
//Máscaras de convolución
glm::vec2 texIdx[25] = {
   glm::vec2(-2.0, 2.0), glm::vec2(-1.0, 2.0), glm::vec2(0.0, 2.0), glm::vec2(1.0, 2.0), glm::vec2(2.0, 2.0),
   glm::vec2(-2.0, 1.0), glm::vec2(-1.0, 1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 1.0), glm::vec2(2.0, 1.0),
   glm::vec2(-2.0, 0.0), glm::vec2(-1.0, 0.0), glm::vec2(0.0, 0.0), glm::vec2(1.0, 0.0), glm::vec2(2.0, 0.0),
   glm::vec2(-2.0, -1.0),glm::vec2(-1.0, -1.0), glm::vec2(0.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(2.0, -1.0),
   glm::vec2(-2.0, -2.0),glm::vec2(-1.0, -2.0), glm::vec2(0.0, -2.0), glm::vec2(1.0, -2.0), glm::vec2(2.0, -2.0)
};

float maskFactor = float(1.0 / 65.0);

float mask[25] = {
   1.0 * maskFactor, 2.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor, 1.0 * maskFactor,
   2.0 * maskFactor, 3.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor,
   3.0 * maskFactor, 4.0 * maskFactor, 5.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor,
   2.0 * maskFactor, 3.0 * maskFactor, 4.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor,
   1.0 * maskFactor, 2.0 * maskFactor, 3.0 * maskFactor, 2.0 * maskFactor, 1.0 * maskFactor
};



//int mask = 0;

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int emiTexId;
unsigned int specularTexId;

//Plano
unsigned int planeVAO;
unsigned int planeVertexVBO;

//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform 
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uTexIdx;
int uMask;
int uAlpha;
int uFocalDistance;
int uMaxDistanceFactor;

//Texturas Uniform
int uColorTex;
int uEmiTex;
int uSpecularTex;
int uZettTex;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//Post-Processor
unsigned int postProccesProgram;
unsigned int postProccesVShader;
unsigned int postProccesFShader;

//Uniform
unsigned int uColorTexPP;

//Atributos
int inPosPP;

//FBO
unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;
unsigned int zTestBuffTexId;
unsigned int postionBuffTexId;
unsigned int normalBuffTexId;
unsigned int emiBuffTexId;
unsigned int specBuffTexId;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();

void resizeFBO(float width, float height);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char* vname, const char* fname);
void initObj();
void initPlane();
void destroy();



//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar
void updateMask(unsigned int nMask);



int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.DeferredShading.vert", "../shaders_P4/fwRendering.DeferredShading.frag");
	initShaderPP("../shaders_P4/postProcessing.DeferredShading.vert", "../shaders_P4/postProcessing.DeferredShading.frag");
	//initShaderFw("../shaders_P4/fwRendering.v1.vert", "../shaders_P4/fwRendering.v1.frag");
	//initShaderPP("../shaders_P4/postProcessing.v3.vert", "../shaders_P4/postProcessing.v5.frag");
	initObj();
	initPlane();
	
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
	glGenTextures(1, &zTestBuffTexId);
	glGenTextures(1, &postionBuffTexId);
	glGenTextures(1, &normalBuffTexId);
	glGenTextures(1, &emiBuffTexId);
	glGenTextures(1, &specBuffTexId);
	resizeFBO(SCREEN_SIZE);


	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}


void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
	glDeleteTextures(1, &zTestBuffTexId);
	glDeleteTextures(1, &postionBuffTexId);
	glDeleteTextures(1, &normalBuffTexId);
	glDeleteTextures(1, &emiBuffTexId);
	glDeleteTextures(1, &specBuffTexId);
}

void initShaderFw(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");


	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");
	uSpecularTex = glGetUniformLocation(program, "specularTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initShaderPP(const char* vname, const char* fname)
{
	unsigned int postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	unsigned int postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesFShader);

	glBindAttribLocation(postProccesProgram, 0, "inPos");
	glLinkProgram(postProccesProgram);
	int linked;

	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}
	//Convolution Mask
	uTexIdx = glGetUniformLocation(postProccesProgram, "texIdx");
	uMask = glGetUniformLocation(postProccesProgram, "mask");
	//Motion Blur
	uAlpha = glGetUniformLocation(postProccesProgram, "alpha");
	//DOF
	uFocalDistance = glGetUniformLocation(postProccesProgram, "focalDistance");
	uMaxDistanceFactor = glGetUniformLocation(postProccesProgram, "maxDistanceFactor");

	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}


	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex*sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
	specularTexId = loadTex("../img/specMap.png");

}

void initPlane() {
	glGenBuffers(1, &planeVertexVBO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW);

	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glVertexAttribPointer(0, //Indice Atrib
		3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	

	//Estamos usando el buffer por default

	//Tras activar el FBO que hemos configurado en ResizeFBO, podremos usarlo
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Borra framebuffer actual

	//PRIMERA PASADA
	/**/
	glUseProgram(program);

	//Texturas
	if (uColorTex != -1) //uColorTex actua como puntero para subir informacion a la GPU
	{
		glUniform1i(uColorTex, 0);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}
	if (uSpecularTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, specularTexId);
		glUniform1i(uSpecularTex, 2);
	}


	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	renderCube();

	std::srand(RAND_SEED);
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
			float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle*2.0f*size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle*2.0f*size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size*0.7f)));
		renderCube();
	}
	//*/

	//SEGUNDA PASADA

	glBindFramebuffer(GL_FRAMEBUFFER, 0);	//Volvemos a activar el default FrameBuffer. No borramos para conservar la salida del ForwardRender
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(postProccesProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, zTestBuffTexId);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, normalBuffTexId);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, postionBuffTexId);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, emiBuffTexId);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, specBuffTexId);


	//Convolution Mask
	if (uTexIdx != -1)
	{
		glUniform2fv(uTexIdx, 25, &texIdx[0].x);
	}
	if (uMask != -1)
		glUniform1fv(uMask, 25, mask);
	//Motion Blur
	if (uAlpha != -1)
		glUniform1fv(uAlpha, 1, &alpha);
	//DOF
	if (uFocalDistance != -1)
		glUniform1fv(uFocalDistance, 1, &focalDistance);
	if (uMaxDistanceFactor != -1)
		glUniform1fv(uMaxDistanceFactor, 1, &maxDistanceFactor);
	//Motion Blur
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glBindVertexArray(planeVAO);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//glDisable(GL_BLEND);

	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}

void resizeFBO(float width, float height) 
{
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, //Reserva de espacio de memoria. Nivel de Bitmap 0. GL_RGBA8 tipo interno en la tarjeta. 
		GL_RGBA, GL_FLOAT, NULL); //GL_RGBA formato externo. Puntero nulo porque no voy a subir nada, solo reservamos memoria.

	/*
	Solo reserva espacio, no es capaz de subir nada. Crea texturas inmutables, por eso no sirve para este caso
		glTexStorage2D();
	*/


	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //Min_Filter para pocos fragmentos, submuestrada
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //Mag_Filter para muchos fragmentos, textura supermuestreada
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //Linear para que el DOF sea progresivo
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //GL_REPEAT para cuando te quedas corto o te pasas en coordenadas
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	
	//Textura de profundidad almacenada en textura de color
	glBindTexture(GL_TEXTURE_2D, zTestBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, depthBuffTexId); //Ya no es color, estamos en profundidad
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// G buffer
	//Position
	glBindTexture(GL_TEXTURE_2D, postionBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Normal
	glBindTexture(GL_TEXTURE_2D, normalBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Emisive
	glBindTexture(GL_TEXTURE_2D, emiBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Specular
	glBindTexture(GL_TEXTURE_2D, specBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	

	// Ahora es el G buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, //COLOR
		GL_TEXTURE_2D, colorBuffTexId, 0); //Nivel de bitmap 0

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, zTestBuffTexId, 0); //ZTest

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, //PROFUNDIDAD
		depthBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalBuffTexId, 0); //Normal

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, postionBuffTexId, 0);//Position

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, emiBuffTexId, 0);//Emisive

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, specBuffTexId, 0);//Specular

	// zTest, color, normal, position, emisive, specular
	const GLenum buffs[6] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, buffs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), 1.0f, 50.0f);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.02f;
	
	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
		// Motion Blur
		case 'w':
			if (alpha < 1.0f)
				alpha += 0.1f;
			std::cout << "alpha: " << alpha << std::endl;
			break;
		case 's':
			if (alpha > 0.0f)
				alpha -= 0.1f;
			std::cout << "alpha: " << alpha << std::endl;
			break;

		// DOF
		case 'e': 
			if (focalDistance < 0.0f)
				focalDistance += 1.0f;
			std::cout << "focalDistance: " << focalDistance << std::endl;
			break;
		case 'd': 
			if (focalDistance > -25.0f)
				focalDistance -= 1.0f;
			std::cout << "focalDistance: " << focalDistance << std::endl;
			break;
		case 'r':
			if (maxDistanceFactor < 1.0f)
				maxDistanceFactor += 0.1f;
			std::cout << "maxDistanceFactor: " << maxDistanceFactor << std::endl;
			break;
		case 'f':
			if (maxDistanceFactor > 0.01f)
				maxDistanceFactor -= 0.1f;
			std::cout << "maxDistanceFactor: " << maxDistanceFactor << std::endl;
			break;
		// Máscaras de convolución
		case '1':
			std::cout << "Máscara 1 Desenfoque Gausiano" << std::endl;
			updateMask(1);
			break;
		case '2':
			std::cout << "Máscara 2 Detección de bordes" << std::endl;
			updateMask(2);
			break;
		case '3':
			std::cout << "Máscara 3 Repujado" << std::endl;
			updateMask(3);
			break;
		case '4':
			std::cout << "Máscara 4 Realzado Bordes" << std::endl;
			updateMask(4);
			break;
		case '5':
			std::cout << "Máscara 5 Enfoque" << std::endl;
			updateMask(5);
			break;
		default:
			break;
	}
}
void updateMask(unsigned int nMask)
{
	switch (nMask)
	{
	case 1:
		mask[0] = 1.0 * maskFactor; mask[1] = 2.0 * maskFactor; mask[2] = 3.0 * maskFactor; mask[3] = 2.0 * maskFactor; mask[4] = 1.0 * maskFactor;
		mask[5] = 2.0 * maskFactor; mask[6] = 3.0 * maskFactor; mask[7] = 4.0 * maskFactor; mask[8] = 3.0 * maskFactor; mask[9] = 2.0 * maskFactor;
		mask[10] = 3.0 * maskFactor; mask[11] = 4.0 * maskFactor; mask[12] = 5.0 * maskFactor; mask[13] = 4.0 * maskFactor; mask[14] = 3.0 * maskFactor;
		mask[15] = 2.0 * maskFactor; mask[16] = 3.0 * maskFactor; mask[17] = 4.0 * maskFactor; mask[18] = 3.0 * maskFactor; mask[19] = 2.0 * maskFactor;
		mask[20] = 1.0 * maskFactor; mask[21] = 2.0 * maskFactor; mask[22] = 3.0 * maskFactor; mask[23] = 2.0 * maskFactor; mask[24] = 1.0 * maskFactor;
		break;
	case 2:
		mask[0] = 0;  mask[1] = 0;  mask[2] = -1; mask[3] = 0;  mask[4] = 0;
		mask[5] = 0;  mask[6] = -1; mask[7] = -2; mask[8] = -1; mask[9] = 0;
		mask[10] = -1; mask[11] = -2; mask[12] = 16; mask[13] = -2; mask[14] = -1;
		mask[15] = 0;  mask[16] = -1; mask[17] = -2; mask[18] = -1; mask[19] = 0;
		mask[20] = 0;  mask[21] = 0;  mask[22] = -1; mask[23] = 0;  mask[24] = 0;
		break;
	case 3:
		mask[0] = 0; mask[1] = 0; mask[2] = 0; mask[3] = 0.0; mask[4] =0;
		mask[5] = 0; mask[6] = -2.0; mask[7] = -1.0; mask[8] = 0; mask[9] = 0;
		mask[10] = 0; mask[11] = -1.0; mask[12] = 1.0; mask[13] = 1.0; mask[14] = 0;
		mask[15] = 0.0; mask[16] = 0; mask[17] = 1.0; mask[18] = 2.0; mask[19] = 0;
		mask[20] = 0; mask[21] = 0; mask[22] = 0; mask[23] = 0; mask[24] = 0;
		break;
	case 4:
		mask[0] = 0; mask[1] = 0; mask[2] = 0; mask[3] = 0.0; mask[4] = 0;
		mask[5] = 0; mask[6] = 0; mask[7] = 0; mask[8] = 0; mask[9] = 0;
		mask[10] = 0; mask[11] = -1.0; mask[12] = 1.0; mask[13] = 0; mask[14] = 0;
		mask[15] = 0.0; mask[16] = 0; mask[17] = 0; mask[18] = 0; mask[19] = 0;
		mask[20] = 0; mask[21] = 0; mask[22] = 0; mask[23] = 0; mask[24] = 0;
		break;
	case 5:
		mask[0] = 0; mask[1] = 0; mask[2] = 0; mask[3] = 0; mask[4] = 0;
		mask[5] = 0; mask[6] = 0; mask[7] = -1; mask[8] = 0; mask[9] = 0;
		mask[10] = 0; mask[11] = -1.0; mask[12] = 5.0; mask[13] = -1; mask[14] = 0;
		mask[15] = 0; mask[16] = 0; mask[17] = -1; mask[18] = 0; mask[19] = 0;
		mask[20] = 0; mask[21] = 0; mask[22] = 0; mask[23] = 0; mask[24] = 0;
		break;
	default:
		break;
	}
}
void mouseFunc(int button, int state, int x, int y){}

