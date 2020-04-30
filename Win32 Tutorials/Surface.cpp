#define FULL_WINDOWS
#include "Surface.h"
#include <algorithm>
namespace Gdiplus {
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#include <sstream>

#pragma comment(lib, "gdiplus.lib")

void Surface::Clear(Color fillValue) noexcept
{
	memset(pBuffer.get(), fillValue.dword, width * height * sizeof(Color));
}

void Surface::PutPixel(unsigned int x, unsigned int y, Color c) noxnd
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < width);
	assert(y < height);
	pBuffer[y * width + x] = c;
}

Surface::Color Surface::GetPixel(unsigned int x, unsigned int y) const noxnd
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < width);
	assert(y < height);
	return pBuffer[y * width + x];
}

Surface Surface::FromFile(const std::string& name)
{
	unsigned int width = 0;
	unsigned int height = 0;
	std::unique_ptr<Color[]> pBuffer;

	bool alphaLoaded = false;
	{
		// convert filename to wide string (for Gdiplus)
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, name.c_str(), _TRUNCATE);

		Gdiplus::Bitmap bitmap(wideName);
		if (bitmap.GetLastStatus() != Gdiplus::Status::Ok)
		{
			std::stringstream ss;
			ss << "Loading image [" << name << "]: failed to load.";
			throw Exception(__LINE__, __FILE__, ss.str());
		}

		width = bitmap.GetWidth();
		height = bitmap.GetHeight();
		pBuffer = std::make_unique<Color[]>(width * height);

		for (unsigned int y = 0; y < height; y++)
		{
			for (unsigned int x = 0; x < width; x++)
			{
				Gdiplus::Color c;
				bitmap.GetPixel(x, y, &c);
				pBuffer[y * width + x] = c.GetValue();
				if (c.GetAlpha() != 255) {
					alphaLoaded = true;
				}
			}
		}
	}

	return Surface(width, height, std::move(pBuffer), alphaLoaded);
}

void Surface::Copy(const Surface& src) noxnd
{
	assert(width == src.width);
	assert(height == src.height);
	memcpy(pBuffer.get(), src.pBuffer.get(), width * height * sizeof(Color));
}

Surface& Surface::operator=(Surface&& donor) noexcept
{
	width = donor.width;
	height = donor.height;
	pBuffer = std::move(donor.pBuffer);
	donor.pBuffer = nullptr;
	return *this;
}

#pragma region Suface Exception Class
const char* Surface::Exception::what() const noexcept
{
	std::ostringstream oss;
	oss << RedSkyException::what() << std::endl
		<< "[Note] " << GetNote();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}
#pragma endregion
