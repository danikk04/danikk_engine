#include <danikk_engine/dynamic_mesh.h>
#include <danikk_engine/built_in_meshes.h>
#include <danikk_engine/danikk_engine.h>
#include <danikk_framework/assert.h>
#include <danikk_framework/glm.h>

namespace danikk_engine
{
	Mesh cube_mesh;

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
		DynamicMesh<DefaultVertex> dynamic_cube;
		float start = -0.50f;
		float end = 1.0f;
		float step = 1.0f;
		for(float x = start; x <end; x += step)
		{
			dynamic_cube.addSquare(vec3(x, 0, 0), vec3(x * 2, 0, 0));
		}
		for(float y = start; y < end; y += step)
		{
			dynamic_cube.addSquare(vec3(0, y, 0), vec3(0, y * 2, 0));
		}
		for(float z = start; z < end; z += step)
		{
			dynamic_cube.addSquare(vec3(0, 0, z), vec3(0, 0, z * 2));
		}

		dynamic_cube.setDataToMesh(cube_mesh);
		DefaultVertex::setAttributes();
		dynamic_sprite_mesh.setDataToMesh(sprite_mesh);
		DefaultVertex::setAttributes();
	}
}
