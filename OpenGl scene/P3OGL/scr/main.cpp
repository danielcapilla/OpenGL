#include "BOX.h"
#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model1 = glm::mat4(1.0f);
glm::mat4	model2 = glm::mat4(1.0f);

// Variables globales de la luz
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 lightInt = glm::vec3(1.0f, 1.0f, 1.0f);

// Vectores de la cámara
glm::vec3 up = glm::vec3(0, 1, 0);		// Vector que apunta hacia arriba, perpendicular a forward
glm::vec3 forward = glm::vec3(0, 0, -1);		// El vector en el que apunta la cámara (en OpenGL es -k en el s. ref. de la cámara)
glm::vec3 right = glm::cross(forward, up);	// Como usamos forward en -Z, right es forward X up
glm::vec3 cop = glm::vec3(0, 0, 6);


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//Shaders:
unsigned int vshader1;
unsigned int fshader1;
unsigned int program1;

unsigned int vshader2;
unsigned int fshader2;
unsigned int program2;

int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

int uLightPos;
int uLightInt;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int texId;



//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname, unsigned int &program, unsigned int& vshader, unsigned int& fshader);
void initObj();
void destroy();

//Funciones auxiliares
void renderModel(unsigned int program, const glm::mat4& model);
void setUniforms(unsigned int program, const glm::mat4& model);

//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName1, const char* fileName2);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag",program1,vshader1,fshader1);
	initShader("../shaders_P3/shader.v1.2.vert", "../shaders_P3/shader.v1.2.frag", program2, vshader2, fshader2);
	initObj();

	glutMainLoop();


	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");


	//glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;


	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);


}

void initOGL(){
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);	
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -6;

}

void destroy(){}

void initShader(const char *vname, const char *fname, unsigned int &program, unsigned int& vshader, unsigned int& fshader)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Link error: \n" << logString << std::endl;
		delete[] logString;
		exit(-1);
	}
	std::cout << "Todo OK, eres un máquina\n";


	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uLightPos = glGetUniformLocation(program, "lpos");
	uLightInt = glGetUniformLocation(program, "Id");

	int uColorTex = glGetUniformLocation(program, "colorTex");
	int uEmiTex = glGetUniformLocation(program, "emiTex");
	int uAtlasTex = glGetUniformLocation(program, "atlasTex");

	int inPos = glGetAttribLocation(program, "inPos");
	int inColor = glGetAttribLocation(program, "inColor");
	int inNormal = glGetAttribLocation(program, "inNormal");
	int inTexCoord = glGetAttribLocation(program, "inTexCoord");

	glUseProgram(program);

	if (uAtlasTex != -1) glUniform1i(uAtlasTex, 3);



}

void initObj()
{
	unsigned int stride = 3 + 3 + 3 + 2;
	float* cpu_buff = new float[cubeNVertex * stride];

	for (unsigned int i = 0; i < cubeNVertex; i++)
	{
		unsigned int idx = stride * i;
		unsigned int i3 = i * 3;
		cpu_buff[idx + 0] = cubeVertexPos[i3 + 0];
		cpu_buff[idx + 1] = cubeVertexPos[i3 + 1];
		cpu_buff[idx + 2] = cubeVertexPos[i3 + 2];
		cpu_buff[idx + 3] = cubeVertexColor[i3 + 0];
		cpu_buff[idx + 4] = cubeVertexColor[i3 + 1];
		cpu_buff[idx + 5] = cubeVertexColor[i3 + 2];
		cpu_buff[idx + 6] = cubeVertexNormal[i3 + 0];
		cpu_buff[idx + 7] = cubeVertexNormal[i3 + 1];
		cpu_buff[idx + 8] = cubeVertexNormal[i3 + 2];
		cpu_buff[idx + 9] = cubeVertexTexCoord[(i * 2) + 0];
		cpu_buff[idx + 10] = cubeVertexTexCoord[(i * 2) + 1];
		//std::cout << i << "\n";
	}



	unsigned int buff;
	glGenBuffers(1, &buff);

	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER,
		(cubeNVertex * stride * sizeof(float)) + (cubeNTriangleIndex * 3 * sizeof(unsigned int)),
		NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, cubeNVertex * stride * sizeof(float), (void*)cpu_buff);
	glBufferSubData(GL_ARRAY_BUFFER,
		(3 + 3 + 3 + 2) * cubeNVertex * sizeof(float),
		3 * cubeNTriangleIndex * sizeof(unsigned int), cubeTriangleIndex);



	//Conf Geom.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buff);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2); glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff);

	model1 = glm::mat4(1.0f);



	//colorTexId = loadTex("../img/color2.png");
	//emiTexId = loadTex("../img/emissive.png");
	texId = loadTex("../img/emissive.png", "../img/color2.png");
}



