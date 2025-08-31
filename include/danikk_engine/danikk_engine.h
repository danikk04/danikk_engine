#pragma once

#include <danikk_engine/internal/global_usings.h>
#include <danikk_engine/internal/error.h>

namespace danikk_engine
{
	extern float screen_ratio_gz;
	extern float screen_ratio_lz;

	static constexpr size_t gl_version_major = 4;
	static constexpr size_t gl_version_minor = 3;

    void exit();

    void setWindowTitle(const String& title);

    float gameTime();

    float getTimeFactor();

    float getTargetFPS();

    float getTargetFrameDelay();

    uint64 frameCount();

    vec2 getPixelSize();

	class Texture;
	class Sprite;
	class TextStyle;
	class Model;
}
