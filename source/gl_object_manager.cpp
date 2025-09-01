#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/danikk_engine.h>
#include <danikk_framework/array.h>
#include <danikk_framework/log.h>
#include <danikk_framework/hex.h>
#include <glad/glad.h>

using namespace danikk_framework;

namespace danikk_engine
{
	namespace internal
	{
		typedef void (*gen_func_t)(GLsizei n, GLuint *arrays);

		static const char* genFuncToObjectManagerName(gen_func_t ptr)
		{
			if(ptr == glGenBuffers)
			{
				return "gl_buffer       ";
			}
			else if (ptr == glGenTextures)
			{
				return "gl_texture      ";
			}
			else if (ptr == glGenVertexArrays)
			{
				return "gl_vertex_array ";
			}
			else
			{
				return "ERR             ";
			}
		}

		struct gl_object_magager
		{
			//true - свободен, false - занят
			DynamicArray<uint32> free_objects;

			gen_func_t gen_func = NULL;

			void display_info(const char* action)
			{
				char hexbuffer[20]{'\0'};
				ptrtohex(hexbuffer, free_objects.data());
				formatLogDebug("%:% ptr:% size:%",
					action,
					genFuncToObjectManagerName(gen_func),
					hexbuffer,
					free_objects.size());
			}

			uint32 gen()
			{
				if(free_objects.empty())
				{
					free_objects.resize(64);
					gen_func(free_objects.size(), free_objects.data());
				}
				uint32 result = free_objects.pop();
				//display_info("gen");
				return result;
			}

			void del(uint32 object)
			{
				//display_info("del");
				if(free_objects.data() == NULL)
				{
					return;
				}
				free_objects.push(object);
			}
		};

		static gl_object_magager gl_buffers;
		static gl_object_magager gl_textures;
		static gl_object_magager gl_vertex_arrays;

		bool constexpr log_gl_allocation = false;

#define LOG(message) if(log_gl_allocation) logInfo(message)

		uint32 glGenBuffer()
		{
			LOG("gl buffer allocated");
			return gl_buffers.gen();
		}

		void glDeleteBuffer(uint32 buffer)
		{
			LOG("gl buffer free");
			gl_buffers.del(buffer);
		}

		uint32 glGenTexture()
		{
			LOG("gl texture allocated");
			return gl_textures.gen();
		}

		void glDeletetexture(uint32 texture)
		{
			LOG("gl texture free");
			gl_textures.del(texture);
		}

		uint32 glGenVertexArray()
		{
			LOG("gl vertex array allocated");
			return gl_vertex_arrays.gen();
		}

		void glDeleteVertexArray(uint32 vertex_array)
		{
			LOG("gl vertex array free");
			gl_vertex_arrays.del(vertex_array);
		}

		void initGlObjectManager()
		{
			gl_buffers.gen_func = glGenBuffers;
			gl_textures.gen_func = glGenTextures;
			gl_vertex_arrays.gen_func  = glGenVertexArrays;
		}
	}
}
