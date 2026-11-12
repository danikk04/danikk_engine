#pragma once

#include <danikk_engine/shader.h>
#include <danikk_engine/model.h>
#include <danikk_engine/texture.h>
#include <danikk_framework/memory_buffer.h>
#include <danikk_framework/path_buffer.h>

#include <danikk_framework/dictionary.h>

namespace danikk_engine
{
	namespace internal
	{
		void initDataManager();

		bool loadDataToBuffer(const char* asset_type, const char* subdir, const char* name, const char* extension, bool is_fatal);

		bool loadDataToBuffer(const char* asset_type, const char* name, const char* extension, bool is_fatal);

		bool loadDataToBuffer(const char* path, bool is_fatal);

	    extern MemoryBuffer asset_load_buffer;
	    extern String localpath_buffer;
	    extern danikk_framework::DynamicDictionary<danikk_framework::String, AssetContainer*> assets;
	}
}
