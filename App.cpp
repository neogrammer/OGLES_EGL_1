#include <App.h>

#define VERTEX_POS_SIZE 3
#define VERTEX_COLOR_SIZE 4
#define VERTEX_POS_INDX 0
#define VERTEX_COLOR_INDX 1
#define VERTEX_STRIDE   (sizeof(GLfloat) * \
						(VERTEX_POS_SIZE + \
						VERTEX_COLOR_SIZE))


#define POSITION_LOC    0
#define COLOR_LOC       1
#define MVP_LOC         2
#define TERRAIN_LOC     4


//
const GLchar* const lineShaderVertStr = 
"#version 300 es\n"
"layout(location = 2) in vec3 vPosition;\n"
"uniform mat4 a_mvpMatrix;   \n"
"void main()\n"
"{\n"
"gl_Position = a_mvpMatrix * vec4(vPosition,1.0);\n"
"}";

const GLchar* const lineShaderFragStr = 
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"fragColor = vec4(0.0,0.0,0.0,1.0);\n"
"}";

const GLchar* const heightTerrainVertStr =
"#version 300 es\n"
"uniform mat4 u_mvpMatrix;\n"
"uniform vec3 u_lightDirection;\n"
"layout(location = 0) in vec4 a_position;\n"
"uniform sampler2D s_texture;\n"
"out vec4 v_color;\n"
"void main()\n"
"{\n"
// compute vertex normal from height map
"float hxl = textureOffset(s_texture, a_position.xy, ivec2(-1, 0)).w;\n"
"float hxr = textureOffset(s_texture, a_position.xy, ivec2(1, 0)).w;\n"
"float hyl = textureOffset(s_texture, a_position.xy, ivec2(0, -1)).w;\n"
"float hyr = textureOffset(s_texture, a_position.xy, ivec2(0, 1)).w;\n"
"vec3 u = normalize(vec3(0.05, 0.0, hxr-hxl));\n"
"vec3 v = normalize(vec3(0.0, 0.05, hyr-hyl));\n"
"vec3 normal = cross(u, v);\n"
"// compute diffuse lighting\n"
"float diffuse = dot(normal, u_lightDirection);\n"
"v_color = vec4(vec3(diffuse), 1.0);\n"
"// get vertex position from height map\n"
"float h = texture(s_texture, a_position.xy).w;\n"
"vec4 v_position = vec4(a_position.xy,h / 2.5, a_position.w);\n"
"gl_Position = u_mvpMatrix * v_position;\n"
"}";

//const GLchar* const vShaderStr =
//"#version 300 es\n"
//"layout(location = 0) in vec4 a_position;\n"
//"layout(location = 1) in vec4 a_color;\n"
//"out vec4 v_color;\n"
//"void main()\n"
//"{\n"
//"	v_color = a_color;\n"
//"	gl_Position = a_position;\n"
//"}";
//
//const GLchar* const fShaderStr =
//"#version 300 es\n"
//"precision mediump float;\n"
//"in vec4 v_color;\n"
//"out vec4 o_fragColor;\n"
//"void main()\n"
//"{\n"
//"	o_fragColor = v_color;\n"
//"}";

//const GLchar* const vShaderStr =
//"#version 300 es\n"
//"layout(location = 0) in vec4 a_position;\n"
//"layout(location = 1) in vec4 a_color;\n"
//"out vec4 v_color;\n"
//"void main()\n"
//"{\n"
//"	v_color = a_color;\n"
//"	gl_Position = a_position;\n"
//"}";
//
//const GLchar* const fShaderStr =
//"#version 300 es\n"
//"precision mediump float;\n"
//"in vec4 v_color;\n"
//"out vec4 o_fragColor;\n"
//"void main()\n"
//"{\n"
//"	o_fragColor = v_color;\n"
//"}";

const GLchar* const vShaderStr = 
"#version 300 es                             \n"
"layout(location = 0) in vec3 a_position;    \n"
"layout(location = 1) in vec4 a_color;       \n"
"uniform mat4 a_mvpMatrix;   \n"
"out vec4 v_color;                           \n"
"void main()                                 \n"
"{                                           \n"
"   v_color = a_color;                       \n"
"   gl_Position = a_mvpMatrix * vec4(a_position, 1.0);  \n"
"}                                           \n";

const GLchar* const fShaderStr =
"#version 300 es                                \n"
"precision mediump float;                       \n"
"in vec4 v_color;                               \n"
"layout(location = 0) out vec4 outColor;        \n"
"void main()                                    \n"
"{                                              \n"
"  outColor = v_color;                          \n"
"}                                              \n";


