#include <danikk_framework/glm.h>

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/font.h>
#include <danikk_engine/color.h>
#include <danikk_engine/style.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/font.h>
#include <danikk_engine/internal/stb.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/matrix/pos.h>
#include <danikk_engine/matrix/uv.h>
#include <danikk_framework/matrix.h>
#include <danikk_framework/dictionary.h>
#include <danikk_framework/config.h>
#include <danikk_framework/string_features.h>
#include <danikk_framework/format.h>

#include <utf8.h>
#include <glad/glad.h>

namespace danikk_engine
{
	using namespace internal;
	using namespace danikk_framework;

	static font_color atlas_buffer[FontData::atlas_size];
	static constexpr bool draw_atlas = false;

	void Font::load(const char* name)
	{
		AssetContainer** container_ptr = assets.get(name);
		if(container_ptr == NULL)
		{
			container = new (structalloc<AssetContainer>()) AssetContainer(asset_type::font, name);
			container->font_ptr = new (structalloc<FontData>()) FontData();
			DynamicArray<FontData::rasterized_page>& pages = container->font_ptr->rasterized_pages;

			loadDataToBuffer("fonts", name, "rfmd", true);
			DynamicInlineStringDictionary<uint16> font_settings;
			asset_load_buffer.reserve(1);
			asset_load_buffer.write("\0", 1);
			inlineParseConfig(font_settings, (char*)asset_load_buffer.data());

			String pages_str = font_settings.get<String>("default.pages");
			DynamicArray<String> page_names = splitStringToArray(pages_str, ';');
			for(const String& page_name : page_names)
			{
				FontData::rasterized_page& page = pages.pushCtor();
				String page_key;

				format(page_key, "%.start_offset", page_name);
				page_key = font_settings.get<String>(page_key);
				page.start_offset = danikk_framework::ivec2_from_string(page_key, ' ');

				format(page_key, "%.char_code_range", page_name);
				page_key = font_settings.get<String>(page_key);
				page.char_code_range = danikk_framework::ivec2_from_string(page_key, ' ');

				format(page_key, "%.char_size", page_name);
				page_key = font_settings.get<String>(page_key);
				page.char_size = danikk_framework::ivec2_from_string(page_key, ' ');

				format(page_key, "%.char_offset", page_name);
				page_key = font_settings.get<String>(page_key);
				page.char_offset = danikk_framework::ivec2_from_string(page_key, ' ');

				format(page_key, "%.margin", page_name);
				page_key = font_settings.get<String>(page_key);
				if(page_key.contains('%'))
				{
					page_key.cutChars('%');
					float margin_ratio = parseNumber<float>(page_key) * 0.01f;
					page.margin = (ivec2)((vec2)page.char_size * margin_ratio);
				}
				else
				{
					page.margin = danikk_framework::ivec2_from_string(page_key, ' ');
				}

				format(page_key, "%.row_char_count", page_name);
				page.row_char_count = font_settings.get<int>(page_key);

				int channels = 0;
				int width = 0;
				int height = 0;
				loadDataToBuffer("fonts", name, page_name.c_string(), "png", true);
				char* atlas_data = (char*)stb::load_from_memory(
					(uint8*)asset_load_buffer.data(), asset_load_buffer.size(), &width, &height, &channels, 0);
				assert(channels == 1);
				page.atlas = FixedRefMatrix<uint8>((uint8*)atlas_data, width, height);
				assert(page.atlas.size() != uvec2(0, 0));
			}

			assets.insert(name, container);
		}
		else
		{
			container = *container_ptr;
		}

		new (this) Asset(container);
	}

	Font::Font(Font& other) : Asset(other.container){}

	Font::Font(AssetContainer* container) : Asset(container){}

	void Font::operator=(Font& other)
	{
		new (this) Font(other);
	}

	void Font::operator=(Font&& other)
	{
		new (this) Font(other);
	}

	void Font::draw(const String& text, vec2 pos, vec2 size, TextStyle& style)
	{
		draw(text.data(), text.size(), pos, size, style);
	}

	void Font::draw(const char* str, size_t str_size, vec2 pos, vec2 size, TextStyle& style)
	{
		Array<uint32, 512> chars;
		uint32 char_counter = 0;
		const char* end = str + str_size;
		for(const char* current = str; current < end;)
		{
			if(char_counter == style.max_char_count)
			{
				break;
			}
			chars.push(utf8::next(current, end));
			char_counter++;
		}
		draw(chars, pos, size, style);
	}

