#include "SceneTitle.h"
#include <DxLib.h>
#include <vector>
#include <sstream>
#include "CursorUtility.h"
#include "UIUtility.h"
#include "TimeUtility.h"
#include "Game.h"
#include "File.h"
#include "Application.h"
#include "FontManager.h"
#include "SceneManager.h"
#include "SceneStageSelect.h"
#include "SaveDataManager.h"
#include "StageDataManager.h"
#include "RankingDataManager.h"
#include "SoundManager.h"

namespace
{
	// メニュー選択
	enum class MenuCurrent : int
	{
		SELECT_SAVE,
		RANKING,
		OPOTION,
		END,
		MAX
	};
	// データ確認
	enum class DecideCurrent : int
	{
		CONTINU,
		DELETE_DATA,
		MAX
	};
	// 削除確認
	enum class CheckCurrent : int
	{
		CANCEL,
		OK,
		MAX
	};

	// ファイルID
	const wchar_t* const FILE_COMMON_FRAME			= L"I_CommonFrame";
	const wchar_t* const FILE_COMMON_SELECT_FRAME	= L"I_CommonSelectFrame";
	const wchar_t* const FILE_SAVE_ARM				= L"I_SaveArm";
	const wchar_t* const FILE_SAVE_ARM_FRAME		= L"I_SaveArmFrame";
	const wchar_t* const FILE_SAVE_WINDOW			= L"I_SaveWindow";
	const wchar_t* const FILE_DECIDE_ARM			= L"I_DecideArm";
	const wchar_t* const FILE_DECIDE_ARM_FRAME		= L"I_DecideArmFrame";
	const wchar_t* const FILE_DECIDE_WINDOW			= L"I_DecideWindow";
	const wchar_t* const FILE_ALERT_WINDOW			= L"I_OptionWindow";
	const wchar_t* const FILE_RANKING_WINDOW		= L"I_RankingWindow";
	const wchar_t* const FILE_RANKING_ARROW			= L"I_RankingArrow";
	const wchar_t* const FILE_RANK_S				= L"I_RankS";
	const wchar_t* const FILE_RANK_A				= L"I_RankA";
	const wchar_t* const FILE_RANK_B				= L"I_RankB";
	const wchar_t* const FILE_RANK_C				= L"I_RankC";
	const wchar_t* const FILE_CANCEL				= L"S_Cancel";
	const wchar_t* const FILE_DECIDE				= L"S_Decide";
	const wchar_t* const FILE_MOVE					= L"S_CursorMove";

	/* メニュー */
	// 文字列
	const std::vector<std::wstring> STR_MENU =
	{
		L"ステージ選択",
		L"ランキング",
		L"オプション",
		L"終了",
	};
	// 描画位置
	constexpr int DRAW_MENU_X = static_cast<int>(Game::WINDOW_W * 0.5f);
	constexpr int DRAW_MENU_Y = 340;
	// 描画間隔
	constexpr int DRAW_MENU_X_INTERVAL_1 = 48;
	constexpr int DRAW_MENU_X_INTERVAL_2 = 256;
	constexpr int DRAW_MENU_Y_INTERVAL = 96;
	// フォントサイズ
	constexpr int FONT_SIZE_MENU = 48;

	/* セーブデータ選択 */
	// 文字列
	const std::vector<std::wstring> STR_SAVE =
	{
		L"セーブデータ1",
		L"セーブデータ2",
		L"セーブデータ3",
	};
	// 描画位置
	constexpr int DRAW_SAVE_X = 755;
	constexpr int DRAW_SAVE_Y = 340;
	// 描画間隔
	constexpr int DRAW_SAVE_X_INTERVAL = 40;
	constexpr int DRAW_SAVE_Y_INTERVAL = 90;
	// フォントサイズ
	constexpr int FONT_SIZE_SAVE = 36;
	// ファイルサイズ
	constexpr float FILE_SIZE_SAVE = 0.75f;
	// ウィンドウ位置
	constexpr int DRAW_SAVE_WINDOW_X = 735;
	constexpr int DRAW_SAVE_WINDOW_Y = 430;
	// アームの位置との差
	constexpr int DRAW_SAVE_SUB_ARM_FRAME_X = 145;
	constexpr int DRAW_SAVE_SUB_ARM_FRAME_Y = 0;
	// アームフレームの位置との差
	constexpr int DRAW_SAVE_SUB_ARM_X = 222;
	constexpr int DRAW_SAVE_SUB_ARM_Y = 90;
	// セーブデータ情報描画位置
	constexpr int DRAW_SAVE_INFO_X = 1000;
	constexpr int DRAW_SAVE_INFO_Y = 312;
	constexpr int DRAW_SAVE_INFO_AND_DECIDE_OUT_VAL = 100;

