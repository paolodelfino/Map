#include <iostream>
#include <raylib.h>
#include "style_dark.h"
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#include <ViGEm/Client.h>
#include "../MouHidMonitor/driver.h"
#include "../WinApi/WinApi.h"
#include <algorithm>
#include <limits>
#undef min
#undef max
#include <string>

#pragma comment(lib, "setupapi.lib")

#define RETURN_DEFER(status_value)\
{\
	status = status_value;\
	goto defer;\
}

template <class _Ty>
struct GVector2 {
	_Ty x;
	_Ty y;
};

constexpr int fps = 144;
static int sleep_time = ceil(1000.0f / fps);
constexpr float initial_ms_before_reset = 50;
static float ms_before_reset = initial_ms_before_reset;
static int frame_count;
static int frames_before_reset = ceil((double)ms_before_reset / sleep_time);

PVIGEM_CLIENT vigem_client;
PVIGEM_TARGET vigem_target;

static GET_INPUT_PACKET_REPLY last_input_packet_reply;
static GET_INPUT_PACKET_REPLY input_packet_reply;
static XUSB_REPORT xusb_report;

constexpr GVector2<float> initial_sens{ 160, 220 };
static GVector2<float> sens = initial_sens;
static GVector2<short> rthumb_mov_acc;

int HandleKeyboard();
int HandleMouse();
void Drawing();

int main()
{
	int status(0);

	if (!DrvInitialization())
	{
		std::cerr << "DrvInitialization() failed" << std::endl;
		RETURN_DEFER(1);
	}

	if (!DrvEnableMouHidInputMonitor())
	{
		std::cerr << "DrvEnableMouHidInputMonitor() failed" << std::endl;
		RETURN_DEFER(1);
	}

	vigem_client = vigem_alloc();

	if (vigem_client == nullptr)
	{
		std::cerr << "Could not allocate vigem client" << std::endl;
		RETURN_DEFER(1);
	}

	vigem_target = vigem_target_x360_alloc();

	if (vigem_target == nullptr)
	{
		std::cerr << "Could not allocate vigem target" << std::endl;
		RETURN_DEFER(1);
	}

	VIGEM_ERROR vigem_error;

	if (!VIGEM_SUCCESS((vigem_error = vigem_connect(vigem_client))))
	{
		std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << vigem_error << std::endl;
		RETURN_DEFER(1);
	}

	if (!VIGEM_SUCCESS((vigem_error = vigem_target_add(vigem_client, vigem_target))))
	{
		std::cerr << "vigem_target_add() failed with error code: 0x" << std::hex << vigem_error << std::endl;
		RETURN_DEFER(1);
	}

	InitWindow(1280, 720, "");

	SetTargetFPS(fps);

	GuiLoadStyleDark();

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);

		if (HandleKeyboard() != 0)
		{
			std::cerr << "HandleKeyboard() failed" << std::endl;
			RETURN_DEFER(1);
		}

		if (HandleMouse() != 0)
		{
			std::cerr << "HandleMouse() failed" << std::endl;
			RETURN_DEFER(1);
		}

		if (!VIGEM_SUCCESS((vigem_error = vigem_target_x360_update(vigem_client, vigem_target, xusb_report))))
		{
			std::cerr << "vigem_target_x360_update() failed with error code: 0x" << std::hex << vigem_error << std::endl;
			RETURN_DEFER(1);
		}

		Drawing();

		EndDrawing();
	}

	CloseWindow();

defer:
	if (!DrvDisableMouHidInputMonitor())
	{
		std::cerr << "DrvDisableMouHidInputMonitor() failed" << std::endl;
		status = 1;
	}

	DrvTermination();

	if (!VIGEM_SUCCESS((vigem_error = vigem_target_remove(vigem_client, vigem_target))))
	{
		std::cerr << "vigem_target_remove() failed with error code: 0x" << std::hex << vigem_error << std::endl;
		status = 1;
	}
	vigem_target_free(vigem_target);

	vigem_disconnect(vigem_client);
	vigem_free(vigem_client);

	return status;
}

int HandleKeyboard()
{
	xusb_report.sThumbLY = 0, xusb_report.sThumbLX = 0;

	if (WinApi::GetAsyncKeyState(0x57) & 0x8000) xusb_report.sThumbLY = MAXSHORT;
	else if (WinApi::GetAsyncKeyState(0x53) & 0x8000) xusb_report.sThumbLY = MINSHORT;

	if ((WinApi::GetAsyncKeyState(0x41) & 0x8000) != 0) xusb_report.sThumbLX = MINSHORT;
	else if ((WinApi::GetAsyncKeyState(0x44) & 0x8000) != 0) xusb_report.sThumbLX = MAXSHORT;

	return 0;
}

