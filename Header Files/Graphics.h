#pragma once

#include "Point.h"
#include <numeric>
#include "Definations.h"

namespace graphics
{
	// Checked!
	inline uint rgb_color(const uint8_t& r, const uint8_t& g, const uint8_t& b)
	{
		static uint color;
		static auto color_ptr = reinterpret_cast<uint8_t*>(&color);
		color_ptr[0] = b;
		color_ptr[1] = g;
		color_ptr[2] = r;
		return color;
	}

	struct surface
	{
		uint* buffer = nullptr, * end = nullptr;
		point size;
		size_t buffer_size;

		surface(cpr size, const bool& create_buffer);
		~surface();
	};

	inline surface::surface(cpr size, const bool& create_buffer = true) : size{ size }
	{
		buffer_size = static_cast<size_t>(size.x) * size.y;
		if (create_buffer)
		{
			buffer = static_cast<uint*>(malloc(buffer_size << 2));
			end = buffer + buffer_size;
		}
	}

	inline surface::~surface()
	{
		free(buffer);
		buffer = nullptr;
	}

	inline void copy(SRC_DEST)
	{
		if (src.size != dest.size)
			return;
		for (uint* src_pixel = src.buffer, *dest_pixel = dest.buffer; src_pixel < src.end; src_pixel++, dest_pixel++)
			*dest_pixel = *src_pixel;
	}

	inline void operator >>(SRC_DEST) { copy(src, dest); }

	inline void fill(COLOR, SURF)
	{
		for (uint* pixel = surf.buffer; pixel < surf.end; pixel++)
			*pixel = color;
	}

	inline void clear(SURF) { fill(0, surf); }

	inline void operator >>(COLOR, SURF) { fill(color, surf); }

	inline void clamp_to_surface(point& p, SURF)
	{
		CLAMP_EQ(p.x, 0, surf.size.x);
		CLAMP_EQ(p.y, 0, surf.size.y);
	}

	// Checked!
	inline uint hsv_to_rgb(int h, const uint8_t& s, const uint8_t& v)
	{
		h = MODULO(h, 360);

		const uint8_t c = v * s / UINT8_MAX;
		const uint8_t x = c - c * fabsf(fmodf(static_cast<float>(h) * float_1div60, 2.0f) - 1.0f);

		static uint color;
		static auto b = reinterpret_cast<uint8_t*>(&color);
		b[0] = v - c;
		b[1] = b[0];
		b[2] = b[0];

		const uint8_t i = (h / 60 + 1) % 6;

		b[2 - (i >> 1)] += c;
		b[i % 3] += x;

		return color;
	}

	// Checked!
	inline uint hsv_to_rgb(int h)
	{
		h = MODULO(h, 360);

		const uint8_t x = UINT8_MAX - UINT8_MAX * fabsf(fmodf(static_cast<float>(h) * float_1div60, 2.0f) - 1.0f);

		static uint color;
		static auto b = reinterpret_cast<uint8_t*>(&color);
		color = 0;

		const uint8_t i = (h / 60 + 1) % 6;

		b[2 - (i >> 1)] = UINT8_MAX;
		b[i % 3] = x;

		return color;
	}

	inline bool is_inside(cpr pos, cpr start, cpr end)
	{
		return pos.x >= start.x && pos.y >= start.y && pos.x < end.x && pos.y < end.y;
	}

	inline bool is_inside_size(cpr pos, cpr start, cpr size)
	{
		return is_inside(pos, start, start + size);
	}

	inline bool is_inside(cpr pos, cpr lim)
	{
		return pos.x >= 0 && pos.y >= 0 && pos.x < lim.x && pos.y < lim.y;
	}

	inline bool is_inside(cpr pos, SURF) { return is_inside(pos, surf.size); }

	inline uint* get_raw_pixel(cpr pos, const surface& surf)
	{
		return surf.buffer + pos.x + pos.y * surf.size.x;
	}

	inline uint* get_pixel(cpr pos, SURF)
	{
		return is_inside(pos, surf) ? get_raw_pixel(pos, surf) : nullptr;
	}

	inline void set_sure_pixel(cpr pos, COLOR, SURF)
	{
		*get_raw_pixel(pos, surf) = color;
	}

