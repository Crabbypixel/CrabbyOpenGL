#pragma once

#include <glad/glad.h>

class IndexBuffer
{
private:
	unsigned int m_IndexBufferID = 0;

public:
	IndexBuffer() = default;

	void generate();

	void bind() const;

	void unbind() const;

	void setBuffer(size_t bytes, const void* data) const;

	void free() const;

	const unsigned int getID() const;
};

void IndexBuffer::generate()
{
	glGenBuffers(1, &m_IndexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
}

void IndexBuffer::bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferID);
}

void IndexBuffer::unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::setBuffer(size_t bytes, const void* data) const
{
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
}

void IndexBuffer::free() const
{
	glDeleteBuffers(1, &m_IndexBufferID);
}

const unsigned int IndexBuffer::getID() const
{
	return m_IndexBufferID;
}