#pragma once
#include "RedSkyWin.h"
#include "RedSkyException.h"
#include <string>
#include <assert.h>
#include <memory>

class Surface
{
public:
	class Color {
	public:
		unsigned int dword;
	public:
		constexpr Color() noexcept : dword() {}

		constexpr Color(const Color& col) noexcept : dword(col.dword) {}

		constexpr Color(unsigned int dw) noexcept : dword(dw) {}

		constexpr Color(unsigned char x, unsigned char r, unsigned char g, unsigned char b) noexcept : dword((x << 24u) | (r << 16u) | (g << 8u) | b) {}

		constexpr Color(unsigned char r, unsigned char g, unsigned char b) noexcept : dword((r << 16u) | (g << 8u) | b) {}

		constexpr Color(Color col, unsigned char x) noexcept : Color((x << 24u) | col.dword) {}

		Color& operator=(Color color) noexcept {
			dword = color.dword;
			return *this;
		}

		constexpr char GetX() const noexcept { return dword >> 24u; }
		constexpr unsigned char GetA() const noexcept { return GetX(); }
		constexpr unsigned char GetR() const noexcept { return (dword >> 16u) & 0xFFu; }
		constexpr unsigned char GetG() const noexcept { return (dword >> 8u) & 0xFFu; }
		constexpr unsigned char GetB() const noexcept { return dword & 0xFFu; }

		void SetX(unsigned char x) noexcept { dword = (dword & 0xFFFFFFu) | (x << 24u); }
		void SetA(unsigned char a) noexcept { SetX(a); }
		void SetR(unsigned char r) noexcept { dword = (dword & 0xFF00FFFFu) | (r << 16u); }
		void SetG(unsigned char g) noexcept { dword = (dword & 0xFFFF00FFu) | (g << 8u); }
		void SetB(unsigned char b) noexcept { dword = (dword & 0xFFFFFF00u) | b; }
	};

public:
	class Exception : public RedSkyException {
	public:
		Exception(int line, const char* file, std::string note) noexcept : RedSkyException(line, file), note(std::move(note)) {}
		const char* what() const noexcept override;
		const char* GetType() const noexcept override { return "RedSky Graphic Exception"; }
		const std::string& GetNote() const noexcept { return note; }
	private:
		std::string note;
	};
public:
	Surface(unsigned int width, unsigned int height, unsigned int pitch) noexcept :
		pBuffer(std::make_unique<Color[]>(pitch* height)), width(width), height(height) {}

	Surface(unsigned int width, unsigned int height) noexcept : Surface(width, height, width) {}

	Surface(Surface&& source) noexcept : width(source.width), height(source.height), pBuffer(std::move(source.pBuffer)) {}
	Surface(Surface&) = delete;
	Surface& operator=(Surface&& donor) noexcept;
	Surface& operator=(const Surface&) = delete;
	~Surface() {}

	void Clear(Color fillValue) noexcept;
	void PutPixel(unsigned int x, unsigned int y, Color c) noexcept (!IS_DEBUG);
	Color GetPixel(unsigned int x, unsigned int y) const noexcept (!IS_DEBUG);

	unsigned int GetWidth() const noexcept { return width; }
	unsigned int GetHeight() const noexcept { return height; }
	Color* GetBufferPtr() noexcept { return pBuffer.get(); }
	const Color* GetBufferPtr() const noexcept { return pBuffer.get(); }

	static Surface FromFile(const std::string& name);
	void Save(const std::string& fileName);
	void Copy(const Surface& src) noexcept (!IS_DEBUG);

private:
	Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBufferParam) noexcept : width(width), height(height), pBuffer(std::move(pBufferParam)) {}
private:
	std::unique_ptr<Color[]> pBuffer;
	unsigned int width;
	unsigned int height;
	
};

