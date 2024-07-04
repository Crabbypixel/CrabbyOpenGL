#pragma once

#include <glad/glad.h>

class IndexBuffer
{
private:
	unsigned int m_IndexBufferID = 0;

public:
	IndexBuffer()
	{
	}

	void generate()
	{
		glGenBuffers(1, &m_IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
	}

	void bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
	}

	void unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void setBuffer(size_t bytes, const void* data) const
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
	}

	void free() const
	{
		glDeleteBuffers(1, &m_IndexBufferID);
	}

	const unsigned int getID() const
	{
		return m_IndexBufferID;
	}
};