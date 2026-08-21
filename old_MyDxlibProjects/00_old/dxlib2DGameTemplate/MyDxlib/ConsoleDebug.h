#pragma once
//STL.
#include <iostream>
#include <consoleapi.h>

class ConsoleDebug
{
public:
	void Init();
	void Close();
	void Update();

private:
	FILE* out = 0;
	FILE* in = 0;
};

void ConsoleDebug::Init()
{
	AllocConsole();// コンソールを開く
	freopen_s(&out, "CON", "w", stdout); // stdout
	freopen_s(&in, "CON", "r", stdin);   // stdin
}
void ConsoleDebug::Close()
{
	fclose(out);
	fclose(in);
	FreeConsole();
}
void ConsoleDebug::Update()
{

}
