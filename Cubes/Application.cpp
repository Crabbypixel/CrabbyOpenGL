#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utils.h"
#include "Camera.h"
#include "OpenGL_Graphics.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

const size_t len = 10000;

class Console : public OpenGL_Graphics
{
private:
	// For axes
	VertexArray axesVAO;
	VertexBuffer<float> axesVBO;
	BufferLayout axesLayout;
	Shader axesShader;

	// Cube
	VertexBuffer<float> cubeVBO;
	VertexArray cubeVAO;
	BufferLayout cubeLayout;

	// Light cube
	VertexArray lightCubeVAO;
	BufferLayout lightCubeLayout;

	// Cube Shaders
	Shader cubeShader;
	Shader lightCubeShader;

	// Textures
	Texture2D diffuseTexture;
	Texture2D specularTexture;

	// Matrices
	glm::mat4 matProjection;

	// Camera
	Camera camera;

	// FOV (in degrees!)
	float fFov = 80.0f;

	// Light direction
	glm::vec3 vLightPos = glm::vec3(1.2f, 1.0f, 2.0f);
	glm::vec3 vLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	// Other stuffs
	float fTime = 0.0f;
	std::array<glm::vec3, len> aCubePos;

protected:
	bool Setup() override
	{
		camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		// Draw axes
		float line_vertices[] = {
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,

			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,

			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f
		};

		axesVAO.generate();
		axesVBO.generate(3);	// 3 floats per vertex
		axesVBO.setBuffer(sizeof(line_vertices), (const void*)line_vertices);
		axesLayout.setBufferLayout(axesVAO, axesVBO, 3, BufferType::FLOAT);
		axesShader.load("shaders/Line.shader");

		// Cubes vertices
		float cube_vertices[] = {
			// Positions		  // Normals		   //  Texture Coords (2D)
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};
		cubeVBO.generate(8);	// 8 floats per vertex
		cubeVBO.setBuffer(sizeof(cube_vertices), (const void*)cube_vertices);

		// Cube
		cubeVAO.generate();
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 3, BufferType::FLOAT);
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 3, BufferType::FLOAT);
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 2, BufferType::FLOAT);
		cubeShader.load("shaders/Cube.shader");

		// Light cube
		lightCubeVAO.generate();
		lightCubeLayout.setBufferLayout(lightCubeVAO, cubeVBO, 3, BufferType::FLOAT);
		lightCubeShader.load("shaders/LightCube.shader");

		// Texture
		diffuseTexture.loadTexture("resources/textures/container2.png");
		specularTexture.loadTexture("resources/textures/container2_specular.png");
		//specularTexture.loadTexture("resources/textures/lighting_maps_specular_color.png");

		// Set the textures
		cubeShader.use();
		cubeShader.setInt("material.diffuse", 0);
		cubeShader.setInt("material.specular", 1);
		cubeShader.setInt("material.emission", 2);

		// Set projection matrices for each shader as they do not change often
		matProjection = glm::perspective(glm::radians(fFov), (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		cubeShader.use();
		cubeShader.setMat4("matProjection", matProjection);
		lightCubeShader.use();
		lightCubeShader.setMat4("matProjection", matProjection);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);

		for (auto& cubePos : aCubePos)
			cubePos = glm::vec3(GetRandom(), GetRandom(), GetRandom());

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.06f, 0.06f, 0.06f, 1.0f);

		HandleInputs(fElapsedTime);

		DrawLamp(fElapsedTime);

		DrawCube();

		//for (const auto& cubePos : aCubePos)
		//	DrawCube(cubePos);

		// For debug
		//DrawAxes();

		DisplayDistance(fElapsedTime);

		return true;
	}

	void DrawCube()
	{
		cubeShader.use();
		cubeVAO.bind();

		cubeShader.setVec3("light.vPosition", vLightPos);
		//cubeShader.setVec3("light.vDirection", -0.2f, -1.0f, -0.3f);
		cubeShader.setVec3("light.vLightColor", 1.0f, 1.0f, 1.0f);

		cubeShader.setVec3("light.vAmbient", 0.4f, 0.4f, 0.4f);
		cubeShader.setVec3("light.vDiffuse", 1.0f, 1.0f, 1.0f);
		cubeShader.setVec3("light.vSpecular", 1.0f, 1.0f, 1.0f);

		cubeShader.setVec3("vViewPos", camera.vCameraPos);

		cubeShader.setFloat("material.fShininess", 64.0f);

		// Update lamp color if lamp color is going to be changed (not for now)
		lightCubeShader.use();
		lightCubeShader.setVec3("vLampColor", vLightColor);

		// Set the current shader in use back to cube shader
		cubeShader.use();

		glActiveTexture(GL_TEXTURE0);
		diffuseTexture.bindTexture();

		glActiveTexture(GL_TEXTURE1);
		specularTexture.bindTexture();

		// Draw cubes
		for (const auto& cubePos : aCubePos)
		{
			glm::mat4 matModel = glm::mat4(1.0f);
			
			matModel = glm::translate(matModel, cubePos);
			matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
			matModel = glm::scale(matModel, glm::vec3(2.0f, 2.0f, 2.0f));

			cubeShader.setMat4("matModel", matModel);
			
			// Rasterize the cube
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	void DrawLamp(float fElapsedTime)
	{
		lightCubeShader.use();
		lightCubeVAO.bind();

		glm::mat4 matModel = glm::mat4(1.0f);

		matModel = glm::translate(matModel, vLightPos);
		matModel = glm::scale(matModel, glm::vec3(0.2f));
		lightCubeShader.setMat4("matModel", matModel);

		// Rasterize the lamp
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	void DrawAxes()
	{
		axesShader.use();
		axesVAO.bind();

		glm::mat4 matLineModel = glm::mat4(1.0f);
		axesShader.setMat4("matModel", matLineModel);

		// Increase line width
		glLineWidth(2.0f);

		// Draw axes
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
		// Keyboard
		if (GetKey('W').bHeld)
			camera.ProcessKeyboard(CameraMovement::FORWARD, fElapsedTime);
		else if (GetKey('S').bHeld)
			camera.ProcessKeyboard(CameraMovement::BACKWARD, fElapsedTime);

		if (GetKey('A').bHeld)
			camera.ProcessKeyboard(CameraMovement::LEFT, fElapsedTime);

		else if (GetKey('D').bHeld)
			camera.ProcessKeyboard(CameraMovement::RIGHT, fElapsedTime);

		if (GetKey(GLFW_KEY_SPACE).bHeld)
			camera.ProcessKeyboard(CameraMovement::UP, fElapsedTime);

		else if (GetKey(GLFW_KEY_LEFT_SHIFT).bHeld)
			camera.ProcessKeyboard(CameraMovement::DOWN, fElapsedTime);

		if (GetKey(GLFW_KEY_HOME).bPressed)
			camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		if (GetKey('C').bHeld)
		{
			if (fFov > 10.0f)
				fFov -= fElapsedTime * 200.0f;

			matProjection = glm::perspective(glm::radians(fFov), (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);

			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			cubeShader.use();
			cubeShader.setMat4("matProjection", matProjection);
			lightCubeShader.use();
			lightCubeShader.setMat4("matProjection", matProjection);
		}

		else if (GetKey('C').bReleased)
		{
			fFov = 80.0f;
			matProjection = glm::perspective(glm::radians(fFov), (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);

			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			cubeShader.use();
			cubeShader.setMat4("matProjection", matProjection);
			lightCubeShader.use();
			lightCubeShader.setMat4("matProjection", matProjection);

		}

		if (GetKey(GLFW_KEY_LEFT_CONTROL).bHeld)
			camera.fCameraSpeed = 30.0f;
		else
			camera.fCameraSpeed = 10.0f;

		// Mouse
		camera.ProcessMouse(this, GetMousePosX(), GetMousePosY());

		camera.UpdateView(axesShader, "matView");
		camera.UpdateView(cubeShader, "matView");
		camera.UpdateView(lightCubeShader, "matView");
	}

	void Destroy() override
	{
		axesVBO.free();
		cubeVBO.free();

		axesVAO.free();
		cubeVAO.free();
		lightCubeVAO.free();

		std::cout << "\nDuration: " << std::fixed << std::setprecision(2) << fTimeSinceStart << 's' << std::endl;
	}

	int GetRandom()
	{
		return Random::get<int>(-1000, 1000);
	}

	void DisplayDistance(float fElapsedTime)
	{
		fTime += fElapsedTime;

		if (fTime > 2.0f)
		{
			std::cout << "Pos: (" << std::fixed << std::setprecision(2) << camera.vCameraPos.x << ", " << camera.vCameraPos.y << ", " << camera.vCameraPos.z << ")";
			std::cout << '\t' << "Distance: " << sqrtf(camera.vCameraPos.x * camera.vCameraPos.x + camera.vCameraPos.y * camera.vCameraPos.y + camera.vCameraPos.z * camera.vCameraPos.z) << '\n';
			fTime = 0.0f;

			std::cout.unsetf(std::ios_base::fixed);
			std::cout << std::setprecision(6);
		}
	}
};

int main()
{
	Console console;
	console.ConstructWindow(800, 600, "OpenGL");
	console.Start();

	std::cout << "Goodbye!" << std::endl;

	return 0;
}