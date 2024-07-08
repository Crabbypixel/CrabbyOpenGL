#pragma once

// Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"

class TextureMap
{
public:
	Model grassModel;
	Model dirtModel;
	Model stoneModel;

	TextureMap()
	{
		grassModel.load("models/grass.obj");
		grassModel.setTextures({ "resources/textures/Grass4.png" });

		dirtModel.load("models/Grass2.obj");
		dirtModel.setTextures({ "resources/textures/Dirt2.png" });

		stoneModel.load("models/Stone.obj");
		stoneModel.setTextures({"resources/textures/Stone.png"});
	}
};

TextureMap& texturemap()
{
	static TextureMap textureMap;
	return textureMap;
}

class Block
{
private:
	glm::vec3 vPosition;

public:
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	Block(const glm::vec3& pos, float fRotateY, float fRotateZ) : vPosition(pos)
	{
		modelMatrix = glm::translate(modelMatrix, pos);
		modelMatrix = glm::rotate(modelMatrix, fRotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, fRotateZ, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	virtual void draw(Shader& shader) = 0;
};

class GrassBlock : public Block
{
private:
	Model model;
public:
	GrassBlock(const glm::vec3& pos, float fRotateY, float fRotateZ) : Block(pos, fRotateY, fRotateZ)
	{
	}

	void draw(Shader& shader) override
	{
		shader.setMat4("matModel", modelMatrix);
		texturemap().grassModel.draw();
	}
};

class DirtBlock : public Block
{
private:
	Model model;
public:
	DirtBlock(const glm::vec3& pos, float fRotateY, float fRotateZ) : Block(pos, fRotateY, fRotateZ)
	{
	}

	void draw(Shader& shader) override
	{
		shader.setMat4("matModel", modelMatrix);
		texturemap().dirtModel.draw();
	}
};

class StoneBlock : public Block
{
private:
	Model model;
public:
	StoneBlock(const glm::vec3& pos, float fRotateY, float fRotateZ) : Block(pos, fRotateY, fRotateZ)
	{
	}

	void draw(Shader& shader) override
	{
		shader.setMat4("matModel", modelMatrix);
		texturemap().stoneModel.draw();
	}
};