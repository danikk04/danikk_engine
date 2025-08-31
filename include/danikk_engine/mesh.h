#pragma once
#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/texture.h>

namespace danikk_engine
{
	struct Material
	{
		Texture diffuse_texture;
	};

	typedef uint16 gl_point_index_t;

	void vertexAttribFloatPointer(uint index, int size, int offset, int vertex_size);

	struct DefaultVertex
	{
		vec3 pos;
		vec3 normal;
		vec2 uv;

		bool operator==(DefaultVertex other)
		{
			return other.pos != pos && other.normal != normal && other.uv != uv;
		}

		DefaultVertex() = default;

		DefaultVertex(float posx, float posy, float posz,
				   float normalx, float normaly, float normalz,
				   float uvx, float uvz)
		{
			pos = vec3(posx, posy, posz);
			normal = vec3(normalx, normaly, normalz);
			uv = vec2(uvx, uvz);
		}

		DefaultVertex(vec3 pos, vec3 normal, vec2 uv)
		{
			this->pos = pos;
			this->normal = normal;
			this->uv = uv;
		}

		DefaultVertex(const DefaultVertex& other) = default;

		inline static void setAttributes()
		{
			vertexAttribFloatPointer(0, 3, 0,					sizeof(DefaultVertex));	//Позиция вершины.
			vertexAttribFloatPointer(1, 3, sizeof(float) * 3, 	sizeof(DefaultVertex));	//Нормаль вершины.
			vertexAttribFloatPointer(2, 2, sizeof(float) * 6, 	sizeof(DefaultVertex));	//UV координаты.
		}
	};

	struct Mesh
	{
		uint32 element_buffer_object;
		uint32 vertex_buffer_object;
		uint32 vertex_array_object;
		uint32 indexes_count;

		Mesh();

		Mesh(Mesh&) = delete;

		Mesh(Mesh&&) = delete;

		Mesh(const char* model_name, const char* name);//загрузка из obj файла

		void free();

		bool isNull();

		Mesh& operator=(Mesh&) = delete;

		Mesh& operator=(Mesh&&) = default;

		void generateBuffers();

		void setData(const float* vertexes, size_t vertexes_array_size, const gl_point_index_t* indexes,  size_t indexes_count);
		/*void bind();

		void unbind();*/

		void draw();
	};
}