void App::shutdown()
{

    glDeleteBuffers(1, &positionVBO);
    glDeleteBuffers(1, &colorVBO);
   // glDeleteBuffers(1, &mvpVBO);
    glDeleteBuffers(1, &indicesIBO);
    glDeleteBuffers(1, &edgeVBO);
    glDeleteTextures(1, &texture1);
    
    if (terrainData)
    {
        glDeleteBuffers(1, &terrainData->positionVBO);
        glDeleteBuffers(1, &terrainData->indicesIBO);
        glDeleteProgram(terrainData->programObject);
        delete terrainData;
    }



}

bool App::init(void* display, void* terrData)
{
    srand((unsigned int)time(0));
    angle = 0.f;
	basicShdr = {};
    lineShader = {};
	if (!basicShdr.setup(&vShaderStr, &fShaderStr)) {
		std::cout << "Shader not setup!" << std::endl;
		return false;
	}
    basicShdr.use();
    mvpLocation = glGetUniformLocation(basicShdr.id(), "a_mvpMatrix");

    glUseProgram(0);
  
    if (!lineShader.setup(&lineShaderVertStr, &lineShaderFragStr)) {
        std::cout << "line Shader not setup!" << std::endl;
        return false;
    }
    lineShader.use();
    mvpLocationLines = glGetUniformLocation(lineShader.id(), "a_mvpMatrix");


    GLfloat* positions;
    GLuint* indices;
    basicShdr.use();
    numIndices = myCube(1.f, &positions, NULL, NULL, &indices);

    // Index buffer object
    glGenBuffers(1, &indicesIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    free(indices);

    // Position VBO for cube model
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat) * 3, positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)NULL);
    glEnableVertexAttribArray(0);
    free(positions);

    // Random color for each instance
    {
        GLubyte colors[8][4]{};
        int vertexIdx;  

        GLfloat cube_colors[32]{};

        for (vertexIdx = 0; vertexIdx < 8; vertexIdx++)
        {
            colors[vertexIdx][0] = rand() % 255;
            colors[vertexIdx][1] = rand() % 255;
            colors[vertexIdx][2] = rand() % 255;
            colors[vertexIdx][3] = 255;

            cube_colors[vertexIdx * 4 + 0] = (GLfloat)((float)colors[vertexIdx][0] / 255.f);
            cube_colors[vertexIdx * 4 + 1] = (GLfloat)((float)colors[vertexIdx][1] / 255.f);
            cube_colors[vertexIdx * 4 + 2] = (GLfloat)((float)colors[vertexIdx][2] / 255.f);
            cube_colors[vertexIdx * 4 + 3] = (GLfloat)((float)colors[vertexIdx][3] / 255.f);

        }

        
        
        cube_colors[0] = cube_colors[3];
        cube_colors[1] = cube_colors[3];
        cube_colors[2] = cube_colors[3];
       

       /* cube_colors[1] = cube_colors[6];
        cube_colors[4] = cube_colors[3];
        cube_colors[5] = cube_colors[7];*/

        GLfloat cubecolors[32] = {
            // R     G     B     A
             0.0f, 0.0f, 1.0f, 1.0f,   // Red
             0.0f, 0.0f, 1.0f, 1.0f,   // Red
             0.0f, 0.0f, 1.0f, 1.0f,   // Red
             0.0f, 0.0f, 1.0f, 1.0f,   // Red
             0.0f, 0.0f, 1.0f, 1.0f,   // Blue
             0.0f, 0.0f, 1.0f, 1.0f,   // Blue
             0.0f, 0.0f, 1.0f, 1.0f,   // Blue
             0.0f, 0.0f, 1.0f, 1.0f   // Blue
        };



        //0.0f, 1.0f, 0.0f, 1.0f,   // Green
        //    0.0f, 0.0f, 1.0f, 1.0f,   // Blue
        //    1.0f, 1.0f, 0.0f, 1.0f,   // Yellow
        //    1.0f, 0.0f, 1.0f, 1.0f,   // Magenta
        //    0.0f, 1.0f, 1.0f, 1.0f,   // Cyan
        //    1.0f, 1.0f, 1.0f, 1.0f,   // White
        //    0.0f, 0.0f, 0.0f, 1.0f    // Black

        glGenBuffers(1, &colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat) *  4, cubecolors, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 4, GL_FLOAT,
            GL_FALSE, 4 * sizeof(GLfloat), (const void*)0);
        glEnableVertexAttribArray(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    GLfloat cubeEdges[72] = {
        // Bottom face
        -1.0f, -1.0f, -1.0001f,   1.0f, -1.0f, -1.0001f,   // Edge: bottom front
         1.0001f, -1.0f, -1.0f,   1.0001f, -1.0f,  1.0f,   // Edge: bottom right
         1.0f, -1.0f,  1.0001f,  -1.0f, -1.0f,  1.0001f,   // Edge: bottom back
        -1.0001f, -1.0f,  1.0f,  -1.0001f, -1.0f, -1.0f,   // Edge: bottom left

        // Top face
        -1.0f,  1.0f, -1.0001f,   1.0f,  1.0f, -1.0001f,   // Edge: top front
         1.0001f,  1.0f, -1.0f,   1.0001f,  1.0f,  1.0f,   // Edge: top right
         1.0f,  1.0f,  1.0001f,  -1.0f,  1.0f,  1.0001f,   // Edge: top back
        -1.0001f,  1.0f,  1.0f,  -1.0001f,  1.0f, -1.0f,   // Edge: top left

        // Vertical edges
        -1.0001f, -1.0f, -1.0f,  -1.0001f,  1.0f, -1.0f,   // Edge: front left
         1.0001f, -1.0f, -1.0f,   1.0001f,  1.0f, -1.0f,   // Edge: front right
         1.0001f, -1.0f,  1.0f,   1.0001f,  1.0f,  1.0f,   // Edge: back right
        -1.0001f, -1.0f,  1.0f,  -1.0001f,  1.0f,  1.0f    // Edge: back left
    };

    lineShader.use();

    glGenBuffers(1, &edgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat) * 3, cubeEdges, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray(2);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);


    //GLubyte pixels[4 * 3] =
    //{
    //    255, 0, 0,
    //    0, 255, 0,
    //    0, 0, 255,
    //    255, 255, 0
    //};

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //glGenTextures(1, &texture1);
    //glBindTexture(GL_TEXTURE_2D, texture1);



    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    //// Set the filtering mode
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    //    GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    //    GL_NEAREST);


    GLfloat* tpositions;
    GLuint* tindices;
    terrData = nullptr;
    terrainData = new TerrainData{};
    const GLchar* const vShaderTerrainStr =
        "#version 300 es                                      \n"
        "uniform mat4 u_mvpMatrix;                            \n"
        "uniform vec3 u_lightDirection;                       \n"
        "layout(location=4)in vec4 a_position;             \n"
        "uniform sampler2D s_texture;                         \n"
        "out vec4 v_color;                                    \n"
        "void main()                                          \n"
        "{                                                    \n"
        "   // compute vertex normal from height map          \n"
        "   float hxl = textureOffset( s_texture,             \n"
        "                  a_position.xy, ivec2(-1,  0) ).w;  \n"
        "   float hxr = textureOffset( s_texture,             \n"
        "                  a_position.xy, ivec2( 1,  0) ).w;  \n"
        "   float hyl = textureOffset( s_texture,             \n"
        "                  a_position.xy, ivec2( 0, -1) ).w;  \n"
        "   float hyr = textureOffset( s_texture,             \n"
        "                  a_position.xy, ivec2( 0,  1) ).w;  \n"
        "   vec3 u = normalize( vec3(0.05, 0.0, hxr-hxl) );   \n"
        "   vec3 v = normalize( vec3(0.0, 0.05, hyr-hyl) );   \n"
        "   vec3 normal = cross( u, v );                      \n"
        "                                                     \n"
        "   // compute diffuse lighting                       \n" 
        "   float diffuse = dot( normal, u_lightDirection );  \n"
        "   v_color = vec4( vec3(diffuse), 1.0 );                 \n"
        "                                                     \n"
        "   // get vertex position from height map            \n"
        "   float h = texture ( s_texture, a_position.xy ).w; \n"
        "   vec4 v_position = vec4 ( a_position.xy,           \n"
        "                            h/2.5,                   \n"
        "                            a_position.w );          \n"
        "   gl_Position = u_mvpMatrix * v_position;           \n"
        "}                                                    \n";

    const GLchar* const fShaderTerrainStr =
        "#version 300 es                                      \n"
        "precision mediump float;                             \n"
        "in vec4 v_color;                                     \n"
        "out vec4 outColor;              \n"
        "void main()                                          \n"
        "{                                                    \n"
        "  outColor = v_color;                                \n"
        "}                                                    \n";

    // Load the shaders and get a linked program object
    
    Shader terrShd = {};
 
    if (!terrShd.setup(&vShaderTerrainStr, &fShaderTerrainStr))
    {
        std::cout << "Shader not setup!" << std::endl;
        return false;
    }

    terrainData->programObject = terrShd.id();

    terrShd.setNULL();

    glUseProgram(terrainData->programObject);

    // Get the uniform locations
    terrainData->mvpLoc = glGetUniformLocation(terrainData->programObject, "u_mvpMatrix");
    terrainData->lightDirectionLoc = glGetUniformLocation(terrainData->programObject, "u_lightDirection");

    // Get the sampler terrainData
    terrainData->samplerLoc = glGetUniformLocation(terrainData->programObject, "s_texture");

    // Load the heightmap
    terrainData->textureId = LoadTexture(R"(assets/textures/heightmap3.tga)");

    if (terrainData->textureId == 0)
    {
        return FALSE;
    }



    // Generate the position and indices of a square grid for the base terrain
    terrainData->gridSize = 200;
    terrainData->numIndices = genSquareGrid(terrainData->gridSize, &tpositions, &tindices);

    // Index buffer for base terrain
    glGenBuffers(1, &terrainData->indicesIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->indicesIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainData->numIndices * sizeof(GLuint),
        tindices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    free(tindices);

    // Position VBO for base terrain
    glGenBuffers(1, &terrainData->positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainData->positionVBO);
    glBufferData(GL_ARRAY_BUFFER,
        terrainData->gridSize * terrainData->gridSize * sizeof(GLfloat) * 3,
        tpositions, GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray(4);
    free(tpositions);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(4);
    glBindTexture(GL_TEXTURE_2D, 0);

    glClearColor(0.2f, 0.6f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    terrData = (void*)terrainData;
   
    initialized = true;

    return GL_TRUE;
}

void App::update(float dt, int width, int height)
{

    windowWidth = width;
    windowHeight = height;

  /* 

    ESMatrix matrixBuf;
    ESMatrix matrixBuf2;

    ESMatrix perspective;*/
    //float    aspect;
    angle += 40.f * dt;
    if (angle >= 360.f)
        angle -= 360.f;
    // Compute the window aspect ratio
    //aspect = (GLfloat)width / (GLfloat)height;

    //// Generate a perspective matrix with a 60 degree FOV
    //matrixLoadIdentity(&perspective);
    //getPerspective(&perspective, 60.0f, aspect, 1.0f, 200.0f);

    //ESMatrix modelView;
    //matrixLoadIdentity(&modelView);
    //translate(&modelView, 0.f, 0.f, -20.f);
    //rotate(&modelView, angle, 1.f, 1.f, 0.f);
    //matrixMultiply(&matrixBuf, &modelView, &perspective);
    //basicShdr.use();
    //glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &matrixBuf.m[0][0]);

    //lineShader.use();
    //matrixMultiply(&matrixBuf2, &modelView, &perspective);
    //glUniformMatrix4fv(mvpLocationLines, 1, GL_FALSE, &matrixBuf2.m[0][0]);

       // Set the viewport
    glViewport(0, 0, windowWidth, windowHeight);



}

void App::render(void* display)
{

    
    // Clear only inside viewport with your scene color
    glClearColor(0.2f, 0.6f, 0.95f, 1.0f); // scene background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    ESMatrix perspective{};
    ESMatrix modelviewTerrain{};
    float    aspect;
   

    // Compute the window aspect ratio
    aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;

    // Generate a perspective matrix with a 60 degree FOV
    matrixLoadIdentity(&perspective);
    getPerspective(&perspective, 60.0f, aspect, 0.1f, 1000.0f);

    // Generate a model view matrix to rotate/translate the terrain
    matrixLoadIdentity(&modelviewTerrain);

    // Center the terrain
    translate(&modelviewTerrain, -40.f, -40.5f, -20.0f);
    scale(&modelviewTerrain, 100.f, 100.f, 100.f);
    // Rotate
    rotate(&modelviewTerrain, 45.0f, 1.0, 0.0, 0.0);

    // Compute the final MVP by multiplying the
    // modelview and perspective matrices together
    matrixMultiply(&terrainData->mvpMatrix, &modelviewTerrain, &perspective);

    //initMVP((void*)terrainData, windowWidth, windowHeight);

 
    

    // Use the program object
    glUseProgram(terrainData->programObject);

    // Load the vertex position
    glBindBuffer(GL_ARRAY_BUFFER, terrainData->positionVBO);
    glVertexAttribPointer(4, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)NULL);
    glEnableVertexAttribArray(4);

    // Bind the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainData->indicesIBO);

    // Bind the height map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainData->textureId);

    // Load the MVP matrix
    glUniformMatrix4fv(terrainData->mvpLoc, 1, GL_FALSE, (GLfloat*)&terrainData->mvpMatrix.m[0][0]);

    // Load the light direction
    glUniform3f(terrainData->lightDirectionLoc, 0.86f, 0.14f, 0.49f);

    // Set the height map sampler to texture unit to 0
    glUniform1i(terrainData->samplerLoc, 0);

    // Draw the grid
    glDrawElements(GL_TRIANGLES, terrainData->numIndices, GL_UNSIGNED_INT, (const void*)NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(4);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ESMatrix matrixBuf;
    ESMatrix matrixBuf2;

   // ESMatrix perspective;
  //  float    aspect;
   
    // Compute the window aspect ratio
 //   aspect = (GLfloat)windowWidth / (GLfloat)windowHeight;

    // Generate a perspective matrix with a 60 degree FOV
  //  matrixLoadIdentity(&perspective);
 //   getPerspective(&perspective, 60.0f, aspect, 1.0f, 200.0f);

    ESMatrix modelView;
    matrixLoadIdentity(&modelView);
    translate(&modelView, 0.f, 0.f, -5.f);
    rotate(&modelView, angle, 1.f, 1.f, 0.f);
    matrixMultiply(&matrixBuf, &modelView, &perspective);
   

  
    basicShdr.use();
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &matrixBuf.m[0][0]);


    // Load the vertex positio
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)NULL);
    glEnableVertexAttribArray(0);
    // Load the instance color buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT,
        GL_FALSE, 4 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

   
    lineShader.use();
    matrixMultiply(&matrixBuf2, &modelView, &perspective);
    glUniformMatrix4fv(mvpLocationLines, 1, GL_FALSE, &matrixBuf2.m[0][0]);

    
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray(2);
    glDrawArrays(GL_LINES, 0, 24);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);




 //  glVertexAttribDivisor(COLOR_LOC, 1); // One color per instance


    // Load the instance MVP buffer
   // glBindBuffer(GL_ARRAY_BUFFER, mvpVBO);

    // Load each matrix row of the MVP.  Each row gets an increasing attribute location.
    //glVertexAttribPointer(MVP_LOC + 0, 4, GL_FLOAT, GL_FALSE, sizeof(ESMatrix), (const void*)NULL);
    //glVertexAttribPointer(MVP_LOC + 1, 4, GL_FLOAT, GL_FALSE, sizeof(ESMatrix), (const void*)(sizeof(GLfloat) * 4));
    //glVertexAttribPointer(MVP_LOC + 2, 4, GL_FLOAT, GL_FALSE, sizeof(ESMatrix), (const void*)(sizeof(GLfloat) * 8));
    //glVertexAttribPointer(MVP_LOC + 3, 4, GL_FLOAT, GL_FALSE, sizeof(ESMatrix), (const void*)(sizeof(GLfloat) * 12));
    //glEnableVertexAttribArray(MVP_LOC + 0);
    //glEnableVertexAttribArray(MVP_LOC + 1);
    //glEnableVertexAttribArray(MVP_LOC + 2);
    //glEnableVertexAttribArray(MVP_LOC + 3);

    //// One MVP per instance
    //glVertexAttribDivisor(MVP_LOC + 0, 1);
    //glVertexAttribDivisor(MVP_LOC + 1, 1);
    //glVertexAttribDivisor(MVP_LOC + 2, 1);
    //glVertexAttribDivisor(MVP_LOC + 3, 1);

    // Bind the index buffer
   // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);

    // Draw the cubes
//glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (const void*)NULL, NUM_INSTANCES);



	/*basicShdr.use();

	glBindVertexArray(vaoId);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (const void*)0);

	glBindVertexArray(0);*/

	/*GLfloat vertices[] = { 
		0.f,  0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};
	basicShdr.use();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);*/
	
}

void App::resize(void* display, void* surface, int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    const float targetAspect = 16.f / 9.f;

    int viewWidth = width;
    int viewHeight = height;
    float windowAspect = (float)width / (float)height;

    if (windowAspect > targetAspect)
    {
        viewWidth = (int)(height * targetAspect);
        viewHeight = height;
    }
    else
    {
        viewWidth = width;
        viewHeight = (int)(width / targetAspect);
    }

    int viewX = (width - viewWidth) / 2;
    int viewY = (height - viewHeight) / 2;

    // Clear entire window to black for letterbox bars
    glViewport(0, 0, width, height);
   // glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black bars
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set viewport to letterboxed area for rendering
    glViewport(viewX, viewY, viewWidth, viewHeight);

    // DO NOT clear here again - your render() will clear viewport to blue
    if (terrainData != nullptr)
    {
        render(display);
    }
 //   eglSwapBuffers((EGLDisplay)display, (EGLSurface)surface);

}

bool App::isInitialized()
{
	return initialized;
}

