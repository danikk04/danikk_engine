#pragma once

#include <danikk_engine/gui.h>

namespace danikk_engine
{
	class SubWindow : public GUIElement
	{
	public:
		GUIElement header;
		GUIElement title;
		GUIElement close_button;

		GUIElement body;

		SubWindow();

		GUIElement& operator=(const GUIElement&) = delete;

		GUIElement& operator=(const GUIElement&&) = delete;

		~SubWindow() = default;
	};
}
