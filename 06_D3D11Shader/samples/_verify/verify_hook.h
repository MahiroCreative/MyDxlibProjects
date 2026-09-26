// サンプルの検証用。ビルド時に /FI で差し込み、サンプルのソースを書き換えずに次のことをさせる。
//   - ウィンドウモード・裏でも止まらない設定で、VERIFY_D3D の Direct3D を使って起動する
//   - VERIFY_FRAME 回目の ScreenFlip の直前(または WaitKey)で画面を capture.png に保存して終了する
// 公式の Direct3D 9 版(原本)と Direct3D 11 移植版の両方に同じものを差し込み、同じフレームの絵を比べる。
#pragma once
#include "DxLib.h"
#include <cstdlib>

#ifndef VERIFY_D3D
#define VERIFY_D3D DX_DIRECT3D_11
#endif
#ifndef VERIFY_FRAME
#define VERIFY_FRAME 30
#endif

namespace verify_hook
{
	inline int g_frame = 0;

	inline void CaptureAndExit()
	{
		int w, h;
		GetDrawScreenSize(&w, &h);
		SaveDrawScreenToPNG(0, 0, w, h, "capture.png");
		DxLib_End();
		std::exit(0);
	}

	inline int Init()
	{
		ChangeWindowMode(TRUE);
		SetAlwaysRunFlag(TRUE);
		SetUseDirect3DVersion(VERIFY_D3D);
#ifdef VERIFY_COLOR_BIT_DEPTH
		SetGraphMode(640, 480, VERIFY_COLOR_BIT_DEPTH);	// 調査用: 画面の色のビット数を変える
#endif
		const int r = DxLib_Init();
		SRand(12345);	// GetRand を使うサンプル(NPC の動きなど)が、Direct3D 9 版と 11 版で同じ動きになるように種を固定する
		if (r == 0 && GetUseDirect3DVersion() != VERIFY_D3D)
		{
			DxLib_End();
			std::exit(3);	// 指定の Direct3D で起動できなかった
		}
		return r;
	}

	inline int Flip()
	{
		if (++g_frame >= VERIFY_FRAME)
		{
			CaptureAndExit();
		}
		return ScreenFlip();
	}

	inline int Wait()
	{
		CaptureAndExit();
		return 0;
	}
}

// サンプル側の呼び出しを差し替える(関数の宣言は上の DxLib.h で済んでいる)
#define DxLib_Init() verify_hook::Init()
#define ScreenFlip() verify_hook::Flip()
#define WaitKey() verify_hook::Wait()
#define ChangeWindowMode(flag) 0
#define SetUseDirect3DVersion(version) 0
