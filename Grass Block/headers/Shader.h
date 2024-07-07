#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class Shader
{
public:
	unsigned int id;

	Shader() = default;

	void load(const std::string& shaderPath);

	//Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void use();

	void setBool(const std::string& name, bool value);
	void setInt(const std::string& name, int value);
	void setFloat(const std::string& name, float value);
	void setMat4(const std::string& name, const glm::mat4& mat);
	void setVec3(const std::string& name, const float& f1, const float& f2, const float& f3);
	void setVec3(const std::string& name, const glm::vec3& vec);

private:
	unsigned int CompileShader(unsigned int type, const std::string& source, const std::string& shaderPath);
};

void Shader::load(const std::string& shaderPath)
{
	std::ifstream stream(shaderPath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	ShaderType type = ShaderType::NONE;
	std::stringstream ss[2];

	while (getline(stream, line))
	{
		if (line.find("SHADER") != std::string::npos)
		{
			if (line.find("VERTEX") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("FRAGMENT") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}

		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	const std::string vertexShader = "#version 330 core\n #define SHADER_VERTEX\n #ifdef SHADER_VERTEX\n" + ss[0].str();
	const std::string fragmentShader = "#version 330 core\n #define SHADER_FRAGMENT\n #ifdef SHADER_FRAGMENT\n" + ss[1].str();

	// Compile shaders
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader, shaderPath);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader, shaderPath);

	// Link shaders
	id = glCreateProgram();
	glAttachShader(id, vs);
	glAttachShader(id, fs);
	glLinkProgram(id);
	glValidateProgram(id);

	// Check for linking errors
	int result;
	glGetProgramiv(id, GL_LINK_STATUS, &result);

	if (!result)
	{
		int length;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*)alloca(length * sizeof(char));
		//char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);

		std::cout << "[OpenGL Error] Linking error in \'" << shaderPath << "\'" << std::endl;
		std::cout << "Log: " << message << std::endl;

		glDeleteShader(vs);
		glDeleteShader(fs);

		glfwTerminate();
		exit(0);
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void Shader::use()
{
	glUseProgram(id);
}

void Shader::setBool(const std::string& name, bool value)
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const float& f1, const float& f2, const float& f3)
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), f1, f2, f3);
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec)
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), vec.x, vec.y, vec.z);
}

// Private utility function - to compile vertex and fragment shader
unsigned int Shader::CompileShader(unsigned int type, const std::string& source, const std::string& shaderPath)
{
	unsigned int shader = glCreateShader(type);
	const char* shaderSource = source.c_str();
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);


	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		// Generate info log
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(shader, length, &length, message);

		std::cout << "[OpenGL Error] Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader in \'" << shaderPath << "\'" << std::endl;
		std::cout << "Log: " << message << std::endl;

		glDeleteShader(shader);

		glfwTerminate();
		exit(0);
	}

	return shader;
}

#endif