int HandleMouse()
{
	if (!DrvGetInputPacket(&input_packet_reply))
	{
		std::cerr << "DrvGetInputPacket() failed" << std::endl;
		return 1;
	}

	if (last_input_packet_reply.PacketIndex == input_packet_reply.PacketIndex)
	{
		if (frames_before_reset < frame_count++)
		{
			frame_count = 0;
			rthumb_mov_acc.x = 0;
			rthumb_mov_acc.y = 0;
			xusb_report.sThumbRX = 0;
			xusb_report.sThumbRY = 0;
		}
		//return 0;
	}
	else
	{
		frame_count = 0;
		last_input_packet_reply = input_packet_reply;

		long xp = input_packet_reply.LastX, yp = input_packet_reply.LastY;

		//printf("%ld %ld\n", xp, yp);

		long xt = (xp * sens.x) + rthumb_mov_acc.x;
		long yt = (-yp * sens.y) + rthumb_mov_acc.y;

		//printf("xp=%ld yp=%ld xt=%ld yt=%ld ax=%hd ay=%hd\n", xp, yp, xt, yt, rthumb_mov_acc.x, rthumb_mov_acc.y);

		rthumb_mov_acc.x = static_cast<short>(std::clamp(xt, static_cast<long>(std::numeric_limits<short>::min()), static_cast<long>(std::numeric_limits<short>::max())));
		rthumb_mov_acc.y = static_cast<short>(std::clamp(yt, static_cast<long>(std::numeric_limits<short>::min()), static_cast<long>(std::numeric_limits<short>::max())));

		xusb_report.sThumbRX = rthumb_mov_acc.x;
		xusb_report.sThumbRY = rthumb_mov_acc.y;
	}

	//printf("Packet Index=%llu LX=%ld LY=%ld AX=%hd XY=%hd sx=%hd sy=%hd\n", input_packet_reply.PacketIndex, input_packet_reply.LastX, input_packet_reply.LastY, rthumb_mov_acc.x, rthumb_mov_acc.y, sens.x, sens.y);

	return 0;
}

void Drawing()
{
	DrawFPS(0, 0);

	float multiplier = 80.0f;

	float x = (float)xusb_report.sThumbRX / std::numeric_limits<USHORT>::max() * multiplier;
	float y = -(float)xusb_report.sThumbRY / std::numeric_limits<USHORT>::max() * multiplier;

	int w = (int)(1 * multiplier), h = w;

	int offset = 40;

	DrawRectangleLines(offset, offset, w, h, WHITE);

	DrawCircle((int)x + offset + w / 2, (int)y + offset + h / 2, multiplier * 1 / 40, WHITE);

	int gap = 30;

	DrawCircleLines(offset + w + gap + w / 2, offset + h / 2, w / 2, WHITE);

	float scale = 0.8f;

	float scaleCoord = 0.3f;

	DrawCircleLines((int)((float)offset + w + gap + (float)w / 2 + x * scaleCoord), (int)((float)offset + (float)h / 2 + y * scaleCoord), w * scale / 2, WHITE);

	std::string sens_x_str = std::to_string(sens.x);
	GuiSliderBar(Rectangle{ static_cast<float>(offset) + 40, static_cast<float>(offset) + h + gap, 100, 40 }, "sens x", sens_x_str.c_str(), &sens.x, 0, 1000);

	if (GuiButton(Rectangle{ static_cast<float>(offset) + 40 + 100 + 15 + 76, static_cast<float>(offset) + h + gap, 55, 40 }, "Reset")) {
		sens.x = initial_sens.x;
	}

	std::string sens_y_str = std::to_string(sens.y);
	GuiSliderBar(Rectangle{ static_cast<float>(offset) + 40, static_cast<float>(offset) + h + gap + 40, 100, 40 }, "sens y", sens_y_str.c_str(), &sens.y, 0, 1000);

	if (GuiButton(Rectangle{ static_cast<float>(offset) + 40 + 100 + 15 + 76, static_cast<float>(offset) + h + gap + 40, 55, 40 }, "Reset")) {
		sens.y = initial_sens.y;
	}

	static bool keep_ratio = true;
	GuiCheckBox(Rectangle{ 40, static_cast<float>(offset) + h + gap + 40 + 40 + 10, 20, 20 }, "keep ratio", &keep_ratio);

	if (keep_ratio)
	{
		sens.y = sens.x * initial_sens.y / initial_sens.x;
	}

	std::string ms_before_reset_str = std::to_string(ms_before_reset);
	GuiSliderBar(Rectangle{ 130, static_cast<float>(offset) + h + gap + 40 + 40 + 10 + 20, 100, 40 }, "ms before reset", ms_before_reset_str.c_str(), &ms_before_reset, 0, 1000);
	std::string frames_before_reset_str = "frames before reset: " + std::to_string(frames_before_reset);
	GuiDrawText(frames_before_reset_str.c_str(), Rectangle{ 40, static_cast<float>(offset) + h + gap + 40 + 40 + 10 + 20 + 40, 300, 20 }, TEXT_ALIGN_LEFT, WHITE);

	if (GuiButton(Rectangle{ static_cast<float>(offset) + 40 + 100 + 15 + 100, static_cast<float>(offset) + h + gap + 40 + 40 + 10 + 20, 55, 40 }, "Reset")) {
		ms_before_reset = initial_ms_before_reset;
	}

	if (GuiButton(Rectangle{ static_cast<float>(offset) + 40 + 100 + 15 + 100 + 55, static_cast<float>(offset) + h + gap + 40 + 40 + 10 + 20, 55, 40 }, "Apply")) {
		frames_before_reset = ceil((double)ms_before_reset / sleep_time);
		frame_count = 0;
	}
}