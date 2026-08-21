#pragma once
#include "MyDxlib.h"
#include <string>

/// <summary>
/// シンプルなテキストオブジェクトを作るクラス
/// フォントとフォントサイズはDxlibデフォルトのものを使用。
/// 文字・色・位置・拡大縮小・回転ができる。
/// </summary>
class SimpleText :public _baseGameObject2D
{
public:
	/* メンバ変数 */
	// 位置・倍率・回転率
	Transform2D Transform;
	//表示するかしないか
	bool Visible = true;

	/* メンバ関数 */
	// コンストラクタ
	SimpleText()
		: Transform(), _text(), _rotCenter(), _color()
	{
		// 初期化
		Init();
	}

	// 初期化
	void Init() override
	{
		// 初期位置
		Transform.Position = Vector2(100.0f, 100.0f);

		// 初期文字
		_text = "null";

		// 初期色
		_color = GetColor(255, 255, 255); // 白色

		// 回転の中心座標（左上）
		_rotCenter = Vector2(0,0);
	}
	// 初期化(オーバーロード)
	void Init(std::string text)
	{
		// 初期化
		Init();

		// 初期文字の更新
		_text = text;
	}
	// 初期化(オーバーロード)
	void Init(std::string text,Vector2 position)
	{
		// 初期化
		Init();

		// 初期文字の更新
		_text = text;

		// 初期位置の更新
		Transform.Position = position;
	}

	// 更新
	void Update() override
	{
		// 更新処理（必要に応じて実装）
	}

	// 描画
	void Draw() override
	{
		if (Visible)
		{
			// 文字列を回転・拡大縮小して描画
			DrawRotaString(
				static_cast<int>(Transform.Position.X),       // 描画位置X
				static_cast<int>(Transform.Position.Y),       // 描画位置Y
				Transform.Scale.X,                            // 拡大率X
				Transform.Scale.Y,                            // 拡大率Y
				_rotCenter.X,                                 // 回転の中心X
				_rotCenter.Y,                                 // 回転の中心Y
				Transform.Rotation,                           // 回転角度（ラジアン）
				_color,                                       // 文字色
				0,                                            // 縁取り色（不要なので0）
				FALSE,                                        // 縦書きフラグ（横書きなのでFALSE）
				_text.c_str()                                 // 描画する文字列
			);
		}

	}

	//文字の変更
	void SetText(const std::string& text)
	{
		_text = text;
	}

	//色の変更
	void SetColor(unsigned int color)
	{
		_color = color;
	}

	//回転の中心座標の変更
	//(Dxlibの機能で文字列の中心を出そうとしたがうまくいかなった)
	void SetRotCenter(const Vector2& rotCenter)
	{
		_rotCenter = rotCenter;
	}

private:
	// 文字列
	std::string _text;
	// 回転の中心座標
	Vector2 _rotCenter;
	// 色
	unsigned int _color;
};