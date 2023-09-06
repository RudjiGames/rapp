//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#if RTM_PLATFORM_WINRT

#include <rapp/src/entry_p.h>
#include <future>

#if RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif // RAPP_WITH_BGFX

#include "Windows.Gaming.Input.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Gaming::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Platform;

static char* g_emptyArgs[] = { "" };
static rapp::WindowHandle g_defaultWindow = { 0 };
static rapp::EventQueue g_eventQueue;

int32_t MainThreadFunc(void*)
{
	return rapp::main(0, g_emptyArgs);
}

struct GamepadRemap
{
	uint16_t					m_bit;
	rapp::GamepadState::Buttons	m_button;
};

static GamepadRemap s_gamepadRemap[] =
{
	{ (uint16_t)GamepadButtons::DPadUp,			rapp::GamepadState::Up        },
	{ (uint16_t)GamepadButtons::DPadDown,		rapp::GamepadState::Down      },
	{ (uint16_t)GamepadButtons::DPadLeft,		rapp::GamepadState::Left      },
	{ (uint16_t)GamepadButtons::DPadRight,		rapp::GamepadState::Right     },
	{ (uint16_t)GamepadButtons::Menu,			rapp::GamepadState::Start     },
	{ (uint16_t)GamepadButtons::View,			rapp::GamepadState::Back      },
	{ (uint16_t)GamepadButtons::LeftThumbstick,	rapp::GamepadState::LThumb    },
	{ (uint16_t)GamepadButtons::RightThumbstick,rapp::GamepadState::RThumb    },
	{ (uint16_t)GamepadButtons::LeftShoulder,	rapp::GamepadState::LShoulder },
	{ (uint16_t)GamepadButtons::RightShoulder,	rapp::GamepadState::RShoulder },
	{ (uint16_t)GamepadButtons::None,			rapp::GamepadState::Guide     },
	{ (uint16_t)GamepadButtons::A,				rapp::GamepadState::A         },
	{ (uint16_t)GamepadButtons::B,				rapp::GamepadState::B         },
	{ (uint16_t)GamepadButtons::X,				rapp::GamepadState::X         },
	{ (uint16_t)GamepadButtons::Y,				rapp::GamepadState::Y         },
};

static uint8_t translateKeyModifiers(CoreWindow const ^ _sender, Windows::System::VirtualKey _vk, Windows::UI::Core::CorePhysicalKeyStatus & _ks)
{
	RTM_UNUSED(_sender);
	uint8_t modifiers = 0;

	if (_vk == Windows::System::VirtualKey::Control)
	{
		if (_ks.IsExtendedKey)	modifiers |= rapp::KeyboardState::Modifier::RCtrl;
		else					modifiers |= rapp::KeyboardState::Modifier::LCtrl;
	}

	if (_vk == Windows::System::VirtualKey::Shift)
	{
		if (_ks.ScanCode == 0x36)	modifiers |= rapp::KeyboardState::Modifier::RShift;
		else						modifiers |= rapp::KeyboardState::Modifier::LShift;
	}

	if (_vk == Windows::System::VirtualKey::LeftWindows)
		modifiers |= rapp::KeyboardState::Modifier::LMeta;

	if (_vk == Windows::System::VirtualKey::RightWindows)
		modifiers |= rapp::KeyboardState::Modifier::RMeta;

	return modifiers;
}

inline void winrtSetWindow(::IUnknown* _window)
{
	RTM_UNUSED(_window);
#if RAPP_WITH_BGFX
	bgfx::PlatformData pd;
	pd.ndt          = NULL;
	pd.nwh          = _window;
	pd.context      = NULL;
	pd.backBuffer   = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
#endif // RAPP_WITH_BGFX
}

enum Command : uint8_t
{
	None,
	RunFunc,
	Quit
};

static rtm::SpScQueue<>		s_channel(1024);

	#define RAPP_CMD_READ(_type, _name)		\
		_type _name;						\
		while (!s_channel.read(&_name))

	#define RAPP_CMD_WRITE(_val)			\
		while (!s_channel.write(_val));

static rapp::KeyboardState::Key		s_translateKey[512];

