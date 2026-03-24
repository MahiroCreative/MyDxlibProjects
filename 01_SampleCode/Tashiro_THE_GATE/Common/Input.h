#pragma once
#include <unordered_map>
#include <string>
#include "Geometry/Vec2.h"

struct TriggerData
{
	// 左トリガー
	float LT = 0.0f;
	// 右トリガー
	float RT = 0.0f;
	// 左スティック
	Vec2 LStick = {};
	// 右スティック
	Vec2 RStick = {};
};

using InputTable_t = std::unordered_map<std::string, int>;
using InputData_t = int;
using InputTriggerData_t = TriggerData;

class Input
{
private :
	Input();

	Input(const Input& input) = delete;
	void operator=(const Input& input) = delete;

public:
	static Input& GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init();
	/// <summary>
	/// 入力情報の更新
	/// </summary>
	void Update();

	/// <summary>
	/// 押されているか
	/// </summary>
	/// <param name="command">コマンド</param>
	/// <returns>true : 押している / false : 押していない</returns>
	bool IsPress(const char* const command) const;
	/// <summary>
	/// 押した瞬間
	/// </summary>
	/// <param name="command">コマンド</param>
	/// <returns>true : 押した瞬間 / false : 押していないor押し続けている</returns>
	bool IsTriggerd(const char* const command) const;
	/// <summary>
	/// 離した瞬間
	/// </summary>
	/// <param name="command">コマンド</param>
	/// <returns>true : 離した / false : 押していないor押し続けている</returns>
	bool IsReleased(const char* const command) const;
	/// <summary>
	/// 押し続けている間一定間隔でtrueに
	/// </summary>
	/// <param name="command">コマンド</param>
	/// <param name="repeatInterval">リピート間隔(def = 5)</param>
	/// <param name="startInterval">最初のリピート間隔　-値ならリピート間隔をそのまま入れる</param>
	/// <returns>true: 押した瞬間or一定間隔経った/ false: 押していないor一定時間経っていない</returns>
	bool IsRepeat(const char* const command, int repeatInterval = 5, int startInterval = -1);

	InputTriggerData_t GetTriggerData() const { return m_trigger; }
#ifdef _DEBUG
	bool IsPress(int key) const;
	bool IsTriggerd(int key) const;
#endif

private:
	float GetTriggerRate(unsigned char trigger);
	float GetStickRate(short stick);
	float ApplyDeadZone(float rate);

private:
	// コマンドと入力のペア
	InputTable_t m_commandTable;

	// 入力の記憶
	InputData_t m_padstate;		// 現在の入力
	InputData_t m_prePadstate;	// 直前の入力
	InputTriggerData_t m_trigger;
	std::unordered_map<const char*, int> m_repeatFrame;
#ifdef _DEBUG
	char m_keystate[256];
	char m_preKeystate[256];
#endif
};

