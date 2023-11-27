#pragma once

#include "Graphics.h"

#include <fstream>

// ReSharper disable StringLiteralTypo CppNonInlineFunctionDefinitionInHeaderFile CppInconsistentNaming
inline const char* test_text = R"(!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~)";

namespace data
{
	inline char* string_buffer;
	constexpr int string_buffer_size = 0x100;
}

namespace font
{
	inline const char* font_path = "font.bin";

	constexpr size_t buffer_size = 9185;
	constexpr int chars_size = 93;

	constexpr point max_font_dim = { 12, 17 };

	struct character_surface
	{
		uint8_t* buffer, * end;
		point size, bias;
		size_t buffer_size;
	};

	inline character_surface* characters;

	inline uint8_t* binary_buffer, * buffer_end;

	bool init()
	{
		binary_buffer = static_cast<uint8_t*>(malloc(buffer_size));
		buffer_end = binary_buffer + buffer_size;
		std::ifstream file(font_path, std::ios::binary);
		if (!file.is_open())
			return true;
		file.read(reinterpret_cast<char*>(binary_buffer), buffer_size);
		file.close();

		characters = static_cast<character_surface*>(malloc(chars_size * sizeof(character_surface)));

		const uint8_t* _byte = binary_buffer;
		for (character_surface* current = characters; current < characters + chars_size; current++)
		{
			current->size.x = *_byte++;
			current->size.y = *_byte++;
			current->bias.x = *_byte++;
			current->bias.y = *_byte++;
			current->buffer_size = static_cast<size_t>(current->size.x) * current->size.y;

			current->buffer = static_cast<uint8_t*>(malloc(current->buffer_size));
			current->end = current->buffer + current->buffer_size;

			for (uint8_t* pixel = current->buffer; pixel < current->end; pixel++, _byte++)
				*pixel = *_byte;
		}

		return false;
	}

	void unsafe_draw_char(const char& c, cpr pos, COLOR, const graphics::surface& dest)
	{
		if (c == ' ')
			return;
		const character_surface* char_surf = characters + c - 33;

		uint* dest_pixel = get_raw_pixel(pos, dest);

		const int y_step = dest.size.x - char_surf->size.x;

		for (const uint8_t* src_pixel = char_surf->buffer; src_pixel < char_surf->end; dest_pixel += y_step)
			for (const uint8_t* x_end = src_pixel + char_surf->size.x; src_pixel < x_end; src_pixel++, dest_pixel++)
				*dest_pixel = *src_pixel * color;
	}

	void draw_string(point pos, const char* str, COLOR, const graphics::surface& dest)
	{
		if (pos.y < 0 || pos.y >= dest.size.y) return;
		if (pos.x < 0)
		{
			const int start_bias = -pos.x / max_font_dim.x + (-pos.x % max_font_dim.x != 0);
			pos.x += max_font_dim.x * start_bias;
			str++;
		}
		while (*str != '\0')
		{
			if (pos.x >= dest.size.x) return;
			unsafe_draw_char(*str, pos, color, dest);
			pos.x += max_font_dim.x;
			str++;
		}
	}
}