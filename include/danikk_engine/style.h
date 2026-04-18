#pragma once
#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/shader.h>
#include <danikk_engine/font.h>
#include <danikk_engine/internal/anchor.h>

namespace danikk_engine
{
	struct DrawStyle
	{
		Shader shader;
		vec4 color = vec4(1);

	};

	struct BorderDrawStyle : DrawStyle
	{
		vec2 width = getPixelSize() * 2.0f;

		BorderDrawStyle();

		void draw(vec2 pos, vec2 size);
	};

	struct TextStyle : DrawStyle
	{
		Font font;
		float absolute_char_height = 0.04f;
		float absolute_char_width = 0.04f;
		float char_interval = 0.00f;
		int16 anchor = TextAnchor::center;
		uint16 max_char_count = 300;
		TextStyle() = default;

		void draw(const String& text, vec2 pos, vec2 size);

	};

	extern BorderDrawStyle default_border_draw_style;
	extern DrawStyle default_draw_style;
	extern TextStyle default_text_style;

	void setDefaultGUIShader(Shader& shader);

	void setDefaultFont(Font& font);

	void setDefaultCharSize(float height, float width);

	void setDefaultCharInterval(float value);
}
