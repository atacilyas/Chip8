#include "Chip8.h"
#include <windows.h>
#include <GLFW/glfw3.h>
#include "Logger.h"
#include <sstream>
#include <thread>

//layout "x123qweasdzc4rfv"
const int Chip8::KeyBoardLayout[AMOUNT_OF_KEYS] =
{
	GLFW_KEY_X, /*0*/	GLFW_KEY_1, /*1*/	GLFW_KEY_2, /*2*/	GLFW_KEY_3, /*3*/
	GLFW_KEY_Q, /*4*/	GLFW_KEY_W, /*5*/	GLFW_KEY_E, /*6*/	GLFW_KEY_A, /*7*/
	GLFW_KEY_S, /*8*/	GLFW_KEY_D, /*9*/	GLFW_KEY_Z, /*A*/	GLFW_KEY_C, /*B*/
	GLFW_KEY_4, /*C*/	GLFW_KEY_R, /*D*/	GLFW_KEY_F, /*E*/	GLFW_KEY_V  /*F*/
};

const unsigned char Chip8::chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, /*0*/	0x20, 0x60, 0x20, 0x20, 0x70, /*1*/
	0xF0, 0x10, 0xF0, 0x80, 0xF0, /*2*/	0xF0, 0x10, 0xF0, 0x10, 0xF0, /*3*/

	0x90, 0x90, 0xF0, 0x10, 0x10, /*4*/	0xF0, 0x80, 0xF0, 0x10, 0xF0, /*5*/
	0xF0, 0x80, 0xF0, 0x90, 0xF0, /*6*/	0xF0, 0x10, 0x20, 0x40, 0x40, /*7*/

	0xF0, 0x90, 0xF0, 0x90, 0xF0, /*8*/	0xF0, 0x90, 0xF0, 0x10, 0xF0, /*9*/
	0xF0, 0x90, 0xF0, 0x90, 0x90, /*A*/	0xE0, 0x90, 0xE0, 0x90, 0xE0, /*B*/

	0xF0, 0x80, 0x80, 0x80, 0xF0, /*C*/	0xE0, 0x90, 0x90, 0x90, 0xE0, /*D*/
	0xF0, 0x80, 0xF0, 0x80, 0xF0, /*E*/	0xF0, 0x80, 0xF0, 0x80, 0x80  /*F*/
};

bool Chip8::LoadGame(string path)
{
	m_Path = path;

	//reset memory
	m_Memory.empty();
	m_Stack.empty();

	//reset timers
	m_SoundTimer = 0;
	m_DelayTimer = 0;

	//set the program counter at the start of the program 0x200
	m_ProgramCounter = PROGRAM_STARTPOS;

	m_IndexRegister = 0;

	//load fontset
	for (size_t i = 0; i < 80; i++)
	{
		m_Memory[i] = chip8_fontset[i];
	}

	//clear the entire screen (hires included)
	for (size_t i = 0; i < 64 * 64; i++)
	{
		m_ScreenBuffer[i] = 0;
	}

	//clear the registers
	for (size_t i = 0; i < 16; i++)
	{
		m_Registers[i] = 0;
	}

	//open file in binary read mode
	ifstream t;
	t.open(path.c_str(),ifstream::in | ifstream::binary);

	//start in non hires mode
	hiresmode = false;

	//disable logging at the start
	m_Log = false;

	//check if file exists
	m_GameLoaded = t.good();
	if (m_GameLoaded == false)
	{
		return false;
	}
	cout << endl << endl;

	//counter
	int i = 0;
	do
	{
		int a = t.get();
		if (a != 0xffffffff)
		{
			//m_Memory needs to be loaded in at location 200
			m_Memory[i + PROGRAM_STARTPOS] = a & 0xFF;

			//purely to write it pretty in command window
			if (a < 10)
			{
				//numbers lower than 10 need to show the 0
				cout << std::hex << "0" << a;
			}
			else
			{
				cout << std::hex << a;
			}
			if (i % 2 == 1)
			{
				//after every opcodepart place a space for readability
				cout << " ";
			}
			if ((i + 2) % (6*2) == 1)
			{
				//devide in mulitple lines
				cout << endl;
			}
			++i;
		}
	} while (!t.eof());
	//filesize
	m_Size = i;
	cout << endl << endl;
	return m_GameLoaded;
}

