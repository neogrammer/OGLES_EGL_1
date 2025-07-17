#ifndef APP_H__
#define APP_H__

#include <Shader.h>


#define NUM_INSTANCES   100
#define PI 3.1415926535897932384626433832795f

class App
{
public:
	typedef struct
	{
		GLfloat   m[4][4];
	} ESMatrix;
private:
	Shader basicShdr;
	Shader lineShader;
	//GLuint vboId1;
	//GLuint vboId2;
	//GLuint vaoId;

	GLuint edgeVBO;
	// VBOs
	GLuint positionVBO;
	GLuint colorVBO;
	//GLuint mvpVBO;
	GLuint indicesIBO;
	GLuint mvpLocation;
	GLuint mvpLocationLines;

	// Number of indices
	int       numIndices;

	float angle;


	bool initialized{ false };

	// Rotation angle
	//GLfloat   angle[NUM_INSTANCES];

public:


	
	void shutdown();
	bool init(void* display);
	void update(float dt, int width, int height);
	void render(void* display);
	void resize(void* display, void* surface, int width, int height);
	bool isInitialized();
	void matrixLoadIdentity(ESMatrix* result);

	void matrixMultiply(ESMatrix* result, ESMatrix* srcA, ESMatrix* srcB);
	void frustum(ESMatrix* result, float left, float right, float bottom, float top, float nearZ, float farZ);
	void getPerspective(ESMatrix* result, float fovy, float aspect, float nearZ, float farZ);
	int genCube(float scale, GLfloat** vertices, GLfloat** normals,
		GLfloat** texCoords, GLuint** indices);

	int myCube(float scale, GLfloat** vertices, GLfloat** normals,
		GLfloat** texCoords, GLuint** indices);


	void translate(ESMatrix* result, GLfloat tx, GLfloat ty, GLfloat tz);


	void scale(ESMatrix* result, GLfloat sx, GLfloat sy, GLfloat sz);


	void rotate(ESMatrix* result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
};

#endif