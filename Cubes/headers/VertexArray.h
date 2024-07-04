#pragma once

#include <glad/glad.h>

class VertexArray
{
private:
	unsigned int m_VertexArrayID = 0;

public:
	VertexArray()
	{
	}

	void generate()
	{
		glGenVertexArrays(1, &m_VertexArrayID);
		glBindVertexArray(m_VertexArrayID);
	}

	void bind() const
	{
		glBindVertexArray(m_VertexArrayID);
	}

	void unbind() const
	{
		glBindVertexArray(0);
	}

	void free() const
	{
		glDeleteVertexArrays(1, &m_VertexArrayID);
	}
};