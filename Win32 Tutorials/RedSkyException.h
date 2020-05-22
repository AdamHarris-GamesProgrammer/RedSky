#pragma once

#include <exception>
#include <string>

class RedSkyException : public std::exception
{
public:
	RedSkyException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept { return mLine; }
	const std::string& GetFile() const noexcept { return mFile; }
	std::string GetOriginString() const noexcept;

private:
	int mLine;
	std::string mFile;

protected:
	mutable std::string whatBuffer;
};

