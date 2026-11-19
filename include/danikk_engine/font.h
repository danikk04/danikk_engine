#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/shader.h>
#include <danikk_engine/internal/asset.h>
#include <danikk_engine/color.h>
#include <danikk_engine/dynamic_mesh.h>

namespace danikk_engine
{
	/** @brief CPU and GPU geometry for one font atlas texture page. */
	struct FontMeshBatch
	{
		uint32 atlas_texture_handle = 0;
		DynamicMesh<DefaultVertex> geometry;
		Mesh mesh;

		FontMeshBatch() = default;
		FontMeshBatch(const FontMeshBatch&) = delete;
		FontMeshBatch& operator=(const FontMeshBatch&) = delete;
		FontMeshBatch(FontMeshBatch&&) noexcept = default;
		FontMeshBatch& operator=(FontMeshBatch&&) noexcept = default;
	};

	/** @brief Reusable text geometry grouped by the font atlas texture used by each glyph. */
	class FontMesh
	{
		DynamicArray<FontMeshBatch> batches;
		friend class Font;
	public:
		/** @brief Clears generated glyph geometry while retaining batches and GPU mesh capacity. */
		void clear();
		/** @brief Returns atlas batches for upload and drawing. */
		DynamicArray<FontMeshBatch>& getBatches() { return batches; }
	};

	class Font : private internal::Asset
	{
	public:
		Font() = default;

		void load(const char* name);

		Font(Font& other);

		Font(internal::AssetContainer* container);

        void operator=(Font& other);

        void operator=(Font&& other);

		void draw(const String& text, vec2 pos, vec2 size, TextStyle& style);

		void draw(const char* str, size_t str_size, vec2 pos, vec2 size, TextStyle& style);

		void draw(Array<uint32, 512>& chars, vec2 pos, vec2 size, TextStyle& style);

		/**
		 * @brief Appends one centered line of textured glyph quads to 3D atlas batches.
		 * @param mesh Reusable geometry accumulator; call FontMesh::clear before a new frame.
		 * @param text UTF-8 string whose characters are laid out with fixed-size quads.
		 * @param center Center of the text line in the caller's coordinate space.
		 * @param right Normalized horizontal axis of the text plane.
		 * @param up Normalized vertical axis of the text plane.
		 * @param glyph_size Width and height of each glyph quad in coordinate-space units.
		 * @param char_spacing Additional distance between adjacent glyphs.
		 * Glyphs are grouped by their lazily generated atlas texture, so each texture page can be
		 * uploaded and drawn as one mesh regardless of how many characters use it.
		 */
		void append3DText(FontMesh& mesh, const String& text, const vec3& center,
			const vec3& right, const vec3& up, const vec2& glyph_size, float char_spacing = 0.0f);
	};
}