	/* データ決定 */
	// 文字列
	const std::vector<std::wstring> STR_DECIDE =
	{
		L"続きから",
		L"初めから",
	};
	// 描画位置
	constexpr int DRAW_DECIDE_X = 1130;
	constexpr int DRAW_DECIDE_Y = 312;
	// 描画間隔
	constexpr int DRAW_DECIDE_X_INTERVAL = 48;
	constexpr int DRAW_DECIDE_Y_INTERVAL = 54;
	// フォントサイズ
	constexpr int FONT_SIZE_DECIDE = 32;
	// ファイルサイズ
	constexpr float FILE_SIZE_DECIDE = 0.5f;
	// 
	constexpr float TIME_SIZE_2 = 1.0f;
	// ウィンドウ位置
	constexpr int DRAW_DECIDE_WINDOW_X = 1095;
	constexpr int DRAW_DECIDE_WINDOW_Y = DRAW_SAVE_Y;
	// アームの位置との差
	constexpr int DRAW_DECIDE_SUB_ARM_FRAME_X = 115;
	constexpr int DRAW_DECIDE_SUB_ARM_FRAME_Y = 0;
	// アームフレームの位置との差
	constexpr int DRAW_DECIDE_SUB_ARM_X = 185;
	constexpr int DRAW_DECIDE_SUB_ARM_Y = 0;
	// アーム描画間隔
	constexpr int DRAW_DECIDE_WINDOW_INTERVAL_Y = DRAW_SAVE_Y_INTERVAL;

	/* 削除確認 */
	// 文字列
	std::vector<std::wstring> STR_CHECK =
	{
		L"キャンセル",
		L"削除する",
	};
	const std::wstring STR_ALERT = L"データを削除しますか？";
	// 描画位置
	constexpr int DRAW_CHECK_X = 490;
	constexpr int DRAW_CHECK_Y = 450;
	// 描画間隔
	constexpr int DRAW_CHECK_X_INTERVAL = 300;
	// フォントサイズ
	constexpr float FILE_SIZE_CHECK = 0.75f;
	constexpr int FONT_SIZE_CHECK = 32;
	// 画面外まで動かす量
	constexpr int DRAW_ALERT_OUT_VAL = 700;

	/* 警告関係 */
	// 文字カラー
	constexpr unsigned int COLOR_ALERT = 0xffffff;
	// 描画位置
	constexpr int DRAW_ALERT_X = Game::CENTER_X;
	constexpr int DRAW_ALERT_Y = Game::CENTER_Y;
	// 警告文字との差
	constexpr int DRAW_ALERT_STR_SUB_Y = 100;
	// フォントサイズ
	constexpr int FONT_SIZE_ALERT = 48;
	// 揺らす大きさ
	constexpr int SHAKE_SIZE_ALERT = 2;
	// 画像サイズ
	constexpr float FILE_SIZE_ALERT = 0.75f;

	/* ランキング */
	// 矢印描画位置
	constexpr int DRAW_RANKING_ARROW_SUB_X = 500;
	constexpr int DRAW_RANKING_ARROW_SUB_Y = 275;
	// 矢印スワイプの大きさ
	constexpr int DRAW_RANKING_ARROW_X_SWIPE_SIZE = 32;
	// 矢印スワイプスピード
	constexpr float SWIPE_SPEED_ARROW = 1.3f;
	// ステージ名描画
	constexpr unsigned int COLOR_RANKING = 0xffffff;
	// ステージ名描画位置
	constexpr int DRAW_RANKING_STAGE_NAME_SUB_X = 0;
	constexpr int DRAW_RANKING_STAGE_NAME_SUB_Y = 275;
	// ステージ名フォントサイズ
	constexpr int FONT_SIZE_RANKING_STAGE_NAME = 64;
	// ステージ名ウェーブの大きさ
	constexpr int DRAW_RANKING_STAGE_NAME_WAVE_SIZE = 16;
	// ステージ名ウェーブスピード
	constexpr float WAVE_SPEED_STAGE_NAME = 2.0f;
	// 文字描画位置
	constexpr int DRAW_RANKING_STR_SUB_X = 580;
	constexpr int DRAW_RANKING_STR_SUB_Y = 168;
	// 文字描画間隔
	constexpr int DRAW_RANKING_STR_X_INTERVAL = 600;
	constexpr int DRAW_RANKING_STR_Y_INTERVAL = 110;
	// 「記録なし」ずれ具合
	constexpr int DRAW_RANKING_NOT_DATA_SUB_X = 180;
	// 時間ずれ具合
	constexpr int DRAW_RANKING_TIME_SUB_X = 256;
	// ランクずれ具合
	constexpr int DRAW_RANKING_RANK_SUB_X = 160;
	// ランキング文字フォントサイズ
	constexpr int FONT_SIZE_RANKING_RANK = 64;
	constexpr int FONT_SIZE_RANKING_TIME = 56;
	// ランク画像サイズ
	constexpr float FILE_SIZE_RANKING_RANK = 0.125f;
	constexpr float TIME_SIZE = 2.0f;
	// ランク毎のファイルパス
	const std::unordered_map<RankKind, const wchar_t* const> RANK_FILE_PATH =
	{
		{ RankKind::S, FILE_RANK_S },
		{ RankKind::A, FILE_RANK_A },
		{ RankKind::B, FILE_RANK_B },
		{ RankKind::C, FILE_RANK_C },
	};

