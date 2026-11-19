#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/internal/asset.h>

namespace danikk_engine
{
	namespace shader_layout_locations
	{
		static constexpr uint vertex_pos = 0;
		static constexpr uint vertex_normal = 1;
		static constexpr uint vertex_uv = 2;

		static constexpr uint world = 3;
		static constexpr uint view = 4;
		static constexpr uint projection = 5;
		static constexpr uint uv = 6;
		static constexpr uint color = 7;
		static constexpr uint time_factor = 8;
		static constexpr uint alpha_discard_enabled = 9;
	}

	namespace internal
	{
		void initShaderCompiler();
	}

    class Shader : private internal::Asset
    {
    private:
    	size_t handle();
    public:
        Shader() = default;

        Shader(Shader& other);

        void load(const char* name);

        Shader(internal::AssetContainer* container);

        void operator=(Shader& other);

        void operator=(Shader&& other);

        void use(bool force = false);

        void unuse();
    };
}
