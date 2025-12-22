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

	GUIElement::GUIElement()
	{
	}

	RecursionWatcher recursive_draw_recursion_watcher;

	void GUIElement::recursiveDraw()
	{
		static RecursionWatcher rec_watcher;
		RecursionWatcher::Watch watch(rec_watcher);
		if(!flags[GUIElementFlags::visible])
		{
			return;
		}
		if(flags[GUIElementFlags::absolute_recalc])
		{
			flags[GUIElementFlags::absolute_recalc] = false;
			absolute_size = parent->absolute_size * relative_size + getPixelSize() * pixel_size;
			absolute_pos = parent->absolute_pos + applyAnchor(pos, absolute_size, parent->absolute_size, anchor);
			danikk_framework::formatLogDebug("% gui recalc pos:% % size:% %",
					danikk_framework::n_chars(' ', rec_watcher.get() - 1), absolute_pos.x, absolute_pos.y, absolute_size.x, absolute_size.y);
		}
		draw(absolute_pos, absolute_size);
		for(GUIElement* ch : childs)
		{
			ch->recursiveDraw();
		}
	}

	void GUIElement::handleClick(int32 is_pushed)
	{
		if(!flags[GUIElementFlags::visible])
		{
			return;
		}
		Button* this_button = dynamic_cast<Button*>(this);
		if(this_button != NULL)
		{
			vec2 cursor_pos = getCursorPos();
			if(	cursor_pos.x > absolute_pos.x &&
				cursor_pos.y > absolute_pos.y &&
				cursor_pos.x < absolute_pos.x + absolute_size.x &&
				cursor_pos.y < absolute_pos.y + absolute_size.y &&
				this_button->on_click != NULL &&
				!is_pushed)
			{
				this_button->on_click();
			}
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

	void Panel::draw(vec2 offset, vec2 scale)
	{
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

	void Label::draw(vec2 offset, vec2 scale)
	{
		TextStyle* current_text_style = text_style ? text_style : &default_draw_styles::label_text;
		current_text_style->draw(text, offset, scale);
	}

	Button::Button()
	{

	}

	void Button::draw(vec2 offset, vec2 scale)
	{
		if(draw_panel)
		{
			Panel::draw(offset, scale);
		}
		text_style->draw(text, offset, scale);
	}

	/*void composeGUI(int compose_type, GUIElement& target, vec2 offset)
	{
		DynamicArray<GUIElement*>& elements = target.childs;
		GUIElement* non_size_element = NULL;
		vec2 summar_size = vec2();
		for(GUIElement* element : elements)
		{
			if(element->size == vec2())
			{
				non_size_element = element;
				break;
			}
			summar_size += element->size;
		}
		vec2 target_autosize;
		switch(compose_type)
		{
			case gui_compose_type::vertical:
			break;
		}
	}*/

	Panel gui_root;
}
