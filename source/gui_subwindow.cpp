#include <danikk_engine/gui/subwindow.h>

namespace danikk_engine
{
	SubWindow::SubWindow()
	{
		border_draw_style = &default_border_draw_style;
		flags.set(GUIElementFlags::draggable, true);
	}
}
