#include "World.h"
#include "Colors.h"
#include "Font.h"

#define _USE_MATH_DEFINES
#include <cmath>

#pragma comment(lib, "Winmm.lib")

LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
	case WM_CLOSE:
		data::running = false;
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		data::running = false;
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProcW(hwnd, u_msg, w_param, l_param);
	}
}

const float float_1div_sqrt3 = 1.0f / sqrtf(3.0f);

constexpr point extra_size = { 16, 39 };

BITMAPINFO bitmap_info;

#pragma warning(disable: 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (font::init())
	{
		MessageBoxW(nullptr, L"Font initialization failed!", L"Error", MB_OK);
		return 0;
	}

	WNDCLASSW wc = {};
	wc.lpfnWndProc = window_proc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"v3";

	if (!RegisterClassW(&wc))
	{
		MessageBoxW(nullptr, L"Window registration failed!", L"Error", MB_OK);
		return 0;
	}

	const HWND window = CreateWindowExW(
		0,
		wc.lpszClassName,
		wc.lpszClassName,
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		screen.size.x + extra_size.x, screen.size.y + extra_size.y,
		nullptr, nullptr, hInstance, nullptr
	);

	if (window == nullptr)
	{
		// Clean up
		UnregisterClassW(wc.lpszClassName, hInstance);

		MessageBoxW(nullptr, L"Window creation failed!", L"Error", MB_OK);
		return 0;
	}

	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = screen.size.x;
	bitmap_info.bmiHeader.biHeight = screen.size.y;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;

	const HDC hdc = GetDC(window);

	ShowWindow(window, nShowCmd);
	UpdateWindow(window);

	FILE* fp = nullptr;
	if constexpr (CONSOLE)
	{
		AllocConsole();
		freopen_s(&fp, "CONOUT$", "w", stdout); // NOLINT(cert-err33-c)
	}

	static MSG msg = { };
	static DWORD d_start_time;

	const graphics::surface cut({ 96, 96 });
	const graphics::surface resize({ 180, 180 });
	// const graphics::surface recursion(screen.size >> 1);
	graphics::fill(0xffff, cut);

	// ReSharper disable CppExpressionWithoutSideEffects
	while (data::running)
	{
		d_start_time = timeGetTime();
		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		GetCursorPos(&data::mouse.win_pos);
		ScreenToClient(window, &data::mouse.win_pos);
		data::mouse.old_pos = data::mouse.pos;
		data::mouse.pos = { data::mouse.win_pos.x, screen.size.y - data::mouse.win_pos.y };
		data::mouse.delta = data::mouse.pos - data::mouse.old_pos;

		graphics::rgb_color(0, 0, 0) >> screen;

		uint* pixel = screen.buffer + 40;
		for (int y = 0; y < 32; y++, pixel += 120)
			for (int x = 0; x < 360; x++, pixel++)
				*pixel = graphics::hsv_to_rgb(x);

		// graphics::cut_surface(screen, cut, { 0, 0 }, true);
		// graphics::blit_surface(screen, cut, { 440, 320 }, false, true);
		// graphics::draw::rect({ -16, -16 }, cut.size - 15, false, true, false, true, colors::white, screen);

		/*
		for (int i = 0; i < 10; i++)
		{
			graphics::resize_surface(screen, recursion);
			graphics::blit_surface(screen, recursion, screen.size >> 2, false, true);
		}
		*/

		const int cache = static_cast<int>(float_1div_sqrt3 * 48.0f);
		graphics::draw::triangle({ 180 - cache, 48 }, { 180 + cache, 48 }, { 180, 96 }, colors::green, screen);

		graphics::draw::line({ 64, 64 }, { 80, 96 }, colors::white, screen);
		graphics::draw::line({ 64, 64 }, { 96, 72 }, colors::white, screen);
		graphics::draw::circle({ 64, 64 }, 5, colors::red, screen);
		graphics::draw::circle({ 80, 96 }, 5, colors::red, screen);
		graphics::draw::circle({ 96, 72 }, 5, colors::red, screen);

		graphics::cut_surface(screen, cut, { 64, 64 }, false);
		resize_surface(cut, resize);
		graphics::blit_surface(screen, resize, { 96, 96 }, false, false);

		graphics::draw::fill_rect_size({ 96, 96 }, { 96, 96 }, colors::red, screen, 0x80);

		graphics::draw::straight_line({ 32, 240 }, { 240, 240 }, colors::white, screen, 1);
		graphics::draw::straight_line({ 32, 241 }, { 240, 241 }, colors::white, screen);

		graphics::draw::straight_line({ 260, 240 }, { 360, 240 }, colors::white, screen, 0, 5);

		font::unsafe_draw_char('#', { 5, 5 }, colors::gray, screen);

		font::draw_string({ 10, 330 }, "Hello, World!", colors::white, screen);
		// font::draw_string({ 10, 330 }, test_text, colors::white, screen);

		StretchDIBits(
			hdc,
			0, 0, screen.size.x, screen.size.y,
			0, 0, screen.size.x, screen.size.y,
			screen.buffer, &bitmap_info,
			DIB_RGB_COLORS, SRCCOPY
		);

		data::performance = timeGetTime() - d_start_time;
		if (data::performance < data::target_frame_time)
		{
			Sleep(data::target_frame_time - data::performance);
			data::delta_time = data::target_frame_time;
		}
		else data::delta_time = data::performance;
		data::tick++;
	}

	ReleaseDC(window, hdc);
	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, hInstance);

	if constexpr (CONSOLE)
	{
		fclose(fp);
		FreeConsole();
	}

	return 0;
}