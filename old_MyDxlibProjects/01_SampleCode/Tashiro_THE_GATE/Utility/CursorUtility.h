#pragma once
#include "Input.h"
#include "CommandList.h"

class CursorUtility
{
public:
	static bool CursorUp(int& index, int max, const char* command = Command::BTN_UP)
	{
		auto& input = Input::GetInstance();

		if (input.IsTriggerd(command))
		{
			index = (index - 1 + max) % max;
			return true;
		}
		return false;
	}
	static bool CursorDown(int& index, int max, const char* command = Command::BTN_DOWN)
	{
		auto& input = Input::GetInstance();

		if (input.IsTriggerd(command))
		{
			index = (index + 1) % max;
			return true;
		}
		return false;
	}
	template<class T>
	static bool CursorUp(int& index, T max, const char* command = Command::BTN_UP)
	{
		return CursorUp(index, static_cast<int>(max), command);
	}
	template<class T>
	static bool CursorDown(int& index, T max, const char* command = Command::BTN_DOWN)
	{
		return CursorDown(index, static_cast<int>(max), command);
	}
};