void App::matrixLoadIdentity(ESMatrix* result)
{
    memset(result, 0x0, sizeof(ESMatrix));
    result->m[0][0] = 1.0f;
    result->m[1][1] = 1.0f;
    result->m[2][2] = 1.0f;
    result->m[3][3] = 1.0f;
}

int App::genCube(float scale, GLfloat** vertices, GLfloat** normals, GLfloat** texCoords, GLuint** indices)
{
    int i;
    int numVertices = 24;
    int numIndices = 36;

    GLfloat cubeVerts[] =
    {
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f,  0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f, -0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
       -0.5f,  0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
       0.5f,  0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
       0.5f,  0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f, 0.5f,
       -0.5f,  0.5f, 0.5f,
       0.5f,  0.5f, 0.5f,
       0.5f, -0.5f, 0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,
       0.5f, -0.5f, -0.5f,
       0.5f, -0.5f,  0.5f,
       0.5f,  0.5f,  0.5f,
       0.5f,  0.5f, -0.5f,
    };

    GLfloat cubeNormals[] =
    {
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
    };

    GLfloat cubeTex[] =
    {
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       1.0f, 0.0f,
       1.0f, 1.0f,
       0.0f, 1.0f,
       0.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
    };

    // Allocate memory for buffers
    if (vertices != NULL)
    {
        *vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numVertices);
        memcpy(*vertices, cubeVerts, sizeof(cubeVerts));

        for (i = 0; i < numVertices * 3; i++)
        {
            (*vertices)[i] *= scale;
        }
    }

    if (normals != NULL)
    {
        *normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numVertices);
        memcpy(*normals, cubeNormals, sizeof(cubeNormals));
    }

    if (texCoords != NULL)
    {
        *texCoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * numVertices);
        memcpy(*texCoords, cubeTex, sizeof(cubeTex));
    }


    // Generate the indices
    if (indices != NULL)
    {
        GLuint cubeIndices[] =
        {
           0, 2, 1,
           0, 3, 2,
           4, 5, 6,
           4, 6, 7,
           8, 9, 10,
           8, 10, 11,
           12, 15, 14,
           12, 14, 13,
           16, 17, 18,
           16, 18, 19,
           20, 23, 22,
           20, 22, 21
        };

        *indices = (GLuint*)malloc(sizeof(GLuint) * numIndices);
        memcpy(*indices, cubeIndices, sizeof(cubeIndices));
    }

    return numIndices;
}


