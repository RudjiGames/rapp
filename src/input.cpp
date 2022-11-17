//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <memory.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/rapp_config.h>
#include <rapp/src/entry_p.h>
#include <rapp/src/input.h>
#include <rapp/src/cmd.h>

#if RAPP_WITH_BGFX
#include <bgfx/bgfx.h>
#endif // RAPP_WITH_BGFX

namespace rapp {

InputBinding::InputBinding(int)	: m_type(InputBinding::Count) {}

InputBinding::InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingKeyboard& _kb)
	: m_type(InputBinding::Keyboard)
	, m_fn(_fn)
	, m_userData(_userData)
	, m_flags(_flag)
{
	m_bindingKeyboard = _kb;
}

InputBinding::InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingMouse& _mouse)
	: m_type(InputBinding::Mouse)
	, m_fn(_fn)
	, m_userData(_userData)
	, m_flags(_flag)
{
	m_bindingMouse = _mouse;
}

InputBinding::InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingGamepad& _gp)
	: m_type(InputBinding::Gamepad)
	, m_fn(_fn)
	, m_userData(_userData)
	, m_flags(_flag)
{
	m_bindingGamepad = _gp;
}

InputBinding::InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingTouch& _touch)
	: m_type(InputBinding::Touch)
	, m_fn(_fn)
	, m_userData(_userData)
	, m_flags(_flag)
{
	m_bindingTouch = _touch;
}

struct Mouse
{
	Mouse()
		: m_width(1280)
		, m_height(720)
		, m_wheelDelta(120)
		, m_lock(false)
	{
		m_absoluteOld[0] = 0;
		m_absoluteOld[1] = 0;
		m_absoluteOld[2] = 0;

		m_absolute[0] = 0;
		m_absolute[1] = 0;
		m_absolute[2] = 0;

		m_norm[0] = 0.0f;
		m_norm[1] = 0.0f;
		m_norm[2] = 0.0f;

		m_wheel = 0;

		memset(m_buttons, 0x00, sizeof(m_buttons));
		memset(m_once, 0xff, sizeof(m_once));
	}

	void reset()
	{
		if (m_lock)
		{
			m_norm[0] = 0.0f;
			m_norm[1] = 0.0f;
			m_norm[2] = 0.0f;
		}

		memset(m_buttons, 0, sizeof(m_buttons) );
	}

	void setResolution(uint16_t _width, uint16_t _height)
	{
		m_width = _width;
		m_height = _height;
	}

	void setPos(int32_t _mx, int32_t _my, int32_t _mz)
	{
		resetMovement();

		m_absolute[0] = _mx;
		m_absolute[1] = _my;
		m_absolute[2] = _mz;

		m_norm[0] = float(_mx)/float(m_width);
		m_norm[1] = float(_my)/float(m_height);
		m_norm[2] = float(_mz)/float(m_wheelDelta);
	}

	void resetMovement()
	{
		m_absoluteOld[0] = m_absolute[0];
		m_absoluteOld[1] = m_absolute[1];
		m_absoluteOld[2] = m_absolute[2];
	}

	void setButtonState(rapp::MouseState::Button _button, uint8_t _state)
	{
		m_buttons[_button] = _state;
		m_once[_button] = false;
	}

	int32_t m_absoluteOld[3];
	int32_t m_absolute[3];
	float m_norm[3];
	int32_t m_wheel;
	uint8_t m_buttons[MouseState::Button::Count];
	uint8_t m_once[MouseState::Button::Count];
	uint16_t m_width;
	uint16_t m_height;
	uint16_t m_wheelDelta;
	bool m_lock;
};

struct Keyboard
{
	Keyboard()
		: m_channel(1024)
	{
	}

	void reset()
	{
		memset(m_key, 0, sizeof(m_key) );
		memset(m_once, 0xff, sizeof(m_once) );
	}

	static uint32_t encodeKeyState(uint8_t _modifiers, bool _down)
	{
		uint32_t state = 0;
		state |= uint32_t(_down ? _modifiers : 0)<<16;
		state |= uint32_t(_down)<<8;
		return state;
	}

	static bool decodeKeyState(uint32_t _state, uint8_t& _modifiers)
	{
		_modifiers = (_state>>16)&0xff;
		return 0 != ( (_state>> 8)&0xff);
	}

