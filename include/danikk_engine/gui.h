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
		using TextWriter = void (*)(const void*, String&);

	private:
		/** @brief Optional source passed to the bound text writer; null is a valid source. */
		const void* text_source = nullptr;
		/** @brief Rebuilds the element text from its bound source before drawing. */
		TextWriter text_writer = nullptr;

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
		const char* name = "NONE";

		String text;
		TextStyle* text_style = &default_text_style;
		std::function<void()> on_click;
		/** @brief Optional custom drawing callback invoked after this element's background and border. */
		std::function<void(vec2, vec2)> draw_callback;

		Sprite sprite;
		DrawStyle* draw_style = NULL;
		BorderDrawStyle* border_draw_style = NULL;

		GUIElement() = default;

		GUIElement& operator=(const GUIElement&) = delete;

		GUIElement& operator=(const GUIElement&&) = delete;

		/** @brief Binds the element text to a source and its text-generation callback. */
		void bind(const void* source, TextWriter writer)
		{
			text_source = source;
			text_writer = writer;
		}

		/** @brief Removes the text binding, preserving the current text as static content. */
		void unbind()
		{
			text_source = nullptr;
			text_writer = nullptr;
		}

		void recursiveDraw();

		void draw(vec2 offset, vec2 scale);

		void handleClick(int32 is_pushed);

		void addChild(GUIElement& child);

		virtual ~GUIElement() = default;
	};

	extern GUIElement gui_root;
}