	/* その他共通とかの */
	// 選択文字カラー
	constexpr unsigned int COLOR_SELECT = 0x000000;
	// 未選択文字カラー
	constexpr unsigned int COLOR_NOT_SELECT = 0xffffff;
	// 文字ウェーブの大きさ
	constexpr int STR_WAVE_SIZE = 16;
	// ウェーブスピード
	constexpr float STR_WAVE_SPEED = 2;
	// 画面外に飛ばす用の値
	constexpr int DRAW_OUT_VAL = 900;
	// フェードにかかるフレーム
	constexpr int FADE_FRAME = 40;
	// UIの移動速度
	constexpr float MOVE_SPEED = 1.0f / FADE_FRAME;
	// 時間のフェード速度
	constexpr int TIME_FADE_SPEED = 2;
}

SceneTitle::SceneTitle() :
	SceneBase(SceneKind::TITLE),
	m_updateFunc(&SceneTitle::SelectMenuUpdate),
	m_drawFunc(&SceneTitle::DrawSelectMenu),
	m_menuCurrent(0),
	m_saveDataNo(0),
	m_preSaveDataNo(0),
	m_decideCurrent(0),
	m_checkCurrent(0),
	m_rankingCurrent(0),
	m_fadeFrame(0),
	m_waveCount(0),
	m_isFade(false),
	m_isFadeTime(false),
	m_isDrawMenu(true),
	m_isDrawSelectSave(false)
{
}

void SceneTitle::AsyncInit()
{
	// ファイル読み込み
	auto& fileMgr = FileManager::GetInstance();
	m_files[FILE_COMMON_FRAME] = fileMgr.Load(FILE_COMMON_FRAME);
	m_files[FILE_COMMON_SELECT_FRAME] = fileMgr.Load(FILE_COMMON_SELECT_FRAME);
	m_files[FILE_SAVE_ARM] = fileMgr.Load(FILE_SAVE_ARM);
	m_files[FILE_SAVE_ARM_FRAME] = fileMgr.Load(FILE_SAVE_ARM_FRAME);
	m_files[FILE_SAVE_WINDOW] = fileMgr.Load(FILE_SAVE_WINDOW);
	m_files[FILE_DECIDE_ARM] = fileMgr.Load(FILE_DECIDE_ARM);
	m_files[FILE_DECIDE_ARM_FRAME] = fileMgr.Load(FILE_DECIDE_ARM_FRAME);
	m_files[FILE_DECIDE_WINDOW] = fileMgr.Load(FILE_DECIDE_WINDOW);
	m_files[FILE_ALERT_WINDOW] = fileMgr.Load(FILE_ALERT_WINDOW);
	m_files[FILE_RANKING_WINDOW] = fileMgr.Load(FILE_RANKING_WINDOW);
	m_files[FILE_RANKING_ARROW] = fileMgr.Load(FILE_RANKING_ARROW);
	m_files[FILE_RANK_S] = fileMgr.Load(FILE_RANK_S);
	m_files[FILE_RANK_A] = fileMgr.Load(FILE_RANK_A);
	m_files[FILE_RANK_B] = fileMgr.Load(FILE_RANK_B);
	m_files[FILE_RANK_C] = fileMgr.Load(FILE_RANK_C);
	m_files[FILE_CANCEL] = fileMgr.Load(FILE_CANCEL);
	m_files[FILE_DECIDE] = fileMgr.Load(FILE_DECIDE);
	m_files[FILE_MOVE] = fileMgr.Load(FILE_MOVE);
}

void SceneTitle::Init()
{
	// UIデータ生成
	m_menuUIList		= UIMoveData::Make(static_cast<int>(MenuCurrent::MAX), DRAW_MENU_X, DRAW_MENU_Y, 0, DRAW_MENU_Y_INTERVAL);
	m_saveUIList		= UIMoveData::Make(SaveDataManager::GetInstance().GetSaveDataNum(), DRAW_SAVE_X + DRAW_OUT_VAL, DRAW_SAVE_Y, 0, DRAW_SAVE_Y_INTERVAL);
	m_decideUIList		= UIMoveData::Make(static_cast<int>(DecideCurrent::MAX), DRAW_DECIDE_X + DRAW_OUT_VAL, DRAW_DECIDE_Y, 0, DRAW_DECIDE_Y_INTERVAL);
	m_checkUIList		= UIMoveData::Make(static_cast<int>(CheckCurrent::MAX), DRAW_CHECK_X, DRAW_CHECK_Y - DRAW_ALERT_OUT_VAL, DRAW_CHECK_X_INTERVAL, 0);
	m_saveInfoUI		= UIMoveData::Make(DRAW_DECIDE_X + DRAW_OUT_VAL, DRAW_DECIDE_Y);
	m_saveArmWindowUI	= UIMoveData::Make(DRAW_SAVE_WINDOW_X + DRAW_OUT_VAL, DRAW_SAVE_WINDOW_Y);
	m_decideArmWindowUI = UIMoveData::Make(DRAW_DECIDE_WINDOW_X + DRAW_OUT_VAL, DRAW_DECIDE_WINDOW_Y);
	m_alertUI			= UIMoveData::Make(DRAW_ALERT_X, DRAW_ALERT_Y - DRAW_ALERT_OUT_VAL);
	m_rankingUI			= UIMoveData::Make(Game::CENTER_X, Game::CENTER_Y);
	// 初期化
	m_menuUIList[0]->ChangeHorizontal(DRAW_MENU_X - DRAW_MENU_X_INTERVAL_1);
}

