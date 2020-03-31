#include "RedSkyException.h"

#include <sstream>

RedSkyException::RedSkyException(int line, const char* file) noexcept
{
	mLine = line;
	mFile = file;
}

const char* RedSkyException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl << GetOriginString();
	whatBuffer = oss.str(); //The buffer will live past this function call where as the ostringstream will not
	return whatBuffer.c_str();
}

const char* RedSkyException::GetType() const noexcept
{
	return "Red Sky Exception";
}

std::string RedSkyException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << mFile << std::endl
		<< "[Line] " << mLine;
	return oss.str();
}