int App::myCube(float scale, GLfloat** vertices, GLfloat** normals, GLfloat** texCoords, GLuint** indices)
{
    int i;
    int numVertices = 8;
    int numIndices = 36;

    GLfloat cubeVerts[] = {
        // front
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        // back
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0
    };



    //GLfloat cubeVerts[] =
    //{
    //   -0.5f, -0.5f, -0.5f,
    //   -0.5f, -0.5f,  0.5f,
    //   0.5f, -0.5f,  0.5f,
    //   0.5f, -0.5f, -0.5f,
    //   -0.5f,  0.5f, -0.5f,
    //   -0.5f,  0.5f,  0.5f,
    //   0.5f,  0.5f,  0.5f,
    //   0.5f,  0.5f, -0.5f,
    //   -0.5f, -0.5f, -0.5f,
    //   -0.5f,  0.5f, -0.5f,
    //   0.5f,  0.5f, -0.5f,
    //   0.5f, -0.5f, -0.5f,
    //   -0.5f, -0.5f, 0.5f,
    //   -0.5f,  0.5f, 0.5f,
    //   0.5f,  0.5f, 0.5f,
    //   0.5f, -0.5f, 0.5f,
    //   -0.5f, -0.5f, -0.5f,
    //   -0.5f, -0.5f,  0.5f,
    //   -0.5f,  0.5f,  0.5f,
    //   -0.5f,  0.5f, -0.5f,
    //   0.5f, -0.5f, -0.5f,
    //   0.5f, -0.5f,  0.5f,
    //   0.5f,  0.5f,  0.5f,
    //   0.5f,  0.5f, -0.5f,
    //};

    GLfloat cubeNormals[] =
    {
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, -1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
    };

    GLfloat cubeTex[] =
    {
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       1.0f, 0.0f,
       1.0f, 1.0f,
       0.0f, 1.0f,
       0.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
       0.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f,
       1.0f, 0.0f,
    };

    // Allocate memory for buffers
    if (vertices != NULL)
    {
        *vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numVertices);
        memcpy(*vertices, cubeVerts, sizeof(cubeVerts));

        for (i = 0; i < numVertices * 3; i++)
        {
            (*vertices)[i] *= scale;
        }
    }

    if (normals != NULL)
    {
        *normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numVertices);
        memcpy(*normals, cubeNormals, sizeof(cubeNormals));
    }

    if (texCoords != NULL)
    {
        *texCoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * numVertices);
        memcpy(*texCoords, cubeTex, sizeof(cubeTex));
    }


    // Generate the indices
    if (indices != NULL)
    {
        GLuint cubeIndices[] =
        {
            // front
           0, 1, 2,
           2, 3, 0,
           // right
           1, 5, 6,
           6, 2, 1,
           // back
           7, 6, 5,
           5, 4, 7,
           // left
           4, 0, 3,
           3, 7, 4,
           // bottom
           4, 5, 1,
           1, 0, 4,
           // top
           3, 2, 6,
           6, 7, 3
        };

        *indices = (GLuint*)malloc(sizeof(GLuint) * numIndices);
        memcpy(*indices, cubeIndices, sizeof(cubeIndices));
    }

    return numIndices;
}


