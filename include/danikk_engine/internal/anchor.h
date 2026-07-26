#pragma once

#include <danikk_framework/assert.h>

namespace danikk_engine
{
	struct GUIAnchor
	{
		static constexpr int none = 0;
		static constexpr int top = 1;
		static constexpr int top_right = 2;
		static constexpr int right = 3;
		static constexpr int down_right = 4;
		static constexpr int down = 5;
		static constexpr int down_left = 6;
		static constexpr int left = 7;
		static constexpr int top_left = 8;
		static constexpr int center = 9;
	};

	struct TextAnchor
	{
		static constexpr int none = 0;
		static constexpr int multiline = 1;
		static constexpr int center = 2;
	};

	inline vec2 applyAnchor(const vec2& pos, const vec2& size, const vec2& parent_size, int& anchor)
	{
		#define PRIMARY(c) pos.c * parent_size.c
		#define MID(c) (parent_size.c - size.c) * 0.5f + pos.c * parent_size.c
		#define ALTER(c) parent_size.c - pos.c * parent_size.c - size.c

		#define LEFT_X PRIMARY(x)
		#define MID_X MID(x)
		#define RIGHT_X ALTER(x)

		#define DOWN_Y PRIMARY(y)
		#define MID_Y MID(y)
		#define TOP_Y ALTER(y)

		switch(anchor)
		{
			#define CASE(name, x, y) case GUIAnchor::name: return vec2(x, y);
			CASE(top,  		MID_X	, TOP_Y)
			CASE(top_right,	RIGHT_X	, TOP_Y)
			CASE(right,		RIGHT_X	, MID_Y)
			CASE(down_right,RIGHT_X	, DOWN_Y)
			CASE(down,		MID_X	, DOWN_Y)
			CASE(down_left,	LEFT_X	, DOWN_Y)
			CASE(left,		LEFT_X	, MID_Y)
			CASE(top_left, 	LEFT_X	, TOP_Y)
			CASE(center, 	MID_X	, MID_Y)
			case GUIAnchor::none: return vec2(0,0);
			default: return vec2(0,0);
			#undef CASE
		}

		#undef PRIMARY
		#undef MID
		#undef ALTER

		#undef LEFT_X
		#undef MID_X
		#undef RIGHT_X

		#undef DOWN_Y
		#undef MID_Y
		#undef TOP_Y
	}
}
