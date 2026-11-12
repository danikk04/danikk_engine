#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_framework/matrix.h>
#include <danikk_framework/glm.h>

namespace danikk_engine
{
	namespace internal
	{
		struct FontData
		{
			static constexpr size_t atlas_axis_size = 2048;//Размер аласа по 1 оси.
			static constexpr size_t atlas_size = atlas_axis_size * atlas_axis_size;

			static constexpr size_t char_axis_size = 128;
			static constexpr size_t char_size = char_axis_size * char_axis_size;

			static constexpr size_t char_axis_count = atlas_axis_size / char_axis_size;
			static constexpr size_t char_atlas_count = char_axis_count * char_axis_count;//Количество символов, которое поместится на 1 атласе.

			static constexpr float char_uv_scale = (float)char_axis_size / (float)atlas_axis_size;

			struct CharAtlas
			{
				uint32 gl_handle = 0;
			};

			struct rasterized_page
			{
				danikk_framework::FixedRefMatrix<uint8> atlas;

				/*char* atlas;
				int width;
				int height;*/

				uvec2 start_offset;
				uvec2 char_size;
				uvec2 char_offset;
				uvec2 char_code_range;
				uvec2 margin;

				int row_char_count;
			};

			DynamicArray<rasterized_page> rasterized_pages;

			DynamicArray<FontData::CharAtlas> char_atlases;

			static ivec2 charToAtlasPos(int chr)
			{
				return danikk_framework::index2Pos<ivec2>(chr % FontData::char_atlas_count, FontData::char_axis_count)
										* ivec2(FontData::char_axis_size, FontData::char_axis_size);
			}

			FontData() = default;
		};

	}
}
