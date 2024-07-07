#pragma once

#include "Shader.h"
#include "Model.h"

#include <unordered_map>

class Renderer
{
private:
	// Stores references to each model and its corresponding shader

	std::unordered_map<Model*, Shader*> models;
	Shader* currentShader = nullptr;

	// This is a singleton class
	Renderer() {}

public:
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;

	static Renderer& getInstance();

	void addModel(Model* model, Shader* shader);

	void render();
};

inline Renderer& Renderer::getInstance()
{
	static Renderer renderer;
	return renderer;
}

void Renderer::addModel(Model* model, Shader* shader)
{
	models.insert(std::make_pair(model, shader));
}

void Renderer::render()
{
	for (const auto& [model, shader] : models)
	{
		if (!currentShader || (currentShader->id != shader->id))
		{
			shader->use();
			currentShader = shader;
		}

		currentShader->setMat4("matModel", model->matModel);
		model->draw();
	}
}