bool Chip8::RunCommand(const U16 command)
{
	//F000 remove all but the first opcode part
	int firstopcodepart = (command >> 12) & 0xF;
	switch (firstopcodepart)
	{
	case 0x0:
	{
		int lastOpcodePart = (command)& 0xFFF;
		if (lastOpcodePart == 0x0E0)
		{
			///00E0 	Clears the screen.
			for (size_t i = 0; i < 64 * 32; i++)
			{
				m_ScreenBuffer[i] = 0;
			}
		}
		else if (lastOpcodePart == 0x0EE)
		{
			///00EE 	Returns from a subroutine.
			m_ProgramCounter = m_Stack.back();
			m_Stack.pop_back();
		}
		else if (lastOpcodePart == 0x230)
		{
			///hires command to cler the screen
			for (size_t i = 0; i < 64 * 64; i++)
			{
				m_ScreenBuffer[i] = 0;
			}
		}
		else
		{return false;}
	}break;

	case 0x1:
	{
		///1NNN 	Jumps to address NNN.
		m_ProgramCounter = command & 0x0FFF;
		m_ProgramCounter -= 2; //dont do the automatic move forward
	}break;

	case 0x2:
	{
		///2NNN 	Calls subroutine at NNN.
		m_Stack.push_back(m_ProgramCounter);
		m_ProgramCounter = command & 0x0FFF;
		m_ProgramCounter -= 2;
	}break;

	case 0x3:
	{
		///3XNN 	Skips the next instruction if VX equals NN.
		U8 x = (command >> 8) & 0x0F;
		U16 nn = (command & 0x00FF);
		if ((int)m_Registers[x] == (int)nn)
		{
			m_ProgramCounter += 2; //jump 1
		}
	}break;

	case 0x4:
	{
		///4XNN 	Skips the next instruction if VX doesn't equal NN.
		U8 x = (command >> 8) & 0x0F;
		U16 nn = (command & 0x00FF);
		if ((int)m_Registers[x] != (int)nn)
		{
			m_ProgramCounter += 2;
		}
	}break;

	case 0x5:
	{
		///5XY0 	Skips the next instruction if VX equals VY.
		U8 x = (command >> 8) & 0x0F;
		U8 y = (command >> 4) & 0x0F;
		if ((int)m_Registers[x] == (int)m_Registers[y])
		{
			m_ProgramCounter += 2;
		}
	}break;

	case 0x6:
	{
		///6XNN 	Sets VX to NN.
		U8 x = (command >> 8) & 0x0F;
		U8 nn = (command)& 0x00FF;
		m_Registers[x] = nn;
	}break;

	case 0x7:
	{
		///7XNN 	Adds NN to VX.
		U8 x = (command >> 8) & 0x0F;
		U8 nn = (command)& 0x00FF;
		m_Registers[x] += nn;
	}break;

	case 0x8:
	{
		U8 lastOpcodePart = (command & 0x000F);
		U8 x = (command >> 8) & 0xF;
		U8 y = (command >> 4) & 0xF;

		switch (lastOpcodePart)
		{
			case 0x0:
			{
				///8XY0 	Sets VX to the value of VY.
				m_Registers[x] = m_Registers[y];
			}break;

			case 0x1:
			{
				///8XY1 	Sets VX to VX or VY.
				m_Registers[x] = m_Registers[x] | m_Registers[y];
			}break;

			case 0x2:
			{
				///8XY2 	Sets VX to VX and VY.
				m_Registers[x] = m_Registers[x] & m_Registers[y];
			}break;

			case 0x3:
			{
				///8XY3 	Sets VX to VX xor VY.
				m_Registers[x] = m_Registers[x] ^ m_Registers[y];
			}break;

			case 0x4:
			{
				///8XY4 	Adds VY to VX.VF is set to 1 when there's a carry, and to 0 when there isn't.
				int result = m_Registers[x] + m_Registers[y];
				m_Registers[x] = (U8)result;
				if (result > 255)
					m_Registers[0xF] = 1;
				else
					m_Registers[0xF] = 0;
			}break;

			case 0x5:
			{
				///8XY5 	VY is subtracted from VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
				if (m_Registers[x] <  m_Registers[y])
					m_Registers[0xF] = 0;
				else										  
					m_Registers[0xF] = 1;

				m_Registers[x] -= m_Registers[y];
			}break;

			case 0x6:
			{
				///8XY6 	Shifts VX right by one.VF is set to the value of the least significant bit of VX before the shift.[2]
				m_Registers[0xF] = m_Registers[x] & 0xF;
				m_Registers[x] = m_Registers[x] >> 1;
			} break;

			case 0x7:
			{
				///8XY7 	Sets VX to VY minus VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
				if (m_Registers[y] <  m_Registers[x])
					m_Registers[0xF] = 0;
				else
					m_Registers[0xF] = 1;
				m_Registers[x] -= m_Registers[y];
			} break;

			case 0xE:
			{
				///8XYE 	Shifts VX left by one.VF is set to the value of the most significant bit of VX before the shift.[2]
				m_Registers[0xF] = (m_Registers[x]  >> 7) & 0xF;
				m_Registers[x] = m_Registers[x] << 1;
			} break;

		default: return false;	break;
		}
	}break;

	case 0x9:
	{
		///9XY0 	Skips the next instruction if VX doesn't equal VY.
		U8 x = (command >> 8) & 0xF;
		U8 y = (command >> 4) & 0xF;

		if ((int)m_Registers[x] != (int)m_Registers[y])
		{
			m_ProgramCounter += 2;
		}
	}break;

	case 0xA:
	{
		///ANNN 	Sets I to the address NNN.
		m_IndexRegister = command & 0x0FFF;
	}break;

	case 0xB:
	{
		///BNNN 	Jumps to the address NNN plus V0.
		m_ProgramCounter = (command & 0x0FFF) + m_Registers[0];
		m_ProgramCounter -= 2;
	}break;

	case 0xC:
	{
		///CXNN 	Sets VX to the result of a bitwise and operation on a random number and NN.
		U8 x = (command >> 8) & 0x0F;
		U8 nn = (command & 0x00FF);
		m_Registers[x] = (rand()%256) & nn;
	}break;

	case 0xD:
	{
		///DXYN 	Sprites stored in m_Memory at location in index register (I), 8bits wide. Wraps around the screen.
		///If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
		///All drawing is XOR drawing (i.e. it toggles the screen pixels).
		///Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn. If N is greater than 1,
		///second line continues at position VX, VY+1, and so on.
		//get postition and height of the sprite
		U8 x = m_Registers[(command >> 8) & 0x0F];
		U8 y = m_Registers[(command >> 4) & 0x00F];
		int rows = command & 0x000F;
		U16 pixel;

		//reset the drawflag
		m_Registers[0xF] = 0;
		for (int yline = 0; yline < rows; ++yline)
		{
			//Sprites stored in m_Memory at location in index register (I), 8bits wide
			pixel = m_Memory[m_IndexRegister + yline];
			for (int xline = 0; xline < 8; ++xline)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					int temp = (((x + xline)) + (((y + yline)) * 64));
					bool result = false;
					if (hiresmode)
					{
						result = (temp %= m_ScreenBuffer.size());
					}
					else
					{
						result = (temp %= m_ScreenBuffer.size()/2);
					}
					if (result)
					{
		 // If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero
						if (m_ScreenBuffer[temp] == 1)
							m_Registers[0xF] = 1;
						m_ScreenBuffer[temp] ^= 1;
					}
				}
			}
		}
	}break;

	case 0xE:
	{
		int x = (command >> 8) & 0xF;
		if ((command & 0xF0FF) == 0xE09E)
		{
			///EX9E 	Skips the next instruction if the key stored in VX is pressed.
			if (glfwGetKey(m_WindowPtr, KeyBoardLayout[(int)m_Registers[x]]))
			{
				m_ProgramCounter += 2;
			}
		}
		else if ((command & 0xF0FF) == 0xE0A1)
		{
			///EXA1 	Skips the next instruction if the key stored in VX isn't pressed.
			if (!glfwGetKey(m_WindowPtr, KeyBoardLayout[(int)m_Registers[x]]))
			{
				m_ProgramCounter += 2;
			}
		}
		else
		{
			return false;
		}
	}break;

	case 0xF:
	{
		U8 lastopcodepart = command & 0xFF;
		U8 x = (command >> 8) & 0xF;

		switch (lastopcodepart)
		{
		case 0x07:
		{
			///FX07 	Sets VX to the value of the delay timer.
			m_Registers[x] = m_DelayTimer;
		}break;
		case 0x0A:
		{
			///FX0A 	A key press is awaited, and then stored in VX.
			m_ProgramCounter -= 2;
			for (size_t i = 0; i < AMOUNT_OF_KEYS; i++)
			{
				if (glfwGetKey(m_WindowPtr, KeyBoardLayout[i]))
				{
					m_Registers[x] = i;
					m_ProgramCounter += 2;
					break;
				}
			}
		}break;
		case 0x15:
		{
			///FX15 	Sets the delay timer to VX.
			m_DelayTimer = m_Registers[x];
		}break;
		case 0x18:
		{
			///FX18 	Sets the sound timer to VX.
			m_SoundTimer = m_Registers[x];
		}break;
		case 0x1E:
		{
			///FX1E 	Adds VX to I.[3]
			m_IndexRegister += m_Registers[x];
		}break;
		case 0x29:
		{
			///FX29 	Sets I to the location of the sprite for the character in VX.Characters 0 - F(in hexadecimal) are represented by a 4x5 font.
			m_IndexRegister = (m_Registers[x] * 5);
		}break;
		case 0x33:
		{
			///FX33 	Stores the Binary - coded decimal representation of VX,
			///with the most significant of three digits at the address in I,
			///the middle digit at I plus 1, and the least significant digit at I plus 2.
			///(In other words, take the decimal representation of VX,
			///place the hundreds digit in m_Memory at location in I,
			///the tens digit at location I + 1, and the ones digit at location I + 2.)
			m_Memory[m_IndexRegister] = m_Registers[x] / 100; //honderttallen
			m_Memory[m_IndexRegister + 1] =
				(m_Registers[x] - m_Memory[m_IndexRegister] * 100) / 10; //tientallen
			m_Memory[m_IndexRegister + 2] =
				(m_Registers[x] - m_Memory[m_IndexRegister] * 100 - m_Memory[m_IndexRegister + 1] * 10); //eenheden
		}break;
		case 0x55:
		{
			///FX55 	Stores V0 to VX in m_Memory starting at address I.[4]
			for (size_t i = 0; i <= (size_t)x; i++)
			{
				m_Memory[m_IndexRegister + i] = m_Registers[i];
			}
			m_IndexRegister += x + 1;
		}break;
		case 0x65:
		{
			///FX65 	Fills V0 to VX with values from m_Memory starting at address I.[4]
			for (size_t i = 0; i <= (size_t)x; i++)
			{
				m_Registers[i] = m_Memory[m_IndexRegister + i];
			}

			m_IndexRegister += x + 1;
		}break;
		default: return false;	break;
		}
	}break;

	default: return false; break;
	}

	return true;
}

