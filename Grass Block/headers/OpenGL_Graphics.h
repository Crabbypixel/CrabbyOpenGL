#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

std::mutex mtx;

class OpenGL_Graphics
{
private:
	int m_width;
	int m_height;
	std::string m_sAppName;

	short m_keyNewState[348] = { 0 };
	short m_keyOldState[348] = { 0 };
	short m_mouseOldState[3];
	short m_mouseNewState[3];
	struct sKeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} m_keys[348], m_mouse[3];

	float m_mousePosX;
	float m_mousePosY;
	int m_mouseScroll;
	bool m_bMouseButtonHeld[3] = { false };

	static std::atomic<bool> m_bIsRunning;

protected:
	GLFWwindow* window;

	enum class Mouse
	{
		LEFT = 0,
		RIGHT = 1,
		MIDDLE = 2,
		SCROLL_UP = 3,
		SCROLL_DOWN = 4
	};

public:
	float fTimeSinceStart = 0.0f;
	bool bFirstMouse = true;

private:
	// Main renderer thread which constantly renders to the screen
	void RendererThread()
	{
		// We want the window to be in the renderer thread context (important!)
		glfwMakeContextCurrent(window);

		float fAccumulatedTime = 0.0f;
		int iFrameCount = 0;

		if (!Setup())
			m_bIsRunning = false;

		auto dt1 = std::chrono::system_clock::now();
		auto dt2 = std::chrono::system_clock::now();

		// Run as fast as possible
		while (m_bIsRunning)
		{
			dt2 = std::chrono::system_clock::now();
			std::chrono::duration<float> elapsedTime = dt2 - dt1;
			dt1 = dt2;

			float fElapsedTime = elapsedTime.count();
			fTimeSinceStart += fElapsedTime;

			// Keyboard inputs
			for (int i = 0; i < 348; i++)
			{
				m_keyNewState[i] = glfwGetKey(window, i) == GLFW_PRESS;

				m_keys[i].bPressed = false;
				m_keys[i].bReleased = false;

				if (m_keyNewState[i] != m_keyOldState[i])
				{
					if (m_keyNewState[i])
					{
						m_keys[i].bPressed = !m_keys[i].bHeld;
						m_keys[i].bHeld = true;
					}
					else
					{
						m_keys[i].bReleased = true;
						m_keys[i].bHeld = false;
					}
				}

				m_keyOldState[i] = m_keyNewState[i];
			}
			
			// Mouse inputs
			for (int i = 0; i < 3; i++)
			{
				m_mouseNewState[i] = m_bMouseButtonHeld[i];

				m_mouse[i].bPressed = false;
				m_mouse[i].bReleased = false;

				if (m_mouseNewState[i] != m_mouseOldState[i])
				{
					if (m_mouseNewState[i])
					{
						m_mouse[i].bPressed = !m_mouse[i].bHeld;
						m_mouse[i].bHeld = true;
					}
					else
					{
						m_mouse[i].bReleased = true;
						m_mouse[i].bHeld = false;
					}
					m_mouseOldState[i] = m_mouseNewState[i];
				}
			}

			if (!Update(fElapsedTime))
			{
				m_bIsRunning = false;
			}

			// FPS calculation
			iFrameCount++;
			fAccumulatedTime += fElapsedTime;

			// Update FPS every 0.5 seconds
			if (fAccumulatedTime >= 0.5f)
			{
				int fps = (int)(iFrameCount / fAccumulatedTime);
				
				if (window)
				{
					char s[32];
					sprintf_s(s, 32, "%s : %d FPS", m_sAppName.c_str(), fps);
					glfwSetWindowTitle(window, s);
				}

				fAccumulatedTime = 0.0f;
				iFrameCount = 0;
			}

			m_mouseScroll = 0;
			//m_mouse[2].bPressed = false;
			m_mouse[2].bReleased = false;

			// Swap buffers
			glfwSwapBuffers(window);
		}

		// Give the window context back to the main thread
		glfwMakeContextCurrent(nullptr);
	}

