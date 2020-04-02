#pragma once

#include <queue>
#include <bitset>

class Keyboard
{
	friend class Window;
public:
	//Event class holds the requirements for each key, pressed released and invalid, and keycode as well as getters for each of those
	class Event {
	public:
		enum class Type {
			Press,
			Release,
			Invalid
		};
	private:
		Type type;
		unsigned char code;
	public:
		Event() noexcept : type(Type::Invalid), code(0u) {}
		Event(Type type, unsigned char code) noexcept : type(type), code(code) {}

		bool IsPress() const noexcept { return type == Type::Press; }
		bool IsRelease() const noexcept { return type == Type::Release; }
		bool IsValid() const noexcept { return type == Type::Invalid; }
		unsigned char GetCode() const noexcept { return code; }
	};
public:
	Keyboard() = default; //Default copy constructor
	Keyboard(const Keyboard&) = delete; //Copy constructor
	Keyboard& operator=(const Keyboard&) = delete; //Copy constructor

	//Key Events
	bool KeyIsPressed(unsigned char keycode) const noexcept;
	Event ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void FlushKey() noexcept;

	//Char Events
	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept;

	//Autorepeat
	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatEnabled() const noexcept;

private:
	void OnKeyPressed(unsigned char keycode) noexcept;
	void OnKeyReleased(unsigned char keycode) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	bool autoRepeatEnabled = false;
	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;
};



