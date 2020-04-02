#include "Keyboard.h"
#include "Window.h"

bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept
{
	return keyStates[keycode]; //returns true or false
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keyBuffer.size() > 0u) { //if there is anything on the key buffer
		Keyboard::Event e = keyBuffer.front(); //copy the first thing in the queue
		keyBuffer.pop(); //Pops of the key
		return e; //returns it
	}
	else
	{
		return Keyboard::Event(); //returns invalid event 
	}
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keyBuffer.empty(); 
}

char Keyboard::ReadChar() noexcept
{
	if (charBuffer.size() > 0u) {
		//pulls a character off the queue and returns it
		unsigned char charcode = charBuffer.front(); 
		charBuffer.pop();
		return charcode;
	}
	else
	{
		return 0;
	}
}

void Keyboard::FlushKey() noexcept
{
	keyBuffer = std::queue<Event>(); //Resets the queue
}

bool Keyboard::CharIsEmpty() const noexcept
{
	return charBuffer.empty();
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

void Keyboard::EnableAutorepeat() noexcept
{
	autoRepeatEnabled = true;
}

void Keyboard::DisableAutorepeat() noexcept
{
	autoRepeatEnabled = false;
}

bool Keyboard::AutorepeatEnabled() const noexcept
{
	return autoRepeatEnabled;
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

void Keyboard::ClearState() noexcept
{
	keyStates.reset(); //resets keyStates bit set
}

//Template to work on both the queue<char> and queue<event>
template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > bufferSize) {
		buffer.pop();
	}
}