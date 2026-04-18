#include <danikk_engine/gui.h>
#include <danikk_engine/input.h>
#include <danikk_engine/texture_methods.h>
#include <danikk_engine/matrix/uv.h>
#include <danikk_engine/matrix/pos.h>
#include <danikk_framework/assert.h>
#include <danikk_framework/log.h>
#include <danikk_framework/profiler.h>

namespace danikk_engine
{
	using namespace danikk_framework;
	using namespace internal;

	void GUIElement::recursiveDraw()
	{
		static RecursionWatcher rec_watcher;
		RecursionWatcher::Watch watch(rec_watcher);
		if(flags.get(GUIElementFlags::absolute_recalc))
		{
			flags.set(GUIElementFlags::absolute_recalc, false);
			absolute_size = parent->absolute_size * relative_size + getPixelSize() * pixel_size;
			absolute_pos = parent->absolute_pos + applyAnchor(pos, absolute_size, parent->absolute_size, anchor);
			danikk_framework::formatLogDebug("%% gui recalc pos:% % size:% %",
					danikk_framework::n_chars(' ', rec_watcher.get() - 1), name, absolute_pos.x, absolute_pos.y, absolute_size.x, absolute_size.y);
		}
		if(!flags.get(GUIElementFlags::visible))
		{
			return;
		}
		draw(absolute_pos, absolute_size);
		for(GUIElement* ch : childs)
		{
			ch->recursiveDraw();
		}
	}

	void GUIElement::draw(vec2 offset, vec2 scale)
	{
		if(text.size() > 0 && text_style != NULL)
		{
			text_style->draw(text, offset, scale);
		}
		if(draw_style != NULL)
		{
			draw_style->shader.use();
			sprite.draw(offset, scale, 0, draw_style->color);
		}
		if(border_draw_style != NULL)
		{
			border_draw_style->draw(offset, scale);
		}
	}

	void GUIElement::handleClick(int32 is_pushed)
	{
		if(!flags.get(GUIElementFlags::visible))
		{
			return;
		}
		vec2 cursor_pos = getCursorPos();
		if(	on_click != NULL &&
			cursor_pos.x > absolute_pos.x &&
			cursor_pos.y > absolute_pos.y &&
			cursor_pos.x < absolute_pos.x + absolute_size.x &&
			cursor_pos.y < absolute_pos.y + absolute_size.y &&
			!is_pushed)
		{
			on_click();
		}
		for(GUIElement* ch : childs)
		{
			ch->handleClick(is_pushed);
		}
	}

	void GUIElement::addChild(GUIElement& child)
	{
		childs.push(&child);
		child.parent = this;
	}

	GUIElement gui_root;
}
