#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/texture.h>
#include <danikk_engine/font.h>
#include <danikk_engine/style.h>

#include <danikk_framework/bit_array.h>
#include <functional>

namespace danikk_engine
{
	using danikk_framework::BitArray32;

	enum GUIElementFlags
	{
		visible,
		draggable,
		absolute_recalc
	};


#define define_gui_element(type, name) type name(#name);

	class GUIElement
	{
	public:
		DynamicArray<GUIElement*> childs;
		GUIElement* parent = NULL;

		BitArray32 flags = BitArray32(0b0101);
		int32 anchor;

		vec2 pos = vec2(0.0f, 0.0f);
		vec2 absolute_pos;

		vec2 relative_size = vec2(0);
		vec2 pixel_size = vec2(0);
		vec2 absolute_size;
		const char* name = NULL;

		String text;
		TextStyle* text_style = &default_text_style;
		std::function<void()> on_click;

		Sprite sprite;
		DrawStyle* draw_style = NULL;
		BorderDrawStyle* border_draw_style = NULL;

		GUIElement() = default;

		GUIElement& operator=(const GUIElement&) = delete;

		GUIElement& operator=(const GUIElement&&) = delete;

		void recursiveDraw();

		void draw(vec2 offset, vec2 scale);

		void handleClick(int32 is_pushed);

		void addChild(GUIElement& child);

		virtual ~GUIElement() = default;
	};

	extern GUIElement gui_root;
}
