#include "FileManager.h"

#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include "FileBase.h"
#include "FileImage.h"
#include "FileSound.h"
#include "FileModel.h"
#include "FileEffekseer.h"
#include "FileShader.h"
#include "Utility/StringUtility.h"

namespace
{
	const wchar_t* const PATH_FILE_MASTER = L"Data/Master/FileMaster.csv";
	const wchar_t* const PATH_TEXTURE_DATA_MASTER = L"Data/Master/TextureDataMaster.csv";
	const wchar_t* const MODEL_SHADER_MASTER = L"Data/Master/ModelShaderMaster.csv";
	const enum FileIndex : int
	{
		NAME,
		TYPE,
		PATH,
		EXTENSION
	};
	const enum TextureIndex : int
	{
		MODEL_NAME,
		SEET_NUM,
		TEXTURE_NAME
	};

	struct Header
	{
		float var = 1.0f;
		int size = 0;
	};

	const std::unordered_map<std::wstring, LoadType> NAME_TO_TYPE =
	{
		{ L"Image", LoadType::IMAGE },
		{ L"Sound", LoadType::SOUND },
		{ L"Model", LoadType::MODEL },
		{ L"Eff", LoadType::EFFEKSEER },
		{ L"PS", LoadType::PIXEL_SHADER },
		{ L"VS", LoadType::VERTEX_SHADER }
	};

	const std::unordered_map<LoadType, std::wstring> TYPE_TO_PATH =
	{
		{ LoadType::IMAGE, L"Image/" },
		{ LoadType::SOUND, L"Sound/" },
		{ LoadType::MODEL, L"Model/" },
		{ LoadType::EFFEKSEER, L"Effekseer/" },
		{ LoadType::PIXEL_SHADER, L"Shader/" },
		{ LoadType::VERTEX_SHADER, L"Shader/" },

	};
}

FileManager::FileManager() :
	m_isEnd(false)
{
}

FileManager::~FileManager()
{
}

FileManager& FileManager::GetInstance()
{
	static FileManager mgr;
	return mgr;
}

void FileManager::Init()
{
	SetUseASyncLoadFlag(true);
	m_isEnd = false;

	LoadPath();
	LoadSetTextureDate();
	LoadModelShader();
}

void FileManager::End()
{
	if (m_isEnd) return;

	for (auto& file : m_file)
	{
		file.second->Delete();
		file.second->End();
	}

	m_isEnd = true;
}

CsvData_t FileManager::LoadCsv(const wchar_t* const path)
{
	CsvData_t data;

	// ファイル読み込み
	std::wifstream ifs(path);
	if (!ifs)
	{
		assert(false);
		return data;
	}
	// バッファー
	std::wstring strBuf;
	// 余分な部分の読み込み
	std::getline(ifs, strBuf);
	// データすべて読み込み
	while (std::getline(ifs, strBuf))
	{
		data.push_back(StringUtility::Split(strBuf, ','));
	}

	return data;
}

std::shared_ptr<FileBase> FileManager::Load(const wchar_t* const name, bool isEternal)
{
	auto it = m_file.find(name);

	// 見つかれば参照カウンタを増やして、コピーを返す
	if (it != m_file.end()) 
	{
		auto& file = it->second;
		file->m_refCount++;
		return CastCopyFile(file);
	}

	// 見つかなければ作成し、コピーを返す
	std::shared_ptr<FileBase> file = MakeFile(name);
	m_file.insert(std::make_pair(name, file));
	file->m_name = name;
	file->m_handle = GetHandle(name);
	file->m_refCount++;
	file->m_isEternal = isEternal;

	return CastCopyFile(file);
}

void FileManager::SetTexture(int modelH, const std::wstring& name)
{
	// モデルにセットするテクスチャデータを取得
	const auto& texData = m_texData.at(name);

	int texIdx = 0;
	for (const auto& texName : texData)
	{
		// セットするテクスチャのファイルデータを取得
		const auto& file = m_file.at(texName);
		// 画像のハンドル取得
		int imgH = file->GetHandle();

		// テクスチャの変更
		MV1SetTextureGraphHandle(modelH, texIdx, imgH, false);

		// テクスチャ番号の更新
		texIdx++;
	}
}

void FileManager::Delete(const wchar_t* const name)
{
	// 終了処理が呼ばれていた場合は何もしない
	if (m_isEnd) return;
	// 見つかなければ終了
	if (m_file.find(name) == m_file.end()) return;

	auto& file = m_file.at(name);
	// 永続フラグがtrueなら終了
	if (file->m_isEternal) return;

	// 参照カウンタを減らす
	file->m_refCount--;
	// 見ているものが無くなればデータを消す
	if (file->m_refCount <= 0) 
	{
		file->Delete();
		file->End();
		m_file.erase(name);
	}
}

