#include "Timer.h"

float Timer::getDeltaSec()
{
	float deltaSec = 0;

    const time_point<steady_clock> now = steady_clock::now();

	if (lastTimePoint != time_point<steady_clock>::max())
	{
		deltaSec = duration_cast<milliseconds>(now - lastTimePoint).count() / 1000.0f;
	}

	lastTimePoint = now;

	return deltaSec;
}