void App::matrixMultiply(ESMatrix* result, ESMatrix* srcA, ESMatrix* srcB)
{
    ESMatrix    tmp{};
    int         i;

    for (i = 0; i < 4; i++)
    {
        tmp.m[i][0] = (srcA->m[i][0] * srcB->m[0][0]) +
            (srcA->m[i][1] * srcB->m[1][0]) +
            (srcA->m[i][2] * srcB->m[2][0]) +
            (srcA->m[i][3] * srcB->m[3][0]);

        tmp.m[i][1] = (srcA->m[i][0] * srcB->m[0][1]) +
            (srcA->m[i][1] * srcB->m[1][1]) +
            (srcA->m[i][2] * srcB->m[2][1]) +
            (srcA->m[i][3] * srcB->m[3][1]);

        tmp.m[i][2] = (srcA->m[i][0] * srcB->m[0][2]) +
            (srcA->m[i][1] * srcB->m[1][2]) +
            (srcA->m[i][2] * srcB->m[2][2]) +
            (srcA->m[i][3] * srcB->m[3][2]);

        tmp.m[i][3] = (srcA->m[i][0] * srcB->m[0][3]) +
            (srcA->m[i][1] * srcB->m[1][3]) +
            (srcA->m[i][2] * srcB->m[2][3]) +
            (srcA->m[i][3] * srcB->m[3][3]);
    }

    memcpy(result, &tmp, sizeof(ESMatrix));
}
void App::frustum(ESMatrix* result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    ESMatrix    frust;

    if ((nearZ <= 0.0f) || (farZ <= 0.0f) ||
        (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f))
    {
        return;
    }

    frust.m[0][0] = 2.0f * nearZ / deltaX;
    frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

    frust.m[1][1] = 2.0f * nearZ / deltaY;
    frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

    frust.m[2][0] = (right + left) / deltaX;
    frust.m[2][1] = (top + bottom) / deltaY;
    frust.m[2][2] = -(nearZ + farZ) / deltaZ;
    frust.m[2][3] = -1.0f;

    frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

    matrixMultiply(result, &frust, result);
}
void App::getPerspective(ESMatrix* result, float fovy, float aspect, float nearZ, float farZ)
{
    GLfloat frustumW, frustumH;

    frustumH = tanf(fovy / 360.0f * PI) * nearZ;
    frustumW = frustumH * aspect;

    frustum(result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ);
}


