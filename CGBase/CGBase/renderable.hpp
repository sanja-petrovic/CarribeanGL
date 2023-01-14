#include <iostream>
#include <GL/glew.h>

class Renderable { 
public:
	unsigned int VAO, VBO, EBO;
	unsigned int vCount;
	unsigned int iCount;
	static int rCount;
	Renderable(const float* vertices, const unsigned int verticesSize, const  unsigned int* indices, const int indicesSize);
	~Renderable();
	void Render();
};