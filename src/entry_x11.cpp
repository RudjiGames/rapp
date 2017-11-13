//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_LINUX

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include <X11/Xlib.h> // will include X11 which #defines None... Don't mess with order of includes.
#include <X11/Xutil.h>

#if RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif

#include <unistd.h> // syscall

#undef None

#include <string>

#include <fcntl.h>

namespace rapp
{
	static const char* s_applicationName  = "RAPP";
	static const char* s_applicationClass = "rapp";

	///
	inline void x11SetDisplayWindow(void* _display, uint32_t _window, void* _glx = NULL)
	{
		RTM_UNUSED_3(_display, _window, _glx);
#if RAPP_WITH_BGFX
		bgfx::PlatformData pd;
		pd.ndt          = _display;
		pd.nwh          = (void*)(uintptr_t)_window;
		pd.context      = _glx;
		pd.backBuffer   = NULL;
		pd.backBufferDS = NULL;
		bgfx::setPlatformData(pd);
#endif
	}

#define JS_EVENT_BUTTON 0x01 /* button pressed/released */
#define JS_EVENT_AXIS   0x02 /* joystick moved */
#define JS_EVENT_INIT   0x80 /* initial state of device */

	struct JoystickEvent
	{
		uint32_t time;   /* event timestamp in milliseconds */
		int16_t  value;  /* value */
		uint8_t  type;   /* event type */
		uint8_t  number; /* axis/button number */
	};

	static GamepadState::Buttons s_translateButton[] =
	{
		GamepadState::Buttons::A,
		GamepadState::Buttons::B,
		GamepadState::Buttons::X,
		GamepadState::Buttons::Y,
		GamepadState::Buttons::LShoulder,
		GamepadState::Buttons::RShoulder,
		GamepadState::Buttons::Back,
		GamepadState::Buttons::Start,
		GamepadState::Buttons::Guide,
		GamepadState::Buttons::LThumb,
		GamepadState::Buttons::RThumb,
	};

	static GamepadAxis::Enum s_translateAxis[] =
	{
		GamepadAxis::LeftX,
		GamepadAxis::LeftY,
		GamepadAxis::LeftZ,
		GamepadAxis::RightX,
		GamepadAxis::RightY,
		GamepadAxis::RightZ,
	};

	struct AxisDpadRemap
	{
		GamepadState::Buttons first;
		GamepadState::Buttons second;
	};

	static AxisDpadRemap s_axisDpad[] =
	{
		{ GamepadState::Buttons::Left, GamepadState::Buttons::Right },
		{ GamepadState::Buttons::Up,   GamepadState::Buttons::Down  },
		{ GamepadState::Buttons::None, GamepadState::Buttons::None  },
		{ GamepadState::Buttons::Left, GamepadState::Buttons::Right },
		{ GamepadState::Buttons::Up,   GamepadState::Buttons::Down  },
		{ GamepadState::Buttons::None, GamepadState::Buttons::None  },
	};
	RTM_STATIC_ASSERT(RTM_NUM_ELEMENTS(s_translateAxis) == RTM_NUM_ELEMENTS(s_axisDpad) );

	struct Joystick
	{
		Joystick()
			: m_fd(-1)
		{
		}

		void init()
		{
			m_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

			memset(m_value, 0, sizeof(m_value) );

			// Deadzone values from xinput.h
			m_deadzone[GamepadAxis::LeftX ] =
			m_deadzone[GamepadAxis::LeftY ] = 7849;
			m_deadzone[GamepadAxis::RightX] =
			m_deadzone[GamepadAxis::RightY] = 8689;
			m_deadzone[GamepadAxis::LeftZ ] =
			m_deadzone[GamepadAxis::RightZ] = 30;
		}

		void shutdown()
		{
			if (-1 != m_fd)
			{
				close(m_fd);
			}
		}

		bool filter(GamepadAxis::Enum _axis, int32_t* _value)
		{
			const int32_t old = m_value[_axis];
			const int32_t deadzone = m_deadzone[_axis];
			int32_t value = *_value;
			value = value > deadzone || value < -deadzone ? value : 0;
			m_value[_axis] = value;
			*_value = value;
			return old != value;
		}