void App::translate(ESMatrix* result, GLfloat tx, GLfloat ty, GLfloat tz)
{
    result->m[3][0] += (result->m[0][0] * tx + result->m[1][0] * ty + result->m[2][0] * tz);
    result->m[3][1] += (result->m[0][1] * tx + result->m[1][1] * ty + result->m[2][1] * tz);
    result->m[3][2] += (result->m[0][2] * tx + result->m[1][2] * ty + result->m[2][2] * tz);
    result->m[3][3] += (result->m[0][3] * tx + result->m[1][3] * ty + result->m[2][3] * tz);
}


void App::scale(ESMatrix* result, GLfloat sx, GLfloat sy, GLfloat sz)
{
    result->m[0][0] *= sx;
    result->m[0][1] *= sx;
    result->m[0][2] *= sx;
    result->m[0][3] *= sx;

    result->m[1][0] *= sy;
    result->m[1][1] *= sy;
    result->m[1][2] *= sy;
    result->m[1][3] *= sy;

    result->m[2][0] *= sz;
    result->m[2][1] *= sz;
    result->m[2][2] *= sz;
    result->m[2][3] *= sz;
}


void App::rotate(ESMatrix* result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat sinAngle, cosAngle;
    GLfloat mag = sqrtf(x * x + y * y + z * z);

    sinAngle = sinf(angle * PI / 180.0f);
    cosAngle = cosf(angle * PI / 180.0f);

    if (mag > 0.0f)
    {
        GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
        GLfloat oneMinusCos;
        ESMatrix rotMat;

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
        rotMat.m[0][1] = (oneMinusCos * xy) - zs;
        rotMat.m[0][2] = (oneMinusCos * zx) + ys;
        rotMat.m[0][3] = 0.0F;

        rotMat.m[1][0] = (oneMinusCos * xy) + zs;
        rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
        rotMat.m[1][2] = (oneMinusCos * yz) - xs;
        rotMat.m[1][3] = 0.0F;

        rotMat.m[2][0] = (oneMinusCos * zx) - ys;
        rotMat.m[2][1] = (oneMinusCos * yz) + xs;
        rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
        rotMat.m[2][3] = 0.0F;

        rotMat.m[3][0] = 0.0F;
        rotMat.m[3][1] = 0.0F;
        rotMat.m[3][2] = 0.0F;
        rotMat.m[3][3] = 1.0F;

        matrixMultiply(result, &rotMat, result);
    }
}