void SceneTitle::End()
{
}

void SceneTitle::Update(bool isFade)
{
	++m_waveCount;
	m_alertUI->Update();
	m_saveArmWindowUI->Update();
	m_decideArmWindowUI->Update(MOVE_SPEED, EasingType::LERP);
	m_saveInfoUI->Update(MOVE_SPEED, EasingType::LERP);
	for (auto& data : m_menuUIList)	data->Update();
	for (auto& data : m_saveUIList)	data->Update();
	for (auto& data : m_decideUIList) data->Update();
	for (auto& data : m_checkUIList) data->Update();

	if (isFade) return;
	(this->*m_updateFunc)();
	if (m_isFade)
	{
		--m_fadeFrame;
		if (m_fadeFrame < 0)
		{
			m_fadeFrame = 0;
			m_isFade = false;
		}
	}
	if (m_isFadeTime)
	{
		m_fadeFrame -= TIME_FADE_SPEED;
		if (m_fadeFrame < -FADE_FRAME)
		{
			m_fadeFrame = 0;
			m_isFadeTime = false;
		}
	}
}

void SceneTitle::Draw() const
{
	auto& fontMgr = FontManager::GetInstance();

	/* 共通で描画する部分 */
	// 画像ハンドル
	int selectH = m_files.at(FILE_COMMON_SELECT_FRAME)->GetHandle();
	int notSelectH = m_files.at(FILE_COMMON_FRAME)->GetHandle();
	// メインとなる部分描画
	if (m_isDrawMenu)	UIUtility::DrawFrameAndStr(m_menuUIList, 1.0f, 0.0f, FONT_SIZE_MENU, m_menuCurrent, STR_MENU, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT, UIUtility::DrawStrType::WAVE, STR_WAVE_SIZE, m_waveCount * STR_WAVE_SPEED);
	else				UIUtility::DrawFrameAndStr(m_menuUIList, 1.0f, 0.0f, FONT_SIZE_MENU, m_menuCurrent, STR_MENU, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
	// 追加情報部分の枠描画
	ArmWindowDraw(m_saveArmWindowUI, DRAW_SAVE_SUB_ARM_FRAME_X, DRAW_SAVE_SUB_ARM_FRAME_Y, DRAW_SAVE_SUB_ARM_X, DRAW_SAVE_SUB_ARM_Y, FILE_SAVE_WINDOW, FILE_SAVE_ARM_FRAME, FILE_SAVE_ARM);
	ArmWindowDraw(m_decideArmWindowUI, DRAW_DECIDE_SUB_ARM_FRAME_X, DRAW_DECIDE_SUB_ARM_FRAME_Y, DRAW_DECIDE_SUB_ARM_X, DRAW_DECIDE_SUB_ARM_Y, FILE_DECIDE_WINDOW, FILE_DECIDE_ARM_FRAME, FILE_DECIDE_ARM);
	// セーブデータ選択のUI描画
	if (m_isDrawSelectSave) UIUtility::DrawFrameAndStr(m_saveUIList, FILE_SIZE_SAVE, 0.0f, FONT_SIZE_SAVE, m_saveDataNo, STR_SAVE, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT, UIUtility::DrawStrType::WAVE, STR_WAVE_SIZE, m_waveCount * STR_WAVE_SPEED);
	else					UIUtility::DrawFrameAndStr(m_saveUIList, FILE_SIZE_SAVE, 0.0f, FONT_SIZE_SAVE, m_saveDataNo, STR_SAVE, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
	// それぞれでの描画処理
	(this->*m_drawFunc)();
}

void SceneTitle::SelectMenuUpdate()
{
	if (CurrentUpdate(m_menuCurrent, m_menuUIList, DRAW_MENU_X, DRAW_MENU_X_INTERVAL_1, Command::BTN_UP, Command::BTN_DOWN))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_MOVE)->GetHandle());
		m_waveCount = 0;
	}

	auto& input = Input::GetInstance();
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_DECIDE)->GetHandle());
		// セーブデータ選択に移動
		if (m_menuCurrent == static_cast<int>(MenuCurrent::SELECT_SAVE))
		{
			OnSelectSaveData();
		}
		// ランキングを開く
		else if (m_menuCurrent == static_cast<int>(MenuCurrent::RANKING))
		{
			OnRanking();
		}
		// オプションを開く
		else if (m_menuCurrent == static_cast<int>(MenuCurrent::OPOTION))
		{
			m_scnMgr.OpenOption(SceneKind::TITLE);
		}
		// ゲーム終了
		else if (m_menuCurrent == static_cast<int>(MenuCurrent::END))
		{
			auto& app = Application::GetInstance();
			app.OnEnd();
		}
	}
}

void SceneTitle::RankingUpdate()
{
	bool isMove = false;
	auto stageNum = StageDataManager::GetInstance().GetStageNum();
	if (CursorUtility::CursorUp(m_rankingCurrent, stageNum, Command::BTN_LEFT)) isMove = true;
	if (CursorUtility::CursorDown(m_rankingCurrent, stageNum, Command::BTN_RIGHT)) isMove = true;
	if (isMove)
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_MOVE)->GetHandle());
	}

	auto& input = Input::GetInstance();
	// キャンセル
	if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_CANCEL)->GetHandle());
		m_updateFunc = &SceneTitle::SelectMenuUpdate;
		m_drawFunc = &SceneTitle::DrawSelectMenu;
	}
}