		bool update(EventQueue& _eventQueue)
		{
			if (-1 == m_fd)
			{
				return false;
			}

			JoystickEvent event;
			int32_t bytes = read(m_fd, &event, sizeof(JoystickEvent) );
			if (bytes != sizeof(JoystickEvent) )
			{
				return false;
			}

			WindowHandle defaultWindow = { 0 };
			GamepadHandle handle = { 0 };

			if (event.type & JS_EVENT_BUTTON)
			{
				if (event.number < RTM_NUM_ELEMENTS(s_translateButton) )
				{
					_eventQueue.postGamepadButtonEvent(defaultWindow, handle, s_translateButton[event.number], 0 != event.value);
				}
			}
			else if (event.type & JS_EVENT_AXIS)
			{
				if (event.number < RTM_NUM_ELEMENTS(s_translateAxis) )
				{
					GamepadAxis::Enum axis = s_translateAxis[event.number];
					int32_t value = event.value;
					if (filter(axis, &value) )
					{
						_eventQueue.postAxisEvent(defaultWindow, handle, axis, value);

						if (GamepadState::Buttons::None != s_axisDpad[axis].first)
						{
							if (m_value[axis] == 0)
							{
								_eventQueue.postAxisEvent(defaultWindow, handle, GamepadAxis::LeftZ, value);
								_eventQueue.postAxisEvent(defaultWindow, handle, GamepadAxis::LeftZ, value);
								//_eventQueue.postKeyEvent(defaultWindow, s_axisDpad[axis].first,  0, false);
								//_eventQueue.postKeyEvent(defaultWindow, s_axisDpad[axis].second, 0, false);
							}
							else
							{
								//_eventQueue.postKeyEvent(defaultWindow
								//	, 0 > m_value[axis] ? s_axisDpad[axis].first : s_axisDpad[axis].second
								//	, 0
								//	, true
								//	);
							}
						}

					}
				}
			}

			return true;
		}

		int m_fd;
		int32_t m_value[GamepadAxis::Count];
		int32_t m_deadzone[GamepadAxis::Count];
	};

	static Joystick s_joystick;

	static uint8_t s_translateKey[512];

	static void initTranslateKey(uint16_t _xk, KeyboardState::Key _key)
	{
		_xk += 256;
		RTM_ASSERT(_xk < RTM_NUM_ELEMENTS(s_translateKey), "Out of bounds %d.", _xk);
		s_translateKey[_xk&0x1ff] = (uint8_t)_key;
	}

	KeyboardState::Key fromXk(uint16_t _xk)
	{
		_xk += 256;
		return 512 > _xk ? (KeyboardState::Key)s_translateKey[_xk] : KeyboardState::Key::None;
	}

	struct MainThreadEntry
	{
		int32_t m_argc;
		const char* const* m_argv;

		static int32_t threadFunc(void* _userData);
	};

	struct Msg
	{
		Msg()
			: m_x(0)
			, m_y(0)
			, m_width(0)
			, m_height(0)
			, m_flags(0)
		{
		}

		int32_t  m_x;
		int32_t  m_y;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_flags;
		std::string m_title;
	};

