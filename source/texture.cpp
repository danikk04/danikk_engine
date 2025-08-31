#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <danikk_engine/sprite.h>

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/texture.h>
#include <danikk_engine/texture_methods.h>

#include <danikk_engine/internal/stb.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_engine/matrix/uv.h>

namespace danikk_engine
{
	using namespace internal;

	uint& Texture::handle()
	{
		return container->texture_data.handle;
	}

    Texture::Texture(Texture& other) : Asset(other.container){}

    Texture::Texture(Texture&& other) : Asset(other.container){}

    Texture::Texture(AssetContainer* container) : Asset(container){}

    static inline uint generateErrorTexture()
    {
    	uint texture_handle;

    	static const char* data =
			"\255\0\255\255"
			"\255\255\255\255"
			"\255\0\255\255"
			"\255\255\255\255";

    	glexec
		(
        	texture_handle = glGenTexture();
    		glBindTexture(GL_TEXTURE_2D, texture_handle);
    		setupDefaultTextureParameters();
    		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
		)
		return texture_handle;
    }

	void Texture::free()
	{
    	this->~Asset();
	}

    Texture::Texture(const char* name, int filter)
    {
    	AssetContainer** container_ptr = assets.get(name);
    	AssetContainer* container;

    	if(container_ptr == NULL)
    	{
            container = new (structalloc<AssetContainer>()) AssetContainer(asset_type::texture, name);
            TextureData& texture_data = container->texture_data;

            uint texture_handle;
    		int width = 0;
    		int height = 0;
            int channels = 0;
            int format = 0;

            if(loadDataToBuffer("textures", name, "png", false))
            {
                char* data = (char*)stb::load_from_memory((uint8*)asset_load_buffer.data(),
                	asset_load_buffer.size(), &width, &height, &channels, 0);

            	if(channels >= 3 && channels <= 4)
				{
                	if(channels == 3)
                	{
                		format = GL_RGB;
                	}
                	else if(channels == 4)
                	{
                		format = GL_RGBA;
                	}

                    glexec
            		(
            			texture_handle = glGenTexture();
            			glBindTexture(GL_TEXTURE_2D, texture_handle);
            			setupDefaultTextureParameters(filter);
            			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            			glBindTexture(GL_TEXTURE_2D, 0);
            		)
				}
            	else
            	{
            		texture_handle = generateErrorTexture();
            		danikk_framework::formatLogWarning("Mismatch color channels for texture \"%\"", name);
            	}

                ::free(data);
            }
            else
			{
                texture_handle = generateErrorTexture();
                danikk_framework::formatLogWarning("Failed to load texture %", name);
			}
            texture_data.handle = texture_handle;
            texture_data.width = width;
            texture_data.height = height;
            assets.insert(name, container);
    	}
    	else
    	{
    		container = *container_ptr;
    	}

    	new (this) Asset(container);
    }

    Texture& Texture::operator=(Texture& other)
    {
    	new (this) Texture(other);
    	return *this;
    }

    Texture& Texture::operator=(Texture&& other)
    {
    	this->~Asset();
    	new (this) Texture(other);
    	return *this;
    }

	void Texture::bind(uint32 index)
	{
		bindTexture(this->container->texture_data.handle, 0);
	}

    void Texture::draw(const mat4& world, const mat4& uv, const vec4& color)
    {
    	setWorldMatrix(world);
    	setUVMatrix(uv);
    	setDrawColor(color);
    	drawSpriteMesh();
    }

    Sprite Texture::createSprite(float x, float y, float width, float height)
    {
    	Sprite result;

    	result.container = container;
    	container->ref_count++;

    	result.uv = create_uv_matrix(x, y, width, height);
    	return result;
    }

    Sprite Texture::createSprite(uint x, uint y, uint width, uint height)
    {
    	Sprite result;

    	result.container = container;
    	container->ref_count++;

    	uint texture_width = container->texture_data.width;
    	uint texture_height = container->texture_data.height;

    	result.uv = create_uv_matrix(x,y, width, height, texture_width, texture_height);
    	return result;
    }

	Sprite Texture::createSpriteByIndex(uint32 index, uint32 sprite_size, uint32 sprite_count_at_row)
	{
		return createSprite(index % sprite_count_at_row, index / sprite_count_at_row, sprite_size, sprite_size);
	}

    Sprite Texture::createSprite()
    {
    	Sprite result;

    	result.container = container;
    	container->ref_count++;

    	float uv_x_scale = 1;
    	float uv_y_scale = 1;
    	float uv_x_offset = 0;
    	float uv_y_offset = 0;

    	result.uv = create_uv_matrix(uv_x_offset, uv_y_offset, uv_x_scale, uv_y_scale);//uv будет во всю текстуру.

    	return result;
    }
}