bool Chip8::GameLoop()
{
	if (m_GameLoaded)
	{
		//get the opcode
		U16 opcode = m_Memory[m_ProgramCounter] << 8 | m_Memory[m_ProgramCounter + 1];
		//check if were dealing with a hires game
		if ((m_ProgramCounter == 0x200) && (opcode == 0x1260))
		{
			hiresmode = true; // Init 64x64 hires mode
			opcode = 0x12C0;  // Make the interperter jump to address 0x2c0
		}

		if (glfwGetKey(m_WindowPtr, GLFW_KEY_O))
		{
			LoadGame(m_Path); // reset the game
		}

		if (glfwGetKey(m_WindowPtr, GLFW_KEY_P))
		{
			m_Log = !m_Log; // enable/disable logging
		}

		if (m_Log)
		{
			Logger::getInstance()->LogOpcode(opcode);
		}
		//run the opcode
		if (!RunCommand(opcode))
		{
			return false;
		}

		//move to next position in m_Memory
		m_ProgramCounter += 2;

		//count down delay timer
		if (m_DelayTimer > 0)
		{
			--m_DelayTimer;
		}

		//count down sound timer
		if (m_SoundTimer > 0)
		{
			if (m_SoundTimer == 1)
			{
				BeepPlay();
			}
			--m_SoundTimer;
		}

		//if program goes outside of the usable memory
		if (m_ProgramCounter >= m_Size + PROGRAM_STARTPOS)
		{
			return false;
		}
	}
	return true;
}

void Chip8::Draw()
{
	if (m_GameLoaded)
	{
		if (hiresmode)
		{
			U8 pixelbuffer[64 * 64 * 3];
			for (size_t i = 0; i < 64 * 64; i++)
			{
				U16 j = m_ScreenBuffer[i];
				pixelbuffer[(i * 3) + 0] = j * 255;
				pixelbuffer[(i * 3) + 1] = j * 255;
				pixelbuffer[(i * 3) + 2] = j * 255;
			}			//do hires drawing
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelbuffer);
		}
		else
		{
			U8 pixelbuffer[64 * 32 * 3];
			for (size_t i = 0; i < 64 * 32; i++)
			{
				U16 j = m_ScreenBuffer[i];
				pixelbuffer[(i * 3) + 0] = j * 255;
				pixelbuffer[(i * 3) + 1] = j * 255;
				pixelbuffer[(i * 3) + 2] = j * 255;
			}			//do lowres drawing
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelbuffer);
		}
	}
}

void Chip8::BeepPlay()
{
	//play a random sound
	Beep(rand() % 800 + 500, 100);
}