	struct Context
	{
		Context()
			: m_modifiers(KeyboardState::Modifier::NoMods)
			, m_exit(false)
		{
			memset(s_translateKey, 0, sizeof(s_translateKey) );
			initTranslateKey(XK_Escape,       KeyboardState::Key::Esc);
			initTranslateKey(XK_Return,       KeyboardState::Key::Return);
			initTranslateKey(XK_Tab,          KeyboardState::Key::Tab);
			initTranslateKey(XK_BackSpace,    KeyboardState::Key::Backspace);
			initTranslateKey(XK_space,        KeyboardState::Key::Space);
			initTranslateKey(XK_Up,           KeyboardState::Key::Up);
			initTranslateKey(XK_Down,         KeyboardState::Key::Down);
			initTranslateKey(XK_Left,         KeyboardState::Key::Left);
			initTranslateKey(XK_Right,        KeyboardState::Key::Right);
			initTranslateKey(XK_Insert,       KeyboardState::Key::Insert);
			initTranslateKey(XK_Delete,       KeyboardState::Key::Delete);
			initTranslateKey(XK_Home,         KeyboardState::Key::Home);
			initTranslateKey(XK_KP_End,       KeyboardState::Key::End);
			initTranslateKey(XK_Page_Up,      KeyboardState::Key::PageUp);
			initTranslateKey(XK_Page_Down,    KeyboardState::Key::PageDown);
			initTranslateKey(XK_Print,        KeyboardState::Key::Print);
			initTranslateKey(XK_equal,        KeyboardState::Key::Plus);
			initTranslateKey(XK_minus,        KeyboardState::Key::Minus);
			initTranslateKey(XK_bracketleft,  KeyboardState::Key::LeftBracket);
			initTranslateKey(XK_bracketright, KeyboardState::Key::RightBracket);
			initTranslateKey(XK_semicolon,    KeyboardState::Key::Semicolon);
			initTranslateKey(XK_apostrophe,   KeyboardState::Key::Quote);
			initTranslateKey(XK_comma,        KeyboardState::Key::Comma);
			initTranslateKey(XK_period,       KeyboardState::Key::Period);
			initTranslateKey(XK_slash,        KeyboardState::Key::Slash);
			initTranslateKey(XK_backslash,    KeyboardState::Key::Backslash);
			initTranslateKey(XK_grave,        KeyboardState::Key::Tilde);
			initTranslateKey(XK_F1,           KeyboardState::Key::F1);
			initTranslateKey(XK_F2,           KeyboardState::Key::F2);
			initTranslateKey(XK_F3,           KeyboardState::Key::F3);
			initTranslateKey(XK_F4,           KeyboardState::Key::F4);
			initTranslateKey(XK_F5,           KeyboardState::Key::F5);
			initTranslateKey(XK_F6,           KeyboardState::Key::F6);
			initTranslateKey(XK_F7,           KeyboardState::Key::F7);
			initTranslateKey(XK_F8,           KeyboardState::Key::F8);
			initTranslateKey(XK_F9,           KeyboardState::Key::F9);
			initTranslateKey(XK_F10,          KeyboardState::Key::F10);
			initTranslateKey(XK_F11,          KeyboardState::Key::F11);
			initTranslateKey(XK_F12,          KeyboardState::Key::F12);
			initTranslateKey(XK_KP_Insert,    KeyboardState::Key::NumPad0);
			initTranslateKey(XK_KP_End,       KeyboardState::Key::NumPad1);
			initTranslateKey(XK_KP_Down,      KeyboardState::Key::NumPad2);
			initTranslateKey(XK_KP_Page_Down, KeyboardState::Key::NumPad3);
			initTranslateKey(XK_KP_Left,      KeyboardState::Key::NumPad4);
			initTranslateKey(XK_KP_Begin,     KeyboardState::Key::NumPad5);
			initTranslateKey(XK_KP_Right,     KeyboardState::Key::NumPad6);
			initTranslateKey(XK_KP_Home,      KeyboardState::Key::NumPad7);
			initTranslateKey(XK_KP_Up,        KeyboardState::Key::NumPad8);
			initTranslateKey(XK_KP_Page_Up,   KeyboardState::Key::NumPad9);
			initTranslateKey('0',             KeyboardState::Key::Key0);
			initTranslateKey('1',             KeyboardState::Key::Key1);
			initTranslateKey('2',             KeyboardState::Key::Key2);
			initTranslateKey('3',             KeyboardState::Key::Key3);
			initTranslateKey('4',             KeyboardState::Key::Key4);
			initTranslateKey('5',             KeyboardState::Key::Key5);
			initTranslateKey('6',             KeyboardState::Key::Key6);
			initTranslateKey('7',             KeyboardState::Key::Key7);
			initTranslateKey('8',             KeyboardState::Key::Key8);
			initTranslateKey('9',             KeyboardState::Key::Key9);
			initTranslateKey('a',             KeyboardState::Key::KeyA);
			initTranslateKey('b',             KeyboardState::Key::KeyB);
			initTranslateKey('c',             KeyboardState::Key::KeyC);
			initTranslateKey('d',             KeyboardState::Key::KeyD);
			initTranslateKey('e',             KeyboardState::Key::KeyE);
			initTranslateKey('f',             KeyboardState::Key::KeyF);
			initTranslateKey('g',             KeyboardState::Key::KeyG);
			initTranslateKey('h',             KeyboardState::Key::KeyH);
			initTranslateKey('i',             KeyboardState::Key::KeyI);
			initTranslateKey('j',             KeyboardState::Key::KeyJ);
			initTranslateKey('k',             KeyboardState::Key::KeyK);
			initTranslateKey('l',             KeyboardState::Key::KeyL);
			initTranslateKey('m',             KeyboardState::Key::KeyM);
			initTranslateKey('n',             KeyboardState::Key::KeyN);
			initTranslateKey('o',             KeyboardState::Key::KeyO);
			initTranslateKey('p',             KeyboardState::Key::KeyP);
			initTranslateKey('q',             KeyboardState::Key::KeyQ);
			initTranslateKey('r',             KeyboardState::Key::KeyR);
			initTranslateKey('s',             KeyboardState::Key::KeyS);
			initTranslateKey('t',             KeyboardState::Key::KeyT);
			initTranslateKey('u',             KeyboardState::Key::KeyU);
			initTranslateKey('v',             KeyboardState::Key::KeyV);
			initTranslateKey('w',             KeyboardState::Key::KeyW);
			initTranslateKey('x',             KeyboardState::Key::KeyX);
			initTranslateKey('y',             KeyboardState::Key::KeyY);
			initTranslateKey('z',             KeyboardState::Key::KeyZ);

			m_mx = 0;
			m_my = 0;
			m_mz = 0;
		}

