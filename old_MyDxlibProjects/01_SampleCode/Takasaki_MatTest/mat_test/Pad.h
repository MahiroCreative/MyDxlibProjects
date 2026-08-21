#pragma once
namespace Pad
{
	// パッドの更新処理 1フレームに1回行う
	void Update();

	// 押されているかを取得
	bool IsPress(int key);
	// 押した瞬間を取得
	bool IsTrigger(int key);
	// 離した瞬間を取得
	bool IsRelase(int key);
	//スティック入力

	// リピート処理 (これは今後消すor変更する予定あり)
	bool IsRepeat(int key, int& frame, int RepeatFrame);
}