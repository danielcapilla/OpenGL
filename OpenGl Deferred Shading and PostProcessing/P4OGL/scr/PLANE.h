#ifndef __PLANEFILE__
#define __PLANEFILE__

//Describimos un plano en coordenadas normalizadas
//*************************************************

//N�mero de v�rtices
const int planeNVertex = 4; // 4 v�rtices 

//Posic�on de los vertices
const float planeVertexPos[] = { 
	//Cara z = 1
	-1.0f,	-1.0f,	 0.99f, 
	 1.0f,	-1.0f,	 0.99f, 
	-1.0f,	 1.0f,	 0.99f, 
	 1.0f,	 1.0f,	 0.99f, 
 };

#endif