	inline void set_pixel(cpr pos, COLOR, SURF)
	{
		if (is_inside(pos, surf)) *get_raw_pixel(pos, surf) = color;
	}

	inline bool straighten_line(cpr start, point& end)
	{
		if (abs(end.y - start.y) > abs(end.x - start.x))
		{
			end.x = start.x;
			return true;
		}
		end.y = start.y;
		return false;
	}

	// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
	// Checked!
	void resize_surface(SRC_DEST)
	{
		if (src.size == dest.size) return src >> dest;

		point main_step = src.size / dest.size;
		--main_step.y *= src.size.x;
		main_step.y++;

		point error = { 0, 0 };
		const uint* src_pixel = src.buffer;
		const point error_value = src.size % dest.size;

		for (uint* dest_pixel = dest.buffer; dest_pixel < dest.end; src_pixel += main_step.y)
		{
			error.x = 0;
			for (const uint* x_end = dest_pixel + dest.size.x; dest_pixel < x_end; dest_pixel++,
				src_pixel += main_step.x)
			{
				*dest_pixel = *src_pixel;

				if (error.x >= dest.size.x)
				{
					src_pixel++;
					error.x -= dest.size.x;
				}
				error.x += error_value.x;
			}

			if (error.y >= dest.size.y)
			{
				src_pixel += src.size.x;
				error.y -= dest.size.y;
			}
			error.y += error_value.y;
		}
	}

	inline surface* create_resize_surface(SURF, cpr new_size)
	{
		const auto new_surface = new surface(new_size);
		resize_surface(surf, *new_surface);
		return new_surface;
	}

	// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile CppInconsistentNaming
	// Checked!
	void _blit_cut_surface(const surface& bs, const surface& ss, cpr pos, const uint8_t& alpha, const bool& check)
	{
		point start = pos, size = ss.size;

		if (check)
		{
			size += pos;
			clamp_to_surface(start, bs);
			clamp_to_surface(size, bs);
			size -= start;

			if (size.x == 0 || size.y == 0) return;
		}

		uint* bs_pixel = get_raw_pixel(start, bs), * ss_pixel = get_raw_pixel(start - pos, ss);

		const point y_step = { bs.size.x - size.x, ss.size.x - size.x };

		uint*& src = alpha ? ss_pixel : bs_pixel;
		uint*& dest = alpha ? bs_pixel : ss_pixel;

		if (alpha & 0b1)
		{
			for (const uint* ss_end = ss_pixel + ss.size.x * size.y; ss_pixel < ss_end; ss_pixel += y_step.y,
				bs_pixel += y_step.x)
				for (const uint* ss_x_end = ss_pixel + size.x; ss_pixel < ss_x_end; ss_pixel++, bs_pixel++)
					if (*src)
						*dest = *src;
			return;
		}

		for (const uint* ss_end = ss_pixel + ss.size.x * size.y; ss_pixel < ss_end; ss_pixel += y_step.y,
			bs_pixel += y_step.x)
			for (const uint* ss_x_end = ss_pixel + size.x; ss_pixel < ss_x_end; ss_pixel++, bs_pixel++)
				*dest = *src;
	}

	// Checked!
	inline void blit_surface(DEST_SRC, cpr pos, const bool& alpha = true, const bool& check = true)
	{
		_blit_cut_surface(dest, src, pos, alpha | 0b10, check);
	}

	// Checked!
	inline void cut_surface(SRC_DEST, cpr pos, const bool& check = true)
	{
		_blit_cut_surface(src, dest, pos, false, check);
	}

