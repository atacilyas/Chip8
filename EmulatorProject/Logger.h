#pragma once
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

using namespace std;

typedef unsigned char  U8;
typedef unsigned short U16;

class Logger
{
public:
	static Logger* getInstance()
	{
		if (m_Instance != nullptr)
		{
			m_Instance = new Logger();
		}
		return m_Instance;
	}
	void SetWindow(GLFWwindow* window)
	{
		if (m_Instance != nullptr)
		{
			m_WindowPtr = window;
		}
	}
	static void Log(string message,int color = 0x07);
	void LogOpcode(unsigned short opcode);

private:
	static const int KeyBoardLayout[16];
	static const int AMOUNT_OF_KEYS = 16;

	GLFWwindow* m_WindowPtr;
	Logger() {};
	static Logger* m_Instance;
};