#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utils.h"
#include "OpenGL_Graphics.h"
#include "VertexData.h"
#include "Model.h"

#include "Block.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

// Upper limit of random number generator. For lower limit, negate this value.
constexpr int MAX_RAND = 100;

class Window : public OpenGL_Graphics
{
private:
	// For axes
	VertexArray	axesVAO;
	VertexBuffer<float> axesVBO;
	BufferLayout axesLayout;
	Shader axesShader;

	// Models
	Model lampModel;

	// Block shader
	Shader blockShader;
	Shader lampShader;

	// Blocks
	std::vector<Block*> blocks;

	// Projection matrix
	glm::mat4 matProjection;
	float fFov = 80.0f;

	// Camera
	Camera camera;

	// Light positions and colors
	glm::vec3 vLampPos = glm::vec3(1.2f, 1.0f, 2.0f);
	glm::vec3 vLampColor = glm::vec3(1.0f, 1.0f, 1.0f);

public:
	bool Setup() override
	{
		camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		// Axes
		axesVAO.generate();
		axesVBO.generate(3);		// 3 floats per vertex
		axesVBO.setBuffer(sizeof(line_vertices), (const void*)line_vertices);
		axesLayout.setBufferLayout(axesVAO, axesVBO, 3, BufferType::FLOAT);
		axesShader.load("shaders/Line.glsl");

		// ---------------------------- Load Models ------------------------------
		lampModel.load("models/Cube.obj");

		// ---------------------------- Load Shaders -----------------------------
		blockShader.load("shaders/Block.glsl");
		lampShader.load("shaders/Lamp.glsl");

		// ---------------------------- Set Shaders ----------------------------
		blockShader.use();
		blockShader.setInt("u_material.diffuse", 0);
		blockShader.setInt("u_material.specular", 1);

		// Initalize block shader
		InitalizeBlockShader();

		// Initalize lamp shader but it's not too complicated so we'll just initalize it here without using a function
		lampShader.use();
		lampShader.setVec3("vLampColor", vLampColor);

		// ---------------------------- Others ---------------------------------
		auto dt1 = std::chrono::system_clock::now();

		std::cout << "Generating blocks..." << std::endl;
		Block* block = new GrassBlock({ 0.0f, 0.0f, 0.0f }, 0.0f, pi);
		blocks.push_back(block);
		for (int x = 0; x < 200; x++)
		{
			for (int z = 0; z < 200; z++)
			{
				Block* block = new GrassBlock(glm::vec3((float)x, -10.0f, (float)z), glm::radians(Random::get(0, 3) * 90.0f), pi);
				blocks.push_back(block);
			}
		}

		auto dt2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = dt2 - dt1;
		float dt = elapsedTime.count();

		std::cout << "Finished generating! Time taken: " << dt << " seconds" << std::endl;

		SetProjectionMatrix();

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.38f, 0.76f, 0.93f, 1.0f);

		HandleInputs(fElapsedTime);

		UpdateShader();

		// Draw models
		blockShader.use();
		for (const auto& block : blocks)
		{
			block->draw(blockShader);
		}

		// Displays coordinate axes (for debugging)
		RenderAxis();