	void setKeyState(KeyboardState::Key _key, uint8_t _modifiers, bool _down)
	{
		m_key[_key] = encodeKeyState(_modifiers, _down);
		m_once[_key] = false;
	}

	bool getKeyState(KeyboardState::Key _key, uint8_t* _modifiers)
	{
		uint8_t modifiers;
		_modifiers = NULL == _modifiers ? &modifiers : _modifiers;

		return decodeKeyState(m_key[_key], *_modifiers);
	}

	uint8_t getModifiersState()
	{
		uint8_t modifiers = 0;
		for (uint32_t ii = 0; ii < KeyboardState::Key::Count; ++ii)
		{
			modifiers |= (m_key[ii]>>16)&0xff;
		}
		return modifiers;
	}

	union CharQueue
	{
		uint32_t	m_int;
		uint8_t		m_chars[4];
	};

	void pushChar(uint8_t /*_len*/, const uint8_t _char[4])
	{
		CharQueue cq;
		memcpy(cq.m_chars, _char, 4);

		while (!m_channel.write(cq.m_int))
		{
			RAPP_DBG("No space in keyboard buffer!");
			uint8_t tmp_char[4];
			popChar(tmp_char);
		}

		m_channel.write(cq.m_int);
	}

	void popChar(uint8_t _char[4])
	{
		CharQueue cq;
		if (m_channel.read(&cq.m_int))
			memcpy(_char, cq.m_chars, 4);
		else
			memset(_char, 0, 4);
	}

	void charFlush()
	{
		m_channel.reset();
	}

	uint32_t m_key[256];
	bool m_once[256];

	rtm::SpScQueue<uint32_t> m_channel;
};

struct Gamepad
{
	Gamepad()
	{
		reset();
	}

	void reset()
	{
		memset(m_axis, 0, sizeof(m_axis));
		memset(m_axisOld, 0, sizeof(m_axis));
		m_buttons	= 0;
		m_once		= 0xffff;
		m_connected = false;
	}

	void setAxis(rapp::GamepadAxis::Enum _axis, int32_t _value)
	{
		m_axis[_axis] = _value;
	}

	int32_t getAxis(rapp::GamepadAxis::Enum _axis)
	{
		return m_axis[_axis];
	}

	void setButtonState(GamepadState::Buttons _button, bool _down)
	{
		if (_down)
			m_buttons |= _button;
		else
			m_buttons &= ~_button;

		m_once &= ~_button;
	}

	void resetMovement()
	{
		for (int i=0; i<rapp::GamepadAxis::Count; ++i)
			m_axisOld[i] = m_axis[i];
	}

	int32_t		m_axis[rapp::GamepadAxis::Count];
	int32_t		m_axisOld[rapp::GamepadAxis::Count];
	uint16_t	m_buttons;
	uint16_t	m_once;
	bool		m_connected;
};

struct Input
{
	Input()
		: m_inputBindingsMap(0)
	{
		reset();
	}

	~Input()
	{
	}

	void addBindings(const char* _name, const InputBinding* _bindings)
	{
		m_inputBindingsMap.insert(std::make_pair(_name, _bindings) );
	}

	void removeBindings(const char* _name)
	{
		InputBindingMap::iterator it = m_inputBindingsMap.find(_name);
		if (it != m_inputBindingsMap.end() )
		{
			m_inputBindingsMap.erase(it);
		}
	}

	void removeAllBindings()
	{
	}

	void execBinding(App* _app, const InputBinding* _binding)
	{
		if (NULL == _binding->m_fn)
		{
			cmdExec(_app, (const char*)_binding->m_userData, 0);
		}
		else
		{
			_binding->m_fn(_binding->m_userData, _binding);
		}
	}