ref class ViewProvider sealed : public IFrameworkView
{
public:
    ViewProvider()
		: m_windowVisible(false)
		, m_windowClosed(false)
		, m_modifiers(0)
    {
		memset(m_gamepadIDs, 0, sizeof(m_gamepadIDs));
		memset(m_gamepadState, 0, sizeof(m_gamepadState));

		memset(s_translateKey, 0, sizeof(s_translateKey) );
		s_translateKey[(uint16_t)0x000000c0]				= rapp::KeyboardState::Key::Tilde;
		s_translateKey[(uint16_t)0x000000db]				= rapp::KeyboardState::Key::LeftBracket;
		s_translateKey[(uint16_t)0x000000dd]				= rapp::KeyboardState::Key::RightBracket;
		s_translateKey[(uint16_t)0x000000ba]				= rapp::KeyboardState::Key::Semicolon;
		s_translateKey[(uint16_t)0x000000de]				= rapp::KeyboardState::Key::Quote;
		s_translateKey[(uint16_t)0x000000bc]				= rapp::KeyboardState::Key::Comma;
		s_translateKey[(uint16_t)0x000000be]				= rapp::KeyboardState::Key::Period;
		s_translateKey[(uint16_t)0x000000bf]				= rapp::KeyboardState::Key::Slash;
		s_translateKey[(uint16_t)0x000000dc]				= rapp::KeyboardState::Key::Backslash;
		s_translateKey[(uint16_t)VirtualKey::Back]			= rapp::KeyboardState::Key::Backspace;
		s_translateKey[(uint16_t)VirtualKey::Tab]			= rapp::KeyboardState::Key::Tab;
		s_translateKey[(uint16_t)VirtualKey::Enter]			= rapp::KeyboardState::Key::Return;
		s_translateKey[(uint16_t)VirtualKey::Escape]		= rapp::KeyboardState::Key::Esc;
		s_translateKey[(uint16_t)VirtualKey::Space]			= rapp::KeyboardState::Key::Space;
		s_translateKey[(uint16_t)VirtualKey::PageUp]		= rapp::KeyboardState::Key::PageUp;
		s_translateKey[(uint16_t)VirtualKey::PageDown]		= rapp::KeyboardState::Key::PageDown;
		s_translateKey[(uint16_t)VirtualKey::End]			= rapp::KeyboardState::Key::End;
		s_translateKey[(uint16_t)VirtualKey::Home]			= rapp::KeyboardState::Key::Home;
		s_translateKey[(uint16_t)VirtualKey::Left]			= rapp::KeyboardState::Key::Left;
		s_translateKey[(uint16_t)VirtualKey::Up]			= rapp::KeyboardState::Key::Up;
		s_translateKey[(uint16_t)VirtualKey::Right]			= rapp::KeyboardState::Key::Right;
		s_translateKey[(uint16_t)VirtualKey::Down]			= rapp::KeyboardState::Key::Down;
		s_translateKey[(uint16_t)VirtualKey::Print]			= rapp::KeyboardState::Key::Print;
		s_translateKey[(uint16_t)VirtualKey::Insert]		= rapp::KeyboardState::Key::Insert;
		s_translateKey[(uint16_t)VirtualKey::Delete]		= rapp::KeyboardState::Key::Delete;
		s_translateKey[(uint16_t)VirtualKey::Number0]		= rapp::KeyboardState::Key::Key0;
		s_translateKey[(uint16_t)VirtualKey::Number1]		= rapp::KeyboardState::Key::Key1;
		s_translateKey[(uint16_t)VirtualKey::Number2]		= rapp::KeyboardState::Key::Key2;
		s_translateKey[(uint16_t)VirtualKey::Number3]		= rapp::KeyboardState::Key::Key3;
		s_translateKey[(uint16_t)VirtualKey::Number4]		= rapp::KeyboardState::Key::Key4;
		s_translateKey[(uint16_t)VirtualKey::Number5]		= rapp::KeyboardState::Key::Key5;
		s_translateKey[(uint16_t)VirtualKey::Number6]		= rapp::KeyboardState::Key::Key6;
		s_translateKey[(uint16_t)VirtualKey::Number7]		= rapp::KeyboardState::Key::Key7;
		s_translateKey[(uint16_t)VirtualKey::Number8]		= rapp::KeyboardState::Key::Key8;
		s_translateKey[(uint16_t)VirtualKey::Number9]		= rapp::KeyboardState::Key::Key9;
		s_translateKey[(uint16_t)VirtualKey::A]				= rapp::KeyboardState::Key::KeyA;
		s_translateKey[(uint16_t)VirtualKey::B]				= rapp::KeyboardState::Key::KeyB;
		s_translateKey[(uint16_t)VirtualKey::C]				= rapp::KeyboardState::Key::KeyC;
		s_translateKey[(uint16_t)VirtualKey::D]				= rapp::KeyboardState::Key::KeyD;
		s_translateKey[(uint16_t)VirtualKey::E]				= rapp::KeyboardState::Key::KeyE;
		s_translateKey[(uint16_t)VirtualKey::F]				= rapp::KeyboardState::Key::KeyF;
		s_translateKey[(uint16_t)VirtualKey::G]				= rapp::KeyboardState::Key::KeyG;
		s_translateKey[(uint16_t)VirtualKey::H]				= rapp::KeyboardState::Key::KeyH;
		s_translateKey[(uint16_t)VirtualKey::I]				= rapp::KeyboardState::Key::KeyI;
		s_translateKey[(uint16_t)VirtualKey::J]				= rapp::KeyboardState::Key::KeyJ;
		s_translateKey[(uint16_t)VirtualKey::K]				= rapp::KeyboardState::Key::KeyK;
		s_translateKey[(uint16_t)VirtualKey::L]				= rapp::KeyboardState::Key::KeyL;
		s_translateKey[(uint16_t)VirtualKey::M]				= rapp::KeyboardState::Key::KeyM;
		s_translateKey[(uint16_t)VirtualKey::N]				= rapp::KeyboardState::Key::KeyN;
		s_translateKey[(uint16_t)VirtualKey::O]				= rapp::KeyboardState::Key::KeyO;
		s_translateKey[(uint16_t)VirtualKey::P]				= rapp::KeyboardState::Key::KeyP;
		s_translateKey[(uint16_t)VirtualKey::Q]				= rapp::KeyboardState::Key::KeyQ;
		s_translateKey[(uint16_t)VirtualKey::R]				= rapp::KeyboardState::Key::KeyR;
		s_translateKey[(uint16_t)VirtualKey::S]				= rapp::KeyboardState::Key::KeyS;
		s_translateKey[(uint16_t)VirtualKey::T]				= rapp::KeyboardState::Key::KeyT;
		s_translateKey[(uint16_t)VirtualKey::U]				= rapp::KeyboardState::Key::KeyU;
		s_translateKey[(uint16_t)VirtualKey::V]				= rapp::KeyboardState::Key::KeyV;
		s_translateKey[(uint16_t)VirtualKey::W]				= rapp::KeyboardState::Key::KeyW;
		s_translateKey[(uint16_t)VirtualKey::X]				= rapp::KeyboardState::Key::KeyX;
		s_translateKey[(uint16_t)VirtualKey::Y]				= rapp::KeyboardState::Key::KeyY;
		s_translateKey[(uint16_t)VirtualKey::Z]				= rapp::KeyboardState::Key::KeyZ;
		s_translateKey[(uint16_t)VirtualKey::NumberPad0]	= rapp::KeyboardState::Key::NumPad0;
		s_translateKey[(uint16_t)VirtualKey::NumberPad1]	= rapp::KeyboardState::Key::NumPad1;
		s_translateKey[(uint16_t)VirtualKey::NumberPad2]	= rapp::KeyboardState::Key::NumPad2;
		s_translateKey[(uint16_t)VirtualKey::NumberPad3]	= rapp::KeyboardState::Key::NumPad3;
		s_translateKey[(uint16_t)VirtualKey::NumberPad4]	= rapp::KeyboardState::Key::NumPad4;
		s_translateKey[(uint16_t)VirtualKey::NumberPad5]	= rapp::KeyboardState::Key::NumPad5;
		s_translateKey[(uint16_t)VirtualKey::NumberPad6]	= rapp::KeyboardState::Key::NumPad6;
		s_translateKey[(uint16_t)VirtualKey::NumberPad7]	= rapp::KeyboardState::Key::NumPad7;
		s_translateKey[(uint16_t)VirtualKey::NumberPad8]	= rapp::KeyboardState::Key::NumPad8;
		s_translateKey[(uint16_t)VirtualKey::NumberPad9]	= rapp::KeyboardState::Key::NumPad9;
		s_translateKey[(uint16_t)VirtualKey::Add]			= rapp::KeyboardState::Key::Plus;
		s_translateKey[(uint16_t)VirtualKey::Subtract]		= rapp::KeyboardState::Key::Minus;
		s_translateKey[(uint16_t)VirtualKey::Divide]		= rapp::KeyboardState::Key::Slash;
		s_translateKey[(uint16_t)VirtualKey::F1 ]			= rapp::KeyboardState::Key::F1;
		s_translateKey[(uint16_t)VirtualKey::F2 ]			= rapp::KeyboardState::Key::F2;
		s_translateKey[(uint16_t)VirtualKey::F3 ]			= rapp::KeyboardState::Key::F3;
		s_translateKey[(uint16_t)VirtualKey::F4 ]			= rapp::KeyboardState::Key::F4;
		s_translateKey[(uint16_t)VirtualKey::F5 ]			= rapp::KeyboardState::Key::F5;
		s_translateKey[(uint16_t)VirtualKey::F6 ]			= rapp::KeyboardState::Key::F6;
		s_translateKey[(uint16_t)VirtualKey::F7 ]			= rapp::KeyboardState::Key::F7;
		s_translateKey[(uint16_t)VirtualKey::F8 ]			= rapp::KeyboardState::Key::F8;
		s_translateKey[(uint16_t)VirtualKey::F9 ]			= rapp::KeyboardState::Key::F9;
		s_translateKey[(uint16_t)VirtualKey::F10]			= rapp::KeyboardState::Key::F10;
		s_translateKey[(uint16_t)VirtualKey::F11]			= rapp::KeyboardState::Key::F11;
		s_translateKey[(uint16_t)VirtualKey::F12]			= rapp::KeyboardState::Key::F12;
    }

    virtual void Initialize(CoreApplicationView ^ _applicationView)
    {
        _applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ViewProvider::OnActivated );
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &ViewProvider::OnSuspending);
		CoreApplication::Resuming   += ref new EventHandler<Platform::Object^>(this, &ViewProvider::OnResuming);
    }

    virtual void Uninitialize()
    {
    }

	virtual void SetWindow(CoreWindow ^ _window)
	{
		_window->VisibilityChanged	+= ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &ViewProvider::OnVisibilityChanged);
		_window->Closed				+= ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ViewProvider::OnWindowClosed);
		winrtSetWindow(reinterpret_cast<IUnknown*>(_window));

		//_window->IsInputEnabled(true);
		_window->KeyDown			+= ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &ViewProvider::KeyDown);
		_window->KeyUp				+= ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &ViewProvider::KeyUp);

		// can't get mouse to work ?!
		_window->PointerEntered		+= ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &ViewProvider::PointerEntered);
		_window->PointerExited		+= ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &ViewProvider::PointerExited);
		_window->PointerMoved		+= ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &ViewProvider::PointerMoved);

		Gamepad::GamepadAdded		+= ref new EventHandler<Gamepad^ >(this, &ViewProvider::OnGamePadAdded);
		Gamepad::GamepadRemoved		+= ref new EventHandler<Gamepad^ >(this, &ViewProvider::OnGamePadRemoved);
	}

    virtual void Load(String ^ _entryPoint)
    {
		RTM_UNUSED(_entryPoint);
    }

    virtual void Run()
    {
		rtm::Thread thread;
		thread.start(MainThreadFunc, nullptr);

		CoreWindow^ window = CoreWindow::GetForCurrentThread();

#if RTM_PLATFORM_WINRT
		auto bounds = window->Bounds;
		auto dpi = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi;
		static const float dipsPerInch = 96.0f;
		g_eventQueue.postSizeEvent(g_defaultWindow
			, lround(floorf(bounds.Width  * dpi / dipsPerInch + 0.5f) )
			, lround(floorf(bounds.Height * dpi / dipsPerInch + 0.5f) )
			);
#endif // RTM_PLATFORM_WINRT

		while (!m_windowClosed)
		{
			if (m_windowVisible)
			{
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			}
			else
			{
				window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
			
			uintptr_t cmd = 0;
			if (s_channel.read(&cmd))
			{
				switch (cmd)
				{
					case Command::RunFunc:
						{
							RAPP_CMD_READ(rapp::ThreadFn,	fn);
							RAPP_CMD_READ(void*,				userData);

							fn(userData);
						}
						break;

					case Command::Quit:
						{
							RAPP_CMD_READ(rapp::App*, app);
							app->quit();
						}
						break;

				default:
					RTM_ASSERT(false, "Invalid command!");
				};
			}

			IVectorView<Gamepad^> ^ gamepads = Gamepad::Gamepads;

			for (uint32_t i=0; i<gamepads->Size; ++i )
			{
				if (i > ENTRY_CONFIG_MAX_GAMEPADS) break;

				GamepadReading reading = gamepads->GetAt(i)->GetCurrentReading();
				Gamepad^ id = gamepads->GetAt(i);

				if (id != m_gamepadIDs[i])
				{
					m_gamepadIDs[i]		= id;
					m_gamepadState[i]	= reading;
					g_eventQueue.postGamepadEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, true);
				}

				if (m_gamepadIDs[i] == nullptr)
					continue;

				GamepadButtons buttons = reading.Buttons;

				const GamepadButtons changed = m_gamepadState[i].Buttons ^ buttons;
				const GamepadButtons current = m_gamepadState[i].Buttons;

				if (GamepadButtons::None != changed)
				{
					for (uint32_t jj=0; jj<RTM_NUM_ELEMENTS(s_gamepadRemap); ++jj)
					{
						uint16_t bit = s_gamepadRemap[jj].m_bit;
						if (bit & (uint16_t)changed)
						{
							g_eventQueue.postGamepadButtonEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, s_gamepadRemap[jj].m_button, !!(0 == (bit & (uint16_t)current)));
						}
					}
				}

				double lt = reading.LeftTrigger;
				if (lt != m_gamepadState[i].LeftTrigger)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::LeftZ, (int32_t)(lt*255));

				double rt = reading.RightTrigger;
				if (rt != m_gamepadState[i].RightTrigger)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::RightZ, (int32_t)(rt*255));

				double ltsx = reading.LeftThumbstickX;
				double ltsy = reading.LeftThumbstickY;
				double rtsx = reading.RightThumbstickX;
				double rtsy = reading.RightThumbstickY;

				if (ltsx != m_gamepadState[i].LeftThumbstickX)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::LeftX, (int32_t)(ltsx*32767.0f));
				
				if (ltsy != m_gamepadState[i].LeftThumbstickY)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::LeftY, (int32_t)(ltsy*32767.0f));

				if (rtsx != m_gamepadState[i].RightThumbstickX)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::RightX, (int32_t)(rtsx*32767.0f));

				if (rtsy != m_gamepadState[i].RightThumbstickY)
					g_eventQueue.postAxisEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, rapp::GamepadAxis::RightY, (int32_t)(rtsy*32767.0f));

				m_gamepadState[i] = reading;
			}
		}

		g_eventQueue.postExitEvent();

		thread.stop();
    }

