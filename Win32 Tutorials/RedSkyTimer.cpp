#include "RedSkyTimer.h"

using namespace std::chrono;

RedSkyTimer::RedSkyTimer() noexcept
{
	last = steady_clock::now();
}

//Mark gives a time elapsed since the last time mark was called, resets mark point
float RedSkyTimer::Mark() noexcept
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

//Peek returns the last time mark was called without resetting mark
float RedSkyTimer::Peek() const noexcept
{
	return duration<float>(steady_clock::now() - last).count();
}