	bool process(App* _app, const InputBinding* _bindings)
	{
		bool keyBindings = false;
		for (const InputBinding* binding = _bindings; binding->m_type != InputBinding::Count; ++binding)
		{
			switch (binding->m_type)
			{
			case InputBinding::Keyboard:
				{
					uint8_t modifiers;
					bool down =	Keyboard::decodeKeyState(m_keyboard.m_key[binding->m_bindingKeyboard.m_key], modifiers);

					if (binding->m_flags == 1)
					{
						if (down)
						{
							if (modifiers == binding->m_bindingKeyboard.m_modifiers
							&&  !m_keyboard.m_once[binding->m_bindingKeyboard.m_key])
							{
								execBinding(_app, binding);
								m_keyboard.m_once[binding->m_bindingKeyboard.m_key] = true;
								keyBindings = true;
							}
						}
						else
						{
							m_keyboard.m_once[binding->m_bindingKeyboard.m_key] = false;
						}
					}
					else
					{
						if (down
						&&  modifiers == binding->m_bindingKeyboard.m_modifiers)
						{
							execBinding(_app, binding);
							keyBindings = true;
						}
					}
				}
				break;

			case InputBinding::Mouse:
				{
					uint8_t modifiers = inputGetModifiersState();
					
					MouseState::Button button = binding->m_bindingMouse.m_button;

					if ((modifiers == binding->m_bindingMouse.m_modifiers) && (button == MouseState::Button::None))
					{
						if ((m_mouse.m_absolute[0] != m_mouse.m_absoluteOld[0]) ||
							(m_mouse.m_absolute[1] != m_mouse.m_absoluteOld[1]) ||
							(m_mouse.m_absolute[2] != m_mouse.m_absoluteOld[2]))
						{
							execBinding(_app, binding);
						}
						break;
					}

					bool down = m_mouse.m_buttons[button] != 0;

					if (binding->m_flags == 1)
					{
						if (down)
						{
							if (modifiers == binding->m_bindingMouse.m_modifiers
							&&  !m_mouse.m_once[button])
							{
								execBinding(_app, binding);
								m_mouse.m_once[binding->m_bindingMouse.m_button] = true;
							}
						}
						else
						{
							m_mouse.m_once[binding->m_bindingMouse.m_button] = false;
						}
					}
					else
					{
						if (down && (modifiers == binding->m_bindingMouse.m_modifiers))
						{
							execBinding(_app, binding);
						}
					}
				}
				break;

			case InputBinding::Gamepad:
				{
					GamepadState::Buttons button = binding->m_bindingGamepad.m_button;

					rapp::Gamepad& gp = m_gamepad[binding->m_bindingGamepad.m_gamepadIndex];

					if (button == GamepadState::Buttons::None)
					{
						RTM_ASSERT(binding->m_bindingGamepad.m_stick != InputBindingGamepad::Stick::None, "");

						switch (binding->m_bindingGamepad.m_stick)
						{
						case InputBindingGamepad::LeftStick:
							if ((gp.m_axis[GamepadAxis::LeftX] != gp.m_axisOld[GamepadAxis::LeftX]) ||
								(gp.m_axis[GamepadAxis::LeftY] != gp.m_axisOld[GamepadAxis::LeftY]))
								execBinding(_app, binding);
							break;

						case InputBindingGamepad::LeftTrigger:
							if ((gp.m_axis[GamepadAxis::LeftZ] != gp.m_axisOld[GamepadAxis::LeftZ]))
								execBinding(_app, binding);
							break;

						case InputBindingGamepad::RightStick:
							if ((gp.m_axis[GamepadAxis::RightX] != gp.m_axisOld[GamepadAxis::RightX]) ||
								(gp.m_axis[GamepadAxis::RightY] != gp.m_axisOld[GamepadAxis::RightY]))
								execBinding(_app, binding);
							break;

						case InputBindingGamepad::RightTrigger:
							if ((gp.m_axis[GamepadAxis::RightZ] != gp.m_axisOld[GamepadAxis::RightZ]))
								execBinding(_app, binding);
							break;

						default: RTM_ASSERT(false, "");
						}

						break;
					}

					bool down = (gp.m_buttons & button) != 0;

					if (binding->m_flags == 1)
					{
						if (down)
						{
							if (!(gp.m_once & button))
							{
								execBinding(_app, binding);
								gp.m_once |= button;
							}
						}
						else
						{
							gp.m_once &= ~button;
						}
					}
					else
					{
						if (down)
						{
							execBinding(_app, binding);
						}
					}
				}
				break;

			case InputBinding::Touch:
				break;

			default:
				RTM_ERROR("Should not reach here!");
				break;
			};

		}
		return keyBindings;
	}

