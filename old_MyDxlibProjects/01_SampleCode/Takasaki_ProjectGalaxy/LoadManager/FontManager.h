#pragma once
#include<map>
#include<string>
#include"DxLib.h"
struct FontInfo {
	int handle;//ハンドル
	bool used;//一度以上使用済みか？
};
class FontManager
{
private:
	std::map<std::string, FontInfo> m_pathAndFontInfoes;

public:
	FontManager();
	~FontManager();

	static FontManager& GetInstance();
	int GetFontData(const char* fontfilename, const char* fontname, int size = 60, int thick = 7, int fonttype = DX_FONTTYPE_NORMAL);
};
