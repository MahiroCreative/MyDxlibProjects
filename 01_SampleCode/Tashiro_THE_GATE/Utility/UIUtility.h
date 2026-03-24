#pragma once
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include "Easing.h"
#include "FontManager.h"
#include "Application.h"

enum class EasingType;
// 移動するUIのデータ
class UIMoveData
{
public:
	// 実際に描画する座標
	int x = 0, y = 0;

private:
	// 元の座標
	int m_preX = -1, m_preY = -1;
	// 移動した後の座標
	int m_nextX = -1, m_nextY = -1;
	// 移動するか
	bool m_isMoveX = false, m_isMoveY = false;
	// 移動割合
	float m_moveRate = 0.0f;
	// 水平・垂直移動割合
	float m_angle = 0.0f;

public:
	/// <summary>
	/// データの生成
	/// </summary>
	/// <param name="x">初期X</param>
	/// <param name="y">初期Y</param>
	/// <returns>データ</returns>
	static std::shared_ptr<UIMoveData> Make(int x, int y)
	{
		// データの生成・初期化
		auto data = std::make_shared<UIMoveData>();
		data->x = x;
		data->y = y;
		data->m_preX = x;
		data->m_preY = y;
		data->m_angle = 0.0f;
		return data;
	}
	/// <summary>
	/// データ群の生成
	/// </summary>
	/// <param name="num">データ数</param>
	/// <param name="x">初期X</param>
	/// <param name="y">初期Y</param>
	/// <param name="intervalX">間隔X</param>
	/// <param name="intervalY">間隔Y</param>
	/// <returns>データ群</returns>
	static std::vector<std::shared_ptr<UIMoveData>> Make(int num, int x, int y, int intervalX, int intervalY)
	{
		std::vector<std::shared_ptr<UIMoveData>> list(num);
		for (int i = 0; i < num; ++i)
		{
			auto& data = list[i];
			data = Make(x, y);

			x += intervalX;
			y += intervalY;
		}
		return list;
	}

	/// <summary>
	/// 水平方向(X方向)に変更
	/// </summary>
	/// <param name="list">データ群</param>
	/// <param name="movePos">移動先</param>
	/// <param name="current">現在選択しているものを変える場合は現在選択している位置を入れる</param>
	/// <param name="currentMovePos">選択しているものの移動先</param>
	static void ChangeHorizontal(std::vector<std::shared_ptr<UIMoveData>>& list, int movePos, int current = -1, int currentMovePos = -1)
	{
		for (int i = 0; i < list.size(); ++i)
		{
			auto& data = list.at(i);
			if (i == current)
			{
				data->ChangeHorizontal(currentMovePos);
			}
			else
			{
				data->ChangeHorizontal(movePos);
			}
		}
	}

	/// <summary>
	/// 垂直方向(Y方向)に変更
	/// </summary>
	/// <param name="list">データ群</param>
	/// <param name="movePos">移動先</param>
	/// <param name="current">現在選択しているものを変える場合は現在選択している位置を入れる</param>
	/// <param name="currentMovePos">選択しているものの移動先</param>
	static void ChangeVertical(std::vector<std::shared_ptr<UIMoveData>>& list, int movePos, int current = -1, int currentMovePos = -1)
	{
		for (int i = 0; i < list.size(); ++i)
		{
			auto& data = list.at(i);
			if (i == current)
			{
				data->ChangeVertical(currentMovePos);
			}
			else
			{
				data->ChangeVertical(movePos);
			}
		}
	}