		return true;
	}

	void UpdateShader()
	{
		blockShader.use();
		blockShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		blockShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		blockShader.setVec3("u_vViewPos", camera.vCameraPos);

		lampShader.use();
		glm::mat4 matLampModel = glm::mat4(1.0f);
		matLampModel = glm::translate(matLampModel, vLampPos);
		matLampModel = glm::scale(matLampModel, glm::vec3(0.2f));
		lampShader.setMat4("matModel", matLampModel);
		lampModel.draw();
	}

	void RenderAxis()
	{
		/**
		* To draw on the screen:
		*	1) Bind shader
		*	2) Bind vertex array
		*	3) Set uniform in shaders
		*	4) Activate and use textures
		*	5) Compute model matrix
		*	6) Call glDrawElements() or glDrawArrays() to draw
		*/

		axesShader.use();
		axesVAO.bind();

		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::scale(matModel, glm::vec3(5.0f, 5.0f, 5.0f));
		axesShader.setMat4("matModel", matModel);

		// Increase line width
		glLineWidth(2.0f);

		// Draw lines
		axesShader.setVec3("vColor", 1.0f, 0.0f, 0.0f);
		glDrawArrays(GL_LINES, 0, 2);
		axesShader.setVec3("vColor", 0.0f, 1.0f, 0.0f);
		glDrawArrays(GL_LINES, 2, 2);
		axesShader.setVec3("vColor", 0.0f, 0.0f, 1.0f);
		glDrawArrays(GL_LINES, 4, 2);

		// Set line width back to normal
		glLineWidth(1.0f);
	}

	void InitalizeBlockShader()
	{
		blockShader.use();

		// ---------------------------------------- Directional light ---------------------------------------- 
		blockShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		blockShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		blockShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		blockShader.setVec3("u_dirLight.vDiffuse", glm::vec3(0.2f, 0.2f, 0.2f));
		blockShader.setVec3("u_dirLight.vSpecular", glm::vec3(0.2f, 0.2f, 0.2f));

		// ---------------------------------------- Point light ---------------------------------------- 
		blockShader.setVec3("u_pointLights[0].vPosition", vLampPos);
		blockShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		blockShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		blockShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		blockShader.setVec3("u_pointLights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		blockShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		blockShader.setFloat("u_pointLights[0].fLinear", 0.14f);
		blockShader.setFloat("u_pointLights[0].fQuadratic", 0.07f);

		blockShader.setFloat("u_material.fShininess", 64.0f);

		// ---------------------------------------- Spot light ---------------------------------------- 
		blockShader.setVec3("u_spotLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		blockShader.setVec3("u_spotLight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		blockShader.setVec3("u_spotLight.vDiffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		blockShader.setVec3("u_spotLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		blockShader.setFloat("u_spotLight.fConstant", 1.0f);
		blockShader.setFloat("u_spotLight.fLinear", 0.22f);
		blockShader.setFloat("u_spotLight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		blockShader.setFloat("u_dirLight.fCutOff", 30.0f * pi / 180.0f);
		blockShader.setFloat("u_dirLight.fOuterCutOff", 45.0f * pi / 180.0f);
	}

	void HandleInputs(float fElapsedTime)
	{
		/* ------------------------------------------ - Keyboard Control - ------------------------------------------- */
		if (GetKey('W').bHeld && !GetKey('S').bHeld)
			camera.ProcessKeyboard(CameraMovement::FORWARD, fElapsedTime);
		else if (GetKey('S').bHeld && !GetKey('W').bHeld)
			camera.ProcessKeyboard(CameraMovement::BACKWARD, fElapsedTime);

		if (GetKey('A').bHeld && !GetKey('D').bHeld)
			camera.ProcessKeyboard(CameraMovement::LEFT, fElapsedTime);
		else if (GetKey('D').bHeld && !GetKey('A').bHeld)
			camera.ProcessKeyboard(CameraMovement::RIGHT, fElapsedTime);

		if (GetKey(GLFW_KEY_SPACE).bHeld && !GetKey(GLFW_KEY_LEFT_SHIFT).bHeld)
			camera.ProcessKeyboard(CameraMovement::UP, fElapsedTime);
		else if (GetKey(GLFW_KEY_LEFT_SHIFT).bHeld && !GetKey(GLFW_KEY_SPACE).bHeld)
			camera.ProcessKeyboard(CameraMovement::DOWN, fElapsedTime);

		// Emulate a "zoom-in" view by decreasing the FOV if 'C' is pressed
		if (GetKey('C').bHeld)
		{
			if (fFov > 10.0f)
				fFov -= fElapsedTime * 200.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			blockShader.use();
			blockShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
		}

		else if (GetKey('C').bReleased)
		{
			fFov = 80.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			blockShader.use();
			blockShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
		}

		if (GetKey(GLFW_KEY_LEFT_CONTROL).bHeld)
			camera.fCameraSpeed = 20.0f;
		else
			camera.fCameraSpeed = 5.0f;

		if (GetKey(GLFW_KEY_HOME).bPressed)
			camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		/* ------------------------------------------ - Mouse Control - ------------------------------------------- */
		camera.ProcessMouse(this, GetMousePosX(), GetMousePosY());

		// Update view matrix
		camera.UpdateView(axesShader, "matView");
		camera.UpdateView(blockShader, "matView");
		camera.UpdateView(lampShader, "matView");
	}

	void SetProjectionMatrix()
	{
		// Set projection matrix in shaders as they do not change often
		matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);
		blockShader.use();
		blockShader.setMat4("matProjection", matProjection);
		lampShader.use();
		lampShader.setMat4("matProjection", matProjection);
	}

	void Destroy() override
	{
		for (auto& block : blocks)
		{
			delete block;
			block = nullptr;
		}

		blocks.clear();

		axesVAO.free();
		axesVBO.free();

		std::cout << "\nDuration: " << std::fixed << std::setprecision(2) << fTimeSinceStart << 's' << std::endl;
	}

	static int random()
	{
		return Random::get<int>(-MAX_RAND, MAX_RAND);
	}
};

int main()
{
	Window window;
	window.ConstructWindow(800, 600, "OpenGL");
	window.Start();

	std::cout << "Goodbye!" << std::endl;

	return 0;
}