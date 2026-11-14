#include <danikk_engine/danikk_engine.h>
#include <danikk_engine/mesh.h>
#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_engine/internal/gl_object_manager.h>

#include <danikk_framework/glm.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <limits>

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

	Mesh::Mesh(const char* model_name, const char* name) : Mesh()
	{
		loadObj(model_name, name);
	}

	/*
	 * Loads one OBJ file into a single GPU mesh. The parser collects all vertex
	 * positions, UVs, and normals using Assimp, triangulates faces, and appends
	 * every imported scene mesh to shared vertex and index buffers. Object, group,
	 * and material boundaries are ignored. Missing UVs become zero coordinates.
	 */
	bool Mesh::loadObj(const char* model_name, const char* name)
	{
		free();
		element_buffer_object = 0;
		vertex_buffer_object = 0;
		vertex_array_object = 0;
		indexes_count = 0;

		bool can_load;
		if(model_name == NULL)
		{
			can_load = loadDataToBuffer("meshes", name, "obj", true);
		}
		else
		{
			can_load = loadDataToBuffer("models", model_name, name, "obj", true);
		}
		if(!can_load) return false;

		DynamicArray<DefaultVertex> vertexes;
		DynamicArray<gl_point_index_t> indexes;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFileFromMemory(
			asset_load_buffer.data(),
			asset_load_buffer.size(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices,
			"obj");
		if(scene == nullptr || scene->mNumMeshes == 0)
		{
			formatLogWarning("Failed to import OBJ mesh: %", importer.GetErrorString());
			return false;
		}

		size_t triangle_count = 0;
		for(uint32 mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++)
		{
			const aiMesh& imported_mesh = *scene->mMeshes[mesh_index];
			for(uint32 face_index = 0; face_index < imported_mesh.mNumFaces; face_index++)
			{
				if(imported_mesh.mFaces[face_index].mNumIndices == 3)
				{
					triangle_count++;
				}
			}
		}
		const size_t expanded_vertex_count = triangle_count * 3;
		if(expanded_vertex_count > static_cast<size_t>(std::numeric_limits<gl_point_index_t>::max()) + 1)
		{
			formatLogWarning("OBJ mesh exceeds the 16-bit index limit");
			return false;
		}
		vertexes.reserve(expanded_vertex_count);
		indexes.reserve(expanded_vertex_count);

		for(uint32 mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++)
		{
			const aiMesh& imported_mesh = *scene->mMeshes[mesh_index];
			for(uint32 face_index = 0; face_index < imported_mesh.mNumFaces; face_index++)
			{
				const aiFace& face = imported_mesh.mFaces[face_index];
				if(face.mNumIndices != 3) continue;

				vec3 face_positions[3];
				for(uint8 vertex_index = 0; vertex_index < 3; vertex_index++)
				{
					const aiVector3D& position = imported_mesh.mVertices[face.mIndices[vertex_index]];
					face_positions[vertex_index] = vec3(position.x, position.y, position.z);
				}
				const vec3 face_normal = glm::normalize(glm::cross(
					face_positions[1] - face_positions[0],
					face_positions[2] - face_positions[0]));

				for(uint8 vertex_index = 0; vertex_index < 3; vertex_index++)
				{
					const uint32 imported_index = face.mIndices[vertex_index];
					vec3 normal = face_normal;
					if(imported_mesh.HasNormals())
					{
						const aiVector3D& imported_normal = imported_mesh.mNormals[imported_index];
						normal = vec3(imported_normal.x, imported_normal.y, imported_normal.z);
					}
					vec2 uv(0.0f);
					if(imported_mesh.HasTextureCoords(0))
					{
						const aiVector3D& imported_uv = imported_mesh.mTextureCoords[0][imported_index];
						uv = vec2(imported_uv.x, imported_uv.y);
					}
					vertexes.pushCtor(face_positions[vertex_index], normal, uv);
					indexes.push(static_cast<gl_point_index_t>(indexes.size()));
				}
			}
		}
		if(vertexes.size() == 0)
		{
			formatLogWarning("OBJ mesh contains no triangles");
			return false;
		}
		setData((float*)vertexes.data(), vertexes.size() * sizeof(DefaultVertex) / sizeof(float), indexes.data(), indexes.size());
		DefaultVertex::setAttributes();
		return true;
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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes_count * sizeof(gl_point_index_t), indexes, GL_STATIC_DRAW);
	}

	void Mesh::free()
	{
		if(element_buffer_object) glDeleteBuffer(element_buffer_object);
		if(vertex_buffer_object) glDeleteBuffer(vertex_buffer_object);
		if(vertex_array_object) glDeleteVertexArray(vertex_array_object);
		element_buffer_object = 0;
		vertex_buffer_object = 0;
		vertex_array_object = 0;
		indexes_count = 0;
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
		drawRange(0, indexes_count);
	}

	void Mesh::drawRange(uint32 first_index, uint32 index_count)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
		glBindVertexArray(vertex_array_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
		glDrawElements(
			GL_TRIANGLES,
			static_cast<GLsizei>(index_count),
			GL_UNSIGNED_INT,
			reinterpret_cast<void*>(static_cast<size_t>(first_index) * sizeof(gl_point_index_t)));
	}

	void vertexAttribFloatPointer(uint index, int size, int offset, int vertex_size)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, vertex_size, (void*)(size_t)offset);
	}
}
