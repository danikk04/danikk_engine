#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/shader.h>
#include <danikk_engine/internal/asset.h>
#include <danikk_engine/color.h>

namespace danikk_engine
{
	class Font : private internal::Asset
	{
	public:
		Font() = default;

		void load(const char* name);

		Font(Font& other);

		Font(internal::AssetContainer* container);

        void operator=(Font& other);

        void operator=(Font&& other);

		void draw(const String& text, vec2 pos, vec2 size, TextStyle& style);

		void draw(const char* str, size_t str_size, vec2 pos, vec2 size, TextStyle& style);

		void draw(Array<uint32, 512>& chars, vec2 pos, vec2 size, TextStyle& style);
	};
}
