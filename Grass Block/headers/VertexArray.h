#pragma once

#include <glad/glad.h>

class VertexArray
{
private:
	unsigned int m_VertexArrayID = 0;

public:
	VertexArray() = default;

	void generate();

	void bind() const;

	void unbind() const;

	void free() const;
};

void VertexArray::generate()
{
	glGenVertexArrays(1, &m_VertexArrayID);
	glBindVertexArray(m_VertexArrayID);
}

void VertexArray::bind() const
{
	glBindVertexArray(m_VertexArrayID);
}

void VertexArray::unbind() const
{
	glBindVertexArray(0);
}

void VertexArray::free() const
{
	glDeleteVertexArrays(1, &m_VertexArrayID);
}