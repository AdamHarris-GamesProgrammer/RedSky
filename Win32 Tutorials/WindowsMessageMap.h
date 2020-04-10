#pragma once

#include <unordered_map>
#include <Windows.h>

//This class stores all of the possible window event messages
class WindowsMessageMap
{
public:
	WindowsMessageMap() noexcept;
	std::string operator()(DWORD msg, LPARAM lp, WPARAM wp) const noexcept;

private:
	std::unordered_map<DWORD, std::string> map;
};

