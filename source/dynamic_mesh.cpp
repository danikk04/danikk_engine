#include <danikk_engine/dynamic_mesh.h>
#include <danikk_engine/built_in_meshes.h>
#include <danikk_engine/danikk_engine.h>
#include <danikk_framework/assert.h>
#include <danikk_framework/glm.h>

namespace danikk_engine
{

	uvec2 square_tex_coords[4]
	{
		uvec2(1, 0),
		uvec2(1, 1),
		uvec2(0, 1),
		uvec2(0, 0),
	};

	//в массиве из 4 элементов должны быть следующие индексы:
	//2, 3, 0
	//0, 1, 2
	gl_point_index_t square_end_indices[6]
	{
		2, 3, 0,
		0, 1, 2
	};

	void initBuiltInMeshes()
	{
		DynamicMesh<DefaultVertex> dynamic_sprite_mesh = DynamicMesh<DefaultVertex>
		(
			{
				DefaultVertex(0.5f,  0.5f, 0.0f,	0.0f,  0.0f, 1.0f,	1.0f, 0.0f),
				DefaultVertex(0.5f, -0.5f, 0.0f,	0.0f,  0.0f, 1.0f,	1.0f, 1.0f),
				DefaultVertex(-0.5f, -0.5f, 0.0f,	0.0f,  0.0f, 1.0f,	0.0f, 1.0f),
				DefaultVertex(-0.5f,  0.5f, 0.0f,	0.0f,  0.0f, 1.0f,	0.0f, 0.0f),
			},
			{
				0, 3, 2,
				2, 1, 0,
			}
		);
		DefaultVertex::setAttributes();
		dynamic_sprite_mesh.setDataToMesh(sprite_mesh);
		DefaultVertex::setAttributes();
	}
}