	void Font::draw(Array<uint32, 512>& chars, vec2 pos, vec2 size, TextStyle& style)
	{
		FontData& data = *container->font_ptr;
		DynamicArray<FontData::CharAtlas>& char_atlases = data.char_atlases;
		FontData::CharAtlas* char_atlas = NULL;
		const vec2 char_size(style.absolute_char_width, style.absolute_char_height);
		vec2 draw_start;
		switch(style.anchor)
		{
			case TextAnchor::none: logWarning("style.anchor == TextAnchor::none");
			case TextAnchor::multiline:
				draw_start = pos + vec2(0.0f, size.y - char_size.y);
				break;
			case TextAnchor::center:
			{
				vec2 text_size = char_size;
				text_size.x *= chars.size();
				draw_start = pos + (size - text_size) / vec2(2.0f);
				break;
			}
		}

		uint32 line_char_counter = 0;
		uint32 line_counter = 0;
		for(uint32* current = chars.begin(); current < chars.end(); current++)
		{
			const uint32 character = *current;
			if(character == '\n' && style.anchor == TextAnchor::multiline)
			{
				line_char_counter = 0;
				line_counter++;
				continue;
			}

			const uint32 atlas_index = character / FontData::char_atlas_count;
			const uint32 char_atlas_index = character % FontData::char_atlas_count;
			const uint32 char_atlases_target_size = atlas_index + 1;
			if(char_atlases_target_size > char_atlases.size())
			{
				char_atlases.resize(char_atlases_target_size, FontData::CharAtlas());
				char_atlas = &char_atlases[atlas_index];
			}
			else if(char_atlases[atlas_index].gl_handle == 0)
			{
				char_atlas = &char_atlases[atlas_index];
			}
			else
			{
				if(char_atlas == NULL || char_atlases[atlas_index].gl_handle != char_atlas->gl_handle)
				{
					char_atlas = &char_atlases[atlas_index];
					glBindTexture(GL_TEXTURE_2D, char_atlas->gl_handle);
				}
				goto char_render;
			}

			{
				uint32 current_char = atlas_index * FontData::char_atlas_count;
				const uint32 end_char = current_char + FontData::char_atlas_count;
				FixedRefMatrix<font_color> atlas_bitmap((font_color*)atlas_buffer, FontData::atlas_axis_size, FontData::atlas_axis_size);
				SubMatrix<font_color> atlas_char_subbitmap(atlas_bitmap, 0, 0, FontData::char_axis_size, FontData::char_axis_size);
				ivec2 atlas_char_pos;

				while(current_char < end_char)
				{
					FontData::rasterized_page* page = NULL;
					for(FontData::rasterized_page& current_page : data.rasterized_pages)
					{
						if(inRange(current_page.char_code_range, current_char))
						{
							page = &current_page;
						break;
						}
					}
					if(page != NULL)
					{
						const int char_index = current_char - page->char_code_range.x;
					assert((uint)char_index < (page->char_code_range.y - page->char_code_range.x));
					ivec2 source_pos = index2Pos<ivec2>(char_index, page->row_char_count);
					source_pos *= page->char_offset;
					source_pos += page->start_offset;
					atlas_char_pos = FontData::charToAtlasPos(current_char);
					atlas_char_subbitmap.move(atlas_char_pos.x, atlas_char_pos.y);

					SubMatrix<uint8> page_subbitmap(page->atlas,
						source_pos.x + page->margin.x,
						source_pos.y + page->margin.y,
						page->char_size.x - page->margin.x * 2.0f,
						page->char_size.y - page->margin.y * 2.0f);
					matrix_functions::scaleCopy(page_subbitmap, atlas_char_subbitmap);
					}
					current_char++;
				}

				char_atlas->gl_handle = glGenTexture();
				glBindTexture(GL_TEXTURE_2D, char_atlas->gl_handle);
				setupDefaultTextureParameters();
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontData::atlas_axis_size, FontData::atlas_axis_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_buffer);
			}
		char_render:
			if(character != ' ')
			{
				vec2 char_atlas_pos(
					(float)(char_atlas_index % FontData::char_axis_count),
					(float)(char_atlas_index / FontData::char_axis_count));
				char_atlas_pos /= (float)FontData::char_axis_count;
				const vec2 char_pos(
					draw_start.x + line_char_counter * (char_size.x + style.char_interval),
					draw_start.y - line_counter * char_size.y);

				uv_matrix pos_matrix;
				uv_matrix uv_matrix;
				if constexpr (draw_atlas)
				{
					pos_matrix = create_pos_matrix(vec2(0, 0), vec2(1, 1), 0);
					uv_matrix = create_uv_matrix(0, 0, 1);
				}
				else
				{
					pos_matrix = create_pos_matrix(char_pos, char_size, 0);
					uv_matrix = create_uv_matrix(char_atlas_pos.x, char_atlas_pos.y, FontData::char_uv_scale);
				}
				setWorldMatrix(pos_matrix);
				setUVMatrix(uv_matrix);
				setDrawColor(style.color);
				bindTexture(char_atlas->gl_handle, 0);
				drawSpriteMesh();
			}
			line_char_counter++;
		}
	}
}