void SceneTitle::SelectSaveDataUpdate()
{
	// フェードしていない場合またはフェードフレームが-の場合は現在の番号を保存
	if (!m_isFadeTime || m_fadeFrame < 0) m_preSaveDataNo = m_saveDataNo;
	// カーソルが移動したら
	if (CurrentUpdate(m_saveDataNo, m_saveUIList, DRAW_SAVE_X, DRAW_SAVE_X_INTERVAL, Command::BTN_UP, Command::BTN_DOWN))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_MOVE)->GetHandle());
		// ウェーブカウント初期化
		m_waveCount = 0;
		// 決定側のwindowとセーブデータ情報の位置を移動
		m_decideArmWindowUI->ChangeVertical(DRAW_DECIDE_WINDOW_Y + DRAW_DECIDE_WINDOW_INTERVAL_Y * m_saveDataNo, -1, true);
		m_saveInfoUI->ChangeVertical(DRAW_SAVE_INFO_Y + DRAW_DECIDE_WINDOW_INTERVAL_Y * m_saveDataNo, -1, true);
		// フレーム変更
		m_fadeFrame = FADE_FRAME;
		// フェードしている状態に
		m_isFadeTime = true;
	}

	auto& input = Input::GetInstance();
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_DECIDE)->GetHandle());
		auto& saveDataMgr = SaveDataManager::GetInstance();
		// セーブデータが存在する場合
		if (saveDataMgr.IsExist(m_saveDataNo))
		{
			// セーブデータ決定に移動
			OnDecideSaveData();
			m_decideCurrent = 0;
			m_fadeFrame = FADE_FRAME;
			m_isFade = true;
		}
		// 存在しない場合
		else
		{
			// ゲームスタート
			OnStart();
		}
		
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_CANCEL)->GetHandle());
		// メニュー選択に戻る
		OnSelectMenu();
	}
}

void SceneTitle::DecideSaveDataUpdate()
{
	if (CurrentUpdate(m_decideCurrent, m_decideUIList, DRAW_DECIDE_X, DRAW_DECIDE_X_INTERVAL, Command::BTN_UP, Command::BTN_DOWN))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_MOVE)->GetHandle());
		m_waveCount = 0;
	}

	auto& input = Input::GetInstance();
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_DECIDE)->GetHandle());
		// 続きから
		if (m_decideCurrent == static_cast<int>(DecideCurrent::CONTINU))
		{
			// ゲームスタート
			OnStart();
		}
		// 初めから
		else if (m_decideCurrent == static_cast<int>(DecideCurrent::DELETE_DATA))
		{
			// 削除確認に移動
			OnCheckDeleteSaveData();
		}
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_CANCEL)->GetHandle());
		// セーブデータ選択に移動
		OnSelectSaveData();
		m_isFade = true;
		m_fadeFrame = FADE_FRAME;
	}
}

void SceneTitle::CheckDeleteSaveDataUpdate()
{
	bool isMove = false;
	if (CursorUtility::CursorUp<CheckCurrent>(m_checkCurrent, CheckCurrent::MAX, Command::BTN_LEFT)) isMove = true;
	if (CursorUtility::CursorDown<CheckCurrent>(m_checkCurrent, CheckCurrent::MAX, Command::BTN_RIGHT)) isMove = true;
	if (isMove)
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_MOVE)->GetHandle());
		m_waveCount = 0;
	}

	auto& input = Input::GetInstance();
	// 決定
	if (input.IsTriggerd(Command::BTN_OK))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_DECIDE)->GetHandle());
		// キャンセル
		if (m_checkCurrent == static_cast<int>(CheckCurrent::CANCEL))
		{
			// セーブデータ決定に移動
			OnDecideSaveData();
		}
		// 削除
		else if (m_checkCurrent == static_cast<int>(CheckCurrent::CANCEL))
		{
			// 初期化
			auto& saveDataMgr = SaveDataManager::GetInstance();
			saveDataMgr.Initialized(m_saveDataNo);
		}
	}
	// キャンセル
	else if (input.IsTriggerd(Command::BTN_CANCEL))
	{
		SoundManager::GetInstance().PlaySe(m_files.at(FILE_CANCEL)->GetHandle());
		// セーブデータ決定に移動
		OnDecideSaveData();
	}
}

void SceneTitle::OnSelectMenu()
{
	// メニューに分類されているものを左から中央に戻す
	UIMoveData::ChangeHorizontal(m_menuUIList, DRAW_MENU_X, m_menuCurrent, DRAW_MENU_X - DRAW_MENU_X_INTERVAL_1);
	// Window関係のものを画面内から画面外に移す
	UIMoveData::ChangeHorizontal(m_saveUIList, DRAW_SAVE_X + DRAW_OUT_VAL);
	m_saveArmWindowUI->ChangeHorizontal(DRAW_SAVE_WINDOW_X + DRAW_OUT_VAL);
	m_decideArmWindowUI->ChangeHorizontal(DRAW_DECIDE_WINDOW_X + DRAW_OUT_VAL);
	m_saveInfoUI->ChangeHorizontal(DRAW_SAVE_INFO_X + DRAW_OUT_VAL);
	// メンバ関数変更
	m_updateFunc = &SceneTitle::SelectMenuUpdate;
	m_drawFunc = &SceneTitle::DrawSelectMenu;
	// カウント初期化
	m_waveCount = 0;
	m_isDrawMenu = true;
	m_isDrawSelectSave = false;
}

