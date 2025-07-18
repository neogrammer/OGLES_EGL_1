#ifndef APP_H__
#define APP_H__

#include <Shader.h>
#include <iostream>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <random>
#include <cstdlib> // For rand() and srand()
#include <ctime>  

#define NUM_INSTANCES   100
#define PI 3.1415926535897932384626433832795f
#define TERRAIN_POS_LOC 0
class App
{
public:
	typedef struct
	{
		GLfloat   m[4][4];
	} ESMatrix;
	
	typedef FILE esFile;


	typedef struct
	{
		unsigned char  IdSize,MapType,ImageType;
		unsigned short PaletteStart,PaletteSize;
		//unsigned char  PaletteEntryDepth;
		unsigned short X,Y,	Width,Height;
		unsigned char  ColorDepth,Descriptor;
	} TGA_HEADER;

	typedef struct
	{
		// Handle to a program object
		GLuint programObject;

		// Uniform locations
		GLint  mvpLoc;
		GLint  lightDirectionLoc;

		// Sampler location
		GLint samplerLoc;

		// Texture handle
		GLuint textureId;

		// VBOs
		GLuint positionVBO;
		GLuint indicesIBO;

		// Number of indices
		int    numIndices;

		// dimension of grid
		int    gridSize;

		// MVP matrix
		ESMatrix  mvpMatrix;
	} TerrainData;

	TerrainData* terrainData;
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

	GLuint texture1;

	// Number of indices
	int       numIndices;

	float angle;
	int windowWidth;
	int windowHeight;

	bool initialized{ false };



	// Rotation angle
	//GLfloat   angle[NUM_INSTANCES];

public:


	///
	// esFileRead()
	//
	//    Wrapper for platform specific File open
	//
	static esFile* fileOpen(const char* fileName)
	{
		FILE* pFile = nullptr;
		fopen_s(&pFile, fileName, "rb");
		if (pFile == nullptr) return nullptr;

		return (esFile*)pFile;
	};


	///
	// esFileRead()
	//
	//    Wrapper for platform specific File close
	//
	static void fileClose(esFile* pFile)
	{
		if (pFile != nullptr)
		{
			fclose(pFile);
			pFile = nullptr;
		}
	};

	///
	// esFileRead()
	//
	//    Wrapper for platform specific File read
	//
	static int fileRead(esFile* pFile, int bytesToRead, void* buffer)
	{
		int bytesRead = 0;

		if (buffer == nullptr)
		{
			return bytesRead;
		}
		bytesRead = (int)fread(buffer, bytesToRead, 1, pFile);
		return bytesRead;
	};
	
	void shutdown();
	bool init(void* display, void* terrData);
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

	int genSquareGrid(int size, GLfloat** vertices,GLuint** indices);



	///
	// esLoadTGA()
	//
	//    Loads a 8-bit, 24-bit or 32-bit TGA image from a file
	//
	char* loadTGA(const char* fileName, int* width, int* height);


	///
// Load texture from disk
//
	GLuint LoadTexture(const char* fileName);
	///
	// Initialize the MVP matrix
	//
	int initMVP(void* context, int winWidth, int winHeight);
};

#endif