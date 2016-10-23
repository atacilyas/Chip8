#include <iostream>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Gip8Emulator
#include "Chip8.h"

#include "Logger.h"

// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec2 position;"
"in vec2 texcoord;"
"in vec3 inColor;"
"out vec3 InColor;"
"out vec2 Texcoord;"
"void main() {"
"   InColor = inColor;"
"   Texcoord = texcoord;"
"   gl_Position = vec4(position, 0.0, 1.0);"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"in vec2 Texcoord;"
"in vec3 InColor;"
"out vec4 outColor;"
"uniform sampler2D tex;"
"void main() {"
"   outColor = texture(tex, Texcoord) * vec4(InColor,1.0f);"
"}";


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 1024, HEIGHT = 512;
bool Initialize(GLFWwindow *wndw);

Chip8* m_Emulator;


// The MAIN function, from here we start the application and run the game loop
int main(int argc, char* argv[])
{
	for (size_t i = 0; i < argc; i++)
	{
		cout << argv[i] << endl;
	}
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	m_Emulator = new Chip8(window);
	Logger::getInstance()->SetWindow(window);

	GLFWimage* t;
	int gamespeed = 5;
	bool lasthiresmode = true;
	if (Initialize(window))
	{
		m_Emulator->LoadGame("Chip-8_Pack/Chip-8 Demos/Maze (alt) [David Winter, 199x].ch8");
		if (argc > 1)
		{
			m_Emulator->LoadGame(argv[1]);
		}
		bool t = true;
		// Game loop
		while (!glfwWindowShouldClose(window) && t)
		{
			// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
			glfwPollEvents();

			if (glfwGetKey(window, GLFW_KEY_UP))
			{
				++gamespeed;
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN))
			{
				--gamespeed;
				if (gamespeed <= 1)
				{
					gamespeed = 1;
				}
			}
			
			if (lasthiresmode != m_Emulator->hiresmode)
			{
				lasthiresmode = m_Emulator->hiresmode;
				if (m_Emulator->hiresmode)
				{
					glfwSetWindowSize(window, WIDTH, WIDTH);
					glViewport(0, 0, WIDTH, WIDTH);
				}
				else
				{
					glfwSetWindowSize(window, WIDTH, HEIGHT);
					glViewport(0, 0, WIDTH, HEIGHT);

				}
			}

			for (size_t i = 0; i < (size_t)gamespeed; i++)
			{
				t = m_Emulator->GameLoop();
			}
			

			glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Emulator->Draw();

			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Swap the screen buffers
			glfwSwapBuffers(window);
		}
	}

	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void OnDragAndDrop(GLFWwindow *wndw, int i, const char **path)
{
	string t = string(*path);
	if (m_Emulator->LoadGame(t), Initialize(wndw))
	{
		cout << "Loaded file " << *path << endl;
		Logger::getInstance()->Log("Loaded file");
	}

	glfwSetWindowTitle(wndw, t.substr(t.find_last_of('\\')+1, t.find_last_of('.') - t.find_last_of('\\')).c_str()-1);
	//system("pause");
}

bool Initialize(GLFWwindow *wndw)
{
	////////////Make the triangle

	//every time you call vertexattributepointer the information will be put in the vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//vertice list
	float vertices[] = {
		//2d pos		//uv coords		//color
		 1.0f, -1.0f,	1.0f,1.0f,		1.0f,1.0f,1.0f,//C
		 1.0f,  1.0f,	1.0f,0.0f,		1.0f,1.0f,1.0f,//B
		-1.0f, -1.0f,	0.0f,1.0f,		1.0f,1.0f,1.0f,//D

		-1.0f, -1.0f,	0.0f,1.0f,		1.0f,1.0f,1.0f,//D
		-1.0f,  1.0f,	0.0f,0.0f,		1.0f,1.0f,1.0f,//A
		 1.0f,  1.0f,	1.0f,0.0f,		1.0f,1.0f,1.0f //B

	};

	//get the vertex buffer
	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate 1 buffer

						   //to upload data to your vertex bufffer on the grafics card you have o set it active
						   //set vbo the active vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//copy your vertex data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/////////////compile the vertex shader

	//make a shader object and load the shader in it
	//see the top for the shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);

	//compile the shader
	glCompileShader(vertexShader);

	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		return false;
	}
	/////////////same for the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);


	//combine the two shaders into a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//specify which fragment shader output is written to which buffer
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	//link the shader programm
	glLinkProgram(shaderProgram);
	//start using the shader programm
	glUseProgram(shaderProgram);

	//link the vertex data to the shader
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	// specify how the data for that input is retrieved from the array
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
	glEnableVertexAttribArray(posAttrib);

	//link the vertex data to the shader
	GLint UvAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	// specify how the data for that input is retrieved from the array
	glVertexAttribPointer(UvAttrib, 2, GL_FLOAT, GL_FALSE,7*sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(UvAttrib);

	//link the vertex data to the shader
	GLint ColorAttrib = glGetAttribLocation(shaderProgram, "inColor");
	// specify how the data for that input is retrieved from the array
	glVertexAttribPointer(ColorAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(ColorAttrib);

	//textures
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glfwSetDropCallback(wndw, OnDragAndDrop);

	return true;
}

