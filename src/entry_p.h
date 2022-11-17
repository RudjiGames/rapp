//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_ENTRY_P_H
#define RTM_RAPP_ENTRY_P_H

#include <rapp/inc/rapp.h>
#include <rapp/src/input.h>
#include <string.h> // memcpy

#ifndef ENTRY_CONFIG_MAX_GAMEPADS
#	define ENTRY_CONFIG_MAX_GAMEPADS 4
#endif // ENTRY_CONFIG_MAX_GAMEPADS

#define ENTRY_IMPLEMENT_EVENT(_class, _type) \
			_class(WindowHandle _handle) : Event(_type, _handle) {}

namespace rapp
{
	int main(int _argc, const char* const* _argv);

	struct Event
	{
		enum Enum
		{
			Axis,
			Char,
			Exit,
			Gamepad,
			GamepadButton,
			Key,
			Mouse,
			Size,
			Window,
			Suspend
		};

		Event(Enum _type)
			: m_type(_type)
		{
			m_handle.idx = UINT16_MAX;
		}

		Event(Enum _type, WindowHandle _handle)
			: m_type(_type)
			, m_handle(_handle)
		{
		}

		Event::Enum m_type;
		WindowHandle m_handle;
	};

	struct AxisEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(AxisEvent, Event::Axis);