	bool process(App* _app)
	{
		bool keyEvents = false;
		for (InputBindingMap::const_iterator it = m_inputBindingsMap.begin(); it != m_inputBindingsMap.end(); ++it)
		{
			keyEvents = keyEvents || process(_app, it->second);
		}
		return keyEvents;
	}

	void reset()
	{
		m_mouse.reset();
		m_keyboard.reset();
		for (uint32_t ii = 0; ii < RTM_NUM_ELEMENTS(m_gamepad); ++ii)
		{
			m_gamepad[ii].reset();
		}
	}

	typedef rtm_unordered_map<const char*, const InputBinding*> InputBindingMap;
	InputBindingMap	m_inputBindingsMap;
	Mouse			m_mouse;
	Keyboard		m_keyboard;
	Gamepad			m_gamepad[ENTRY_CONFIG_MAX_GAMEPADS];
};

Input& getInput()
{
	static Input s_input;
	return s_input;
}

void inputInit()
{
}

void inputShutdown()
{
}

void inputAddBindings(const char* _name, const InputBinding* _bindings)
{
	getInput().addBindings(_name, _bindings);
}

void inputRemoveBindings(const char* _name)
{
	getInput().removeBindings(_name);
}

bool inputProcess(App* _app)
{
	return getInput().process(_app);
}

void inputSetMouseResolution(uint16_t _width, uint16_t _height)
{
	getInput().m_mouse.setResolution(_width, _height);
}

void inputSetKeyState(KeyboardState::Key _key, uint8_t _modifiers, bool _down)
{
	getInput().m_keyboard.setKeyState(_key, _modifiers, _down);
}

bool inputGetKeyState(KeyboardState::Key _key, uint8_t* _modifiers)
{
	return getInput().m_keyboard.getKeyState(_key, _modifiers);
}

uint8_t inputGetModifiersState()
{
	return getInput().m_keyboard.getModifiersState();
}

void inputChar(uint8_t _len, const uint8_t _char[4])
{
	getInput().m_keyboard.pushChar(_len, _char);
}

void inputGetChar(uint8_t _char[4])
{
	return getInput().m_keyboard.popChar(_char);
}

void inputCharFlush()
{
	getInput().m_keyboard.charFlush();
}

void inputSetMousePos(int32_t _mx, int32_t _my, int32_t _mz)
{
	getInput().m_mouse.setPos(_mx, _my, _mz);
}

void inputResetMouseMovement()
{
	getInput().m_mouse.resetMovement();
}

void inputResetGamepadAxisMovement()
{
	for (int i=0; i<ENTRY_CONFIG_MAX_GAMEPADS; ++i)
		getInput().m_gamepad[i].resetMovement();
}

void inputSetMouseButtonState(MouseState::Button _button, uint8_t _state)
{
	getInput().m_mouse.setButtonState(_button, _state);
}

void inputGetMouse(float _mouse[3])
{
	_mouse[0] = getInput().m_mouse.m_norm[0];
	_mouse[1] = getInput().m_mouse.m_norm[1];
	_mouse[2] = getInput().m_mouse.m_norm[2];
}

bool inputIsMouseLocked()
{
	return getInput().m_mouse.m_lock;
}

void inputSetMouseLock(bool _lock)
{
	if (getInput().m_mouse.m_lock != _lock)
	{
		getInput().m_mouse.m_lock = _lock;
		windowSetMouseLock(rapp::kDefaultWindowHandle, _lock);
		if (_lock)
		{
			getInput().m_mouse.m_norm[0] = 0.0f;
			getInput().m_mouse.m_norm[1] = 0.0f;
			getInput().m_mouse.m_norm[2] = 0.0f;
		}
	}
}

///
void inputSetGamepadConnected(GamepadHandle _handle, bool _connected)
{
	getInput().m_gamepad[_handle.idx].m_connected = _connected;
}

void inputSetGamepadButtonState(GamepadHandle _handle, GamepadState::Buttons _button, bool _pressed)
{
	getInput().m_gamepad[_handle.idx].setButtonState(_button, _pressed);
}

