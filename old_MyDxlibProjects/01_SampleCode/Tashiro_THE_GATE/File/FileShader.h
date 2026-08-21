#pragma once
#include "FileBase.h"

class FileShader : public FileBase
{
public:
	FileShader(FileManager& mgr);
	virtual ~FileShader();

private:
	void Delete() override;
};