		GamepadAxis::Enum m_axis;
		int32_t m_value;
		GamepadHandle m_gamepad;
	};

	struct CharEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(CharEvent, Event::Char);

		uint8_t m_len;
		uint8_t m_char[4];
	};

	struct GamepadEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(GamepadEvent, Event::Gamepad);

		GamepadHandle m_gamepad;
		bool m_connected;
	};

	struct GamepadButtonEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(GamepadButtonEvent, Event::GamepadButton);

		GamepadHandle m_gamepad;
		GamepadState::Buttons m_button;
		bool m_pressed;
	};

	struct KeyEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(KeyEvent, Event::Key);

		KeyboardState::Key m_key;
		uint8_t m_modifiers;
		bool m_down;
	};

	struct MouseEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(MouseEvent, Event::Mouse);

		int32_t m_mx;
		int32_t m_my;
		int32_t m_mz;
		MouseState::Button m_button;
		bool m_down;
		bool m_doubleClick;
		bool m_move;
		uint8_t m_modifiers;
	};

	struct SizeEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(SizeEvent, Event::Size);

		uint16_t m_width;
		uint16_t m_height;
	};

	struct WindowEvent : public Event
	{
		ENTRY_IMPLEMENT_EVENT(WindowEvent, Event::Window);

		void* m_nwh;
	};

	struct SuspendEvent : public Event
	{
		enum Enum
		{
			DidSuspend,
			DidResume,
			WillSuspend,
			WillResume
		};

		ENTRY_IMPLEMENT_EVENT(SuspendEvent, Event::Suspend);

		void* m_nwh;
		SuspendEvent::Enum m_eventState;
	};

	const Event* poll();
	const Event* poll(WindowHandle _handle);
	void release(const Event* _event);

	class EventQueue
	{
	public:
		EventQueue()
			: m_queue(2048)
		{}

		~EventQueue()
		{
			for (const Event* ev = poll(); NULL != ev; ev = poll() )
			{
				release(ev);
			}
		}

		void postAxisEvent(WindowHandle _handle, GamepadHandle _gamepad, GamepadAxis::Enum _axis, int32_t _value)
		{
			AxisEvent* ev = new AxisEvent(_handle);
			ev->m_gamepad = _gamepad;
			ev->m_axis    = _axis;
			ev->m_value   = _value;
			while(!m_queue.write(ev));
		}

		void postCharEvent(WindowHandle _handle, uint8_t _len, const uint8_t _char[4])
		{
			CharEvent* ev = new CharEvent(_handle);
			ev->m_len = _len;
			memcpy(ev->m_char, _char, 4);
			while(!m_queue.write(ev));
		}

		void postExitEvent()
		{
			Event* ev = new Event(Event::Exit);
			while(!m_queue.write(ev));
		}

		void postGamepadEvent(WindowHandle _handle, GamepadHandle _gamepad, bool _connected)
		{
			GamepadEvent* ev = new GamepadEvent(_handle);
			ev->m_gamepad   = _gamepad;
			ev->m_connected = _connected;
			while(!m_queue.write(ev));
		}

		void postGamepadButtonEvent(WindowHandle _handle, GamepadHandle _gamepad, GamepadState::Buttons _button, bool _pressed)
		{
			GamepadButtonEvent* ev = new GamepadButtonEvent(_handle);
			ev->m_gamepad	= _gamepad;
			ev->m_button	= _button;
			ev->m_pressed	= _pressed;
			while(!m_queue.write(ev));
		}

		void postKeyEvent(WindowHandle _handle, KeyboardState::Key _key, uint8_t _modifiers, bool _down)
		{
			KeyEvent* ev = new KeyEvent(_handle);
			ev->m_key       = _key;
			ev->m_modifiers = _modifiers;
			ev->m_down      = _down;
			ev->m_modifiers = _modifiers;
			while(!m_queue.write(ev));
		}

		void postMouseEvent(WindowHandle _handle, int32_t _mx, int32_t _my, int32_t _mz, uint8_t _modifiers)
		{
			MouseEvent* ev = new MouseEvent(_handle);
			ev->m_mx          = _mx;
			ev->m_my          = _my;
			ev->m_mz          = _mz;
			ev->m_button      = MouseState::Button::None;
			ev->m_down        = false;
			ev->m_move        = true;
			ev->m_modifiers   = _modifiers;
			ev->m_doubleClick = false;
			while(!m_queue.write(ev));
		}

		void postMouseEvent(WindowHandle _handle, int32_t _mx, int32_t _my, int32_t _mz, MouseState::Button _button, uint8_t _modifiers, bool _down, bool _double)
		{
			MouseEvent* ev = new MouseEvent(_handle);
			ev->m_mx          = _mx;
			ev->m_my          = _my;
			ev->m_mz          = _mz;
			ev->m_button      = _button;
			ev->m_down        = _down;
			ev->m_move        = false;
			ev->m_modifiers   = _modifiers;
			ev->m_doubleClick = _double;
			while(!m_queue.write(ev));
		}

		void postSizeEvent(WindowHandle _handle, uint32_t _width, uint32_t _height)
		{
			SizeEvent* ev = new SizeEvent(_handle);
			ev->m_width  = static_cast<uint16_t>(_width);
			ev->m_height = static_cast<uint16_t>(_height);
			while(!m_queue.write(ev));
		}

		void postWindowEvent(WindowHandle _handle, void* _nwh = NULL)
		{
			WindowEvent* ev = new WindowEvent(_handle);
			ev->m_nwh = _nwh;
			while(!m_queue.write(ev));
		}

		void postSuspendEvent(WindowHandle _handle, SuspendEvent::Enum _suspendState)
		{
			SuspendEvent* ev = new SuspendEvent(_handle);
			ev->m_eventState = _suspendState;
			while(!m_queue.write(ev));
		}

		const Event* poll()
		{
			Event* e = 0;
			m_queue.read(&e);
			return (Event*)e;
		}

		const Event* poll(WindowHandle _handle)
		{
			if (isValid(_handle) )
			{
				Event* ev = 0;
				m_queue.peek((void**)&ev);
				if (NULL == ev
				||  ev->m_handle.idx != _handle.idx)
				{
					return NULL;
				}
			}

			return poll();
		}

		void release(const Event* _event) const
		{
			delete _event;
		}

	private:
		rtm::SpScQueue<void*> m_queue;
	};

} // namespace rapp

#endif // RTM_RAPP_ENTRY_P_H