protected:
    // Event handlers
    void OnActivated(CoreApplicationView ^ _applicationView, IActivatedEventArgs ^ _args)
    {
		RTM_UNUSED_2(_applicationView, _args);
        CoreWindow::GetForCurrentThread()->Activate();
    }

	void OnVisibilityChanged(CoreWindow ^ _sender, VisibilityChangedEventArgs ^ _args)
	{
		RTM_UNUSED(_sender);
		m_windowVisible = _args->Visible;
	}

    void OnSuspending(Platform::Object ^ _sender, SuspendingEventArgs ^ _args)
    {
		SuspendingDeferral^ deferral = _args->SuspendingOperation->GetDeferral();
		RTM_UNUSED(deferral);
    }

    void OnResuming(Platform::Object ^ _sender, Platform::Object ^ _args)
    {
		RTM_UNUSED_2(_sender, _args);
    }

    void OnWindowClosed(CoreWindow ^ _sender, CoreWindowEventArgs ^ _args)
    {
		RTM_UNUSED_2(_sender, _args);
        m_windowClosed = true;
    }

	void OnGamePadAdded(Platform::Object ^ _sender, Gamepad^ _args)
	{
		RTM_UNUSED(_sender);
		uint32_t index;
		for (index=0; index<ENTRY_CONFIG_MAX_GAMEPADS; ++index)
			if (m_gamepadIDs[index] == _args)
				break;
		RTM_ASSERT(index<ENTRY_CONFIG_MAX_GAMEPADS, "");
		
		rapp::WindowHandle rapp::kDefaultWindowHandle = { 0 };
		g_eventQueue.postGamepadEvent(rapp::kDefaultWindowHandle, {(uint16_t)index}, true);
	}
	
	void OnGamePadRemoved(Platform::Object ^ _sender, Gamepad^ _args)
	{
		RTM_UNUSED(_sender);
		uint32_t index;
		for (index=0; index<ENTRY_CONFIG_MAX_GAMEPADS; ++index)
			if (m_gamepadIDs[index] == _args)
				break;
		RTM_ASSERT(index<ENTRY_CONFIG_MAX_GAMEPADS, "");
		m_gamepadIDs[index]		= nullptr;
		m_gamepadState[index]	= GamepadReading();

		rapp::WindowHandle rapp::kDefaultWindowHandle = { 0 };
		g_eventQueue.postGamepadEvent(rapp::kDefaultWindowHandle, {(uint16_t)index}, false);
	}

	void KeyDown(CoreWindow ^ _sender, KeyEventArgs ^ _args)
	{
		Windows::System::VirtualKey _vk = _args->VirtualKey;
		Windows::UI::Core::CorePhysicalKeyStatus _ks = _args->KeyStatus;

		uint8_t modifiers = translateKeyModifiers(_sender, _vk, _ks);
		m_modifiers |= modifiers;

		rapp::KeyboardState::Key key = s_translateKey[(uint16_t)_vk];
		if (key == rapp::KeyboardState::Key::None)
			return;

		rapp::WindowHandle rapp::kDefaultWindowHandle = { 0 };
		g_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, key, m_modifiers, true);
	}

	void KeyUp(CoreWindow ^ _sender, KeyEventArgs ^ _args)
	{
		Windows::System::VirtualKey _vk = _args->VirtualKey;
		Windows::UI::Core::CorePhysicalKeyStatus _ks = _args->KeyStatus;

		uint8_t modifiers = translateKeyModifiers(_sender, _vk, _ks);
		m_modifiers &= ~modifiers;

		rapp::KeyboardState::Key key = s_translateKey[(uint16_t)_vk];
		if (key == rapp::KeyboardState::Key::None)
			return;

		rapp::WindowHandle rapp::kDefaultWindowHandle = { 0 };
		g_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, key, m_modifiers, false);
	}

	void PointerEntered(CoreWindow ^ _sender, PointerEventArgs ^ _args)
	{
		RTM_UNUSED_2(_sender, _args);
		__debugbreak();
	}

	void PointerExited(CoreWindow ^ _sender, PointerEventArgs ^ _args)
	{
		RTM_UNUSED_2(_sender, _args);
		__debugbreak();
	}

	void PointerMoved(CoreWindow ^ _sender, PointerEventArgs ^ _args)
	{
		RTM_UNUSED_2(_sender, _args);
		__debugbreak();
	}

