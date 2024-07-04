#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utils.h"
#include "Camera.h"
#include "VertexData.h"
#include "OpenGL_Graphics.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>

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

	// For cubes
	VertexArray cubeVAO;
	VertexBuffer<float> cubeVBO;
	Shader cubeShader;
	int cubeVertexCount;

	// For model
	VertexArray modelVAO;
	VertexBuffer<float> modelVBO;
	Shader modelShader;
	int modelVertexCount;

	// For lamp
	VertexArray lampVAO;
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

		// Load cube
		if (!LoadModel(cubeVAO, cubeVBO, cubeVertexCount, "models/cube.obj"))
			return false;
		cubeShader.load("shaders/Cube.glsl");

		// Load model
		if (!LoadModel(modelVAO, modelVBO, modelVertexCount, "models/Towers1.obj"))
			return false;
		modelShader.load("shaders/Model.glsl");

		// Lamp
		cubeVBO.generate(3);
		cubeVBO.setBuffer(sizeof(cube_vertices), (const void*)cube_vertices);

		lampVAO.generate();
		BufferLayout lampLayout;
		lampLayout.setBufferLayout(lampVAO, cubeVBO, 3, BufferType::FLOAT);
		lampShader.load("shaders/Lamp.glsl");

		// Set projection matrix in shaders as they do not change often
		matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);
		lampShader.use();
		lampShader.setMat4("matProjection", matProjection);
		cubeShader.use();
		cubeShader.setMat4("matProjection", matProjection);
		modelShader.use();
		modelShader.setMat4("matProjection", matProjection);

		// For cube shader
		// ---------------------------------------- Directional light ---------------------------------------- 
		cubeShader.use();
		cubeShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		cubeShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		cubeShader.setVec3("u_dirLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_dirLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		// ---------------------------------------- Point light ---------------------------------------- 
		// TODO: Implement multiple lamps using a for-loop
		cubeShader.setVec3("u_pointLights[0].vPosition", vLightPos);
		cubeShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		cubeShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_pointLights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		cubeShader.setFloat("u_pointLights[0].fLinear", 0.014f);
		cubeShader.setFloat("u_pointLights[0].fQuadratic", 0.0007f);

		cubeShader.setFloat("u_material.fShininess", 64.0f);
		cubeShader.setVec3("u_material.vColor", glm::vec3(0.5f, 0.5f, 0.5f));

		// ---------------------------------------- Spot light ---------------------------------------- 
		cubeShader.setVec3("u_spotLight.vLightColor", glm::vec3(0.0f, 0.0f, 1.0f));

		cubeShader.setVec3("u_spotLight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		cubeShader.setVec3("u_spotLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_spotLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setFloat("u_spotLight.fConstant", 1.0f);
		cubeShader.setFloat("u_spotLight.fLinear", 0.22f);
		cubeShader.setFloat("u_spotLight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		cubeShader.setFloat("u_dirLight.fCutOff", 30.0f * pi / 180.0f);
		cubeShader.setFloat("u_dirLight.fOuterCutOff", 45.0f * pi / 180.0f);

		// For model shader
		// ---------------------------------------- Directional light ---------------------------------------- 
		modelShader.use();
		modelShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		modelShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		modelShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		modelShader.setVec3("u_dirLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		modelShader.setVec3("u_dirLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		// ---------------------------------------- Point light ---------------------------------------- 
		// TODO: Implement multiple lamps using a for-loop
		modelShader.setVec3("u_pointLights[0].vPosition", vLightPos);
		modelShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		modelShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		modelShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		modelShader.setVec3("u_pointLights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		modelShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		modelShader.setFloat("u_pointLights[0].fLinear", 0.014f);
		modelShader.setFloat("u_pointLights[0].fQuadratic", 0.0007f);

		modelShader.setFloat("u_material.fShininess", 64.0f);
		modelShader.setVec3("u_material.vColor", glm::vec3(0.5f, 0.5f, 0.5f));

		// ---------------------------------------- Spot light ---------------------------------------- 
		modelShader.setVec3("u_spotLight.vLightColor", glm::vec3(0.0f, 0.0f, 1.0f));

		modelShader.setVec3("u_spotLight.vAmbient", glm::vec3(0.6f, 0.6f, 0.6f));
		modelShader.setVec3("u_spotLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		modelShader.setVec3("u_spotLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		modelShader.setFloat("u_spotLight.fConstant", 1.0f);
		modelShader.setFloat("u_spotLight.fLinear", 0.22f);
		modelShader.setFloat("u_spotLight.fQuadratic", 0.20f);

		// Cutoff and outer cutoff angles are 30 and 45 degrees respectively
		modelShader.setFloat("u_dirLight.fCutOff", 30.0f * pi / 180.0f);
		modelShader.setFloat("u_dirLight.fOuterCutOff", 45.0f * pi / 180.0f);

		// Others
		lampShader.use();
		lampShader.setVec3("vLampColor", vLightColor);

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.06f, 0.06f, 0.06f, 1.0f);

		HandleInputs(fElapsedTime);

		DrawLamp(fElapsedTime);

		DrawCubes(fElapsedTime);

		DrawModel(fElapsedTime);

		DrawAxes();

		return true;
	}

	void DrawModel(float fElapsedTime)
	{
		modelShader.use();
		modelVAO.bind();

		// Some uniforms need to change on user input, hence update them every frame
		modelShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		modelShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		modelShader.setVec3("u_vViewPos", camera.vCameraPos);

		// Transform model matrix
		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(-20.0f, 0.0f, 0.0f));
		//matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
		modelShader.setMat4("matModel", matModel);

		glDrawArrays(GL_TRIANGLES, 0, modelVertexCount);

		// Transform model matrix
		matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(-20.0f, 0.0f, 50.0f));
		matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
		modelShader.setMat4("matModel", matModel);

		glDrawArrays(GL_TRIANGLES, 0, modelVertexCount);
	}

	void DrawCubes(float fElapsedTime)
	{
		cubeShader.use();
		cubeVAO.bind();

		// Some uniforms need to change on user input, hence update them every frame
		cubeShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		cubeShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		cubeShader.setVec3("u_vViewPos", camera.vCameraPos);

		// Transform model matrix
		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(5.0f, 0.0f, 0.0f));
		//matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
		cubeShader.setMat4("matModel", matModel);

		glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
	}

	void DrawAxes()
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

	void DrawLamp(float fElapsedTime)
	{
		lampShader.use();
		lampVAO.bind();

		// Draw light cubes
		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, vLightPos);
		matModel = glm::scale(matModel, glm::vec3(0.2f));
		lampShader.setMat4("matModel", matModel);

		// Draw the lamp
		glDrawArrays(GL_TRIANGLES, 0, 36);
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
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
			cubeShader.use();
			cubeShader.setMat4("matProjection", matProjection);
			modelShader.use();
			modelShader.setMat4("matProjection", matProjection);
		}

		else if (GetKey('C').bReleased)
		{
			fFov = 80.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
			cubeShader.use();
			cubeShader.setMat4("matProjection", matProjection);
			modelShader.use();
			modelShader.setMat4("matProjection", matProjection);
		}

		if (GetKey(GLFW_KEY_LEFT_CONTROL).bHeld)
			camera.fCameraSpeed = 10.0f;
		else
			camera.fCameraSpeed = 2.0f;

		if (GetKey(GLFW_KEY_HOME).bPressed)
			camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		/* ------------------------------------------ - Mouse Control - ------------------------------------------- */
		camera.ProcessMouse(this, GetMousePosX(), GetMousePosY());

		// Update view matrix
		camera.UpdateView(axesShader, "matView");
		camera.UpdateView(lampShader, "matView");
		camera.UpdateView(cubeShader, "matView");
		camera.UpdateView(modelShader, "matView");
	}

	void Destroy() override
	{
		axesVAO.free();
		axesVBO.free();

		lampVAO.free();
		cubeVAO.free();

		//cubeIBO.free();
		//cubeNormalIBO.free();

		cubeVBO.free();

		std::cout << "\nDuration: " << std::fixed << std::setprecision(2) << fTimeSinceStart << 's' << std::endl;
	}

	bool LoadModel(VertexArray& vao, VertexBuffer<float>& vbo, int& vertexCount, const std::string& modelFile)
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

		std::cout << "Loading " << modelFile << "...\n";
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

		std::cout << "Finished loading!\n";
		std::cout << "Number of vertices: " << vertexCount << "\n";
		std::cout << "Size (bytes): " << vertexCount * sizeof(Vertex) << " bytes (" << std::fixed << std::setprecision(2) << (float)(vertexCount * sizeof(Vertex) / (1024.0f * 1024.0f)) << " MB)\n\n";

		return true;
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