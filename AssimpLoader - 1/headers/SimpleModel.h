#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include "BufferLayout.h"
#include "Texture2D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

class SimpleModel
{
private:
	VertexArray vao;
	VertexBuffer<float> vbo;
	std::vector<Texture2D> textures;
	int nr_indices = 0;

public:
	//glm::mat4 matModel = glm::mat4(1.0f);

	SimpleModel() = default;
	SimpleModel(const std::string& objfilepath, const std::vector<std::string>&& texturePaths);

	// Load the object file
	bool load(const std::string& objfilePath);

	// Set textures to the model, if any
	// For now, this function is just a placeholder and does nothing
	void setTextures(const std::vector<std::string>& texturePaths);
	void setTextures(const std::vector<std::string>&& texturePaths);

	// Bind textures by using a function, as doing them for every draw call is inefficient.
	void bindTextures();

	// Function which draws the model onto the screen. Make sure to bind shaders before calling this function.
	void draw();

	// Destructor
	~SimpleModel();

private:
	// Utility function to load model
	bool LoadModel(VertexArray& vao, VertexBuffer<float>& vbo, int& vertexCount, const std::string& modelFile);
};

SimpleModel::SimpleModel(const std::string& objfilepath, const std::vector<std::string>&& texturePaths)
{
	LoadModel(vao, vbo, nr_indices, objfilepath);
	setTextures(texturePaths);
}

bool SimpleModel::load(const std::string& objfilePath)
{
	return LoadModel(vao, vbo, nr_indices, objfilePath);
}

void SimpleModel::setTextures(const std::vector<std::string>& texturePaths)
{
	for (const auto& texturePath : texturePaths)
	{
		Texture2D texture;
		texture.loadTexture(texturePath.c_str());
		textures.push_back(texture);
	}

	// Bind textures
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i].bindTexture();
	}
}

void SimpleModel::setTextures(const std::vector<std::string>&& texturePaths)
{
	for (const auto& texturePath : texturePaths)
	{
		Texture2D texture;
		texture.loadTexture(texturePath.c_str());
		textures.push_back(texture);
	}

	// Bind textures
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i].bindTexture();
	}
}

void SimpleModel::bindTextures()
{
	// Draw the model
	vao.bind();

	// Bind textures
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i].bindTexture();
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void SimpleModel::draw()
{
	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, nr_indices);
}

SimpleModel::~SimpleModel()
{
	vbo.free();
	vao.free();
}

// Utility function to load models (written earlier so I'm lazy to properly integrate it in load() function :/
// TODO: Add texture functonality
bool SimpleModel::LoadModel(VertexArray& vao, VertexBuffer<float>& vbo, int& vertexCount, const std::string& modelFile)
{
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec3> temp_normals;
	std::vector<glm::vec2> temp_textures;

	std::ifstream inputFileStream(modelFile);
	std::string line;
	std::stringstream ss;

	if (!inputFileStream.is_open())
	{
		std::cerr << "Failed to open object file: " << modelFile << std::endl;
		return false;
	}

	/** If the object file has "mtllib" on the third line, then the object file is
	  * associated with some MTL file which we need to parse along with OBJ file */

	// Go to the third line
	std::string lineParser;
	
	// Read 3 lines
	for (int i = 0; i < 3; i++)
		std::getline(inputFileStream, lineParser);

	// Read the third line and see if it has mtllib
	//std::getline(inputFileStream, lineParser);
	
	if (lineParser.find("mtllib") != std::string::npos)
	{
		struct VertexTexture
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 textures;
		};
		std::vector<VertexTexture> verticesTexture;

		while (std::getline(inputFileStream, line))
		{
			// Vertex normals
			if (line.starts_with("vn"))
			{
				float f1, f2, f3;
				sscanf_s(line.c_str(), "%*s %f %f %f", &f1, &f2, &f3);
				temp_normals.push_back({f1, f2, f3});
			}

			// Texture coordinates
			else if (line.starts_with("vt"))
			{
				float f1, f2;
				sscanf_s(line.c_str(), "%*s %f %f", &f1, &f2);
				temp_textures.push_back({ f1, f2 });
			}

			// Vertex coordinates
			else if (line.starts_with('v'))
			{
				float f1, f2, f3;
				sscanf_s(line.c_str(), "%*s %f %f %f", &f1, &f2, &f3);
				temp_positions.push_back({ f1, f2, f3 });
			}

			// Indices
			else if (line.starts_with('f'))
			{
				int v[3];
				int n[3];
				int t[3];

				sscanf_s(line.c_str(), "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &v[0], &t[0], &n[0], &v[1], &t[1], &n[1], &v[2], &t[2], &n[2]);

				int posIndex;
				int normIndex;
				int textIndex;

				for (int i = 0; i < 3; i++)
				{
					posIndex = v[i] - 1;
					normIndex = n[i] - 1;
					textIndex = t[i] - 1;

					VertexTexture vertex;
					vertex.position = temp_positions[posIndex];
					vertex.normal = temp_normals[normIndex];
					vertex.textures = temp_textures[textIndex];

					verticesTexture.push_back(vertex);
				}
			}
		}

		BufferLayout layout;

		vao.generate();
		vbo.generate(8);		// 8 floats per vertex: 3 for vertex positions, 3 for vertex normals and 2 for texture coordinates

		const void* vertexBuffer = verticesTexture.data();
		vbo.setBuffer(verticesTexture.size() * 8 * sizeof(float), vertexBuffer);

		layout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);		// Vertices
		layout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);		// Normals
		layout.setBufferLayout(vao, vbo, 2, BufferType::FLOAT);		// Texture coordinates

		vertexCount = verticesTexture.size();
	}

	// Else, it is just a regular object file without any textures
	else
	{
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
		};

		std::vector<Vertex> vertices;

		while (std::getline(inputFileStream, line))
		{
			if (line.starts_with("vn"))
			{
				float f1, f2, f3;
				sscanf_s(line.c_str(), "%*s %f %f %f", &f1, &f2, &f3);
				temp_normals.push_back({ f1, f2, f3 });
			}

			if (line.starts_with('v'))
			{
				float f1, f2, f3;
				sscanf_s(line.c_str(), "%*s %f %f %f", &f1, &f2, &f3);
				temp_positions.push_back({ f1, f2, f3 });
			}

			else if (line.starts_with('f'))
			{
				int v[3];
				int n[3];

				sscanf_s(line.c_str(), "%*s %d//%d %d//%d %d//%d", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);

				int posIndex;
				int normIndex;

				for (int i = 0; i < 3; i++)
				{
					posIndex = v[i] - 1;
					normIndex = n[i] - 1;

					Vertex vertex;
					vertex.position = temp_positions[posIndex];
					vertex.normal = temp_normals[normIndex];

					vertices.push_back(vertex);
				}
			}
		}

		BufferLayout layout;

		vao.generate();
		vbo.generate(6);		// 6 floats per vertex: 3 for vertex positions and another 3 for normal vector

		const void* vertexBuffer = vertices.data();
		vbo.setBuffer(vertices.size() * 6 * sizeof(float), vertexBuffer);

		layout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);	// positions
		layout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);	// normals
		vertexCount = vertices.size();
	}

	inputFileStream.close();

	//std::cout << "Finished loading!\n";
	//std::cout << "Number of vertices: " << vertexCount << "\n";
	//std::cout << "Size (bytes): " << vertexCount * sizeof(Vertex) << " bytes (" << std::fixed << std::setprecision(2) << (float)(vertexCount * sizeof(Vertex) / (1024.0f * 1024.0f)) << " MB)\n\n";

	return true;
}