#include "Shader.h"
#include <iostream>
Shader::~Shader()
{
	destroy();
}
bool Shader::setup(const GLchar* const* vertCodeStr, const GLchar* const* fragCodeStr)
{
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertShader, (GLsizei)1, vertCodeStr, NULL);
	glShaderSource(fragShader, (GLsizei)1, fragCodeStr, NULL);

	glCompileShader(vertShader);
	glCompileShader(fragShader);

	GLint vCompiled;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &vCompiled);
	GLint fCompiled;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fCompiled);

	if (vCompiled != GL_TRUE) {
		GLsizei logLength = 0;
		GLchar message[1024];
		glGetShaderInfoLog(vertShader, 1024, &logLength, message);

		std::cout << "Error compiling vertex shader : " << message << std::endl;

		glDeleteShader(vertShader);
		free(message);
		return false;
	}
	if (fCompiled != GL_TRUE) {
		GLsizei logLength = 0;
		GLchar message[1024];
		glGetShaderInfoLog(fragShader, 1024, &logLength, message);

		std::cout << "Error compiling fragment shader : " << message << std::endl;

		glDeleteShader(fragShader);
		free(message);
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	GLint pLinked;
	glGetProgramiv(program, GL_LINK_STATUS, &pLinked);
	
	if (pLinked != GL_TRUE) {
		GLsizei logLength = 0;
		GLchar message[1024];
		glGetProgramInfoLog(program, 1024, &logLength, message);

		std::cout << "Error linking program : " << message << std::endl;

		glDetachShader(program, vertShader);
		glDetachShader(program, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
		glDeleteProgram(program);
		free(message);
		return false;
	}

	return true;
}

void Shader::use()
{
	glUseProgram(program);
}

void Shader::destroy()
{
	if (program)
		glDeleteProgram(program);
}

GLuint Shader::id()
{
	return program;
}

