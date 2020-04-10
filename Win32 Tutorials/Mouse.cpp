#include "Mouse.h"
#include "RedSkyWin.h"

//Reads the mouse 
std::optional<Mouse::Event> Mouse::Read() noexcept
{
	if (buffer.size() > 0u) {
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	return {};
}

//Clears the buffer
void Mouse::Flush() noexcept
{
	buffer = std::queue<Event>();
}

//Movement event
void Mouse::OnMouseMove(int newX, int newY) noexcept
{
	x = newX; 
	y = newY;

	buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
	TrimBuffer();
}


//Left button events
void Mouse::OnLeftPressed(int x, int y) noexcept
{
	leftIsPressed = true;
	
	buffer.push(Mouse::Event(Event::Type::LPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftReleased(int x, int y) noexcept
{
	leftIsPressed = false;

	buffer.push(Mouse::Event(Event::Type::LRelease, *this));
	TrimBuffer();
}

//Right button events
void Mouse::OnRightPressed(int x, int y) noexcept
{
	rightIsPressed = true;

	buffer.push(Mouse::Event(Event::Type::RPress, *this));
	TrimBuffer();
}

void Mouse::OnRightReleased(int x, int y) noexcept
{
	rightIsPressed = false;

	buffer.push(Mouse::Event(Event::Type::RRelease, *this));
	TrimBuffer();
}

//Mouse Wheel events
void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Event::Type::WheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Event::Type::WheelDown, *this));
	TrimBuffer();
}

//This method reduces the size of the buffer until it is the correct size
void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize) {
		buffer.pop();
	}
}

void Mouse::OnWheelDelta(int x, int y, int delta) noexcept {
	wheelDeltaCarry += delta;

	while (wheelDeltaCarry >= WHEEL_DELTA) 
	{
		wheelDeltaCarry -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (wheelDeltaCarry <= -WHEEL_DELTA) {
		wheelDeltaCarry += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}
