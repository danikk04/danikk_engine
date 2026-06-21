#include <danikk_framework/glm.h>

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/localization.h>
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
#include <danikk_framework/cataloged_dictionary.h>
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

	FT_Library ftlib;

	static constexpr bool draw_atlas = false;//булевая для отладки, если она true, то вместо символа будет рисоваться весь атлас на экране.

	namespace internal
	{
		void initFontRenderer()
		{
			int error = FT_Init_FreeType(&ftlib);

			if (error)
			{
				formatLogFatal("Failde to init freetype2 error code:%", error);
				fatalError();
			}
		}
	}

	void Font::load(const char* name, int font_type)
	{
    	AssetContainer** container_ptr = assets.get(name);

    	if(container_ptr == NULL)
    	{
            container = new (structalloc<AssetContainer>()) AssetContainer(asset_type::font, name);
            container->font_ptr = new (structalloc<FontData>()) FontData();
            container->font_ptr->font_type = font_type;

            if(font_type == FontTypes::true_type)
            {
                internal::FontData& font = *container->font_ptr;
                FT_Face& face = font.true_type_data.face;

        		loadDataToBuffer("fonts", name, "ttf", true);

        		size_t error = FT_New_Memory_Face(ftlib, (uint8*)asset_load_buffer.data(), asset_load_buffer.size(), 0, &face);

        		if(error)
        		{
        			formatFatalError("Failded to init face font name:% error code:%", name, error);
        		}

        		error = FT_Set_Pixel_Sizes(face, FontData::char_axis_size, FontData::char_axis_size);

    			if (error)
    			{
    				formatFatalError("Failded to set size font name:% error code:%", name, error);
    			}

    			error = FT_Select_Charmap(face , FT_ENCODING_UNICODE);

    			if (error)
    			{
    				formatFatalError("Failded to select charmap font name:% error code:%", name, error);
    			}
            }
            else if(font_type == FontTypes::rasterized)
            {
            	FontData::rasterized_data_t& data = container->font_ptr->rasterized_data;
                DynamicArray<FontData::rasterized_page>& pages = data.pages;
                loadDataToBuffer("fonts", name, "rfmd", true);//rfmd - rasterized font meta data
    			Config font_settings;
    			font_settings.openData(asset_load_buffer.data(), asset_load_buffer.size());

    			String pages_str = font_settings.get<String>("default.pages");
    			DynamicArray<String> page_names = splitStringToArray(pages_str, ';');
    			for(const String& page_name : page_names)
    			{
        			loadDataToBuffer("fonts", name, page_name.c_string(), "png", true);
    				FontData::rasterized_page& page = pages.pushCtor();

    				String page_kv;
					#define get_page_ivec2(VAR) \
					page_kv.clear();\
					page_kv << page_name << "." << #VAR;\
					page_kv = font_settings.get<String>(page_kv);\
					page.VAR = danikk_framework::ivec2_from_string(page_kv);

					get_page_ivec2(start_offset);
					get_page_ivec2(char_code_range);
					get_page_ivec2(char_size);
					get_page_ivec2(char_offset);

					page_kv.clear();
					page_kv << page_name << String(".margin");
					page_kv = font_settings.get<String>(page_kv);
					if(page_kv.contains('%'))
					{
						page_kv.cutChars('%');
						float margin_k = parseNumber<float>(page_kv) * 0.01f;
						page.margin = (ivec2)((vec2)page.char_size * margin_k);
					}
					else
					{
						page.margin = danikk_framework::ivec2_from_string(page_kv);
					}
					#undef get_page_ivec2
					page_kv.clear();
					page_kv << page_name << ".row_char_count";
					page_kv.c_string();
					page.row_char_count = font_settings.get<int>(page_kv);

    	            int channels = 0;
    	            int width = 0;
    	            int height = 0;
    	            char* atlas_ptr = NULL;

    	            atlas_ptr = (char*)stb::load_from_memory((uint8*)asset_load_buffer.data(),
                    	asset_load_buffer.size(), &width, &height, &channels, 0);
    	            assert(channels == 1);
    	            page.atlas = FixedRefMatrix<uint8>((uint8*)atlas_ptr, width, height);


					page_kv.clear();
					page_kv << page_name << ".row_char_count";
    	            page.row_char_count = font_settings.get<int>(page_kv);

                    assert(page.atlas.size() != uvec2(0, 0));
    			}
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
		for(const char* cur = str; cur < end;)
		{
			if(char_counter == style.max_char_count)
			{
				break;
			}
			chars.push(utf8::next(cur, end));
			char_counter++;
		}
		draw(chars, pos, size, style);
	}

	void Font::draw(Array<uint32, 512>& chars, vec2 pos, vec2 size, TextStyle& style)
	{
		FontData& data = *container->font_ptr;
		FT_Face& face = data.true_type_data.face;
		DynamicArray<FontData::CharAtlas>& char_atlases = data.char_atlases;
		FontData::CharAtlas* char_atlas = NULL;
		String error_buffer;
		vec2 char_size = vec2(style.absolute_char_width, style.absolute_char_height);
		vec2 draw_start;
		switch(style.anchor)
		{
			case TextAnchor::none: logWarning("style.anchor == TextAnchor::none");
			case TextAnchor::multiline: draw_start = pos; break;
			case TextAnchor::center:
				vec2 text_size = char_size;
				text_size.x *= chars.size();
				draw_start = pos + (size - text_size) / vec2(2.0f);
			break;
		}
		uint32 char_counter = 0;
		for(uint32* cur = chars.begin(); cur < chars.end(); cur++)
		{
			uint32 chr = *cur;
			uint32 atlas_index = chr / FontData::char_atlas_count;						//Индекс атласа
			uint32 char_atlas_index = chr % FontData::char_atlas_count;					//Индекс символа в атласе
			uint32 char_atlases_target_size = atlas_index + 1;							//Размер массива атласов, у которого последний элемент будет нужным нам атласом.

			if(char_atlases_target_size > char_atlases.size())//Если массив атласов слишком мал для содержания атласа, то его там просто нету.
			{
				char_atlases.resize(char_atlases_target_size, FontData::CharAtlas());
				char_atlas = &char_atlases[atlas_index];
			}
			else if(char_atlases[atlas_index].gl_handle == 0)
			{
				char_atlas = &char_atlases[atlas_index];
			}
			else //Если необходимый нам атлас инициализирован, то пропускаем его инициализацию.
			{
				if(char_atlas == NULL || char_atlases[atlas_index].gl_handle != char_atlas->gl_handle)//Если используется алтас, отличный от атласа для предыдущего символа, то меняем его.
				{
					char_atlas = &char_atlases[atlas_index];
					glBindTexture(GL_TEXTURE_2D, char_atlas->gl_handle);
				}
				goto char_render;
			}

			{
				uint32 current_char = atlas_index * FontData::char_atlas_count;

				uint32 end_char = current_char + FontData::char_atlas_count;
				FixedRefMatrix<font_color> atlas_bitmap((font_color*)atlas_buffer, FontData::atlas_axis_size, FontData::atlas_axis_size);
				SubMatrix<font_color> atlas_char_subbitmap(atlas_bitmap, 0,0, FontData::char_axis_size, FontData::char_axis_size);
				FontData* font_data = container->font_ptr;
				int font_type = font_data->font_type;

				ivec2 char_pos;

				if(font_type == FontTypes::rasterized)
				{
					FontData::rasterized_page* page = NULL;
					while(current_char < end_char)
					{
						if(page == NULL || !inRange(page->char_code_range, current_char))
						{
							page = NULL;
							for(FontData::rasterized_page& current_page : font_data->rasterized_data.pages)
							{
								if(inRange(current_page.char_code_range, current_char))
								{
									page = &current_page;
									break;
								}
							}
						}
						if(page == NULL)
						{
							current_char++;
							continue;
						}
						int char_index_in_atlas = current_char - page->char_code_range.x;
						assert((uint)char_index_in_atlas < (page->char_code_range.y - page->char_code_range.x));
						ivec2 char_bitmap_pos = index2Pos<ivec2>(char_index_in_atlas, page->row_char_count);
						char_bitmap_pos *= page->char_offset;
						char_bitmap_pos += page->start_offset;
						char_pos = FontData::charToAtlasPos(current_char);
						atlas_char_subbitmap.move(char_pos.x, char_pos.y);

						SubMatrix<uint8> page_subbitmap(page->atlas,
								char_bitmap_pos.x + page->margin.x,
								char_bitmap_pos.y + page->margin.y,
								page->char_size.x - page->margin.x * 2.0f,
								page->char_size.y - page->margin.y * 2.0f);

						matrix_functions::scaleCopy(page_subbitmap, atlas_char_subbitmap);
						current_char++;
					}
				}
				else if(font_type == FontTypes::true_type)
				{
					while(current_char < end_char)
					{
						uint32 glyph_index = FT_Get_Char_Index(face, current_char);
						uint32 error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
						if (error)
						{
	                    	formatLogWarning("Failded to  load font % glyph for char % with error %", container->name, current_char, error);
							current_char++;
							continue;
						}

						error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

						if (error)
						{
	                    	formatLogWarning("Failded to render glyph of font % for char % with error %", container->name, current_char, error);
							current_char++;
							continue;
						}

						/*error = FT_Load_Char(face, current_char, FT_LOAD_RENDER);

						if (error)
						{
							appendFailError(error_buffer, "Font::draw/render_glyph");
							appendOutputInfo(error_buffer, "font_name", container->name);
							appendOutputInfo(error_buffer, "char_code", current_char);
							appendOutputInfo(error_buffer, "error_code", error);
							logWarning(error_buffer);
							error_buffer.clear();
							current_char++;
							continue;
						}*/

						FT_Bitmap& bitmap = face->glyph->bitmap;

						if(bitmap.width == 0 || bitmap.rows == 0)
						{
	                    	formatLogWarning("Mismatch glyph size of font % for char %", container->name, current_char);
							continue;
						}

						//memset(atlas_buffer, 0, FontData::atlas_size); //я из-за этой строчки столько нервов себе сжёг...

						char_pos = FontData::charToAtlasPos(current_char);
						atlas_char_subbitmap.move(char_pos.x, char_pos.y);
						FixedRefMatrix<byte> glyph_bitmap = FixedRefMatrix<byte>(bitmap.buffer, bitmap.width, bitmap.rows);
						matrix_functions::scaleCopy(glyph_bitmap, atlas_char_subbitmap);
					}
				}
				current_char++;

				char_atlas->gl_handle = glGenTexture();
				glBindTexture(GL_TEXTURE_2D, char_atlas->gl_handle);
				setupDefaultTextureParameters();
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontData::atlas_axis_size, FontData::atlas_axis_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_buffer);
			}
			char_render:;
			if(chr != ' ')
			{
				vec2 char_atlas_pos = vec2((float)(char_atlas_index % FontData::char_axis_count), (float)(char_atlas_index / FontData::char_axis_count));
				char_atlas_pos /= (float)FontData::char_axis_count;
				vec2 char_pos = vec2(draw_start.x + char_counter * (char_size.x + style.char_interval), draw_start.y);

				uv_matrix pos_matrix;
				uv_matrix uv_matrix;
				if constexpr (draw_atlas)
				{
					pos_matrix = create_pos_matrix(vec2(0,0), vec2(1,1), 0);
					uv_matrix = create_uv_matrix(0, 0, 1);
				}
				else
				{
					pos_matrix = create_pos_matrix(char_pos, char_size, 0);
					uv_matrix = create_uv_matrix(char_atlas_pos.x, char_atlas_pos.y, FontData::char_uv_scale * 1.0f);
				}
				setWorldMatrix(pos_matrix);
				setUVMatrix(uv_matrix);
				setDrawColor(style.color);
				bindTexture(char_atlas->gl_handle, 0);
				drawSpriteMesh();
			}

			char_counter++;
		}
	}
}