		int32_t run(int _argc, const char* const* _argv)
		{
			XInitThreads();
			m_display = XOpenDisplay(0);

			int32_t screen = DefaultScreen(m_display);
			m_depth  = DefaultDepth(m_display, screen);
			m_visual = DefaultVisual(m_display, screen);
			m_root   = RootWindow(m_display, screen);

			memset(&m_windowAttrs, 0, sizeof(m_windowAttrs) );
			m_windowAttrs.background_pixmap = 0;
			m_windowAttrs.border_pixel = 0;
			m_windowAttrs.event_mask = 0
					| ButtonPressMask
					| ButtonReleaseMask
					| ExposureMask
					| KeyPressMask
					| KeyReleaseMask
					| PointerMotionMask
					| StructureNotifyMask
					;

			m_windows.allocate();
			Window w = XCreateWindow(m_display
									, m_root
									, 0, 0
									, 1, 1, 0
									, m_depth
									, InputOutput
									, m_visual
									, CWBorderPixel|CWEventMask
									, &m_windowAttrs
									);
			m_windows.setData(0, w);

			// Clear window to black.
			XSetWindowAttributes attr;
			memset(&attr, 0, sizeof(attr) );
			XChangeWindowAttributes(m_display, m_windows.getData(0), CWBackPixel, &attr);

			const char* wmDeleteWindowName = "WM_DELETE_WINDOW";
			Atom wmDeleteWindow;
			XInternAtoms(m_display, (char **)&wmDeleteWindowName, 1, False, &wmDeleteWindow);
			XSetWMProtocols(m_display, m_windows.getData(0), &wmDeleteWindow, 1);

			XMapWindow(m_display, m_windows.getData(0));
			XStoreName(m_display, m_windows.getData(0), s_applicationName);

			XClassHint* hint = XAllocClassHint();
			hint->res_name  = (char*)s_applicationName;
			hint->res_class = (char*)s_applicationClass;
			XSetClassHint(m_display, m_windows.getData(0), hint);
			XFree(hint);

			XIM im;
			im = XOpenIM(m_display, NULL, NULL, NULL);

			XIC ic;
			ic = XCreateIC(im
					, XNInputStyle
					, 0
					| XIMPreeditNothing
					| XIMStatusNothing
					, XNClientWindow
					, m_windows.getData(0)
					, NULL
					);

			//
			x11SetDisplayWindow(m_display, m_windows.getData(0));

			MainThreadEntry mte;
			mte.m_argc = _argc;
			mte.m_argv = _argv;

			rtm::Thread thread;
			thread.start(mte.threadFunc, &mte);

			WindowHandle defaultWindow = { 0 };
			m_eventQueue.postSizeEvent(defaultWindow, 1, 1);

			s_joystick.init();

			while (!m_exit)
			{
				bool joystick = s_joystick.update(m_eventQueue);
				bool xpending = XPending(m_display);

				if (!xpending)
				{
					rtm::Thread::sleep(joystick ? 8 : 16);
				}
				else
				{
					XEvent event;
					XNextEvent(m_display, &event);

					switch (event.type)
					{
						case Expose:
							break;

						case ClientMessage:
							if ( (Atom)event.xclient.data.l[0] == wmDeleteWindow)
							{
								m_eventQueue.postExitEvent();
							}
							break;

						case ButtonPress:
						case ButtonRelease:
							{
								const XButtonEvent& xbutton = event.xbutton;
								MouseState::Button mb = MouseState::Button::None;
								switch (xbutton.button)
								{
									case Button1: mb = MouseState::Button::Left;   break;
									case Button2: mb = MouseState::Button::Middle; break;
									case Button3: mb = MouseState::Button::Right;  break;
									case Button4: ++m_mz; break;
									case Button5: --m_mz; break;
								}

								WindowHandle handle = findHandle(xbutton.window);
								if (MouseState::Button::None != mb)
								{
									//m_eventQueue.postMouseEvent(handle
									//	, xbutton.x
									//	, xbutton.y
									//	, m_mz
									//	, mb
									//	, event.type == ButtonPress
									//	);
								}
								else
								{
									//m_eventQueue.postMouseEvent(handle
									//		, m_mx
									//		, m_my
									//		, m_mz
									//		);
								}
							}
							break;

						case MotionNotify:
							{
								const XMotionEvent& xmotion = event.xmotion;
								WindowHandle handle = findHandle(xmotion.window);

								m_mx = xmotion.x;
								m_my = xmotion.y;

								//m_eventQueue.postMouseEvent(handle
								//		, m_mx
								//		, m_my
								//		, m_mz
								//		);
							}
							break;

						case KeyPress:
						case KeyRelease:
							{
								XKeyEvent& xkey = event.xkey;
								KeySym keysym = XLookupKeysym(&xkey, 0);
								switch (keysym)
								{
								case XK_Meta_L:    setModifier(KeyboardState::Modifier::LMeta,  KeyPress == event.type); break;
								case XK_Meta_R:    setModifier(KeyboardState::Modifier::RMeta,  KeyPress == event.type); break;
								case XK_Control_L: setModifier(KeyboardState::Modifier::LCtrl,  KeyPress == event.type); break;
								case XK_Control_R: setModifier(KeyboardState::Modifier::RCtrl,  KeyPress == event.type); break;
								case XK_Shift_L:   setModifier(KeyboardState::Modifier::LShift, KeyPress == event.type); break;
								case XK_Shift_R:   setModifier(KeyboardState::Modifier::RShift, KeyPress == event.type); break;
								case XK_Alt_L:     setModifier(KeyboardState::Modifier::LAlt,   KeyPress == event.type); break;
								case XK_Alt_R:     setModifier(KeyboardState::Modifier::RAlt,   KeyPress == event.type); break;

								default:
									{
										WindowHandle handle = findHandle(xkey.window);
										if (KeyPress == event.type)
										{
											Status status = 0;
											uint8_t utf8[4];
											int len = Xutf8LookupString(ic, &xkey, (char*)utf8, sizeof(utf8), &keysym, &status);
											switch (status)
											{
											case XLookupChars:
											case XLookupBoth:
												if (0 != len)
												{
													m_eventQueue.postCharEvent(handle, len, utf8);
												}
												break;

											default:
												break;
											}
										}

										KeyboardState::Key key = fromXk(keysym);
										if (KeyboardState::Key::None != key)
										{
											m_eventQueue.postKeyEvent(handle, key, m_modifiers, KeyPress == event.type);
										}
									}
									break;
								}
							}
							break;

						case ConfigureNotify:
							{
								const XConfigureEvent& xev = event.xconfigure;
								WindowHandle handle = findHandle(xev.window);
								if (isValid(handle) )
								{
									m_eventQueue.postSizeEvent(handle, xev.width, xev.height);
								}
							}
							break;
					}
				}
			}

			thread.stop();

			s_joystick.shutdown();

			XDestroyIC(ic);
			XCloseIM(im);

			XUnmapWindow(m_display, m_windows.getData(0));
			XDestroyWindow(m_display, m_windows.getData(0));

			return thread.getExitCode();
		}