std::shared_ptr<FileBase> FileManager::MakeFile(const wchar_t* const name)
{
	auto type = m_path.at(name).type;

	if (type == LoadType::IMAGE)
	{
		return std::make_shared<FileImage>(*this);
	}
	else if (type == LoadType::SOUND) 
	{
		return std::make_shared<FileSound>(*this);
	}
	else if (type == LoadType::MODEL) 
	{
		return std::make_shared<FileModel>(*this);
	}
	else if (type == LoadType::EFFEKSEER)
	{
		return std::make_shared<FileEffekseer>(*this);
	}
	else if (type == LoadType::PIXEL_SHADER || type == LoadType::VERTEX_SHADER)
	{
		return std::make_shared<FileShader>(*this);
	}

	// ここまで来たらおかしい
	assert(false);
	// 動くようにFileGraphを返しておく
	return std::make_shared<FileImage>(*this);
}

std::shared_ptr<FileBase> FileManager::CastCopyFile(std::shared_ptr<FileBase>& file)
{
	auto type = m_path.at(file->m_name).type;

	if (type == LoadType::IMAGE)
	{
		auto fileGraph = std::dynamic_pointer_cast<FileImage>(file);
		return std::make_shared<FileImage>(*fileGraph);
	}
	else if (type == LoadType::SOUND) 
	{
		auto fileSound = std::dynamic_pointer_cast<FileSound>(file);
		return std::make_shared<FileSound>(*fileSound);
	}
	else if (type == LoadType::MODEL) 
	{
		auto fileModel = std::dynamic_pointer_cast<FileModel>(file);
		return std::make_shared<FileModel>(*fileModel);
	}
	else if (type == LoadType::EFFEKSEER)
	{
		auto fileEff = std::dynamic_pointer_cast<FileEffekseer>(file);
		return std::make_shared<FileEffekseer>(*fileEff);
	}
	else if (type == LoadType::PIXEL_SHADER || type == LoadType::VERTEX_SHADER)
	{
		auto fileShader = std::dynamic_pointer_cast<FileShader>(file);
		return std::make_shared<FileShader>(*fileShader);
	}

	// ここまで来たらおかしい
	assert(false);
	// 動くようにFileGraphを返しておく
	std::shared_ptr<FileImage> fileGraph = std::dynamic_pointer_cast<FileImage>(file);
	return std::make_shared<FileImage>(*fileGraph);
}

int FileManager::GetHandle(const wchar_t* name) const
{
	int handle = -1;
	auto path = m_path.at(name).path.c_str();
	auto type = m_path.at(name).type;

	if (type == LoadType::IMAGE) 
	{
		handle = LoadGraph(path);
	}
	else if (type == LoadType::SOUND) 
	{
		handle = LoadSoundMem(path);
	}
	else if (type == LoadType::MODEL) 
	{
		handle = MV1LoadModel(path);
	}
	else if (type == LoadType::EFFEKSEER)
	{
		SetUseASyncLoadFlag(false);
		handle = LoadEffekseerEffect(path);
		SetUseASyncLoadFlag(true);
	}
	else if (type == LoadType::PIXEL_SHADER)
	{
		handle = LoadPixelShader(path);
	}
	else if (type == LoadType::VERTEX_SHADER)
	{
		handle = LoadVertexShader(path);
	}
	else 
	{
		assert(false);
	}
	assert(handle != -1);

	return handle;
}

void FileManager::LoadPath()
{
	const auto& csv = LoadCsv(PATH_FILE_MASTER);
	for (auto& item : csv)
	{
		// 名前取得
		const auto& name = item[FileIndex::NAME];
		// 種類取得
		const std::wstring& typeName = item[FileIndex::TYPE];
		LoadType type = NAME_TO_TYPE.at(typeName);
		// パス取得
		std::wostringstream path;
		path << L"Data/" << TYPE_TO_PATH.at(type) << item[FileIndex::PATH] << L"." << item[FileIndex::EXTENSION];

		auto& data = m_path[name];
		data.type = type;
		data.path = path.str();
	}
}

void FileManager::LoadSetTextureDate()
{
	const auto& csv = LoadCsv(PATH_TEXTURE_DATA_MASTER);
	for (auto& item : csv)
	{
		// 名前取得
		const auto& name = item[TextureIndex::MODEL_NAME];
		// データ生成
		auto& data = m_texData[name];
		// セット数
		int setNum = std::stoi(item[TextureIndex::SEET_NUM]);
		for (int i = 0; i < setNum; ++i)
		{
			data.emplace_back(item[TextureIndex::TEXTURE_NAME + i]);
		}
	}
}

void FileManager::LoadModelShader()
{
	const auto& csv = LoadCsv(MODEL_SHADER_MASTER);
	for (auto& item : csv)
	{
		const auto& model = item[0];
		const auto& vs = item[1];
		const auto& ps = item[2];

		m_shaderData[model].vs = Load(vs.c_str(), true)->GetHandle();
		m_shaderData[model].ps = Load(ps.c_str(), true)->GetHandle();
	}
}
