#define STB_IMAGE_IMPLEMENTATION 1
#define STBI_ONLY_PNG 1
#include <glad/glad.h>
#include <zip.h>

#include <danikk_framework/memory_buffer.h>
#include <danikk_framework/filestream.h>
#include <danikk_framework/filesystem.h>
#include <danikk_framework/path_buffer.h>
#include <danikk_framework/static_string.h>
#include <danikk_framework/cstring_functions.h>
#include <danikk_framework/zip.h>
#include <danikk_framework/memory.h>
#include <danikk_framework/format.h>

#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/shader.h>
#include <danikk_engine/texture.h>
#include <danikk_engine/font.h>
#include <danikk_engine/localization.h>

#include <danikk_engine/internal/gl_object_manager.h>
#include <danikk_engine/internal/asset.h>
#include <danikk_engine/internal/font.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_engine/internal/error.h>

namespace danikk_engine
{
	using namespace danikk_framework;
	using namespace internal;

    //Если data_directory пустой, то чтение идёт из архива.
    //Оба поля не должны быть NULL.
	static PathBuffer data_directory;
	static zip_t* data_archive = NULL;

	#define zip_is_exits data_directory.size() == 0

    namespace internal
	{
		MemoryBuffer asset_load_buffer;
		String localpath_buffer;
		danikk_framework::CatalogedDictionary<AssetContainer*, pathSlash> assets;

		void initDataManager()
		{
			PathBuffer current_path_buffer;
			current_path_buffer
				.getExecutableDirectory()
				.setFileName("data.zip");

			int error;

			data_archive = zip_open(current_path_buffer.c_string(), ZIP_RDONLY, &error);

			if(!error)
			{
				return;
			}
	    	formatLogWarning("DATAPATH : %", current_path_buffer.c_string());

			//В случае ошибки нужно будет искать директорию с
			//ресурсами в более верхних директориях.
			for(index_t i = 0; i < 3; i++)
			{
				current_path_buffer.pushDirectory("data");
				bool data_is_exits = exitsFile(current_path_buffer);
				if(data_is_exits)
				{
			    	formatLogInfo("DATAPATH : %", current_path_buffer.c_string());
					data_directory = current_path_buffer;
					return;
				}
				else
				{
			    	formatLogWarning("DATAPATH : %", current_path_buffer.c_string());
				}
				current_path_buffer.popDirectory();
				current_path_buffer.popDirectory();
			}
			fatalError("Cant find data");
		}

	    bool loadDataToBuffer(const char* asset_type, const char* subdir, const char* name, const char* extension, bool is_fatal)
	    {
	    	localpath_buffer.clear();
	        localpath_buffer << data_directory << asset_type << pathSlash << subdir << pathSlash << name << '.' << extension;
	        return loadDataToBuffer(localpath_buffer.c_string(), is_fatal);
	    }

	    bool loadDataToBuffer(const char* asset_type, const char* name, const char* extension, bool is_fatal)
	    {
	    	localpath_buffer.clear();
	        localpath_buffer << data_directory << asset_type << pathSlash << name << '.' << extension;
	        return loadDataToBuffer(localpath_buffer.c_string(), is_fatal);
	    }

	    bool loadDataToBuffer(const char* path, bool is_fatal)
	    {
	        if(zip_is_exits)
	        {
	            const char* zip_path = localpath_buffer.c_string();
	            zip_stat_t data_stat;

	            zip_stat_init(&data_stat);

	            int error = zip_stat(data_archive, zip_path, 0, &data_stat);

	            if(error)
	            {
	                return false;
	            }

	            zip_file* data_zip_file = zip_fopen(data_archive, zip_path , ZIP_RDONLY);

	            size_t data_size = data_stat.size;

	            asset_load_buffer.resize(data_size);

	            error = zip_fread(data_zip_file, asset_load_buffer.data(), data_size);

	            if(error)
	            {
	                return false;
	            }

	            zip_fclose(data_zip_file);
	        }
	        else
	        {

	        	FileReader reader;
	        	reader.open(path);
	        	if(!reader.isOpen())
	        	{
	            	if(is_fatal)
	            	{
		            	formatFatalError("Cant load file %", path);
	            	}
	            	else
	            	{
		            	formatLogWarning("Cant load file %", path);
	            	}
	                return false;
	        	}
	        	asset_load_buffer.resize(reader.size());
	        	reader.read(asset_load_buffer.data(), asset_load_buffer.size());
	        }

	        return true;
	    }
	}
}
