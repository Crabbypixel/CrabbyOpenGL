#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utils.h"
#include "OpenGL_Graphics.h"
#include "VertexData.h"
#include "Model.h"
#include "Renderer.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

// Upper limit of random number generator. For lower limit, negate this value.
constexpr int MAX_RAND = 100;

class Window : public OpenGL_Graphics
{
private:
	Renderer& renderer = Renderer::getInstance();

	// For axes
	VertexArray	axesVAO;
	VertexBuffer<float> axesVBO;
	BufferLayout axesLayout;
	Shader axesShader;

	// More shaders
	Model cubeModel;
	Model spaceshipModel;
	Model sphereModel;
	Model terrainModel;
	Model lampModel;

	// Shaders
	Shader lightingShader;
	Shader terrainShader;
	Shader lampShader;

	// Projection matrix
	glm::mat4 matProjection;
	float fFov = 80.0f;

	// Camera
	Camera camera;

	// Light positions and colors
	glm::vec3 vLightPos = glm::vec3(1.2f, 1.0f, 2.0f);
	glm::vec3 vLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

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

		// Load lighting shader
		lightingShader.load("shaders/Lighting.glsl");

		// Load terrain shader
		terrainShader.load("shaders/Terrain1.glsl");

		// Load lamp shader
		lampShader.load("shaders/Lamp.glsl");

		// Models
		cubeModel.load("models/Cube.obj");
		spaceshipModel.load("models/SpaceShip.obj");
		sphereModel.load("models/SmoothSphere.obj");
		terrainModel.load("models/Platform.obj");
		lampModel.load("models/Cube.obj");

		// Assign each model to its corresponding shader (by reference)
		renderer.addModel(&cubeModel, &lightingShader);
		renderer.addModel(&spaceshipModel, &lightingShader);
		renderer.addModel(&sphereModel, &lightingShader);
		renderer.addModel(&terrainModel, &terrainShader);
		renderer.addModel(&lampModel, &lampShader);

		// Initalize shaders
		InitalizeLightingShader();
		InitalizeTerrainShader();
		
		// Initalize lamp shader but it's not too complicated so we'll just initalize it here without using a function
		lampShader.use();
		lampShader.setVec3("vLampColor", vLightColor);