	/// <summary>
	/// 水平方向(X方向)に変更
	/// </summary>
	/// <param name="next">次の位置</param>
	/// <param name="pre">前回の位置(-値なら現在の値を入れる)</param>
	/// <param name="isRateInit">移動割合を初期化するか</param>
	void ChangeHorizontal(int next, int pre = -1, bool isRateInit = false)
	{
		// 値の代入
		this->m_nextX = next;
		// -値の場合は現在の位置、+の場合は引数を入れる
		if (pre < 0)	this->m_preX = this->x;
		else			this->m_preX = pre;
		// 初期化
		if (!this->m_isMoveX || isRateInit)
		{
			this->m_moveRate = 0.0f;
		}
		else
		{
			this->m_moveRate = 1.0f - this->m_moveRate;
		}
		this->m_isMoveX = true;
	}
	/// <summary>
	/// 垂直方向(Y方向)に変更
	/// </summary>
	/// <param name="next">次の位置</param>
	/// <param name="pre">前回の位置(-値なら現在の値を入れる)</param>
	/// <param name="isRateInit">移動割合を初期化するか</param>
	void ChangeVertical(int next, int pre = -1, bool isRateInit = false)
	{
		// 値の代入
		this->m_nextY = next;
		// -値の場合は現在の位置、+の場合は引数を入れる
		if (pre < 0)	this->m_preY = this->y;
		else			this->m_preY = pre;
		// 初期化
		if (!this->m_isMoveY || isRateInit)
		{
			this->m_moveRate = 0.0f;
		}
		else
		{
			this->m_moveRate = 1.0f - this->m_moveRate;
		}
		this->m_isMoveY = true;
	}

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="rateSpeed">割合スピード</param>
	/// <param name="type">イージングの種類</param>
	void Update(float rateSpeed = 0.025f, const EasingType& type = EasingType::EASE_IN)
	{
		// X, Y両方とも動かさないなら終了
		if (!m_isMoveX && !m_isMoveY) return;

		// 移動割合の更新
		bool isEnd = false;
		m_moveRate += rateSpeed;
		// 1.0fを超えたら終了・補正
		if (m_moveRate > 1.0f)
		{
			m_moveRate = 1.0f;
			isEnd = true;
		}
		// 移動する軸のみ更新処理
		if (m_isMoveX)
		{
			x = Update(m_preX, m_nextX, m_moveRate, type);
		}
		if (m_isMoveY)
		{
			y = Update(m_preY, m_nextY, m_moveRate, type);
		}
		// 終了ならば動かないように変更
		if (isEnd)
		{
			m_isMoveX = false;
			m_preX = x;
			m_isMoveY = false;
			m_preY = y;
		}
	}
	/// <summary>
	/// 水平方向に揺らす
	/// </summary>
	/// <param name="angleSpeed">回転速度(ラジアン)</param>
	/// <param name="width">揺れ幅</param>
	/// <param name="type">イージングの種類</param>
	void SwingHorizontal(float angleSpeed, int width, const EasingType& type)
	{
		// 速度追加
		m_angle += angleSpeed;
		// 位置更新
		float rate = std::abs(std::sinf(angleSpeed));
		x = Update(m_preX - width, m_preX + width, rate, type);
	}
	/// <summary>
	/// 垂直方向に揺らす
	/// </summary>
	/// <param name="angleSpeed">回転速度(ラジアン)</param>
	/// <param name="width">揺れ幅</param>
	/// <param name="type">イージングの種類</param>
	void SwingVertical(float angleSpeed, int height, const EasingType& type)
	{
		// 速度追加
		m_angle += angleSpeed;
		// 位置更新
		float rate = std::abs(std::sinf(angleSpeed));
		y = Update(m_preY - height, m_preY + height, rate, type);
	}

private:
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="pre">一つ前位置</param>
	/// <param name="next">次の位置</param>
	/// <param name="rate">割合</param>
	/// <param name="type">イージングの種類</param>
	/// <returns>位置</returns>
	int Update(int pre, int next, float rate, const EasingType& type)
	{
		if (type == EasingType::LERP)
		{
			return Easing::Lerp(pre, next, rate);
		}
		else if (type == EasingType::EASE_IN)
		{
			return Easing::EaseIn(pre, next, rate);
		}
		else if (type == EasingType::EASE_OUT)
		{
			return Easing::EaseOut(pre, next, rate);
		}
		else if (type == EasingType::EASE_IN_OUT)
		{
			return Easing::EaseInOut(pre, next, rate);
		}

		assert(false);
		return next;
	}
};

class UIUtility
{
public:
	enum class DrawStrType
	{
		NORMAL,
		WAVE,
		SHAKE,
	};

public:
	/// <summary>
	/// 文字列の中心が引数として渡した値になる開始値を計算する
	/// </summary>
	/// <param name="center">描画中心値</param>
	/// <param name="str">文字列</param>
	/// <param name="fontSize">フォントサイズ</param>
	/// <param name="rate">フォントサイズの調整係数(def = 0.4f)</param>
	/// <returns>開始値</returns>
	static int GetSubCenterFromLen(int center, const std::wstring& str, int fontSize, float rate = 0.4f)
	{
		auto len = str.length();
		return center - static_cast<int>(len * fontSize * rate);
	}

