#include <glad/glad.h>

#include <danikk_engine/model.h>

#include <danikk_engine/internal/data_manager.h>
#include <danikk_engine/internal/glexec.h>
#include <danikk_framework/memory_buffer.h>


namespace danikk_engine
{
	using namespace internal;

    Model::Model(const char* name, bool is_sinle_meshed_obj)
    {
    	AssetContainer** container_ptr = assets.get(name);
    	AssetContainer* container;

    	if(container_ptr == NULL)
    	{
            container = new (structalloc<AssetContainer>()) AssetContainer(asset_type::model, name);


    	}
    	else
    	{
    		container = *container_ptr;
    	}

    	new (this) Asset(container);
    }

    Model::~Model()
    {
    }

    void Model::draw()
    {

    }
}
