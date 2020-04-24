#pragma once
#include <queue>
#include <optional>

class Mouse
{
	friend class Window;
public:
	//struct for the raw information
	struct RawDelta {
		int x, y;
	};

	class Event {
	public:
		enum class Type {
			LPress,
			LRelease,
			RPress,
			RRelease,
			WheelUp,
			WheelDown,
			Move,
			Enter,
			Leave,
		};
	private:
		Type type;
		bool leftIsPressed;
		bool rightIsPressed;
		int x;
		int y;
	public:
		Event(Type type, const Mouse& parent) noexcept :
			type(type),
			leftIsPressed(parent.leftIsPressed),
			rightIsPressed(parent.rightIsPressed),
			x(parent.x),
			y(parent.y) {}

		Type GetType() const noexcept { return type; }
		std::pair<int, int> GetPos() const noexcept { return { x,y }; }
		int GetX() const noexcept { return x; }
		int GetY() const noexcept { return y; }
		bool LeftIsPressed() const noexcept { return leftIsPressed; }
		bool RightIsPressed() const noexcept { return rightIsPressed; }
	};
public:
	//Constructor handlers
	Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;

	//Getters
	std::pair<int, int>GetPos() const noexcept { return { x,y }; }
	int GetPosX() const noexcept { return x; }
	int GetPosY() const noexcept { return y; }
	bool IsInWindow() const noexcept { return isInWindow; }
	bool LeftIsPressed() const noexcept { return leftIsPressed; }
	bool RightIsPressed() const noexcept { return rightIsPressed; }
	bool IsEmpty() const noexcept { return buffer.empty(); }
	std::optional<RawDelta> ReadRawDelta() noexcept;
	//Reads the mouse event queue and places events into the buffer
	std::optional<Mouse::Event> Read() noexcept;

	//Mouse Raw Input Methods
	void EnableRaw() noexcept { rawEnabled = true; }
	void DisableRaw() noexcept { rawEnabled = false; }
	bool RawEnabled() const noexcept { return rawEnabled; }

	//Clears the buffer of events
	void Flush() noexcept;
private:
	//Event Handlers
	void OnMouseMove(int newX, int newY) noexcept;
	void OnMouseLeave() noexcept { isInWindow = false; }
	void OnMouseEnter() noexcept { isInWindow = true; }
	void OnRawDelta(int dx, int dy) noexcept;
	void OnLeftPressed(int x, int y) noexcept;
	void OnLeftReleased(int x, int y) noexcept;
	void OnRightPressed(int x, int y) noexcept;
	void OnRightReleased(int x, int y) noexcept;
	void OnWheelUp(int x, int y) noexcept;
	void OnWheelDown(int x, int y) noexcept;
	void OnWheelDelta(int x, int y, int delta) noexcept;

	//Reduces the buffer down to the bufferSize variable the buffer from growing to ridiculous sizes
	void TrimBuffer() noexcept;
	void TrimRawInputBuffer() noexcept;
private:
	//Size of the buffer
	static constexpr unsigned int bufferSize = 16u;

	//Positional Data
	int x;
	int y;
	bool isInWindow = false;

	//Button/Wheel Data
	bool rawEnabled = false;
	bool leftIsPressed = false;
	bool rightIsPressed = false;
	int wheelDeltaCarry = 0;

	//Event buffer
	std::queue<Event> buffer;
	std::queue<RawDelta> rawDeltaBuffer;
};

