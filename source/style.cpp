#include <danikk_engine/style.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/matrix/pos.h>
#include <danikk_engine/matrix/uv.h>

namespace danikk_engine
{
	using namespace internal;

	BorderDrawStyle default_border_draw_style;
	DrawStyle default_draw_style;
	TextStyle default_text_style;

	BorderDrawStyle::BorderDrawStyle()
	{
		this->color = vec4(0.0f,0.0f,0.0f,1.0f);
	}

	void BorderDrawStyle::draw(vec2 pos, vec2 size)
	{
		mat4 poses[]
		{
			create_pos_matrix(pos, 								vec2(size.x, width.y), 0),
			create_pos_matrix(pos + vec2(0, size.y - width.y), 	vec2(size.x, width.y), 0),
			create_pos_matrix(pos, 								vec2(width.x, size.y), 0),
			create_pos_matrix(pos + vec2(size.x - width.x, 0),	vec2(width.x, size.y), 0)
		};
		for(const mat4& pos : poses)
		{
			setWorldMatrix(pos);
			setUVMatrix(fill_uv_matrix);
			setDrawColor(color);
			bindTexture(0, 0);
			drawSpriteMesh();
		}
	}

	void TextStyle::draw(const String& text, vec2 pos, vec2 size)
	{
		shader.use();
		font.draw(text, pos, size, *this);
	}

	void setDefaultGUIShader(Shader& shader)
	{
		default_border_draw_style.shader = shader;
		default_draw_style.shader = shader;
		default_text_style.shader = shader;
	}

	void setDefaultFont(Font& font)
	{
		default_text_style.font = font;
	}

	void setDefaultCharSize(float height, float width)
	{
		default_text_style.absolute_char_height = height;
		default_text_style.absolute_char_width = width;
	}

	void setDefaultCharInterval(float value)
	{
		default_text_style.absolute_char_width *= value;
		default_text_style.absolute_char_width *= value;
	}
}
