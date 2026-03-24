#include "DxLib.h"
#include "Application.h"
#include <cassert>
#include <algorithm>
#include <array>
#include<windows.h>

namespace MyEngine
{
	int DrawGraph(float x, float y, int handle, int psH, int secondH = -1, int thirdH = -1, int forthH = -1, bool isTrans = false)
	{
		int w, h;
		GetGraphSize(handle, &w, &h);

		std::array<VERTEX2DSHADER, 4> vertices;

		for (auto& v : vertices)
		{
			v.dif = GetColorU8(255, 255, 255, 255);
			v.spc = GetColorU8(0, 0, 0, 0);
			v.rhw = 1.0f;
			v.su = 0.f;
			v.sv = 0.f;
		}

		//矩形になるように並べていく
		//Zの順番になるように頂点を並べていく
		//左上
		vertices[0].pos = VGet(x, y, 0.0f);
		vertices[0].u = 0.0f;
		vertices[0].v = 0.0f;
		//右上
		vertices[1].pos = VGet(x + w, y, 0.0f);
		vertices[1].u = 1.0f;
		vertices[1].v = 0.0f;
		//左下
		vertices[2].pos = VGet(x, y + h, 0.0f);
		vertices[2].u = 0.0f;
		vertices[2].v = 1.0f;
		//右下
		vertices[3].pos = VGet(x + w, y + h, 0.0f);
		vertices[3].u = 1.0f;
		vertices[3].v = 1.0f;

		if (isTrans)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		}

		//今からの描画に使う画像を設定する
		SetUseTextureToShader(0, handle);
		if (secondH != -1)
		{
			SetUseTextureToShader(1, secondH);
		}
		if (thirdH != -1)
		{
			SetUseTextureToShader(2, thirdH);
		}
		if (forthH != -1)
		{
			SetUseTextureToShader(3, forthH);
		}

		//今から使うシェーダーを設定
		SetUsePixelShader(psH);

		std::array<uint16_t, 6> indces;
		indces = { 0,1,3,0,3,2 };

		return DrawPrimitiveIndexed2DToShader(vertices.data(),			//アドレス
			static_cast<int>(vertices.size()),			//頂点数
			indces.data(),
			static_cast<int>(indces.size()),
			DX_PRIMTYPE_TRIANGLELIST);//トポロジ－
	}
}

#include "Quaternion.h"

// メッセージ処理用関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return (0L);
}


// プログラムは WinMain から始まります
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) 
{
	//Quaternion rot;
	//Quaternion rot1 = AngleAxis(45, Vec3::Up());
	//Quaternion rot2 = AngleAxis(30, Vec3::Up());
	//Quaternion rot3 = AngleAxis(0, Vec3::Up());

	//rot = rot1 * rot;
	//rot = rot2 * rot;
	//rot = rot3 * rot;

	int pos, pos_add;
	LONGLONG time;
	LONGLONG temp_time;
	MSG msg;
	HWND hWnd[1];
	WNDCLASS myProg;

	// ウインドウの作成
	myProg.style = CS_HREDRAW | CS_VREDRAW;
	myProg.lpfnWndProc = WndProc;
	myProg.cbClsExtra = 0;
	myProg.cbWndExtra = 0;
	myProg.hInstance = hInstance;
	myProg.hIcon = NULL;
	myProg.hCursor = LoadCursor(NULL, IDC_ARROW);
	myProg.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	myProg.lpszMenuName = NULL;
	myProg.lpszClassName = "ModelWindow";
	if (!RegisterClass(&myProg))
	{
		return FALSE;
	}
	hWnd[0] = CreateWindow(
		myProg.lpszClassName,
		"ウインドウ１",
		WS_OVERLAPPEDWINDOW,
		100, 100, 1600, 900,
		NULL, NULL, hInstance, NULL
	);
	ShowWindow(hWnd[0], 5);
	UpdateWindow(hWnd[0]);

	Application& application = Application::GetInstance();

	if (!application.Init())
	{
		return -1;// このアプリは不正終了しました。
	}
	application.Run(hWnd[0]);



	return 0;
}