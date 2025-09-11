#include <danikk_engine/key_bind.h>
#include <danikk_engine/input.h>

namespace danikk_engine
{
	KeyBindArray::KeyBind::KeyBind(const KeyBindKeys& keys, key_bind_event_t event)
	{
		this->keys = keys;
		this->event = (void*)event;
		this->arg_ptr = NULL;
	}

	KeyBindArray::KeyBind::KeyBind(const KeyBindKeys& keys, key_bind_args_event_t event, void* arg_ptr)
	{
		this->keys = keys;
		this->event = (void*)event;
		this->arg_ptr = arg_ptr;
	}

	void KeyBindArray::KeyBind::execute()
	{
		if(arg_ptr == NULL)
		{
			((key_bind_event_t)this->event)();
		}
		else
		{
			((key_bind_args_event_t)this->event)(arg_ptr);
		}
	}

	KeyBindArray::KeyBind* KeyBindArray::getKeyBind(const KeyBindKeys& keys)
	{
		return data.findWhere([&](KeyBind& key_bind)
				{return key_bind.keys == keys;});//НЕ РАБОТАЕТ
	}

	void KeyBindArray::bind(const KeyBindKeys& keys, key_bind_event_t event)
	{
		data.pushCtor(keys, event);
	}

	void KeyBindArray::bind(const KeyBindKeys& keys, key_bind_args_event_t event, void* arg_ptr)
	{
		data.pushCtor(keys, event, arg_ptr);
	}

	void KeyBindArray::unbind(const KeyBindKeys& keys)
	{
		KeyBind* bind = getKeyBind(keys);
		if(bind)
		{
			data.quickErase(*bind);
		}
	}

	void KeyBindArray::checkBindingPress()
	{
		for(KeyBind& bind : data)
		{
			if(bind.keys.size() > 1)
			{
				for(index_t i = 0; i < bind.keys.size(); i++)
				{
					int target_state = (i < bind.keys.size() - 1) ? button_states::hold : button_states::press;
					if(getKeyboardState(bind.keys[i]) != target_state)
					{
						goto end_of_bind;
					}
				}
			}
			else if(getKeyboardState(bind.keys[0]) != button_states::press)
			{
				goto end_of_bind;
			}
			bind.execute();
			end_of_bind:;
		}
	}
}
