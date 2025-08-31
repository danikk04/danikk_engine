#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/internal/asset.h>

namespace danikk_engine
{
	enum texture_filters
	{
		nearest,
		linear
	};

	typedef mat2 uv_matrix;

	void initTextureRenderer();

    void setupDefaultTextureParameters(int filter = texture_filters::nearest);

    void setWorldMatrix(const mat4& world);

    void setUVMatrix(const uv_matrix& uv);

    void setProjectionMatrix(const mat4& projection);

    void setViewMatrix(const mat4& view);

    void setDrawColor(const vec4& color);

    void bindTexture(uint handle, uint index);

	void drawSpriteMesh();
}
