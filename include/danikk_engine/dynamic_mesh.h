#pragma once

#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/mesh.h>
#include <danikk_framework/dynamic_array.h>

namespace danikk_engine
{
	extern uvec2 square_tex_coords[4];
	extern gl_point_index_t square_end_indices[6];

	template<class vertex_t> class DynamicMesh
	{
	private:
		DynamicArray<vertex_t> vertexes;
		DynamicArray<gl_point_index_t> indexes;
	public:
		DynamicMesh() = default;
		DynamicMesh(const DynamicMesh&) = delete;
		DynamicMesh& operator=(const DynamicMesh& other)
		{
			indexes = other.indexes;
			vertexes = other.vertexes;
			return *this;
		}
		DynamicMesh(DynamicMesh&& other) noexcept
			: vertexes(static_cast<DynamicArray<vertex_t>&&>(other.vertexes)),
			  indexes(static_cast<DynamicArray<gl_point_index_t>&&>(other.indexes)) {}
		DynamicMesh& operator=(DynamicMesh&& other) noexcept
		{
			vertexes = static_cast<DynamicArray<vertex_t>&&>(other.vertexes);
			indexes = static_cast<DynamicArray<gl_point_index_t>&&>(other.indexes);
			return *this;
		}

		DynamicMesh(const InitList<vertex_t>& vertexes, const InitList<gl_point_index_t>& indexes)
		{
			new (&this->vertexes) DynamicArray<vertex_t>(vertexes);
			new (&this->indexes) DynamicArray<gl_point_index_t>(indexes);
		}

		void setDataToMesh(Mesh& target)
		{
			target.setData((float*)vertexes.data(), vertexes.size() * sizeof(vertex_t) / sizeof(float), indexes.data(), indexes.size());
		}

		/** @brief Removes vertices and indices while retaining their allocated capacity. */
		void clear()
		{
			vertexes.clear();
			indexes.clear();
		}

		/** @brief Reports whether this builder contains any triangles. */
		bool empty() const { return indexes.size() == 0; }

		/** @brief Appends one quad with caller-provided vertices and canonical triangle winding. */
		void addQuad(const vertex_t* quad)
		{
			const gl_point_index_t first = static_cast<gl_point_index_t>(vertexes.size());
			for(uint8 corner = 0; corner < 4; ++corner) vertexes.push(quad[corner]);
			const gl_point_index_t quad_indices[] = {
				first, first + 1, first + 2,
				first, first + 2, first + 3};
			for(gl_point_index_t index : quad_indices) indexes.push(index);
		}

		void addSquare(const vec3& pos, const vec3& normal, const vec2& uv_offset = vec2(0.0f), const vec2 uv_size = vec2(1.0f))
		{
			vec2 uv_offsets[4]
			{
				uv_offset,
				vec2(uv_offset.x + uv_size.x, uv_offset.y),
				uv_offset + uv_size,
				vec2(uv_offset.x, uv_offset.y + uv_size.y)
			};
			vertex_t poses[4]
			{
				vertex_t(vec3(0.5f, 0.0f, 0.5f), 	normal, 	uv_offsets[2]),
				vertex_t(vec3(-0.5f, 0.0f, 0.5f), 	normal,		uv_offsets[3]),
				vertex_t(vec3(-0.5f, 0.0f, -0.5f), 	normal,		uv_offsets[0]),
				vertex_t(vec3(0.5f, 0.0f, -0.5f), 	normal,		uv_offsets[1])
			};
			for(vertex_t& square_pos : poses)
			{
				if(normal.y == 1)
				{
					square_pos.pos = glm::rotateX(square_pos.pos, (float)pi * 0.5f);
					square_pos.pos = glm::rotateY(square_pos.pos, pi * 1.0f);
				}
				if(normal.y == -1)
				{
				}
				else
				{
					square_pos.pos = glm::rotateX(square_pos.pos, -0.5f * pi);
					float rotation = atan2(normal.x, normal.z);
					square_pos.pos = glm::rotateY(square_pos.pos, rotation);
				}
				square_pos.pos += pos;
			}
			addSquare((vertex_t*)poses);
		}

		void addSquare(vec3* poses, const vec3& normal)
		{
			gl_point_index_t first_index = vertexes.size();
			for(index_t i = 0; i < 4; i++)
			{
				DefaultVertex vertex;
				vertex.pos = poses[i];
				vertex.normal = normal;
				vertex.uv = square_tex_coords[i];
				vertexes.push(vertex);
			}

			for(gl_point_index_t i : square_end_indices)
			{
				indexes.push(first_index + i);
			}
		}

		void vertexesReserve(size_t size)
		{
			vertexes.reserve(size);
		}

		void addSquare(vertex_t* vertexes)
		{
			gl_point_index_t first_index = this->vertexes.size();
			for(index_t i = 0; i < 4; i++)
			{
				this->vertexes.push(vertexes[i]);
			}

			for(gl_point_index_t i : square_end_indices)
			{
				indexes.push(first_index + i);
			}
		}
	};

	void initBuiltInMeshes();
}