int App::genSquareGrid(int size, GLfloat** vertices, GLuint** indices)
{
    int i, j;
    int numIndices = (size - 1) * (size - 1) * 2 * 3;
    // Allocate memory for buffersb
    if (vertices != NULL)
    {
        int numVertices = size * size;
        float stepSize = (float)size - 1;
        *vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numVertices);
        for (i = 0; i < size; ++i) // row
        {
            for (j = 0; j < size; ++j) // column
            {
                (*vertices)[3 * (j + i * size)] = i / stepSize;
                (*vertices)[3 * (j + i * size) + 1] = j / stepSize;
                (*vertices)[3 * (j + i * size) + 2] = 0.0f;
            }
        }
    }
    // Generate the indices
    if (indices != NULL)
    {
        *indices = (GLuint*)malloc(sizeof(GLuint) * numIndices);
        for (i = 0; i < size - 1; ++i)
        {
            for (j = 0; j < size - 1; ++j)
            {
                // two triangles per quad
                (*indices)[6 * (j + i * (size-1))] = j + (i) * (size);
                (*indices)[6 * (j + i * (size-1)) + 1] = j + (i) * (size)+1;
                (*indices)[6 * (j + i * (size-1)) + 2] = j + (i + 1) * (size)+1;
                (*indices)[6 * (j + i * (size-1)) + 3] = j + (i) * (size);
                (*indices)[6 * (j + i * (size-1)) + 4] = j + (i + 1) * (size)+1;
                (*indices)[6 * (j + i * (size-1)) + 5] = j + (i + 1) * (size);
            }
        }
    }
    return numIndices;
}

char* App::loadTGA(const char* fileName, int* width, int* height)
{
    char* buffer;
    esFile* fp=nullptr;
    TGA_HEADER   Header;
    int          bytesRead;

    // Open the file for reading
    fp = fileOpen(fileName);

    if (fp == nullptr)
    {
        // Log error as 'error in opening the input file from apk'
        std::cerr << "loadTGA FAILED to load : \n" << fileName << std::endl;
        return NULL;
    }

    bytesRead = fileRead(fp, sizeof(TGA_HEADER), &Header);

    *width = Header.Width;
    *height = Header.Height;

    if (Header.ColorDepth == 8 ||
        Header.ColorDepth == 24 || Header.ColorDepth == 32)
    {
        int bytesToRead = sizeof(char) * (*width) * (*height) * Header.ColorDepth / 8;

        // Allocate the image data buffer
        buffer = (char*)malloc(bytesToRead);

        if (buffer)
        {
            bytesRead = fileRead(fp, bytesToRead, buffer);
            fileClose(fp);

            return (buffer);
        }
    }

    return (NULL);
}


GLuint App::LoadTexture(const char* fileName)
{

    int width,
        height;

    char* buffer = loadTGA(fileName, &width, &height);
    GLuint texId;

    if (buffer == NULL)
    {
        std::cerr << "Error loading image\n" << fileName << std::endl;
        return 0;
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(buffer);

    return texId;
}

int App::initMVP(void* context, int winWidth, int winHeight)
{
    ESMatrix perspective;
    ESMatrix modelview;
    float    aspect;
    TerrainData* userData = (TerrainData*)context;

    // Compute the window aspect ratio
    aspect = (GLfloat)winWidth / (GLfloat)winHeight;

    // Generate a perspective matrix with a 60 degree FOV
    matrixLoadIdentity(&perspective);
    getPerspective(&perspective, 60.0f, aspect, 0.1f, 200.0f);

    // Generate a model view matrix to rotate/translate the terrain
    matrixLoadIdentity(&modelview);

    // Center the terrain
    translate(&modelview, -0.5f, -0.5f, -0.7f);

    // Rotate
    rotate(&modelview, 45.0f, 1.0, 0.0, 0.0);

    // Compute the final MVP by multiplying the
    // modelview and perspective matrices together
    matrixMultiply(&userData->mvpMatrix, &modelview, &perspective);

    initialized = true;

    return TRUE;
}