		// Set projection matrix in shaders as they do not change often
		matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);
		lightingShader.use();
		lightingShader.setMat4("matProjection", matProjection);
		terrainShader.use();
		terrainShader.setMat4("matProjection", matProjection);
		lampShader.use();
		lampShader.setMat4("matProjection", matProjection);

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.06f, 0.06f, 0.06f, 1.0f);

		HandleInputs(fElapsedTime);

		// Some shader variables and models need to update every frame, hence call them on every frame
		UpdateShader();
		UpdateModels();

		// Render all objects
		renderer.render();

		// Displays coordinate axes (for debugging)
		RenderAxis();

		return true;
	}

	void UpdateShader()
	{
		lightingShader.use();
		lightingShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		lightingShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		lightingShader.setVec3("u_vViewPos", camera.vCameraPos);

		terrainShader.use();
		terrainShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		terrainShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		terrainShader.setVec3("u_vViewPos", camera.vCameraPos);

		// Changes lamp color but the light emitted is still white (weird isn't it?)
		lampShader.use();
		lampShader.setVec3("vLampColor", glm::vec3(fabs(cosf(fTimeSinceStart)) / 2.0f, 0.0f, fabs(sinf(fTimeSinceStart)) / 2.0f));
	}

	void UpdateModels()
	{
		cubeModel.matModel = glm::mat4(1.0f);
		cubeModel.matModel = glm::rotate(cubeModel.matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));

		spaceshipModel.matModel = glm::mat4(1.0f);
		spaceshipModel.matModel = glm::translate(spaceshipModel.matModel, glm::vec3(7.0f, 0.0f, 0.0f));
		spaceshipModel.matModel = glm::rotate(spaceshipModel.matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));

		sphereModel.matModel = glm::mat4(1.0f);
		sphereModel.matModel = glm::translate(sphereModel.matModel, glm::vec3(0.0f, 0.0f, 6.0f));
		sphereModel.matModel = glm::rotate(sphereModel.matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));

		terrainModel.matModel = glm::mat4(1.0f);
		terrainModel.matModel = glm::translate(terrainModel.matModel, glm::vec3(0.0f, -10.0f, 0.0f));

		lampModel.matModel = glm::mat4(1.0f);
		lampModel.matModel = glm::translate(lampModel.matModel, vLightPos);
		lampModel.matModel = glm::scale(lampModel.matModel, glm::vec3(0.2f));
	}

	void InitalizeLightingShader()
	{
		lightingShader.use();

		lightingShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		lightingShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		lightingShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setVec3("u_dirLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setVec3("u_dirLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		// ---------------------------------------- Point light ---------------------------------------- 
		// TODO: Implement multiple lamps using a for-loop
		lightingShader.setVec3("u_pointLights[0].vPosition", vLightPos);
		lightingShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		lightingShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		lightingShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setVec3("u_pointLights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		lightingShader.setFloat("u_pointLights[0].fLinear", 0.014f);
		lightingShader.setFloat("u_pointLights[0].fQuadratic", 0.0007f);

		lightingShader.setFloat("u_material.fShininess", 64.0f);
		lightingShader.setVec3("u_material.vColor", glm::vec3(0.5f, 0.5f, 0.5f));

		// ---------------------------------------- Spot light ---------------------------------------- 
		lightingShader.setVec3("u_spotLight.vLightColor", glm::vec3(0.0f, 0.0f, 1.0f));

		lightingShader.setVec3("u_spotLight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		lightingShader.setVec3("u_spotLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setVec3("u_spotLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		lightingShader.setFloat("u_spotLight.fConstant", 1.0f);
		lightingShader.setFloat("u_spotLight.fLinear", 0.22f);
		lightingShader.setFloat("u_spotLight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		lightingShader.setFloat("u_dirLight.fCutOff", 30.0f * pi / 180.0f);
		lightingShader.setFloat("u_dirLight.fOuterCutOff", 45.0f * pi / 180.0f);
	}

	void InitalizeTerrainShader()
	{
		terrainShader.use();

		terrainShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		terrainShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		terrainShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		terrainShader.setVec3("u_dirLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));

		// ---------------------------------------- Point light ---------------------------------------- 
		// TODO: Implement multiple lamps using a for-loop
		terrainShader.setVec3("u_pointLights[0].vPosition", vLightPos);
		terrainShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		terrainShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		terrainShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(0.7f, 0.7f, 0.7f));

		terrainShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		terrainShader.setFloat("u_pointLights[0].fLinear", 0.014f);
		terrainShader.setFloat("u_pointLights[0].fQuadratic", 0.0007f);

		terrainShader.setFloat("u_material.fShininess", 64.0f);
		terrainShader.setVec3("u_material.vColor", glm::vec3(0.13f, 0.55f, 0.13f));

		// ---------------------------------------- Spot light ---------------------------------------- 
		terrainShader.setVec3("u_spotLight.vLightColor", glm::vec3(0.0f, 1.0f, 0.0f));

		terrainShader.setVec3("u_spotLight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		terrainShader.setVec3("u_spotLight.vDiffuse", glm::vec3(0.2f, 0.2f, 0.2f));

		terrainShader.setFloat("u_spotLight.fConstant", 1.0f);
		terrainShader.setFloat("u_spotLight.fLinear", 0.22f);
		terrainShader.setFloat("u_spotLight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		terrainShader.setFloat("u_dirLight.fCutOff", 30.0f * pi / 180.0f);
		terrainShader.setFloat("u_dirLight.fOuterCutOff", 45.0f * pi / 180.0f);
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
		matModel = glm::scale(matModel, glm::vec3(10.0f, 10.0f, 10.0f));
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
			lightingShader.use();
			lightingShader.setMat4("matProjection", matProjection);
			terrainShader.use();
			terrainShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
		}

		else if (GetKey('C').bReleased)
		{
			fFov = 80.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			lightingShader.use();
			lightingShader.setMat4("matProjection", matProjection);
			terrainShader.use();
			terrainShader.setMat4("matProjection", matProjection);
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
		camera.UpdateView(lightingShader, "matView");
		camera.UpdateView(terrainShader, "matView");
		camera.UpdateView(lampShader, "matView");
	}

	void Destroy() override
	{
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