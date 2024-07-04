#pragma once

#include <glad/glad.h>

template<typename T>
class VertexBuffer
{
private:
	unsigned int m_VertexBufferID = 0;
	size_t m_BufferBytes = 0;
	size_t m_VertexCount = 0;
public:
	const size_t typeSize = sizeof(T);

	VertexBuffer() = default;

	void generate(size_t vertexCount)
	{
		m_VertexCount = vertexCount;

		glGenBuffers(1, &m_VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
	}

	void bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
	}

	void unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void setBuffer(size_t bytes, const void* data)
	{
		m_BufferBytes = bytes;
		glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
	}

	size_t getBufferBytes() const
	{
		return m_BufferBytes;
	}

	size_t getVertexCount() const
	{
		return m_VertexCount;
	}

	void free() const
	{
		glDeleteBuffers(1, &m_VertexBufferID);
	}

	const unsigned int getID() const
	{
		return m_VertexBufferID;
	}
};