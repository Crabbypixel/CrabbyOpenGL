#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include "Utils.h"
#include "OpenGL_Graphics.h"
#include "VertexData.h"

// Model headers
#include "SimpleModel.h"
#include "AssimpModelLoader.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

class Window : public OpenGL_Graphics
{
private:
	// For axes
	VertexArray	axesVAO;
	VertexBuffer<float> axesVBO;
	BufferLayout axesLayout;
	Shader axesShader;

	// Backpack Model
	Shader backpackShader;
	Model backpackModel;

	// Teapot Model
	Shader teapotShader;
	Model teapotModel;

	// Lamp
	SimpleModel lampModel;
	Shader lampShader;

	// Projection matrix
	glm::mat4 matProjection;
	float fFov = 80.0f;

	// Camera
	Camera camera;

	// Light positions and colors
	//glm::vec3 vLampPos = glm::vec3(1.2f, 1.0f, 2.0f);
	glm::vec3 vLampPos = glm::vec3(0.0f, 0.0f, 0.0f);
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

		// ---------------------------- Load Models -----------------------------
		stbi_set_flip_vertically_on_load(true);
		backpackShader.load("shaders/Backpack.glsl");
		teapotShader.load("shaders/BasicAssimp.glsl");

		lampModel.load("models/Cube.obj");
		lampShader.load("shaders/Lamp.glsl");

		InitShaders();

		auto dt1 = std::chrono::system_clock::now();
		backpackModel.load("models/backpack/backpack.obj");
		teapotModel.load("models/teapot.obj");
		auto dt2 = std::chrono::system_clock::now();

		float fTimeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(dt2 - dt1).count();
		std::cout << "Time taken to load models: " << std::fixed << std::setprecision(2) << fTimeTaken/1000 << " seconds" << std::endl;

		// Set projection matries in shaders
		SetProjectionMatrix();

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(0.38f, 0.76f, 0.93f, 1.0f);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

		HandleInputs(fElapsedTime);

		RenderModels();

		// Displays coordinate axes (for debugging)
		RenderAxis();

		return true;
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

	void RenderModels()
	{
		// Render lamp
		lampShader.use();

		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, vLampPos);
		matModel = glm::scale(matModel, glm::vec3(0.2f));
		lampShader.setMat4("matModel", matModel);

		lampModel.draw();

		// Render backpack
		backpackShader.use();

		backpackShader.setVec3("vViewPos", camera.vCameraPos);
		backpackShader.setVec3("spotlight.vPosition", camera.vCameraPos);
		backpackShader.setVec3("spotlight.vDirection", camera.vCameraFront);

		matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(5.0f, 0.0f, 0.0f));
		matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
		matModel = glm::scale(matModel, glm::vec3(1.0f, 1.0f, 1.0f));
		backpackShader.setMat4("matModel", matModel);

		backpackModel.Draw(backpackShader);

		// Render teapot
		teapotShader.use();

		teapotShader.setVec3("vViewPos", camera.vCameraPos);
		teapotShader.setVec3("spotlight.vPosition", camera.vCameraPos);
		teapotShader.setVec3("spotlight.vDirection", camera.vCameraFront);

		matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(0.0f, 0.0f, -5.0f));
		matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
		matModel = glm::scale(matModel, glm::vec3(0.3f, 0.3f, 0.3f));
		teapotShader.setMat4("matModel", matModel);

		teapotModel.Draw(teapotShader);
	}

	void InitShaders()
	{
		// Set shaders
		lampShader.use();
		lampShader.setVec3("vLampColor", vLampColor);

		backpackShader.use();
		backpackShader.setVec3("pointlights[0].vPosition", vLampPos);
		backpackShader.setVec3("pointlights[0].vLightColor", vLampColor);

		backpackShader.setFloat("pointlights[0].fConstant", 1.0f);
		backpackShader.setFloat("pointlights[0].fLinear", 0.14f);
		backpackShader.setFloat("pointlights[0].fQuadratic", 0.07f);

		backpackShader.setVec3("pointlights[0].vAmbient", glm::vec3(0.2f, 0.2f, 0.2f));
		backpackShader.setVec3("pointlights[0].vDiffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		backpackShader.setVec3("pointlights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		backpackShader.setVec3("spotlight.vLightColor", glm::vec3(0.0f, 0.0f, 1.0f));

		backpackShader.setVec3("spotlight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		backpackShader.setVec3("spotlight.vDiffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		backpackShader.setVec3("spotlight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		backpackShader.setFloat("spotlight.fConstant", 1.0f);
		backpackShader.setFloat("spotlight.fLinear", 0.22f);
		backpackShader.setFloat("spotlight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		backpackShader.setFloat("spotlight.fCutOff", 30.0f * pi / 180.0f);
		backpackShader.setFloat("spotlight.fOuterCutOff", 45.0f * pi / 180.0f);

		backpackShader.setVec3("vViewPos", camera.vCameraPos);

		// For teapot shader
		teapotShader.use();
		teapotShader.setVec3("vLampColor", vLampColor);

		teapotShader.use();
		teapotShader.setVec3("pointlights[0].vPosition", vLampPos);
		teapotShader.setVec3("pointlights[0].vLightColor", vLampColor);

		teapotShader.setFloat("pointlights[0].fConstant", 1.0f);
		teapotShader.setFloat("pointlights[0].fLinear", 0.14f);
		teapotShader.setFloat("pointlights[0].fQuadratic", 0.07f);

		teapotShader.setVec3("pointlights[0].vAmbient", glm::vec3(0.2f, 0.2f, 0.2f));
		teapotShader.setVec3("pointlights[0].vDiffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		teapotShader.setVec3("pointlights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		teapotShader.setVec3("spotlight.vLightColor", glm::vec3(0.0f, 0.0f, 1.0f));

		teapotShader.setVec3("spotlight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		teapotShader.setVec3("spotlight.vDiffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		teapotShader.setVec3("spotlight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		teapotShader.setFloat("spotlight.fConstant", 1.0f);
		teapotShader.setFloat("spotlight.fLinear", 0.22f);
		teapotShader.setFloat("spotlight.fQuadratic", 0.20f);

		teapotShader.setVec3("vMaterialColor", glm::vec3(1.0f, 1.0f, 1.0f));

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		teapotShader.setFloat("spotlight.fCutOff", 30.0f * pi / 180.0f);
		teapotShader.setFloat("spotlight.fOuterCutOff", 45.0f * pi / 180.0f);

		teapotShader.setVec3("vViewPos", camera.vCameraPos);

		// End of set shaders
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
			backpackShader.use();
			backpackShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
			teapotShader.use();
			teapotShader.setMat4("matProjection", matProjection);
		}

		else if (GetKey('C').bReleased)
		{
			fFov = 80.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			backpackShader.use();
			backpackShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
			teapotShader.use();
			teapotShader.setMat4("matProjection", matProjection);
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
		camera.UpdateView(backpackShader, "matView");
		camera.UpdateView(lampShader, "matView");
		camera.UpdateView(teapotShader, "matView");
	}

	void SetProjectionMatrix()
	{
		// Set projection matrix in shaders as they do not change often
		matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);
		backpackShader.use();
		backpackShader.setMat4("matProjection", matProjection);
		lampShader.use();
		lampShader.setMat4("matProjection", matProjection);
		teapotShader.use();
		teapotShader.setMat4("matProjection", matProjection);
	}

	void Destroy() override
	{
		axesVAO.free();
		axesVBO.free();

		std::cout << "\nDuration: " << std::fixed << std::setprecision(2) << fTimeSinceStart << 's' << std::endl;
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