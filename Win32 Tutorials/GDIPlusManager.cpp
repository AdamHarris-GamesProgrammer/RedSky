#define FULL_WINDOWS
#include "RedSkyWin.h"
#include "GDIPlusManager.h"
#include <algorithm>

namespace Gdiplus {
	using std::min;
	using std::max;
}

#include <gdiplus.h>

ULONG_PTR GDIPlusManager::token = 0;

int GDIPlusManager::refCount = 0;

GDIPlusManager::GDIPlusManager()
{
	//if ref count is 0 
	if (refCount++ == 0) {
		//setup GDI
		Gdiplus::GdiplusStartupInput input;
		input.GdiplusVersion = 1;
		input.DebugEventCallback = nullptr;
		input.SuppressBackgroundThread = false;
		Gdiplus::GdiplusStartup(&token, &input, nullptr);
	}
}

GDIPlusManager::~GDIPlusManager()
{
	//if refcout equals 0 
	if (--refCount == 0) {
		//destroy GDI
		Gdiplus::GdiplusShutdown(token);
	}
}