	/// <summary>
	/// 文字をウェーブしながら描画
	/// </summary>
	/// <param name="x">X中心</param>
	/// <param name="y">Y中心</param>
	/// <param name="color">文字カラー</param>
	/// <param name="str">文字列</param>
	/// <param name="fontSize">フォントサイズ</param>
	/// <param name="count">カウント</param>
	/// <param name="upSize">上昇量</param>
	/// <param name="rate">フォントサイズ調整係数</param>
	static void DrawWaveStr(int x, int y, unsigned int color, const std::wstring& str, int fontSize, float count, int upSize, float rate = 0.4f)
	{
		auto fontH = FontManager::GetInstance().GetHandle(fontSize);
		y -= static_cast<int>(fontSize * 0.5f);
		int drawX = GetSubCenterFromLen(x, str, fontSize);
		int addX = static_cast<int>(fontSize * rate * 2);
		float addCount = static_cast<float>(180 / str.size());
		for (const auto& c : str)
		{
			int drawY = y + static_cast<int>(std::max<float>(0, std::cosf(count * Game::DEG_2_RAD)) * upSize * -1);
			DrawFormatStringToHandle(drawX, drawY, color, fontH, L"%c", c);
			drawX += addX;
			count -= addCount;
		}
	}

	/// <summary>
	/// 文字をぶるぶるさせながら描画
	/// </summary>
	/// <param name="x">X中心</param>
	/// <param name="y">Y中心</param>
	/// <param name="color">文字カラー</param>
	/// <param name="str">文字列</param>
	/// <param name="fontSize">フォントサイズ</param>
	/// <param name="shakeSize">揺れる大きさ</param>
	/// <param name="rate">フォントサイズ調整係数</param>
	static void DrawShakeStr(int x, int y, unsigned int color, const std::wstring& str, int fontSize, int shakeSize, float rate = 0.4f)
	{
		auto fontH = FontManager::GetInstance().GetHandle(fontSize);
		y -= static_cast<int>(fontSize * 0.5f);
		int drawX = GetSubCenterFromLen(x, str, fontSize);
		int addX = static_cast<int>(fontSize * rate * 2);
		float addCount = 180.0f / str.size();
		for (const auto& c : str)
		{
			auto& app = Application::GetInstance();
			int randX = app.GetRand(-shakeSize, shakeSize);
			int randY = app.GetRand(-shakeSize, shakeSize);
			DrawFormatStringToHandle(drawX + randX, y + randY, color, fontH, L"%c", c);
			drawX += addX;
		}
	}