void initObj2()
{
	unsigned int buff;
	glGenBuffers(1, &buff);

	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER, 
		(cubeNVertex * sizeof(float) * (3+3+3+2)) + (cubeNTriangleIndex * 3 * sizeof(unsigned int)),
		NULL, GL_STATIC_DRAW);
		
	glBufferSubData(GL_ARRAY_BUFFER, 0, cubeNVertex * sizeof(float) * 3, cubeVertexPos);
	glBufferSubData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeNVertex * sizeof(float) * 3, cubeVertexColor);
	glBufferSubData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * (3+3), cubeNVertex * sizeof(float) * 3, cubeVertexNormal);
	glBufferSubData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * (3 + 3 + 3), cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord);
	glBufferSubData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * (3 + 3 + 3 + 2), cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex);

	//
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buff);


	glVertexAttribPointer(0, //Location
		3, GL_FLOAT,  //vec 3
		GL_FALSE,//Norm
		0, 0); //Stride, Offset
	
	glVertexAttribPointer(1, //Location
		3, GL_FLOAT,  //vec 3
		GL_FALSE,//Norm
		0, (void *)(3 * sizeof(float)* cubeNVertex)); //Stride, Offset

	glVertexAttribPointer(2, //Location
		3, GL_FLOAT,  //vec 3
		GL_FALSE,//Norm
		0, (void*)((3 + 3) * sizeof(float) * cubeNVertex)); //Stride, Offset

	glVertexAttribPointer(3, //Location
		2, GL_FLOAT,  //vec2
		GL_FALSE,//Norm
		0, (void*)((3 + 3 + 3) * sizeof(float) * cubeNVertex)); //Stride, Offset

	glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2); glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff);

	
	model1 = glm::mat4(1.0f);
}

void initObj1()
{
	unsigned int buff[5];
	glGenBuffers(5, buff);
	posVBO = buff[0];
	colorVBO = buff[1];
	normalVBO = buff[2];
	texCoordVBO = buff[3];
	triangleIndexVBO = buff[4];

	//glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, cubeNVertex * sizeof(float) * 3, cubeVertexPos);

	//glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexColor, GL_STATIC_DRAW);

	//glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
		cubeVertexNormal, GL_STATIC_DRAW);

	//glGenBuffers(1, &texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,
		cubeVertexTexCoord, GL_STATIC_DRAW);

	//glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	//
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glVertexAttribPointer(0, //Location
		3, GL_FLOAT,  //vec 3
		GL_FALSE,//Norm
		0, 0); //Stride,Offset
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);


	model1 = glm::mat4(1.0f);
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	
	
	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error en " << fileName << ":\n" << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader; 
}


unsigned int loadTex(const char *fileName1, const char* fileName2){
	unsigned char* map1, *map2;
	unsigned int w1, h1, w2,h2;
	map1 = loadTexture(fileName1, w1, h1);
	map2 = loadTexture(fileName2, w2, h2);
	
	if (!map1 || !map2 )
	{
		std::cout << "Error cargando texturas \n";
		exit(-1);
	}
	std::cout << w1 << " " << h1 << std::endl;

	unsigned int texId=0;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8, w1+w2, h1, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0, w1, h1,
		GL_RGBA, GL_UNSIGNED_BYTE, (void*)map1);

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		w1, 0, w2, h2,
		GL_RGBA, GL_UNSIGNED_BYTE, (void*)map2);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_2D);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error al cargar la textura: " << gluErrorString(error) << std::endl;
	}


	return texId; 
}
void setUniforms(unsigned int program, const glm::mat4& model) {
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	//Iluminación en coordenadas de la cámara
	glm::vec3 lightPosCamera = glm::vec3(view * glm::vec4(lightPos, 1.0f));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));
	if (uLightPos != -1)
		glUniform3fv(uLightPos, 1, &(lightPosCamera[0]));
	if (uLightInt != -1)
		glUniform3fv(uLightInt, 1, &(lightInt[0]));
}
void renderModel(unsigned int program, const glm::mat4& model) {
	glUseProgram(program);
	setUniforms(program, model);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)(11 * sizeof(float) * cubeNVertex));
}
void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Configuración de textura (la misma para los dos)
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, texId);

	// Renderizar modelos (dos cubos, dos programs, dos pares de shaders)
	renderModel(program1, model1);
	renderModel(program2, model2);

	glutSwapBuffers();
}

