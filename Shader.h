#ifndef SHADER_H__
#define SHADER_H__

#include <GLES3/gl3.h>

class Shader
{
	GLuint program;

public:
	Shader() = default;
	~Shader();
	bool setup(const GLchar* const* vertCodeStr, const GLchar* const* fragCodeStr);
	void use();
	void destroy();
	GLuint id();
};

#endif