	/// <summary>
	/// 文字列とフレームを同時に描画
	/// </summary>
	/// <param name="data">UIMoveData</param>
	/// <param name="size">画像サイズ</param>
	/// <param name="angle">画像回転</param>
	/// <param name="fontSize">フォントサイズ</param>
	/// <param name="current">現在の選択位置</param>
	/// <param name="strs">文字列群</param>
	/// <param name="selectFileH">選択時の画像ハンドル</param>
	/// <param name="notSelectFileH">未選択時の画像ハンドル</param>
	/// <param name="selectColor">選択時の文字カラー</param>
	/// <param name="notSelectColor">非選択時の文字カラー</param>
	/// <param name="type">選択文字描画種類(デフォルトノーマル)</param>
	/// <param name="size">ウェーブの大きさや揺れる大きさなどを入れる</param>
	/// <param name="count">ウェーブの間隔などを入れる</param>
	/// <param name="rate">フォントサイズ調整係数</param>
	static void DrawFrameAndStr(const std::vector<std::shared_ptr<UIMoveData>>& list, float graphSize, float angle, int fontSize, int current, const std::vector<std::wstring>& strs, int selectFileH, int notSelectFileH, unsigned int selectColor, unsigned int notSelectColor, DrawStrType type = DrawStrType::NORMAL, int size = 0, float count = -1, float rate = 0.4f)
	{
		int fontH = FontManager::GetInstance().GetHandle(fontSize);
		int drawStrAdd = static_cast<int>(fontSize * 0.5f);
		int handle = -1;
		unsigned int color = -1;
		for (int i = 0; i < strs.size(); ++i)
		{
			DrawStrType nowType;
			if (i == current)
			{
				handle = selectFileH;
				color = selectColor;
				nowType = type;
			}
			else
			{
				handle = notSelectFileH;
				color = notSelectColor;
				nowType = DrawStrType::NORMAL;
			}
			const auto& data = *list[i];
			const auto& str = strs.at(i);
			DrawRotaGraphFast(data.x, data.y, graphSize, angle, handle, true);
			if (nowType == DrawStrType::NORMAL)
			{

				int drawStrX = GetSubCenterFromLen(data.x, str, fontSize);
				DrawStringToHandle(drawStrX, data.y - drawStrAdd, str.c_str(), color, fontH);
			}
			else if (nowType == DrawStrType::WAVE)
			{
				DrawWaveStr(data.x, data.y, color, str, fontSize, count, size, rate);
			}
			else if (nowType == DrawStrType::SHAKE)
			{
				DrawShakeStr(data.x, data.y, color, str, fontSize, size, rate);
			}
			
		}
	}

	/// <summary>
	/// 文字列とフレームを同時に描画
	/// </summary>
	/// <param name="x">Xスタート中心座標</param>
	/// <param name="y">Yスタート中心座標</param>
	/// <param name="intervalX">X方向の幅</param>
	/// <param name="intervalY">Y方向の幅</param>
	/// <param name="size">画像サイズ</param>
	/// <param name="angle">画像回転</param>
	/// <param name="fontSize">フォントサイズ</param>
	/// <param name="current">現在の選択位置</param>
	/// <param name="strs">文字列群</param>
	/// <param name="selectFileH">選択時の画像ハンドル</param>
	/// <param name="notSelectFileH">非選択時の画像ハンドル</param>
	/// <param name="selectColor">選択時の文字カラー</param>
	/// <param name="notSelectColor">非選択時の文字カラー</param>
	/// <param name="type">選択文字描画種類(デフォルトノーマル)</param>
	/// <param name="size">ウェーブの大きさや揺れる大きさなどを入れる</param>
	/// <param name="count">ウェーブの間隔などを入れる</param>
	/// <param name="rate">フォントサイズ調整係数</param>
	static void DrawFrameAndStr(int x, int y, int intervalX, int intervalY, float graphSize, float angle, int fontSize, int current, const std::vector<std::wstring>& strs, int selectFileH, int notSelectFileH, unsigned int selectColor, unsigned int notSelectColor, DrawStrType type = DrawStrType::NORMAL, int size = 0, float count = -1, float rate = 0.4f)
	{
		int fontH = FontManager::GetInstance().GetHandle(fontSize);
		int drawX = x;
		int drawY = y;
		int drawStrAddY = static_cast<int>(fontSize * 0.5f);
		int handle = -1;
		unsigned int color = -1;
		for (int i = 0; i < strs.size(); ++i)
		{
			DrawStrType nowType;
			if (i == current)
			{
				handle = selectFileH;
				color = selectColor;
				nowType = type;
			}
			else
			{
				handle = notSelectFileH;
				color = notSelectColor;
				nowType = DrawStrType::NORMAL;
			}
			DrawRotaGraphFast(drawX, drawY, graphSize, angle, handle, true);
			const auto& str = strs.at(i);
			if (nowType == DrawStrType::NORMAL)
			{
				int drawStrX = GetSubCenterFromLen(drawX, str, fontSize);
				DrawStringToHandle(drawStrX, drawY - drawStrAddY, str.c_str(), color, fontH);
			}
			else if (nowType == DrawStrType::WAVE)
			{
				DrawWaveStr(drawX, drawY, color, str, fontSize, count, size, rate);
			}
			else if (nowType == DrawStrType::SHAKE)
			{
				DrawShakeStr(drawX, drawY, color, str, fontSize, size, rate);
			}
			

			drawX += intervalX;
			drawY += intervalY;
		}
	}
};

