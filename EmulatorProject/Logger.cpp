#include "Logger.h"
#include <windows.h>
#include <GLFW/glfw3.h>
#include <sstream>

Logger* Logger::m_Instance = nullptr;

const int Logger::KeyBoardLayout[AMOUNT_OF_KEYS] =
{
	GLFW_KEY_X, /*0*/	GLFW_KEY_1, /*1*/	GLFW_KEY_2, /*2*/	GLFW_KEY_3, /*3*/
	GLFW_KEY_Q, /*4*/	GLFW_KEY_W, /*5*/	GLFW_KEY_E, /*6*/	GLFW_KEY_A, /*7*/
	GLFW_KEY_S, /*8*/	GLFW_KEY_D, /*9*/	GLFW_KEY_Z, /*A*/	GLFW_KEY_C, /*B*/
	GLFW_KEY_4, /*C*/	GLFW_KEY_R, /*D*/	GLFW_KEY_F, /*E*/	GLFW_KEY_V  /*F*/
};

void Logger::Log(string message,int level)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, level);
	cout << std::hex << message << endl;
	SetConsoleTextAttribute(hConsole, 0x07);
}

void Logger::LogOpcode(unsigned short command)
{
	cout << std::hex << (int)command << " ";
	int firstopcodepart = (command >> 12) & 0xF;
	switch (firstopcodepart)
	{
	case 0x0:
	{
		int lastOpcodePart = (command)& 0xFF;
		if (lastOpcodePart == 0xE0)
		{
			Logger::Log("Clear screen");
		}
		else if (lastOpcodePart == 0xEE)
		{
			///00EE 	Returns from a subroutine.
			Logger::Log("Return from a subroutine");
		}
	}
	break;

	case 0x1:
	{
		///1NNN 	Jumps to address NNN.
		U16 nnn = command & 0x0FFF;

		std::stringstream stream;
		stream << "jump to " << hex << (int)nnn;
		Logger::Log(stream.str());
	}
	break;

	case 0x2:
	{
		///2NNN 	Calls subroutine at NNN.
		U16 nnn = command & 0x0FFF;

		std::stringstream stream;
		stream << "Calls subroutine at " << hex << (int)nnn;
		Logger::Log(stream.str());
	}
	break;

	case 0x3:
	{
		///3XNN 	Skips the next instruction if VX equals NN.
		U8 x;
		x = (command >> 8) & 0x0F;
		U16 nn;
		nn = (command & 0x00FF);

		std::stringstream stream;
		stream << "Skips the next instruction if V  " << (int)x << " equals " << (int)nn;
		Logger::Log(stream.str());
	}
	break;

	case 0x4:
	{
		///4XNN 	Skips the next instruction if VX doesn't equal NN.
		U8 x;
		x = (command >> 8) & 0x0F;
		U16 nn;
		nn = (command & 0x00FF);

		std::stringstream stream;
		stream << "Skips the next instruction if V  " << (int)x << " doesn't equals " << (int)nn;
		Logger::Log(stream.str());
	}
	break;

	case 0x5:
	{
		///5XY0 	Skips the next instruction if VX equals VY.

		U8 x, y;
		x = (command >> 8) & 0x0F;
		y = (command >> 4) & 0x0F;

		std::stringstream stream;
		stream << "Skips the next instruction if V  " << (int)x << " equals V" << (int)y;
		Logger::Log(stream.str());
	}
	break;

	case 0x6:
	{
		///6XNN 	Sets VX to NN.
		U8 x, nn;
		x = (command >> 8) & 0x0F;
		nn = (command)& 0x00FF;

		std::stringstream stream;
		stream << "Sets V" << (int)x << " to " << (int)nn;
		Logger::Log(stream.str());
	}
	break;

	case 0x7:
	{
		///7XNN 	Adds NN to VX.
		U8 x, nn;
		x = (command >> 8) & 0x0F;
		nn = (command)& 0x00FF;

		std::stringstream stream;
		stream << "Adds " << hex << (int)nn << " to V" << (int)x;
		Logger::Log(stream.str());
	}
	break;

	case 0x8:
	{
		U8 lastOpcodePart = (command & 0x000F);

		U8 x, y;
		x = (command >> 8) & 0xF;
		y = (command >> 4) & 0xF;

		switch (lastOpcodePart)
		{
		case 0x0:
		{
			///8XY0 	Sets VX to the value of VY.
			std::stringstream stream;
			stream << "sets V" << (int)x << " to the value of V" << (int)y;
			Logger::Log(stream.str());
		}break;

		case 0x1:
		{
			///8XY1 	Sets VX to VX or VY.
			std::stringstream stream;
			stream << "sets V" << (int)x << "to V" << (int)x << " or V" << (int)y;
			Logger::Log(stream.str());
		}break;

		case 0x2:
		{
			///8XY2 	Sets VX to VX and VY.
			std::stringstream stream;
			stream << "sets V" << (int)x << "to V" << (int)x << " and V" << (int)y;
			Logger::Log(stream.str());
		}break;

		case 0x3:
		{
			///8XY3 	Sets VX to VX xor VY.
			std::stringstream stream;
			stream << "sets V" << (int)x << "to V" << (int)x << " xor V" << (int)y;
			Logger::Log(stream.str());
		}break;

		case 0x4:
		{
			///8XY4 	Adds VY to VX.VF is set to 1 when there's a carry, and to 0 when there isn't.
			std::stringstream stream;
			stream << "Adds V"<<(int)y<<" to V"<<(int)x<<".VF is set to 1 when there's a carry, and to 0 when there isn't";
			Logger::Log(stream.str());
		}break;

		case 0x5:
		{
			///8XY5 	VY is subtracted from VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
			std::stringstream stream;
			stream << "V" << (int)y << " is subtracted from V" << (int)x << ".VF is set to 0 when there's a borrow, and 1 when there isn't";
			Logger::Log(stream.str());
		}break;

		case 0x6:
		{
			///8XY6 	Shifts VX right by one.VF is set to the value of the least significant bit of VX before the shift.[2]
			std::stringstream stream;
			stream << "Shifts V" << (int)x << " right by one.VF is set to the value of the least significant bit of V"<<(int)x<<" before the shift.[2]";
			Logger::Log(stream.str());
		} break;

		case 0x7:
		{
			///8XY7 	Sets VX to VY minus VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
			std::stringstream stream;
			stream << "Sets V" << (int)x << " to V" << (int)y << "minus V" << (int)x << ".VF is set to 0 when there's a borrow, and 1 when there isn't.";
			Logger::Log(stream.str());
		} break;

		case 0xE:
		{
			///8XYE 	Shifts VX left by one.VF is set to the value of the most significant bit of VX before the shift.[2]
			std::stringstream stream;
			stream << "Shifts V" << (int)x << " left by one.VF is set to the value of the most significant bit of V" << (int)x << " before the shift.[2]";
			Logger::Log(stream.str());
		} break;

		default:
			break;
		}
	}
	break;

	case 0x9:
	{
		///9XY0 	Skips the next instruction if VX doesn't equal VY.
		U8 x, y;
		x = (command >> 8) & 0x0F;
		y = (command >> 4) & 0x0F;

		std::stringstream stream;
		stream << "Skips the next instruction if V" << (int)x << " doesn't equal V" << (int)y;
		Logger::Log(stream.str());
	}
	break;

	case 0xA:
	{
		U16 nnn = command & 0x0FFF;

		///ANNN 	Sets I to the address NNN.
		std::stringstream stream;
		stream << "Sets I to the address " << hex << (int)nnn;
		Logger::Log(stream.str());
	}
	break;

	case 0xB:
	{
		///BNNN 	Jumps to the address NNN plus V0.
		U16 nnn = command & 0x0FFF;
		std::stringstream stream;
		stream << "Jumps to the address " << (int)nnn << "plus V0";
		Logger::Log(stream.str());
	}
	break;

	case 0xC:
	{
		///CXNN 	Sets VX to the result of a bitwise and operation on a random number and NN.
		U8 x, nn;
		x = (command >> 8) & 0x0F;
		nn = (command & 0x00FF);

		std::stringstream stream;
		stream << "Sets V" << hex <<(int)x<<" to the result of a bitwise and operation on a random number and " << (int)nn;
		Logger::Log(stream.str());
	}
	break;

	case 0xD:
	{
		///DXYN 	Sprites stored in m_Memory at location in index register (I), 8bits wide. Wraps around the screen.
		///If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
		///All drawing is XOR drawing (i.e. it toggles the screen pixels).
		///Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn. If N is greater than 1,
		///second line continues at position VX, VY+1, and so on.
		std::stringstream stream;
		stream << "Draw/////////////////////////////////";
		Logger::Log(stream.str());
	}
	break;

	case 0xE:
	{
		int x = (command >> 8) & 0xF;
		
		if ((command & 0xF0FF) == 0xE09E)
		{
			for (size_t i = 0; i < 0; i++)
			{
				if (glfwGetKey(m_WindowPtr, KeyBoardLayout[i]))
				{
					cout << "key pressed " << (int)KeyBoardLayout[i] << endl;
				}
			}
			///EX9E 	Skips the next instruction if the key stored in VX is pressed.
		}
		else if ((command & 0xF0FF) == 0xE0A1)
		{
			///EXA1 	Skips the next instruction if the key stored in VX isn't pressed.
			//for (size_t i = 0; i < AMOUNT_OF_KEYS; i++)
			//{
			//	if (glfwGetKey(m_WindowPtr, KeyBoardLayout[i]))
			//	{
			//		cout << "key pressed " << (int)KeyBoardLayout[i] << endl;
			//	}
			//}
		}
	}
	break;

	case 0xF:
	{
		U8 lastopcodepart = command & 0xFF;
		U8 x = (command >> 8) & 0xF;

		switch (lastopcodepart)
		{
		case 0x07:
		{
			///FX07 	Sets VX to the value of the delay timer.
			std::stringstream stream;
			stream << "Sets V" << (int)x << " to the value of the delay timer";
			Logger::Log(stream.str());
		}
		break;
		case 0x0A:
		{
			///FX0A 	A key press is awaited, and then stored in VX.
			//m_ProgramCounter -= 2;
			//for (size_t i = 0; i < 16; i++)
			//{
			//	if (glfwGetKey(m_WindowPtr, KeyBoardLayout[(int)m_Registers[i]]))
			//	{
			//		m_Registers[x] = i;
			//		m_ProgramCounter += 2;
			//		break;
			//	}
			//}
		}
		break;
		case 0x15:
		{
			///FX15 	Sets the delay timer to VX.
			std::stringstream stream;
			stream << "Sets the delay timer to V" << (int)x;
			Logger::Log(stream.str());
		}
		break;
		case 0x18:
		{
			///FX18 	Sets the sound timer to VX.
			std::stringstream stream;
			stream << "Sets the sound timer to V" << (int)x;
			Logger::Log(stream.str());
		}
		break;
		case 0x1E:
		{
			///FX1E 	Adds VX to I.[3]
			std::stringstream stream;
			stream << "Adds V" << (int)x << " to I";
			Logger::Log(stream.str());
		}
		break;
		case 0x29:
		{
			///FX29 	Sets I to the location of the sprite for the character in VX.Characters 0 - F(in hexadecimal) are represented by a 4x5 font.
			std::stringstream stream;
			stream << "Sets I to the location of the sprite for the character in V" << (int)x << " Characters 0 - F(in hexadecimal) are represented by a 4x5 font.";
			Logger::Log(stream.str());
		}
		break;
		case 0x33:
		{
			///FX33 	Stores the Binary - coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in m_Memory at location in I, the tens digit at location I + 1, and the ones digit at location I + 2.)
		}
		break;
		case 0x55:
		{
			///FX55 	Stores V0 to VX in m_Memory starting at address I.[4]
			std::stringstream stream;
			stream << "Stores V0 to V" << (int)x << " in m_Memory starting at address I";
			Logger::Log(stream.str());
		}
		break;
		case 0x65:
		{
			///FX65 	Fills V0 to VX with values from m_Memory starting at address I.[4]
			std::stringstream stream;
			stream << "Fills V0 to V" << (int)x << " with values from m_Memory starting at address I";
			Logger::Log(stream.str());
		}
		break;
		default:
		{
			std::stringstream stream;
			stream << std::hex << (int)command;
			Logger::Log(stream.str(), 11);
		}
		break;
		}
	}
	break;
	
	default:
		break;
	}
}