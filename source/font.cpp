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

	/** @brief Builds and returns the cached GPU atlas containing a block of character codes. */
	static FontData::CharAtlas& getCharacterAtlas(FontData& data, uint32 atlas_index)
	{
		if(data.char_atlases.size() <= atlas_index)
			data.char_atlases.resize(static_cast<size_t>(atlas_index) + 1, FontData::CharAtlas());
		FontData::CharAtlas& char_atlas = data.char_atlases[atlas_index];
		if(char_atlas.gl_handle != 0) return char_atlas;

		for(font_color& pixel : atlas_buffer)
		{
			pixel.r = pixel.g = pixel.b = 255;
			pixel.a = 0;
		}
		const uint32 first_character = atlas_index * FontData::char_atlas_count;
		const uint32 character_end = first_character + FontData::char_atlas_count;
		FixedRefMatrix<font_color> atlas_bitmap(
			reinterpret_cast<font_color*>(atlas_buffer), FontData::atlas_axis_size, FontData::atlas_axis_size);
		SubMatrix<font_color> atlas_char_subbitmap(
			atlas_bitmap, 0, 0, FontData::char_axis_size, FontData::char_axis_size);
		for(uint32 current_character = first_character; current_character < character_end; ++current_character)
		{
			FontData::rasterized_page* page = nullptr;
			for(FontData::rasterized_page& current_page : data.rasterized_pages)
				if(inRange(current_page.char_code_range, current_character))
				{
					page = &current_page;
					break;
				}
			if(page == nullptr) continue;

			const int character_index = current_character - page->char_code_range.x;
			assert(static_cast<uint>(character_index)
				< (page->char_code_range.y - page->char_code_range.x));
			ivec2 source_position = index2Pos<ivec2>(character_index, page->row_char_count);
			source_position *= page->char_offset;
			source_position += page->start_offset;
			const ivec2 atlas_position = FontData::charToAtlasPos(current_character);
			atlas_char_subbitmap.move(atlas_position.x, atlas_position.y);
			SubMatrix<uint8> page_subbitmap(page->atlas,
				source_position.x + page->margin.x,
				source_position.y + page->margin.y,
				page->char_size.x - page->margin.x * 2,
				page->char_size.y - page->margin.y * 2);
			matrix_functions::scaleCopy(page_subbitmap, atlas_char_subbitmap);
		}

		char_atlas.gl_handle = glGenTexture();
		glBindTexture(GL_TEXTURE_2D, char_atlas.gl_handle);
		setupDefaultTextureParameters();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontData::atlas_axis_size, FontData::atlas_axis_size,
			0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_buffer);
		return char_atlas;
	}

	/** @brief Returns the atlas UV origin for one character code. */
	static vec2 getCharacterAtlasUV(uint32 character)
	{
		const uint32 atlas_character_index = character % FontData::char_atlas_count;
		return vec2(static_cast<float>(atlas_character_index % FontData::char_axis_count),
			static_cast<float>(atlas_character_index / FontData::char_axis_count))
			/ static_cast<float>(FontData::char_axis_count);
	}

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
			FontData::CharAtlas& char_atlas = getCharacterAtlas(data, atlas_index);
			if(character != ' ')
			{
				const vec2 char_atlas_pos = getCharacterAtlasUV(character);
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
				bindTexture(char_atlas.gl_handle, 0);
				drawSpriteMesh();
			}
			line_char_counter++;
		}
	}

	/** @copydoc FontMesh::clear */
	void FontMesh::clear()
	{
		for(FontMeshBatch& batch : batches) batch.geometry.clear();
	}

	/** @brief Appends centered, camera-oriented glyph quads and groups them by atlas handle. */
	void Font::append3DText(FontMesh& mesh, const String& text, const vec3& center,
		const vec3& right_axis, const vec3& up_axis, const vec2& glyph_size, float char_spacing)
	{
		FontData& data = *container->font_ptr;
		Array<uint32, 512> characters;
		const char* current = text.data();
		const char* end = current + text.size();
		while(current < end && characters.size() < characters.capacity())
			characters.push(utf8::next(current, end));
		if(characters.size() == 0) return;

		const vec3 right = glm::normalize(right_axis);
		const vec3 up = glm::normalize(up_axis);
		const vec3 normal = glm::normalize(glm::cross(right, up));
		const float advance = glyph_size.x + char_spacing;
		const float text_width = glyph_size.x + (characters.size() - 1) * advance;
		const vec3 line_start = center - right * (text_width * 0.5f) - up * (glyph_size.y * 0.5f);

		for(uint32 character_index = 0; character_index < characters.size(); ++character_index)
		{
			const uint32 character = characters[character_index];
			if(character != ' ' && character != '\n' && character != '\r')
			{
				const uint32 atlas_index = character / FontData::char_atlas_count;
				const FontData::CharAtlas& atlas = getCharacterAtlas(data, atlas_index);
				FontMeshBatch* batch = nullptr;
				for(FontMeshBatch& candidate : mesh.batches)
					if(candidate.atlas_texture_handle == atlas.gl_handle)
					{
						batch = &candidate;
						break;
					}
				if(batch == nullptr)
				{
					batch = &mesh.batches.pushCtor();
					batch->atlas_texture_handle = atlas.gl_handle;
				}

				const vec2 atlas_uv = getCharacterAtlasUV(character);
				const vec2 atlas_uv_end = atlas_uv + vec2(FontData::char_uv_scale);
				const vec3 bottom_left = line_start + right * (character_index * advance);
				const vec3 bottom_right = bottom_left + right * glyph_size.x;
				const vec3 top_left = bottom_left + up * glyph_size.y;
				const vec3 top_right = bottom_right + up * glyph_size.y;
			const DefaultVertex quad[] = {
					DefaultVertex(bottom_left, normal, vec2(atlas_uv.x, atlas_uv_end.y)),
					DefaultVertex(bottom_right, normal, atlas_uv_end),
					DefaultVertex(top_right, normal, vec2(atlas_uv_end.x, atlas_uv.y)),
					DefaultVertex(top_left, normal, atlas_uv)};
				batch->geometry.addQuad(quad);
			}
		}
	}
}