void SceneTitle::OnRanking()
{
	// メンバ関数変更
	m_updateFunc = &SceneTitle::RankingUpdate;
	m_drawFunc = &SceneTitle::DrawRanking;
}

void SceneTitle::OnSelectSaveData()
{
	// メニューに分類されているものを中央から左にずらす
	UIMoveData::ChangeHorizontal(m_menuUIList, DRAW_MENU_X - DRAW_MENU_X_INTERVAL_2, m_menuCurrent, DRAW_MENU_X - DRAW_MENU_X_INTERVAL_1 - DRAW_MENU_X_INTERVAL_2);
	// 画面外にあるWindowたちを画面内に持ってくる
	UIMoveData::ChangeHorizontal(m_saveUIList, DRAW_SAVE_X, m_saveDataNo, DRAW_SAVE_X - DRAW_SAVE_X_INTERVAL);
	m_decideArmWindowUI->ChangeHorizontal(DRAW_DECIDE_WINDOW_X);
	m_saveArmWindowUI->ChangeHorizontal(DRAW_SAVE_WINDOW_X);
	m_saveInfoUI->ChangeHorizontal(DRAW_SAVE_INFO_X);
	UIMoveData::ChangeHorizontal(m_decideUIList, DRAW_DECIDE_X + DRAW_SAVE_INFO_AND_DECIDE_OUT_VAL, m_decideCurrent, DRAW_DECIDE_X + DRAW_SAVE_INFO_AND_DECIDE_OUT_VAL - DRAW_DECIDE_X_INTERVAL);
	// メンバ関数変更
	m_updateFunc = &SceneTitle::SelectSaveDataUpdate;
	m_drawFunc = &SceneTitle::DrawSelectSaveData;
	// カウント初期化
	m_waveCount = 0;
	m_isDrawMenu = false;
	m_isDrawSelectSave = true;
}

void SceneTitle::OnDecideSaveData()
{
	UIMoveData::ChangeVertical(m_checkUIList, DRAW_CHECK_Y - DRAW_ALERT_OUT_VAL);
	m_alertUI->ChangeVertical(DRAW_ALERT_Y - DRAW_ALERT_OUT_VAL);
	m_saveInfoUI->ChangeHorizontal(DRAW_SAVE_INFO_X + DRAW_SAVE_INFO_AND_DECIDE_OUT_VAL);
	UIMoveData::ChangeHorizontal(m_decideUIList, DRAW_DECIDE_X, m_decideCurrent, DRAW_DECIDE_X - DRAW_DECIDE_X_INTERVAL);
	// メンバ関数変更
	m_updateFunc = &SceneTitle::DecideSaveDataUpdate;
	m_drawFunc = &SceneTitle::DrawDecideSaveData;
	// カウント初期化
	m_waveCount = 0;
	m_isDrawSelectSave = false;
}

void SceneTitle::OnCheckDeleteSaveData()
{
	UIMoveData::ChangeVertical(m_checkUIList, DRAW_CHECK_Y);
	m_alertUI->ChangeVertical(DRAW_ALERT_Y);
	// メンバ関数変更
	m_updateFunc = &SceneTitle::CheckDeleteSaveDataUpdate;
	m_drawFunc = &SceneTitle::DrawCheckDeleteSaveData;
	// カウント初期化
	m_waveCount = 0;
	// チェック位置初期化
	m_checkCurrent = 0;
}

void SceneTitle::DrawSelectMenu() const
{
	// タイム取得
	auto& saveDataMgr = SaveDataManager::GetInstance();
	auto time = saveDataMgr.GetPlayTime(m_saveDataNo);
	// セーブデータの情報描画
	TimeUtility::DrawTime(m_saveInfoUI->x, m_saveInfoUI->y, time, TIME_SIZE_2, 2);
}