public:
	int ScreenWidth() const  { return m_width; }
	int ScreenHeight() const { return m_height; }
	float GetMousePosX() const { return m_mousePosX; }
	float GetMousePosY() const { return m_mousePosY; }
	Mouse GetMouseScroll() const { return (Mouse)m_mouseScroll; }
	sKeyState GetMouseButton(Mouse button) const { return m_mouse[(int)button]; }
	sKeyState GetKey(int nKeyID) const { return m_keys[nKeyID]; }

	OpenGL_Graphics()
	{
		window = NULL;
		m_width = m_height = 0;
	}

	~OpenGL_Graphics()
	{
	}

	void ConstructWindow(int width, int height, std::string windowName)
	{
		m_sAppName = windowName;
		m_width = width;
		m_height = height;
		
		// Initalize GLFW and initalize OpenGL to version 3.3
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		// Make the window non-resizable
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		// Create a window
		window = glfwCreateWindow(m_width, m_height, m_sAppName.c_str(), NULL, NULL);
		if (window == NULL)
			Error("Failed to create window.");

		// Set window position on screen
		glfwSetWindowPos(window, 360, 75);

		// Make the window to in the current context
		glfwMakeContextCurrent(window);

		// Disable cursor
		glfwSetCursorPos(window, m_width / 2.0f, m_height / 2.0f);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Disable V-Sync (to achieve 60+ fps)
		// Comment this out to get 60 fps (max)
		glfwSwapInterval(0);

		// Load all function pointers using GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			Error("Failed to initalize GLAD");

		// Set viewport and callback function when window gets resized 
		glViewport(0, 0, m_width, m_height);
		//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		// Enable z-buffer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Display GPU info
		CheckGPU();
	}

	void Start()
	{
		m_bIsRunning = true;

		glfwSetWindowUserPointer(window, this);

		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);

		// Make the window context null before calling the renderer thread
		glfwMakeContextCurrent(nullptr);

		// Start the renderer
		std::thread rendererThread = std::thread(&OpenGL_Graphics::RendererThread, this);

		// While the renderer thread is running, the main thread handles poll events
		while (m_bIsRunning)
		{
			processInput(window);

			if (glfwWindowShouldClose(window))
				m_bIsRunning = false;

			//glfwPollEvents();
			glfwWaitEvents();
		}

		// Wait until the renderer thread exits
		if (rendererThread.joinable())
			rendererThread.join();

		// Cleanup functions
		Destroy();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

protected:
	// Has to be overriden by subclasses
	virtual bool Setup() = 0;
	virtual bool Update(float fElapsedTime) = 0;

	// Optional to override
	virtual void Destroy() { }

// Private functions
private:
	void Error(const std::string& message)
	{
		std::cerr << "Error: " << message << std::endl;
		glfwTerminate();
		exit(-1);
	}

	void processInput(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			m_bIsRunning = false;
			glfwSetWindowShouldClose(window, true);
		}
	}

	static void mouse_callback(GLFWwindow* window, double xPos, double yPos)
	{
		OpenGL_Graphics* instance = static_cast<OpenGL_Graphics*>(glfwGetWindowUserPointer(window));
		instance->m_mousePosX = (float)xPos;
		instance->m_mousePosY = (float)yPos;
		instance->bFirstMouse = false;
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		OpenGL_Graphics* instance = static_cast<OpenGL_Graphics*>(glfwGetWindowUserPointer(window));
		if ((int)yoffset == 1)
			instance->m_mouseScroll = (int)Mouse::SCROLL_UP;
		else if ((int)yoffset == -1)
			instance->m_mouseScroll = (int)Mouse::SCROLL_DOWN;
		else
			instance->m_mouseScroll = 0;
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		OpenGL_Graphics* instance = static_cast<OpenGL_Graphics*>(glfwGetWindowUserPointer(window));
		instance->m_bMouseButtonHeld[button] = action;
	}

	void CheckGPU()
	{
		const unsigned char* renderer = glGetString(GL_RENDERER);
		const unsigned char* vendor = glGetString(GL_VENDOR);
		const unsigned char* version = glGetString(GL_VERSION);
		const unsigned char* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

		std::cout << "---------- GPU information ----------\n";
		std::cout << std::endl;
		std::cout << "Renderer: " << renderer << '\n';
		std::cout << "Vendor: " << vendor << '\n';
		std::cout << "OpenGL Version: " << version << '\n';
		std::cout << "GLSL Version: " << glslVersion << '\n';
		std::cout << std::endl;
		std::cout << "-------------------------------------\n\n";
	}
};

std::atomic<bool> OpenGL_Graphics::m_bIsRunning(false);
