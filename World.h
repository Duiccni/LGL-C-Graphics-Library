#pragma once

#include <Windows.h>

#include "Graphics.h"

inline graphics::surface screen({ 480, 360 });

struct s_mouse
{
	POINT win_pos;
	point pos;
	point old_pos;
	point delta;
	bool in_screen;
	bool left, middle, right;
};

namespace data
{
	inline uint target_fps = 50, target_frame_time = 1000 / target_fps;
	inline uint tick = 0, delta_time = target_frame_time, performance = 0;
	inline bool running = true;

	inline s_mouse mouse = { };
}
