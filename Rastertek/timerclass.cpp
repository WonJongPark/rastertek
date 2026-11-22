///////////////////////////////////////////////////////////////////////////////
// Filename: timerclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "timerclass.h"

bool TimerClass::Initialize()
{
	INT64 frequency;


	// 이 시스템의 초당 사이클 수(속도)를 가져옴.
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)
	{
		return false;
	}

	// 부동소수점으로 저장.
	m_frequency = (float)frequency;

	// 초기 시작 시간을 가져옴.
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}

void TimerClass::Frame()
{
	INT64 currentTime;
	INT64 elapsedTicks;


	// 현재 시간을 조회.
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	// 마지막으로 현재 시간을 조회한 이후의 시간 차이를 계산.
	elapsedTicks = currentTime - m_startTime;

	// 프레임 시간을 계산.
	m_frameTime = (float)elapsedTicks / m_frequency;

	// 타이머를 다시 시작.
	m_startTime = currentTime;
}

float TimerClass::GetTime()
{
	return m_frameTime;
}