		void setModifier(KeyboardState::Modifier _modifier, bool _set)
		{
			m_modifiers &= ~_modifier;
			m_modifiers |= _set ? _modifier : 0;
		}

		void createWindow(WindowHandle _handle, Msg* msg)
		{
			Window window = XCreateWindow(m_display
									, m_root
									, msg->m_x
									, msg->m_y
									, msg->m_width
									, msg->m_height
									, 0
									, m_depth
									, InputOutput
									, m_visual
									, CWBorderPixel|CWEventMask
									, &m_windowAttrs
									);
			m_windows.setData(_handle.idx, window);

			// Clear window to black.
			XSetWindowAttributes attr;
			memset(&attr, 0, sizeof(attr) );
			XChangeWindowAttributes(m_display, window, CWBackPixel, &attr);

			const char* wmDeleteWindowName = "WM_DELETE_WINDOW";
			Atom wmDeleteWindow;
			XInternAtoms(m_display, (char **)&wmDeleteWindowName, 1, False, &wmDeleteWindow);
			XSetWMProtocols(m_display, window, &wmDeleteWindow, 1);

			XMapWindow(m_display, window);
			XStoreName(m_display, window, msg->m_title.c_str() );

			XClassHint* hint = XAllocClassHint();
			hint->res_name  = (char*)msg->m_title.c_str();
			hint->res_class = (char*)s_applicationClass;
			XSetClassHint(m_display, window, hint);
			XFree(hint);

			m_eventQueue.postSizeEvent(_handle, msg->m_width, msg->m_height);

			union cast
			{
				void* p;
				::Window w;
			};

			cast c;
			c.w = window;
			m_eventQueue.postWindowEvent(_handle, c.p);

			delete msg;
		}

