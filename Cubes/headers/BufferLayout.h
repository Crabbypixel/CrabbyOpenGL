#pragma once

#include <glad/glad.h>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

enum class BufferType
{
	FLOAT = GL_FLOAT,
	INT = GL_INT
};

class BufferLayout
{
private:
	int location;
	int stride;
	VertexArray m_va;

public:
	BufferLayout() : stride{ 0 }, location{ 0 }
	{}

	template<typename T>
	void setBufferLayout(VertexArray& va, VertexBuffer<T>& buffer, IndexBuffer& indexBuffer, int count, BufferType type)
	{
		m_va = va;
		m_va.bind();

		buffer.bind();
		indexBuffer.bind();

		int sz = buffer.getVertexCount() * buffer.typeSize;
		glVertexAttribPointer(location, count, (GLenum)type, GL_FALSE, sz, (const void*)stride);
		glEnableVertexAttribArray(location);

		stride += count * getSizeFromType(type);
		location++;
	}

	template<typename T>
	void setBufferLayout(VertexArray& va, VertexBuffer<T>& buffer, int count, BufferType type)
	{
		m_va = va;
		m_va.bind();

		buffer.bind();

		int sz = buffer.getVertexCount() * buffer.typeSize;
		glVertexAttribPointer(location, count, (GLenum)type, GL_FALSE, sz, (const void*)stride);
		glEnableVertexAttribArray(location);

		stride += count * getSizeFromType(type);
		location++;
	}

private:
	int getSizeFromType(BufferType type)
	{
		switch (type)
		{
		case BufferType::FLOAT:
			return sizeof(float);

		case BufferType::INT:
			return sizeof(int);

		default:
			return 0;
		}
	}
};

//int BufferLayout::location = 0;
//int BufferLayout::stride = 0;