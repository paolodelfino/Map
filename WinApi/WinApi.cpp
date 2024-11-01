#include "WinApi.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

short WinApi::GetAsyncKeyState(int vKey)
{
	return ::GetAsyncKeyState(vKey);
}
