#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Utils.h"
#include "Camera.h"
#include "VertexData.h"
#include "OpenGL_Graphics.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

// Upper limit of random number generator. For lower limit, negate this value.
constexpr int MAX_RAND = 100;

// Number of cubes to be drawn
const size_t numCubes = 1000;

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
	BufferLayout cubeLayout;
	Shader cubeShader;

	// For lamp
	VertexArray lampVAO;
	VertexBuffer<float> cubeVBO;
	BufferLayout lampLayout;
	Shader lampShader;

	// Textures
	Texture2D diffuseTexture;
	Texture2D specularTexture;

	// Projection matrix
	glm::mat4 matProjection;
	float fFov = 80.0f;

	// Camera
	Camera camera;

	// Lamp positions and colors
	glm::vec3 vLightPos = glm::vec3(1.2f, 1.0f, 2.0f);
	glm::vec3 vLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	std::array<glm::vec3, 2> aLampPositions;

	/* ---- Other variables ---- */

	// Container cube positions
	std::array<glm::vec3, numCubes> aCubePos;

	// Container speed & distance in world space
	float fContainerSpeed = 5.0f;
	float fContainerDistance = 0.0f;

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

		/* ! -- Vertex buffer for cubes -- !
		 * We can use the same vertex buffer for both the light cube and containter cube
		 * It is important to note that vertex buffer is simply a region of data in the
		 * GPU's memory and holds no information about the layout of the buffer.
		 * Usually, each vertex contains vertex points, normals, texture coordinates, etc. and vertex buffer
		 * has no information about that. It is simply a region in GPU's memory having some bytes.
		 * 
		 * We can explicitly set the buffer layout to several vertex array object using 
		 * the function glAttribArrayPointer().
		 * The shader then accesses the vertex buffer in accordance to the buffer layout using location tags present
		 * in the shader.
		 * 
		*/
		cubeVBO.generate(8);
		cubeVBO.setBuffer(sizeof(cube_vertices), (const void*)cube_vertices);


		// Cube (container)
		/*
		 * Now we create a vertex array and set the buffer layouts while the vertex array is active.
		 * There is a unique vertex array for every type of object we are going to draw.
		 * In this case, the same vertex buffer is referenced by the containter cube array object (cubeVAO)
		 * and light cube array object (lampVAO).
		*/
		cubeVAO.generate();
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 3, BufferType::FLOAT);		// Vertices
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 3, BufferType::FLOAT);		// Normals
		cubeLayout.setBufferLayout(cubeVAO, cubeVBO, 2, BufferType::FLOAT);		// Texture coordinates
		cubeShader.load("shaders/Cube.glsl");

		// Lamp
		lampVAO.generate();
		lampLayout.setBufferLayout(lampVAO, cubeVBO, 3, BufferType::FLOAT);
		lampShader.load("shaders/Lamp.glsl");

		// Load textures
		diffuseTexture.loadTexture("resources/textures/container2.png");
		specularTexture.loadTexture("resources/textures/container2_specular.png");

		// Set textures
		// It is important to bind the texture to an int so that we can activate the texture in the next few lines
		cubeShader.use();
		cubeShader.setInt("u_material.diffuse", 0);
		cubeShader.setInt("u_material.specular", 1);

		// Activate textures
		glActiveTexture(GL_TEXTURE0);
		diffuseTexture.bindTexture();

		glActiveTexture(GL_TEXTURE1);
		specularTexture.bindTexture();

		// Set projection matrix in shaders as they do not change often
		matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
		axesShader.use();
		axesShader.setMat4("matProjection", matProjection);
		lampShader.use();
		lampShader.setMat4("matProjection", matProjection);
		cubeShader.use();
		cubeShader.setMat4("matProjection", matProjection);

		// Randomly generate cube positions
		for (auto& cubePos : aCubePos)
			cubePos = { random(), random(), random() };

		aLampPositions[0] = glm::vec3(0.7f, 0.2f, 2.0f);
		aLampPositions[1] = glm::vec3(2.3f, -3.3f, -4.0f);

		// ---------------------------------------- Directional light ---------------------------------------- 
		cubeShader.setVec3("u_dirLight.vDirection", glm::vec3(0.0f, -1.0f, 0.0f));
		cubeShader.setVec3("u_dirLight.vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setVec3("u_dirLight.vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		cubeShader.setVec3("u_dirLight.vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_dirLight.vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

		// ---------------------------------------- Point light ---------------------------------------- 
		// TODO: Implement multiple lamps using a for-loop
		cubeShader.setVec3("u_pointLights[0].vPosition", aLampPositions[0]);
		cubeShader.setVec3("u_pointLights[0].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setVec3("u_pointLights[0].vAmbient", glm::vec3(0.3f, 0.3f, 0.3f));
		cubeShader.setVec3("u_pointLights[0].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_pointLights[0].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("u_pointLights[0].fConstant", 1.0f);
		cubeShader.setFloat("u_pointLights[0].fLinear", 0.014f);
		cubeShader.setFloat("u_pointLights[0].fQuadratic", 0.0007f);

		cubeShader.setVec3("u_pointLights[1].vPosition", aLampPositions[1]);
		cubeShader.setVec3("u_pointLights[1].vLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

		cubeShader.setVec3("u_pointLights[1].vAmbient", glm::vec3(0.1f, 0.1f, 0.1f));
		cubeShader.setVec3("u_pointLights[1].vDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("u_pointLights[1].vSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("u_pointLights[1].fConstant", 1.0f);
		cubeShader.setFloat("u_pointLights[1].fLinear", 0.014f);
		cubeShader.setFloat("u_pointLights[1].fQuadratic", 0.0007f);

		cubeShader.setFloat("u_material.fShininess", 64.0f);

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

		// Others - Lamp shader
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

		// For camera debug
		DrawAxes();

		return true;
	}

	void DrawCubes(float fElapsedTime)
	{
		cubeShader.use();
		cubeVAO.bind();

		// Some uniforms need to change on user input, hence update them every frame
		cubeShader.setVec3("u_spotLight.vPosition", camera.vCameraPos);
		cubeShader.setVec3("u_spotLight.vDirection", camera.vCameraFront);
		cubeShader.setVec3("u_vViewPos", camera.vCameraPos);

		// One cube can be controlled by the user (for debugging)
		// Inputs
		if (GetKey(GLFW_KEY_RIGHT).bPressed)
			fContainerSpeed = 5.0f;
		else if (GetKey(GLFW_KEY_LEFT).bPressed)
			fContainerSpeed = -5.0f;
		else if (GetKey(GLFW_KEY_DOWN).bPressed)
			fContainerSpeed = 0.0f;

		// Physics: dist = integral (speed, time)
		fContainerDistance += fContainerSpeed * fElapsedTime;

		// Transform model matrix
		glm::mat4 matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(0.0f, 0.0f, 0.0f));
		cubeShader.setMat4("matModel", matModel);

		// Draw the moving cube
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Transform model matrix
		matModel = glm::mat4(1.0f);
		matModel = glm::translate(matModel, glm::vec3(0.0f, 0.0f, 1.0f));
		cubeShader.setMat4("matModel", matModel);

		// Draw the moving cube
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw other cubes
		for (const auto& cubePos : aCubePos)
		{
			glm::mat4 matModel = glm::mat4(1.0f);
			matModel = glm::translate(matModel, cubePos);
			matModel = glm::rotate(matModel, fTimeSinceStart, glm::vec3(0.0f, 1.0f, 0.0f));
			cubeShader.setMat4("matModel", matModel);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	void DrawLamp(float fElapsedTime)
	{
		lampShader.use();
		lampVAO.bind();

		// Draw light cubes
		for (const auto& pos : aLampPositions)
		{
			glm::mat4 matModel = glm::mat4(1.0f);
			matModel = glm::translate(matModel, pos);
			matModel = glm::scale(matModel, glm::vec3(0.2f));
			lampShader.setMat4("matModel", matModel);

			// Draw the lamp
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
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
		}

		else if(GetKey('C').bReleased)
		{
			fFov = 80.0f;

			matProjection = glm::perspective(fFov * pi / 180.0f, (float)ScreenWidth() / (float)ScreenHeight(), 0.1f, 1000.0f);
			axesShader.use();
			axesShader.setMat4("matProjection", matProjection);
			lampShader.use();
			lampShader.setMat4("matProjection", matProjection);
			cubeShader.use();
			cubeShader.setMat4("matProjection", matProjection);
		}

		if (GetKey(GLFW_KEY_LEFT_CONTROL).bHeld)
			camera.fCameraSpeed = 30.0f;
		else
			camera.fCameraSpeed = 10.0f;

		if (GetKey(GLFW_KEY_HOME).bPressed)
			camera.init(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		/* ------------------------------------------ - Mouse Control - ------------------------------------------- */
		camera.ProcessMouse(this, GetMousePosX(), GetMousePosY());

		// Update view matrix
		camera.UpdateView(axesShader, "matView");
		camera.UpdateView(lampShader, "matView");
		camera.UpdateView(cubeShader, "matView");
	}

	void Destroy() override
	{
		axesVAO.free();
		axesVBO.free();

		lampVAO.free();
		cubeVAO.free();
		cubeVBO.free();

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