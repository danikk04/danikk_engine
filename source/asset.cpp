#include <glad/glad.h>

#include <danikk_framework/memory.h>

#include <danikk_engine/shader.h>
#include <danikk_engine/texture.h>
#include <danikk_engine/font.h>

#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/internal/asset.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/internal/glexec.h>

using namespace danikk_framework;

namespace danikk_engine
{
	namespace internal
	{
		Asset::Asset(): container(NULL){}

		Asset::Asset(AssetContainer* container)
		: container(container)
		{
			if(container == NULL)
			{
				return;
			}
			container->ref_count++;
		}

		Asset& Asset::operator=(Asset& other)
		{
			this->container = other.container;
			return *this;
		}

		Asset& Asset::operator=(Asset&& other)
		{
			this->container = other.container;
			return *this;
		}

		bool Asset::isNull()
		{
			return container == NULL;
		}

		Asset::~Asset()
		{
			if(container != NULL)
			{
				container->ref_count--;

				if(container->ref_count == 0)
				{
					container->type->free_func(*container);
				}
			}
		}

		AssetContainer::AssetContainer(size_t type, const String& name) : type(&asset_types[type]), ref_count(0)
		{
			this->name = name;
			this->ref_count++;
		}

		static void freeTexture(AssetContainer& container)
		{
			glexec
			(
				glDeletetexture(container.texture_data.handle);
			)
		}

		static void freeShader(AssetContainer& container)
		{
			glexec
			(
				glDeleteShader(container.shader_data.handle);
			)
		}

		static void freeFont(AssetContainer& container)
		{
			(void)container;
		}

		static void freeModel(AssetContainer& container)
		{
			logWarning("freeModel is not impl.");
		}

		AssetType asset_types[4];

		void initAssetTypes()
		{
			asset_types[asset_type::texture] = {"texture", freeTexture};
			asset_types[asset_type::shader] = { "shader", freeShader};
			asset_types[asset_type::font] 	= { "font", freeFont};
			asset_types[asset_type::model] 	= { "model", freeModel};
		}
	}
}