void SceneTitle::DrawRanking() const
{
	auto& fontMgr = FontManager::GetInstance();
	auto& stageDataMgr = StageDataManager::GetInstance();
	const auto& stageName = stageDataMgr.GetStageName(m_rankingCurrent);
	auto& rnkMgr = RankingDataManager::GetInstance();
	// ウィンドウ描画
	DrawRotaGraphFast(m_rankingUI->x, m_rankingUI->y, 1.0f, 0.0f, m_files.at(FILE_RANKING_WINDOW)->GetHandle(), true);
	// 矢印描画
	int addArrowX = static_cast<int>(std::sin(m_waveCount * Game::DEG_2_RAD * SWIPE_SPEED_ARROW) * DRAW_RANKING_ARROW_X_SWIPE_SIZE);
	DrawRotaGraphFast(m_rankingUI->x - DRAW_RANKING_ARROW_SUB_X + addArrowX, m_rankingUI->y - DRAW_RANKING_ARROW_SUB_Y, 1.0f, 0.0f, m_files.at(FILE_RANKING_ARROW)->GetHandle(), true);
	DrawRotaGraphFast(m_rankingUI->x + DRAW_RANKING_ARROW_SUB_X - addArrowX, m_rankingUI->y - DRAW_RANKING_ARROW_SUB_Y, 1.0f, 180.0f * Game::DEG_2_RAD, m_files.at(FILE_RANKING_ARROW)->GetHandle(), true);
	UIUtility::DrawWaveStr(m_rankingUI->x - DRAW_RANKING_STAGE_NAME_SUB_X, m_rankingUI->y - DRAW_RANKING_STAGE_NAME_SUB_Y, COLOR_RANKING, stageName, FONT_SIZE_RANKING_STAGE_NAME, m_waveCount * WAVE_SPEED_STAGE_NAME, DRAW_RANKING_STAGE_NAME_WAVE_SIZE);

	// 順位、タイム描画
	constexpr int halfNum = static_cast<int>(RankingDataManager::RANKING_DATA_NUM * 0.5f);
	int drawStrBaseX = m_rankingUI->x - DRAW_RANKING_STR_SUB_X;
	int drawStrBaseY = m_rankingUI->y - DRAW_RANKING_STR_SUB_Y;
	int drawFontSubRank = static_cast<int>(FONT_SIZE_RANKING_RANK * 0.5f);
	int drawFontSubTime = static_cast<int>(FONT_SIZE_RANKING_TIME * 0.5f);
	for (int i = 0; i < RankingDataManager::RANKING_DATA_NUM; ++i)
	{
		int x = drawStrBaseX + DRAW_RANKING_STR_X_INTERVAL * (i / halfNum);
		int y = drawStrBaseY + DRAW_RANKING_STR_Y_INTERVAL * (i % halfNum);
		DrawFormatStringToHandle(x, y - drawFontSubRank, COLOR_RANKING, fontMgr.GetHandle(FONT_SIZE_RANKING_RANK), L"%d位", i + 1);
		auto timeFrame = rnkMgr.GetTime(stageName, i);
		// 記録がない場合
		if (timeFrame < 0)
		{
			DrawStringToHandle(x + DRAW_RANKING_NOT_DATA_SUB_X, y - drawFontSubTime, L"記録がありません", COLOR_RANKING, fontMgr.GetHandle(FONT_SIZE_RANKING_TIME));
		}
		// 記録がある場合
		else
		{
			const auto& rank = stageDataMgr.GetRank(stageName, timeFrame);
			const auto& path = RANK_FILE_PATH.at(rank);
			DrawRotaGraphFast(x + DRAW_RANKING_RANK_SUB_X, y, FILE_SIZE_RANKING_RANK, 0.0f, m_files.at(path)->GetHandle(), true);
			TimeUtility::DrawTime(x + DRAW_RANKING_TIME_SUB_X, y - drawFontSubTime, timeFrame, TIME_SIZE, 2);
		}
	}
}

void SceneTitle::DrawSelectSaveData() const
{
	// タイム取得
	auto& saveDataMgr = SaveDataManager::GetInstance();
	auto time = TimeUtility::FrameToTime(saveDataMgr.GetPlayTime(m_saveDataNo));
	if (m_isFade)
	{
		int alpha = static_cast<int>(Game::ALPHA_VAL * (m_fadeFrame / static_cast<float>(FADE_FRAME)));
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		// 画像ハンドル
		int selectH = m_files.at(FILE_COMMON_SELECT_FRAME)->GetHandle();
		int notSelectH = m_files.at(FILE_COMMON_FRAME)->GetHandle();
		// 続きから・初めからのUI描画
		UIUtility::DrawFrameAndStr(m_decideUIList, FILE_SIZE_DECIDE, 0.0f, FONT_SIZE_DECIDE, m_decideCurrent, STR_DECIDE, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, Game::ALPHA_VAL - alpha);
	}
	else if (m_isFadeTime)
	{
		int alpha = static_cast<int>(Game::ALPHA_VAL * (m_fadeFrame / static_cast<float>(FADE_FRAME)));
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::max<int>(alpha, 0));
		// 選択前のデータのタイム取得
		auto timePre = TimeUtility::FrameToTime(saveDataMgr.GetPlayTime(m_preSaveDataNo));
		// セーブデータの情報描画
		DrawFormatString(m_saveInfoUI->x, m_saveInfoUI->y, COLOR_NOT_SELECT, L"プレイ時間：%d:%d.%d", timePre.min, timePre.sec, timePre.dec);
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::max<int>(alpha * -1, 0));
	}
	// セーブデータの情報描画
	DrawFormatString(m_saveInfoUI->x, m_saveInfoUI->y, COLOR_NOT_SELECT, L"プレイ時間：%d:%d.%d", time.min, time.sec, time.dec);
	// ブレンドなしに
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneTitle::DrawDecideSaveData() const
{
	// 画像ハンドル
	int selectH = m_files.at(FILE_COMMON_SELECT_FRAME)->GetHandle();
	int notSelectH = m_files.at(FILE_COMMON_FRAME)->GetHandle();
	if (m_isFade)
	{
		int alpha = static_cast<int>(Game::ALPHA_VAL * (m_fadeFrame / static_cast<float>(FADE_FRAME)));
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		// タイム取得
		auto& saveDataMgr = SaveDataManager::GetInstance();
		auto time = TimeUtility::FrameToTime(saveDataMgr.GetPlayTime(m_saveDataNo));
		// セーブデータの情報描画
		DrawFormatString(m_saveInfoUI->x, m_saveInfoUI->y, COLOR_NOT_SELECT, L"プレイ時間：%d:%d.%d", time.min, time.sec, time.dec);
		// 透明度変更
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, Game::ALPHA_VAL - alpha);
	}
	// 続きから・初めからのUI描画
	UIUtility::DrawFrameAndStr(m_decideUIList, FILE_SIZE_DECIDE, 0.0f, FONT_SIZE_DECIDE, m_decideCurrent, STR_DECIDE, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT, UIUtility::DrawStrType::WAVE, STR_WAVE_SIZE, m_waveCount * STR_WAVE_SPEED);
	// ブレンドなしに
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	// 警告ウィンドウ描画
	DrawRotaGraphFast(m_alertUI->x, m_alertUI->y, FILE_SIZE_ALERT, 0.0f, m_files.at(FILE_ALERT_WINDOW)->GetHandle(), true);
	// 削除確認のUI描画
	UIUtility::DrawFrameAndStr(m_checkUIList, FILE_SIZE_CHECK, 0.0f, FONT_SIZE_CHECK, m_checkCurrent, STR_CHECK, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
}

