#include "Keyboard.h"
#include "Window.h"

std::optional<Keyboard::Event> Keyboard::ReadKey() noexcept
{
	if (keyBuffer.size() > 0u) { //if there is anything on the key buffer
		Keyboard::Event e = keyBuffer.front(); //copy the first thing in the queue
		keyBuffer.pop(); //Pops of the key
		return e; //returns it
	}
	return {};
}


std::optional<char> Keyboard::ReadChar() noexcept
{
	if (charBuffer.size() > 0u) {
		//pulls a character off the queue and returns it
		unsigned char charcode = charBuffer.front(); 
		charBuffer.pop();
		return charcode;
	}
	return {};
}

void Keyboard::FlushKey() noexcept
{
	keyBuffer = std::queue<Event>(); //Resets the queue
}

void Keyboard::FlushChar() noexcept
{
	charBuffer = std::queue<char>(); //Resets the queue
}

void Keyboard::Flush() noexcept
{
	//Resets both queues
	FlushKey();
	FlushChar();
}


//Windows class uses this
void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	//updates keystate and keybuffer to pressed
	keyStates[keycode] = true;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
	TrimBuffer(keyBuffer); //limits the key buffer size in case it goes over size
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keyStates[keycode] = false;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
	TrimBuffer(keyBuffer);
}

void Keyboard::OnChar(char character) noexcept
{
	//push onto queue and trims
	charBuffer.push(character);
	TrimBuffer(charBuffer);
}


//Template to work on both the queue<char> and queue<event>
template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > bufferSize) {
		buffer.pop();
	}
}