	namespace draw
	{
		void fill_rect(point start, point end, COLOR, SURF, const uint8_t& alpha = UINT8_MAX)
		{
			clamp_to_surface(start, surf);
			clamp_to_surface(end, surf);

			swap_point_if(start, end);

			uint* pixel = get_raw_pixel(start, surf);
			point size = end - start;
			int y_step = surf.size.x - size.x;

			if (alpha == UINT8_MAX)
			{
				for (const uint* y_end = pixel + size.y * surf.size.x; pixel < y_end; pixel += y_step)
					for (const uint* x_end = pixel + size.x; pixel < x_end; pixel++)
						*pixel = color;
				return;
			}

			y_step <<= 2;
			size <<= 2;

			auto pixel_ptr = reinterpret_cast<uint8_t*>(pixel);
			const auto color_ptr = reinterpret_cast<const uint8_t*>(&color);

			for (const uint8_t* y_end = pixel_ptr + size.y * surf.size.x; pixel_ptr < y_end; pixel_ptr += y_step)
				for (const uint8_t* x_end = pixel_ptr + size.x; pixel_ptr < x_end; pixel_ptr += 4)
				{
					SLIDE_INT_EQ(pixel_ptr[0], color_ptr[0], alpha, UINT8_MAX);
					SLIDE_INT_EQ(pixel_ptr[1], color_ptr[1], alpha, UINT8_MAX);
					SLIDE_INT_EQ(pixel_ptr[2], color_ptr[2], alpha, UINT8_MAX);
				}
		}

		void _straight_line(int d1, int d2, const int& s, const bool& slope, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			if (s < 0 || d1 == d2) return;

			if (thickness > 1)
			{
				const int ts = thickness >> 1, te = ts + (thickness & 1);
				return slope
					? fill_rect({ s - te, d1 }, { s + ts, d2 }, color, surf)
					: fill_rect({ d1, s - te }, { d2, s + ts }, color, surf);
			}

			const point size = slope ? ~surf.size : surf.size;

			if (s >= surf.size.x) return;

			int cache = d1;
			CLAMP_EQ(d1, 0, size.x);
			CLAMP_EQ(d2, 0, size.x);

			if (d1 == d2) return;
			if (d1 > d2) std::swap(d1, d2);

			point steps = { 1, surf.size.x };
			if (slope) steps = ~steps;

			uint* pixel = surf.buffer + s * steps.y;
			const uint* end = pixel + d2 * steps.x;
			pixel += d1 * steps.x;

			if (dash)
			{
				const int d_dash = dash << 1;
				cache = (d1 - cache) % d_dash;
				int index = 1;
				const int dash_step = dash * steps.x;

				if (cache > dash)
					pixel += (d_dash - cache) * steps.x;
				else
					index = cache + 1;

				while (pixel < end)
				{
					*pixel = color;
					pixel += steps.x;
					if (index == dash)
					{
						index = 1;
						pixel += dash_step;
					}
					else index++;
				}

				return;
			}

			while (pixel < end)
			{
				*pixel = color;
				pixel += steps.x;
			}
		}

		inline void x_line(const int& x1, const int& x2, const int& y, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			_straight_line(x1, x2, y, false, color, surf, dash, thickness);
		}

		inline void y_line(const int& y1, const int& y2, const int& x, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			_straight_line(y1, y2, x, true, color, surf, dash, thickness);
		}

		inline void straight_line(cpr start, cpr end, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			if (end.x == start.x)
				_straight_line(start.y, end.y, start.x, true, color, surf, dash, thickness);
			else if (end.y == start.y)
				_straight_line(start.x, end.x, start.y, false, color, surf, dash, thickness);
		}

		void line(point start, point end, COLOR, SURF)
		{
			point delta = end - start;

			if (delta.x == 0)
			{
				if (delta.y != 0)
					_straight_line(start.y, end.y, start.x, true, color, surf);
				return;
			}

			if (delta.y == 0)
			{
				if (delta.x != 0)
					_straight_line(start.x, end.x, start.y, false, color, surf);
				return;
			}

			const bool slope = abs(delta.y) > abs(delta.x);

			if (slope)
			{
				std::swap(start.x, start.y);
				std::swap(end.x, end.y);
			}

			if (start.x > end.x)
			{
				std::swap(start.x, end.x);
				std::swap(start.y, end.y);
			}

			delta = end - start;
			int error = delta.x >> 1;
			point step = { 1, GET_SIGN(delta.y) };
			delta.y = abs(delta.y);
			static uint* pixel;

			if (slope)
			{
				pixel = get_raw_pixel(~start, surf);
				step.x = surf.size.x;
			}
			else
			{
				pixel = get_raw_pixel(start, surf);
				step.y *= surf.size.x;
			}

			for (const uint* end_pixel = pixel + step.x * delta.x + step.y * delta.y; pixel < end_pixel;
				pixel += step. x)
			{
				*pixel = color;
				error -= delta.y;

				if (error < 0)
				{
					pixel += step.y;
					error += delta.x;
				}
			}
		}