void inputSetGamepadAxis(GamepadHandle _handle, GamepadAxis::Enum _axis, int32_t _value)
{
	getInput().m_gamepad[_handle.idx].setAxis(_axis, _value);
}

int32_t inputGetGamepadAxis(GamepadHandle _handle, GamepadAxis::Enum _axis)
{
	return getInput().m_gamepad[_handle.idx].getAxis(_axis);
}

void inputGetGamePadState(int _index, GamepadState& _gp)
{
	Gamepad& gamepad = getInput().m_gamepad[_index];

	_gp.m_buttons = gamepad.m_buttons;
	_gp.m_buttons |= gamepad.m_connected ? GamepadState::Connected : 0;

	_gp.m_LStick[0] = gamepad.m_axis[GamepadAxis::LeftX];
	_gp.m_LStick[1] = gamepad.m_axis[GamepadAxis::LeftY];
	_gp.m_LTrigger	= static_cast<int8_t> (gamepad.m_axis[GamepadAxis::LeftZ]);

	_gp.m_RStick[0] = gamepad.m_axis[GamepadAxis::RightX];
	_gp.m_RStick[1] = gamepad.m_axis[GamepadAxis::RightY];
	_gp.m_RTrigger	= static_cast<int8_t> (gamepad.m_axis[GamepadAxis::RightZ]);
}

void inputGetMouseState(MouseState& _ms)
{
	_ms.m_absolute[0] = getInput().m_mouse.m_absolute[0];
	_ms.m_absolute[1] = getInput().m_mouse.m_absolute[1];
	_ms.m_absolute[2] = getInput().m_mouse.m_absolute[2];

	_ms.m_norm[0] = getInput().m_mouse.m_norm[0];
	_ms.m_norm[1] = getInput().m_mouse.m_norm[1];
	_ms.m_norm[2] = getInput().m_mouse.m_norm[2];

	_ms.m_buttons[MouseState::Button::Left]		= getInput().m_mouse.m_buttons[MouseState::Button::Left];
	_ms.m_buttons[MouseState::Button::Middle]	= getInput().m_mouse.m_buttons[MouseState::Button::Middle];
	_ms.m_buttons[MouseState::Button::Right]	= getInput().m_mouse.m_buttons[MouseState::Button::Right];
}


void inputGetKeyboardState(KeyboardState& _ks)
{
	_ks.m_keysPressed				= 0;
	_ks.m_modifiersSinceLastFrame	= inputGetModifiersState();

	for (int i=1; i<KeyboardState::Key::Count; ++i)
	{
		uint8_t mod;
		if (inputGetKeyState((KeyboardState::Key)i, &mod))
		{
			_ks.m_modifiers	[_ks.m_keysPressed] = mod;
			_ks.m_keys		[_ks.m_keysPressed] = (KeyboardState::Key)i;
			++_ks.m_keysPressed;
		}
	}
}

bool inputGetMouseLock()
{
	return inputIsMouseLocked();
}

// 5 digits + sign maximum
char* itoaWithSign(int32_t _val, char _buf[8], int _displayChars, bool _addSign = true)
{
	memset(_buf, ' ', 8);
	_buf[7] = 0;
	char sign = ' ';

	if (_val == 0)
	{
		_buf[6] = '0';
		return &_buf[7 - _displayChars];
	}

	if (_addSign && (_val != 0))
	{
		sign = '+';
		if (_val < 0)
		{
			sign = '-';
			_val = -_val;
		}
	}
	
	int idx = 6;
	while (_val)
	{
		_buf[idx--] = '0' + (_val % 10);
		_val /= 10;
	};

	_buf[idx--] = sign;
	return &_buf[7-_displayChars];
}

