#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

using namespace std;

typedef unsigned char  U8;//8bytes
typedef unsigned short U16;//16bytes

struct GLFWwindow;

struct Chip8
{
	GLFWwindow* m_WindowPtr = nullptr; //the opengl window

	Chip8(GLFWwindow* window)
	{
		//initialize openglwindow for drawing
		m_WindowPtr = window; 
	}

	//std array for safety reasons with memory
	array<U8, (size_t)4096> m_Memory;
	array<U8, (size_t)16> m_Registers;
	U16 m_IndexRegister;
	U16 m_ProgramCounter;
	int m_Size;

	array<U8, (size_t)(64 * 64)> m_ScreenBuffer;
	vector<U16> m_Stack;

	U8 m_DelayTimer;
	U8 m_SoundTimer;
	bool hiresmode;
	bool m_GameLoaded;
	bool m_Log;

	string m_Path;

	static const int KeyBoardLayout[16];
	static const unsigned char chip8_fontset[80];
	static const U16 PROGRAM_STARTPOS = 0x200;
	static const int AMOUNT_OF_KEYS = 16;

	//functions
	bool LoadGame(string path);
	bool RunCommand(const U16 command);
	bool GameLoop();
	void Draw();
	static void BeepPlay();
};