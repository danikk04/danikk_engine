#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/internal/asset.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/sprite.h>

namespace danikk_engine
{
	class Texture : public internal::Asset
	{
	private:
		friend class Sprite;
	public:
		uint& handle();

		Texture() = default;

		Texture(Texture& other);

		Texture(Texture&& other);

		Texture(internal::AssetContainer* container);

		Texture(const char* name, int filter = texture_filters::nearest);

		void free();

		Texture& operator=(Texture& other);

		Texture& operator=(Texture&& other);

		Sprite createSprite();

		Sprite createSprite(float x, float y, float width, float height);

		Sprite createSprite(uint x, uint y, uint width, uint height);

		Sprite createSpriteByIndex(uint32 index, uint32 sprite_size, uint32 sprite_count_at_row);

		void bind(uint32 index = 0);

		void draw(const mat4& model, const mat4& uv, const vec4& color = vec4(1));
	};

	extern Texture white_texture;
}