void displayGamePadDbg(uint16_t _x, uint16_t _y, const GamepadState& _gp)
{
	RTM_UNUSED_3(_x, _y, _gp);
#if RAPP_WITH_BGFX
	if (_gp.m_buttons & GamepadState::Connected)
	{
		bgfx::dbgTextPrintf(_x, _y+0, 0x8b, "Tr[   ] S[ ]  Back Start Tr[   ]  S[ ]");
		bgfx::dbgTextPrintf(_x, _y+1, 0x8b, "X[      ]   [^]   X[      ]    [Y]    ");
		bgfx::dbgTextPrintf(_x, _y+2, 0x8b, "Y[      ] [<   >] Y[      ] [X]   [B] ");
		bgfx::dbgTextPrintf(_x, _y+3, 0x8b, "T[ ]        [v]   T[ ]         [A]    ");

		if (_gp.m_buttons & GamepadState::X)			bgfx::dbgTextPrintf(_x+28, _y+2, 0, "\x1b[7;1m[X]");
		if (_gp.m_buttons & GamepadState::Y)			bgfx::dbgTextPrintf(_x+31, _y+1, 0, "\x1b[7;6m[Y]");
		if (_gp.m_buttons & GamepadState::A)			bgfx::dbgTextPrintf(_x+31, _y+3, 0, "\x1b[7;2m[A]");
		if (_gp.m_buttons & GamepadState::B)			bgfx::dbgTextPrintf(_x+34, _y+2, 0, "\x1b[7;4m[B]");

		if (_gp.m_buttons & GamepadState::Up)			bgfx::dbgTextPrintf(_x+12, _y+1, 0x3b, "[^]");
		if (_gp.m_buttons & GamepadState::Down)			bgfx::dbgTextPrintf(_x+12, _y+3, 0x3b, "[v]");
		if (_gp.m_buttons & GamepadState::Left)			bgfx::dbgTextPrintf(_x+10, _y+2, 0x3b, "[<]");;
		if (_gp.m_buttons & GamepadState::Right)		bgfx::dbgTextPrintf(_x+14, _y+2, 0x3b, "[>]");

		if (_gp.m_buttons & GamepadState::LThumb)		bgfx::dbgTextPrintf(_x+2,  _y+3, 0x8f, "\xfe");
		if (_gp.m_buttons & GamepadState::RThumb)		bgfx::dbgTextPrintf(_x+20, _y+3, 0x8f, "\xfe");

		if (_gp.m_buttons & GamepadState::LShoulder)	bgfx::dbgTextPrintf(_x+10, _y, 0x8f, "\xfe");
		if (_gp.m_buttons & GamepadState::RShoulder)	bgfx::dbgTextPrintf(_x+36, _y, 0x8f, "\xfe");

		if (_gp.m_buttons & GamepadState::Start)		bgfx::dbgTextPrintf(_x+19, _y, 0x8a, "Start");
		if (_gp.m_buttons & GamepadState::Back)			bgfx::dbgTextPrintf(_x+14, _y, 0x8a, "Back");

		char valbuf[8]; // 6 chars + sign + trailing zero
	
		bgfx::dbgTextPrintf(_x+3,  _y, 0x87, itoaWithSign(_gp.m_LTrigger, valbuf, 3, false));
		bgfx::dbgTextPrintf(_x+28, _y, 0x87, itoaWithSign(_gp.m_RTrigger, valbuf, 3, false));

		bgfx::dbgTextPrintf(_x+2, _y+1, 0x87, itoaWithSign(_gp.m_LStick[0], valbuf, 6));
		bgfx::dbgTextPrintf(_x+2, _y+2, 0x87, itoaWithSign(_gp.m_LStick[1], valbuf, 6));

		bgfx::dbgTextPrintf(_x+20, _y+1, 0x87, itoaWithSign(_gp.m_RStick[0], valbuf, 6));
		bgfx::dbgTextPrintf(_x+20, _y+2, 0x87, itoaWithSign(_gp.m_RStick[1], valbuf, 6));
	}
	else
	{
		bgfx::dbgTextPrintf(_x, _y,   0xf, "#------------------------------------#");
		bgfx::dbgTextPrintf(_x, _y+1, 0xf, "|                 Not                |");
		bgfx::dbgTextPrintf(_x, _y+2, 0xf, "|              connected             |");
		bgfx::dbgTextPrintf(_x, _y+3, 0xf, "#------------------------------------#");
	}
#endif // RAPP_WITH_BGFX
}

void inputDgbGamePads(int _maxGamepads)
{
	RTM_UNUSED(_maxGamepads);
#if RAPP_WITH_BGFX
	const bgfx::Stats* stats = bgfx::getStats();

	GamepadState gp;

	for (uint16_t i=0; i<_maxGamepads; ++i)
	{
		inputGetGamePadState(i, gp);
		displayGamePadDbg(1+40*i, stats->textHeight-4, gp);
	}
#endif // RAPP_WITH_BGFX
}

