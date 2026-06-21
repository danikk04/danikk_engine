#pragma once


#include <danikk_framework/danikk_framework.h>
#include <danikk_framework/static_string.h>
#include <danikk_framework/dynamic_array.h>
#include <danikk_framework/array.h>
#include <danikk_framework/string.h>
#include <danikk_framework/log.h>
#include <danikk_framework/assert.h>
#include <danikk_framework/memory.h>
#include <danikk_framework/glm.h>

namespace danikk_engine
{
	#define GLOBAL_DF_USE(NAME) using danikk_framework::NAME;
	GLOBAL_DF_USE(byte)
	GLOBAL_DF_USE(int8)
	GLOBAL_DF_USE(uint8)
	GLOBAL_DF_USE(int16)
	GLOBAL_DF_USE(uint16)
	GLOBAL_DF_USE(int32)
	GLOBAL_DF_USE(uint32)
	GLOBAL_DF_USE(int64)
	GLOBAL_DF_USE(uint64)
	GLOBAL_DF_USE(index_t)

	GLOBAL_DF_USE(bool8)
	GLOBAL_DF_USE(bool16)
	GLOBAL_DF_USE(bool32)
	GLOBAL_DF_USE(bool64)

	GLOBAL_DF_USE(DynamicArray)
	GLOBAL_DF_USE(MemoryBuffer)
	GLOBAL_DF_USE(String)
	GLOBAL_DF_USE(StaticString)
	GLOBAL_DF_USE(Array)

	GLOBAL_DF_USE(InitList)

	GLOBAL_DF_USE(logInfo)
	GLOBAL_DF_USE(logWarning)
	GLOBAL_DF_USE(logError)
	GLOBAL_DF_USE(logFatal)

	GLOBAL_DF_USE(structalloc)
	#undef GLOBAL_DF_USE

	using namespace glm;
	using std::move;
}
