#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
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

class Model
{
private:
	VertexArray vao;
	VertexBuffer<float> vbo;
	std::vector<Texture2D> textures;
	int nr_indices = 0;

public:
	glm::mat4 matModel = glm::mat4(1.0f);

	Model() = default;

	// Load the object file.
	bool load(const std::string& filePath);

	// Bind textures to the model, if any.
	// For now, this function is just a placeholder and does nothing.
	void bindTextures();

	// Function which draws the model onto the screen. Make sure to bind shaders and VAO before calling this function.
	void draw();

	// Destructor
	~Model();

private:
	// Utility functions
	bool LoadModel(VertexArray& vao, VertexBuffer<float>& vbo, int& vertexCount, const std::string& modelFile);
};

bool Model::load(const std::string& filePath)
{
	return LoadModel(vao, vbo, nr_indices, filePath);
}

void Model::bindTextures()
{
	// Bind textures
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		textures[i].bindTexture();
	}
}

void Model::draw()
{
	// Draw the model
	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, nr_indices);
}

Model::~Model()
{
	vbo.free();
	vao.free();
}

// Utility function to load models (written earlier so I'm lazy to properly integrate it in load() function :/
// TODO: Add texture functonality
bool Model::LoadModel(VertexArray& vao, VertexBuffer<float>& vbo, int& vertexCount, const std::string& modelFile)
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec3> temp_normals;
	std::vector<Vertex> vertices;

	std::ifstream inputFileStream(modelFile);
	std::string line;
	std::stringstream ss;

	if (!inputFileStream.is_open())
	{
		std::cerr << "Failed to open object file: " << modelFile << std::endl;
		return false;
	}

	//std::cout << "Loading " << modelFile << "...\n";
	while (std::getline(inputFileStream, line))
	{
		if (line.starts_with("vn"))
		{
			float f1, f2, f3;
			sscanf_s(line.c_str(), "%*s %f %f %f", &f1, &f2, &f3);
			temp_normals.push_back({ f1, f2, f3 });
		}

		else if (line.starts_with('v'))
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

	BufferLayout cubeLayout;

	vao.generate();
	vbo.generate(6);		// 6 floats per vertex (3 for vertex positions and another 3 for normal vector)

	const void* vertexBuffer = vertices.data();
	vbo.setBuffer(vertices.size() * 6 * sizeof(float), vertexBuffer);

	cubeLayout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);	// positions
	cubeLayout.setBufferLayout(vao, vbo, 3, BufferType::FLOAT);	// normals
	vertexCount = vertices.size();

	inputFileStream.close();

	//std::cout << "Finished loading!\n";
	//std::cout << "Number of vertices: " << vertexCount << "\n";
	//std::cout << "Size (bytes): " << vertexCount * sizeof(Vertex) << " bytes (" << std::fixed << std::setprecision(2) << (float)(vertexCount * sizeof(Vertex) / (1024.0f * 1024.0f)) << " MB)\n\n";

	return true;
}