void inputDgbMouse()
{
#if RAPP_WITH_BGFX
	const bgfx::Stats* stats = bgfx::getStats();

	bgfx::dbgTextPrintf(1, stats->textHeight-6, 0x8b, " Mouse X[    ] Y[    ] Z[     ]  NX[        ] NY[        ] NZ[        ] LB[ ] MB[ ] RB[ ]");

	Mouse& m = getInput().m_mouse;

	char valbuf[8];
	bgfx::dbgTextPrintf(10,  stats->textHeight-6, 0x8f, itoaWithSign(m.m_absolute[0], valbuf, 4, false));
	bgfx::dbgTextPrintf(18,  stats->textHeight-6, 0x8f, itoaWithSign(m.m_absolute[1], valbuf, 4, false));
	bgfx::dbgTextPrintf(26,  stats->textHeight-6, 0x8f, itoaWithSign(m.m_absolute[2], valbuf, 5, true));

	bgfx::dbgTextPrintf(37,  stats->textHeight-6, 0x8f, "%5f", m.m_norm[0]);
	bgfx::dbgTextPrintf(50,  stats->textHeight-6, 0x8f, "%5f", m.m_norm[1]);
	bgfx::dbgTextPrintf(63,  stats->textHeight-6, 0x8f, "%5f", m.m_norm[2]);

	if (m.m_buttons[MouseState::Button::Left])
		bgfx::dbgTextPrintf(76, stats->textHeight-6, 0x8f, "\xfe");

	if (m.m_buttons[MouseState::Button::Middle])
		bgfx::dbgTextPrintf(82, stats->textHeight-6, 0x8f, "\xfe");

	if (m.m_buttons[MouseState::Button::Right])
		bgfx::dbgTextPrintf(88, stats->textHeight-6, 0x8f, "\xfe");
#endif // RAPP_WITH_BGFX
}

void inputDgbKeyboard()
{
#if RAPP_WITH_BGFX
	const bgfx::Stats* stats = bgfx::getStats();

	bgfx::dbgTextPrintf(91, stats->textHeight-7, 0x8b, "Kb LShift[ ]                                              RShift[ ]");
	bgfx::dbgTextPrintf(91, stats->textHeight-6, 0x8b, "Kb LCtrl[ ] LMeta[ ] LAlt[ ]   ________   RAlt[ ] RMeta[ ] RCtrl[ ]");

	uint8_t modifiers = inputGetModifiersState();
	if (modifiers & KeyboardState::Modifier::LShift)	bgfx::dbgTextPrintf(101, stats->textHeight-7, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::LCtrl)		bgfx::dbgTextPrintf(100, stats->textHeight-6, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::LMeta)		bgfx::dbgTextPrintf(109, stats->textHeight-6, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::LAlt)		bgfx::dbgTextPrintf(117, stats->textHeight-6, 0x8f, "\xfe");

	if (modifiers & KeyboardState::Modifier::RShift)	bgfx::dbgTextPrintf(156, stats->textHeight-7, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::RAlt)		bgfx::dbgTextPrintf(138, stats->textHeight-6, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::RMeta)		bgfx::dbgTextPrintf(147, stats->textHeight-6, 0x8f, "\xfe");
	if (modifiers & KeyboardState::Modifier::RCtrl)		bgfx::dbgTextPrintf(156, stats->textHeight-6, 0x8f, "\xfe");

	KeyboardState ks;
	inputGetKeyboardState(ks);

	uint16_t startX = 108;
	for (int i=0; i<ks.m_keysPressed; ++i)
	{
		KeyboardState::Key key = ks.m_keys[i];
		const char* name = getName(key);
		bgfx::dbgTextPrintf(startX, stats->textHeight-7, 0x8f /* ks.m_modifiers[i]*/, "(%s) ", name);
		startX += static_cast<uint16_t>(strlen(name)) + 3;
	}
#endif // RAPP_WITH_BGFX
}

void inputDgbTouch()
{
}

} // namespace rapp