		void circle(cpr center, const int& radius, COLOR, SURF)
		{
			point diff = { 0, radius };
			int d = 3 - (radius << 1);

			while (diff.y >= diff.x)
			{
				set_pixel({ center.x - diff.x, center.y - diff.y }, color, surf);
				set_pixel({ center.x - diff.x, center.y + diff.y }, color, surf);
				set_pixel({ center.x + diff.x, center.y - diff.y }, color, surf);
				set_pixel({ center.x + diff.x, center.y + diff.y }, color, surf);

				set_pixel({ center.x - diff.y, center.y - diff.x }, color, surf);
				set_pixel({ center.x - diff.y, center.y + diff.x }, color, surf);
				set_pixel({ center.x + diff.y, center.y - diff.x }, color, surf);
				set_pixel({ center.x + diff.y, center.y + diff.x }, color, surf);

				if (d < 0)
					d += (diff.x++ << 2) + 6;
				else
					d += 4 * (diff.x++ - diff.y--) + 10;
			}
		}

		void circle(cpr center, const int& inner, const int& outer, COLOR, SURF)
		{
			int xo = outer, xi = inner, y = 0, erro = 1 - xo, erri = 1 - xi;

			while (xo >= y)
			{
				x_line(center.x - xo, center.x - xi, center.y - y, color, surf);
				y_line(center.y - xo, center.y - xi, center.x - y, color, surf);
				x_line(center.x - xo, center.x - xi, center.y + y, color, surf);
				y_line(center.y - xo, center.y - xi, center.x + y, color, surf);

				x_line(center.x + xi, center.x + xo, center.y - y, color, surf);
				y_line(center.y + xi, center.y + xo, center.x - y, color, surf);
				x_line(center.x + xi, center.x + xo, center.y + y, color, surf);
				y_line(center.y + xi, center.y + xo, center.x + y, color, surf);

				if (erro < 0)
					erro += (++y << 1) + 1;
				else
					erro += 2 * (++y - --xo + 1);

				if (y > inner)
					xi = y;
				else if (erri < 0)
					erri += (y << 1) + 1;
				else
					erri += 2 * (y - --xi + 1);
			}
		}

		inline void circle(cpr center, const int& radius, const int& thickness,
			const bool& inner, COLOR, SURF)
		{
			if (inner)
				circle(center, radius - thickness, radius, color, surf);
			else
				circle(center, radius, radius + thickness, color, surf);
		}

		inline void rect(cpr start, cpr end, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			x_line(start.x, end.x, start.y, color, surf, dash, thickness);
			x_line(start.x, end.x, end.y, color, surf, dash, thickness);
			y_line(start.y, end.y, start.x, color, surf, dash, thickness);
			y_line(start.y, end.y, end.x, color, surf, dash, thickness);
		}

		void rect(point start, point end, const bool& b, const bool& t, const bool& l, const bool& r,
			COLOR, SURF, const int& dash = 0, const int& thickness = 1)
		{
			swap_point_if(start, end);
			if (b) x_line(start.x, end.x, start.y, color, surf, dash, thickness);
			if (t) x_line(start.x, end.x, end.y, color, surf, dash, thickness);
			if (l) y_line(start.y, end.y, start.x, color, surf, dash, thickness);
			if (r) y_line(start.y, end.y, end.x, color, surf, dash, thickness);
		}

		inline void rect_size(cpr start, cpr size, COLOR, SURF,
			const int& dash = 0, const int& thickness = 1)
		{
			rect(start, start + size, color, surf, dash, thickness);
		}

		inline void fill_rect_size(cpr start, cpr size, COLOR, SURF, const uint8_t& alpha = UINT8_MAX)
		{
			fill_rect(start, start + size, color, surf, alpha);
		}

		inline void triangle(cpr a, cpr b, cpr c, COLOR, SURF)
		{
			line(a, b, color, surf);
			line(b, c, color, surf);
			line(c, a, color, surf);
		}
	}
}