		WindowHandle findHandle(Window _window)
		{
			rtm::ScopedMutexLocker scope(m_lock);

			for (uint32_t ii = 0, num = m_windows.size(); ii < num; ++ii)
			{
				Window w = m_windows.getData(ii);
				if (_window == w)
				{
					WindowHandle handle = { ii };
					return handle;
				}
			}

			WindowHandle invalid = { UINT16_MAX };
			return invalid;
		}

		uint8_t m_modifiers;
		bool m_exit;

		int32_t m_mx;
		int32_t m_my;
		int32_t m_mz;

		EventQueue m_eventQueue;
		rtm::Mutex m_lock;

		rtm::Data<Window, RAPP_MAX_WINDOWS, rtm::Storage::Dense>	m_windows;

		int32_t m_depth;
		Visual* m_visual;
		Window  m_root;

		XSetWindowAttributes m_windowAttrs;

		Display* m_display;
	};

	static Context s_ctx;

	int32_t MainThreadEntry::threadFunc(void* _userData)
	{
		MainThreadEntry* self = (MainThreadEntry*)_userData;
		int32_t result = main(self->m_argc, self->m_argv);
		s_ctx.m_exit = true;
		return result;
	}

	const Event* poll()
	{
		return s_ctx.m_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		return s_ctx.m_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		s_ctx.m_eventQueue.release(_event);
	}

	void appRunOnMainThread(App::threadFn _fn, void* _userData)
	{
		RTM_UNUSED_2(_fn, _userData);

	}

	void windowGetDefaultSize(uint32_t* _width, uint32_t* _height)
	{
		RTM_ASSERT(_width, "");
		RTM_ASSERT(_height, "");
		*_width		= 1920;
		*_height	= 1080;
	}

	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags, const char* _title)
	{
		RTM_UNUSED(_app);
		rtm::ScopedMutexLocker scope(s_ctx.m_lock);
		WindowHandle handle = { s_ctx.m_windows.allocate() };

		if (isValid(handle) )
		{
			Msg* msg = new Msg;
			msg->m_x      = _x;
			msg->m_y      = _y;
			msg->m_width  = _width;
			msg->m_height = _height;
			msg->m_title  = _title;
			msg->m_flags  = _flags;
			s_ctx.createWindow(handle, msg);
		}

		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		if (s_ctx.m_windows.isValid(_handle.idx))
		{
			s_ctx.m_eventQueue.postWindowEvent(_handle, NULL);
			Window w = s_ctx.m_windows.getData(_handle.idx);
			XUnmapWindow(s_ctx.m_display, w);
			XDestroyWindow(s_ctx.m_display, w);

			rtm::ScopedMutexLocker scope(s_ctx.m_lock);

			s_ctx.m_windows.free(_handle.idx);
		}
	}

	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		Display* display = s_ctx.m_display;
		Window   window  = s_ctx.m_windows.getData(_handle.idx);
		XMoveWindow(display, window, _x, _y);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		Display* display = s_ctx.m_display;
		Window   window  = s_ctx.m_windows.getData(_handle.idx);
		XResizeWindow(display, window, int32_t(_width), int32_t(_height) );
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		Display* display = s_ctx.m_display;
		Window   window  = s_ctx.m_windows.getData(_handle.idx);
		XStoreName(display, window, _title);
	}

	void windowToggleFrame(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		RTM_UNUSED_2(_handle, _lock);
	}

} // namespace rapp

int main(int _argc, const char* const* _argv)
{
	using namespace rapp;
	return s_ctx.run(_argc, _argv);
}

#endif // RTM_PLATFORM_LINUX