private:
	Gamepad^				m_gamepadIDs[ENTRY_CONFIG_MAX_GAMEPADS];
	GamepadReading			m_gamepadState[ENTRY_CONFIG_MAX_GAMEPADS];
	bool					m_windowVisible;
	bool					m_windowClosed;
	uint8_t					m_modifiers;
};

ref class ViewProviderFactory sealed : public IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new ViewProvider();
    }
};

namespace rapp
{
	const Event* poll()
	{
		return g_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		return g_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		g_eventQueue.release(_event);
	}

	void appRunOnMainThread(ThreadFn _fn, void* _userData)
	{
		RAPP_CMD_WRITE(Command::RunFunc);
		RAPP_CMD_WRITE(_fn);
		RAPP_CMD_WRITE(_userData);
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
		RTM_UNUSED_6(_x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
	}

	void* windowGetNativeHandle(WindowHandle _handle)
	{
		RTM_UNUSED_1(_handle);
	}

	void* windowGetNativeDisplayHandle()
	{
		return NULL;
	}

	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		RTM_UNUSED_3(_handle, _x, _y);
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		RTM_UNUSED_3(_handle, _width, _height);
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		RTM_UNUSED_2(_handle, _title);
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
}

[MTAThread]
int main(Array<String^>^)
{
	auto viewProvider = ref new ViewProviderFactory();
	CoreApplication::Run(viewProvider);
	return 0;
}

#endif // RTM_PLATFORM_WINRT