void SceneTitle::DrawCheckDeleteSaveData() const
{
	// 画像ハンドル
	int selectH = m_files.at(FILE_COMMON_SELECT_FRAME)->GetHandle();
	int notSelectH = m_files.at(FILE_COMMON_FRAME)->GetHandle();
	// 続きから・初めからのUI描画
	UIUtility::DrawFrameAndStr(m_decideUIList, FILE_SIZE_DECIDE, 0.0f, FONT_SIZE_DECIDE, m_decideCurrent, STR_DECIDE, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
	// 警告ウィンドウ描画
	DrawRotaGraphFast(m_alertUI->x, m_alertUI->y, FILE_SIZE_ALERT, 0.0f, m_files.at(FILE_ALERT_WINDOW)->GetHandle(), true);
	// 削除確認のUI描画
	UIUtility::DrawFrameAndStr(m_checkUIList, FILE_SIZE_CHECK, 0.0f, FONT_SIZE_CHECK, m_checkCurrent, STR_CHECK, selectH, notSelectH, COLOR_SELECT, COLOR_NOT_SELECT);
	// 文字描画
	if (m_checkCurrent == static_cast<int>(CheckCurrent::CANCEL))	UIUtility::DrawWaveStr(m_alertUI->x, m_alertUI->y - DRAW_ALERT_STR_SUB_Y, COLOR_ALERT, STR_ALERT, FONT_SIZE_ALERT, m_waveCount * STR_WAVE_SPEED, STR_WAVE_SIZE);
	else															UIUtility::DrawShakeStr(m_alertUI->x, m_alertUI->y - DRAW_ALERT_STR_SUB_Y, COLOR_ALERT, STR_ALERT, FONT_SIZE_ALERT, SHAKE_SIZE_ALERT);
	
}

bool SceneTitle::CurrentUpdate(int& current, const std::vector<std::shared_ptr<UIMoveData>>& list, int start, int interval, const char* const upCmd, const char* const downCmd)
{
	bool isMove = false;
	int pre = current;
	if (CursorUtility::CursorUp(current, list.size(), upCmd)) isMove = true;
	if (CursorUtility::CursorDown(current, list.size(), downCmd)) isMove = true;
	if (isMove)
	{
		list[pre]->ChangeHorizontal(start);
		list[current]->ChangeHorizontal(start - interval);
	}

	return isMove;
}

void SceneTitle::OnStart()
{
	auto& saveDataMgr = SaveDataManager::GetInstance();
	// 使用セーブデータの決定
	saveDataMgr.DecideUseSaveNo(m_saveDataNo);
	// シーンの変更
	auto next = std::make_shared<SceneStageSelect>();
	m_scnMgr.Change(next);
}

void SceneTitle::ArmWindowDraw(const std::shared_ptr<UIMoveData>& data, int subArmFrameX, int subArmFrameY, int subArmX, int subArmY, const wchar_t* const windowFileName, const wchar_t* const armFrameFileName, const wchar_t* const armFileName) const
{
	// ウィンドウ
	DrawRotaGraphFast(data->x, data->y, 1.25f, 0.0f, m_files.at(windowFileName)->GetHandle(), true);
	// アームフレーム
	DrawRotaGraphFast(data->x - subArmFrameX, data->y - subArmFrameY, 1.25f, 0.0f, m_files.at(armFrameFileName)->GetHandle(), true);
	// アーム
	DrawRotaGraphFast(data->x - subArmX, data->y - subArmY, 1.0f, 0.0f, m_files.at(armFileName)->GetHandle(), true);
}