void resizeFunc(int width, int height)
{
	//Ajusta el aspect ratio al tamaño de la venta
	float aspectRatio = (float)width / (float)height;
	//float n = 0.1;		
	//float f = 50;		
	//proj[0][0] = 1.7320508 / aspectRatio; 
	//proj[1][1] = 1.7320508;
	//proj[2][2] = -(f + n) / (n - f);
	//proj[2][3] = -1.0;
	//proj[3][2] = -2 * n * f / (n - f);
	//glm::mat4 perspective(float fovy, float aspect, float zNear,	float zFar);
	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);
	//Define el area de la ventana donde se va a dibujar
	glViewport(0, 0, width, height);
}

void idleFunc()
{
	model1 = glm::mat4(1.0f);
	model2 = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	//model1 = glm::rotate(model1, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	glm::mat4 model2SelfRotate = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model2Scale = glm::scale(glm::mat4(1.f), glm::vec3(0.3f, 0.3f, 0.3f));
	//glm::mat4 modelTB = glm::translate(glm::mat4(1.f), glm::vec3(xt, yt, zt)); //Posicion respecto a los puntos de control
	glm::mat4 model2Translate = glm::translate(glm::mat4(1.f), glm::vec3(1.8f, 1.0, 1.0));
	glm::mat4 model2OrbitRotate = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	//Transformaciones locales: model2SelfRotate * model2Scale
	//Transformaciones globales: model2OrbitRotate * model2Translate
	model2 = model2OrbitRotate * model2Translate  * model2SelfRotate * model2Scale;

	glutPostRedisplay();
}

/*
* Callback function invocada cuando se presiona una tecla del teclado.
*/
void updateView() {

	// La matriz view puede expresarse como:
	//
	//		|						  |
	//  V =	|  B^(-1)   -B^(-1)*CoP   |
	//		|						  |
	//		|    0           1        |
	//      |                         |
	// 
	// · B^(-1) es la traspuesta de los vectores de la base al ser ortonormal
	// · -B^(-1)*CoP equivale al producto punto de cada vector de la base por CoP
	// · Añadimos las coordenadas homogéneas (0 - vector / 1 - punto)
	// · Le ponemos signo negativo a forward, porque en OpenGL miramos en -Z

	const glm::mat3 R = glm::mat3(right, up, forward);
	const glm::mat3 Rinv = glm::transpose(R);
	const glm::vec3 T = -Rinv * cop;

	 view = glm::mat4(
		Rinv[0][0], Rinv[1][0], -Rinv[2][0], 0.f,	// 1ª columna
		Rinv[0][1], Rinv[1][1], -Rinv[2][1], 0.f,	// 2ª columna
		Rinv[0][2], Rinv[1][2], -Rinv[2][2], 0.f,	// 3ª columna
		T[0], T[1], -T[2], 1.0f	// 4ª columna
	);


}

void keyboardFunc(unsigned char key, int x, int y)
{
	const float distanceStep  = 0.3f;
	const float rotationAngle = 0.03f;

	const static glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	const static glm::mat4 RyInv = glm::transpose(Ry);
	//Controles de la cámara
	if (key == 'a') {
		cop += right * -distanceStep;
		updateView();
	}
	if (key == 'd') {
		cop += right * distanceStep;
		updateView();
	}
	if (key == 'w') {
		cop += forward * distanceStep;
		updateView();
	}
	if (key == 's') {
		cop += forward * -distanceStep;
		updateView();
	}
	if (key == 'q') {
		forward = glm::vec3(Ry * glm::vec4(forward, 0.0f));
		right = glm::cross(forward, up);
		updateView();
	}
	if (key == 'e') {
		forward = glm::vec3(RyInv * glm::vec4(forward, 0.0f));
		right = glm::cross(forward, up);
		updateView();
	}
	//Controles de la luz
	if (key == 'i'){
		lightInt *= 1.1;
	}
	if (key == 'j'){
		lightInt *= 0.9;
	}
	
	if (key == 'o'){
		lightPos += glm::vec3(0.0f, 0.0f, -1.0f);
	}
	if (key == 'l'){
		lightPos += glm::vec3(0.0f, 0.0f, 1.0f);
	}
}

void mouseFunc(int button, int state, int x, int y){}









