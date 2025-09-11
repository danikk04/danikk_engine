#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/mesh.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_engine/internal/gl_object_manager.h>

#include <danikk_framework/misc/line_getter.h>
#include <danikk_framework/cstring_functions.h>
#include <danikk_framework/glm.h>

#include <glad/glad.h>

namespace danikk_engine
{
	using namespace internal;
	using namespace danikk_framework;

	Mesh::Mesh()
	{
		element_buffer_object =
		vertex_buffer_object =
		vertex_array_object =
		indexes_count = 0;
	}

	Mesh::Mesh(const char* model_name, const char* name)
	{
		bool can_load;
		if(model_name == NULL)
		{
			can_load = loadDataToBuffer("meshes", name, "obj", true);
		}
		else
		{
			can_load = loadDataToBuffer("models", model_name, name, "obj", true);
		}
		generateBuffers();
		DynamicArray<vec3> vertex_pos;
		DynamicArray<vec2> vertex_uv;
		DynamicArray<vec3> vertex_normal;
		DynamicArray<DefaultVertex> vertexes;
		DynamicArray<gl_point_index_t> indexes;

		if(can_load)
		{
			char* current_obj_ptr = asset_load_buffer.c_string();
			Array<char*, 16> splitted;
			Array<char, 8> splitters
			{
				' ',
				'/'
			};
			while (*current_obj_ptr != '\0')
			{
				char* line = getLine(current_obj_ptr);
				splitted.clear();
				strsplit(line, splitted, splitters);
				//SplitToFirstSelectedChar(OBJReader.ReadLine(), ' ', out string DataType, out string Value);
				char* obj_key = splitted[0];
				if(strequal(obj_key, "v"))
				{
					strreplacefirst8(splitted[1], '.', ',');
					strreplacefirst8(splitted[2], '.', ',');
					strreplacefirst8(splitted[3], '.', ',');
					vertex_pos.push(parseVec3(splitted.data() + 1));
				}
				else if(strequal(obj_key, "vt"))
				{
					strreplacefirst8(splitted[1], '.', ',');
					strreplacefirst8(splitted[2], '.', ',');
					vertex_uv.push(parseVec2(splitted.data() + 1));
				}
				else if(strequal(obj_key, "vn"))
				{
					strreplacefirst8(splitted[1], '.', ',');
					strreplacefirst8(splitted[2], '.', ',');
					strreplacefirst8(splitted[3], '.', ',');
					vertex_normal.push(parseVec3(splitted.data() + 1));
				}
				else if(strequal(obj_key, "f"))
				{
					char** current_data = splitted.data() + 1;
					index_t base_index = vertexes.size();
					for(index_t i = 0; i < 3; i++)
					{
						uvec3 index_vec = parseUvec3(current_data + i * 3);
						vertexes.pushCtor(
								vertex_pos[index_vec[0] - 1],
								vertex_normal[index_vec[1] - 1],
								vertex_uv[index_vec[2] - 1]);
						indexes.push(base_index + i);

					}
				}
			}
			setData((float*)vertexes.data(), vertexes.size() * sizeof(DefaultVertex) / sizeof(float), indexes.data(), indexes.size());
			DefaultVertex::setAttributes();
		}
	}

	void Mesh::generateBuffers()
	{
    	vertex_buffer_object = glGenBuffer();
    	vertex_array_object = glGenVertexArray();
		element_buffer_object = glGenBuffer();
	}

	void Mesh::setData(const float* vertexes, size_t vertexes_array_size, const gl_point_index_t* indexes,  size_t indexes_count)
	{
		if(vertex_buffer_object == 0)
		{
			generateBuffers();
		}
		this->indexes_count = indexes_count;
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
		glBufferData(GL_ARRAY_BUFFER, vertexes_array_size * sizeof(float), vertexes, GL_STATIC_DRAW);

		glBindVertexArray(vertex_array_object);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_count * sizeof(uint), indexes, GL_STATIC_DRAW);
	}

	void Mesh::free()
	{
		if(isNull())
		{
			return;
		}
		glDeleteBuffer(element_buffer_object);
		glDeleteBuffer(vertex_buffer_object);
		glDeleteVertexArray(vertex_array_object);
	}

	bool Mesh::isNull()
	{
		return !(vertex_buffer_object && element_buffer_object);
	}

	/*void Mesh::bind()
	{
		glBindVertexArray(vertex_array_object);
	}

	void Mesh::unbind()
	{
		glBindVertexArray(0);
	}*/

	void Mesh::draw()
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
		glBindVertexArray(vertex_array_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
		glDrawElements(GL_TRIANGLES, indexes_count, GL_UNSIGNED_SHORT, (void*)(0));
	}

	void vertexAttribFloatPointer(uint index, int size, int offset, int vertex_size)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, vertex_size, (void*)(size_t)offset);
	}
}
