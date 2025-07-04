//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#if RTM_PLATFORM_WINRT

#include <rapp/src/entry_p.h>
#include <future>

#ifdef RAPP_WITH_BGFX
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
	rapp::GamepadButtons	m_button;
};

static GamepadRemap s_gamepadRemap[] =
{
	{ (uint16_t)GamepadButtonss::DPadUp,			rapp::GamepadButtons::Up        },
	{ (uint16_t)GamepadButtonss::DPadDown,		rapp::GamepadButtons::Down      },
	{ (uint16_t)GamepadButtonss::DPadLeft,		rapp::GamepadButtons::Left      },
	{ (uint16_t)GamepadButtonss::DPadRight,		rapp::GamepadButtons::Right     },
	{ (uint16_t)GamepadButtonss::Menu,			rapp::GamepadButtons::Start     },
	{ (uint16_t)GamepadButtonss::View,			rapp::GamepadButtons::Back      },
	{ (uint16_t)GamepadButtonss::LeftThumbstick,	rapp::GamepadButtons::LThumb    },
	{ (uint16_t)GamepadButtonss::RightThumbstick,rapp::GamepadButtons::RThumb    },
	{ (uint16_t)GamepadButtonss::LeftShoulder,	rapp::GamepadButtons::LShoulder },
	{ (uint16_t)GamepadButtonss::RightShoulder,	rapp::GamepadButtons::RShoulder },
	{ (uint16_t)GamepadButtonss::None,			rapp::GamepadButtons::Guide     },
	{ (uint16_t)GamepadButtonss::A,				rapp::GamepadButtons::A         },
	{ (uint16_t)GamepadButtonss::B,				rapp::GamepadButtons::B         },
	{ (uint16_t)GamepadButtonss::X,				rapp::GamepadButtons::X         },
	{ (uint16_t)GamepadButtonss::Y,				rapp::GamepadButtons::Y         },
};

static uint8_t translateKeyModifiers(CoreWindow const ^ _sender, Windows::System::VirtualKey _vk, Windows::UI::Core::CorePhysicalKeyStatus & _ks)
{
	RTM_UNUSED(_sender);
	uint8_t modifiers = 0;

	if (_vk == Windows::System::VirtualKey::Control)
	{
		if (_ks.IsExtendedKey)	modifiers |= rapp::KeyboardModifier::RCtrl;
		else					modifiers |= rapp::KeyboardModifier::LCtrl;
	}

	if (_vk == Windows::System::VirtualKey::Shift)
	{
		if (_ks.ScanCode == 0x36)	modifiers |= rapp::KeyboardModifier::RShift;
		else						modifiers |= rapp::KeyboardModifier::LShift;
	}

	if (_vk == Windows::System::VirtualKey::LeftWindows)
		modifiers |= rapp::KeyboardModifier::LMeta;

	if (_vk == Windows::System::VirtualKey::RightWindows)
		modifiers |= rapp::KeyboardModifier::RMeta;

	return modifiers;
}

inline void winrtSetWindow(::IUnknown* _window)
{
	RTM_UNUSED(_window);
#ifdef RAPP_WITH_BGFX
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

static rapp::KeyboardKey		s_translateKey[512];

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
		s_translateKey[(uint16_t)0x000000c0]				= rapp::KeyboardKey::Tilde;
		s_translateKey[(uint16_t)0x000000db]				= rapp::KeyboardKey::LeftBracket;
		s_translateKey[(uint16_t)0x000000dd]				= rapp::KeyboardKey::RightBracket;
		s_translateKey[(uint16_t)0x000000ba]				= rapp::KeyboardKey::Semicolon;
		s_translateKey[(uint16_t)0x000000de]				= rapp::KeyboardKey::Quote;
		s_translateKey[(uint16_t)0x000000bc]				= rapp::KeyboardKey::Comma;
		s_translateKey[(uint16_t)0x000000be]				= rapp::KeyboardKey::Period;
		s_translateKey[(uint16_t)0x000000bf]				= rapp::KeyboardKey::Slash;
		s_translateKey[(uint16_t)0x000000dc]				= rapp::KeyboardKey::Backslash;
		s_translateKey[(uint16_t)VirtualKey::Back]			= rapp::KeyboardKey::Backspace;
		s_translateKey[(uint16_t)VirtualKey::Tab]			= rapp::KeyboardKey::Tab;
		s_translateKey[(uint16_t)VirtualKey::Enter]			= rapp::KeyboardKey::Return;
		s_translateKey[(uint16_t)VirtualKey::Escape]		= rapp::KeyboardKey::Esc;
		s_translateKey[(uint16_t)VirtualKey::Space]			= rapp::KeyboardKey::Space;
		s_translateKey[(uint16_t)VirtualKey::PageUp]		= rapp::KeyboardKey::PageUp;
		s_translateKey[(uint16_t)VirtualKey::PageDown]		= rapp::KeyboardKey::PageDown;
		s_translateKey[(uint16_t)VirtualKey::End]			= rapp::KeyboardKey::End;
		s_translateKey[(uint16_t)VirtualKey::Home]			= rapp::KeyboardKey::Home;
		s_translateKey[(uint16_t)VirtualKey::Left]			= rapp::KeyboardKey::Left;
		s_translateKey[(uint16_t)VirtualKey::Up]			= rapp::KeyboardKey::Up;
		s_translateKey[(uint16_t)VirtualKey::Right]			= rapp::KeyboardKey::Right;
		s_translateKey[(uint16_t)VirtualKey::Down]			= rapp::KeyboardKey::Down;
		s_translateKey[(uint16_t)VirtualKey::Print]			= rapp::KeyboardKey::Print;
		s_translateKey[(uint16_t)VirtualKey::Insert]		= rapp::KeyboardKey::Insert;
		s_translateKey[(uint16_t)VirtualKey::Delete]		= rapp::KeyboardKey::Delete;
		s_translateKey[(uint16_t)VirtualKey::Number0]		= rapp::KeyboardKey::Key0;
		s_translateKey[(uint16_t)VirtualKey::Number1]		= rapp::KeyboardKey::Key1;
		s_translateKey[(uint16_t)VirtualKey::Number2]		= rapp::KeyboardKey::Key2;
		s_translateKey[(uint16_t)VirtualKey::Number3]		= rapp::KeyboardKey::Key3;
		s_translateKey[(uint16_t)VirtualKey::Number4]		= rapp::KeyboardKey::Key4;
		s_translateKey[(uint16_t)VirtualKey::Number5]		= rapp::KeyboardKey::Key5;
		s_translateKey[(uint16_t)VirtualKey::Number6]		= rapp::KeyboardKey::Key6;
		s_translateKey[(uint16_t)VirtualKey::Number7]		= rapp::KeyboardKey::Key7;
		s_translateKey[(uint16_t)VirtualKey::Number8]		= rapp::KeyboardKey::Key8;
		s_translateKey[(uint16_t)VirtualKey::Number9]		= rapp::KeyboardKey::Key9;
		s_translateKey[(uint16_t)VirtualKey::A]				= rapp::KeyboardKey::KeyA;
		s_translateKey[(uint16_t)VirtualKey::B]				= rapp::KeyboardKey::KeyB;
		s_translateKey[(uint16_t)VirtualKey::C]				= rapp::KeyboardKey::KeyC;
		s_translateKey[(uint16_t)VirtualKey::D]				= rapp::KeyboardKey::KeyD;
		s_translateKey[(uint16_t)VirtualKey::E]				= rapp::KeyboardKey::KeyE;
		s_translateKey[(uint16_t)VirtualKey::F]				= rapp::KeyboardKey::KeyF;
		s_translateKey[(uint16_t)VirtualKey::G]				= rapp::KeyboardKey::KeyG;
		s_translateKey[(uint16_t)VirtualKey::H]				= rapp::KeyboardKey::KeyH;
		s_translateKey[(uint16_t)VirtualKey::I]				= rapp::KeyboardKey::KeyI;
		s_translateKey[(uint16_t)VirtualKey::J]				= rapp::KeyboardKey::KeyJ;
		s_translateKey[(uint16_t)VirtualKey::K]				= rapp::KeyboardKey::KeyK;
		s_translateKey[(uint16_t)VirtualKey::L]				= rapp::KeyboardKey::KeyL;
		s_translateKey[(uint16_t)VirtualKey::M]				= rapp::KeyboardKey::KeyM;
		s_translateKey[(uint16_t)VirtualKey::N]				= rapp::KeyboardKey::KeyN;
		s_translateKey[(uint16_t)VirtualKey::O]				= rapp::KeyboardKey::KeyO;
		s_translateKey[(uint16_t)VirtualKey::P]				= rapp::KeyboardKey::KeyP;
		s_translateKey[(uint16_t)VirtualKey::Q]				= rapp::KeyboardKey::KeyQ;
		s_translateKey[(uint16_t)VirtualKey::R]				= rapp::KeyboardKey::KeyR;
		s_translateKey[(uint16_t)VirtualKey::S]				= rapp::KeyboardKey::KeyS;
		s_translateKey[(uint16_t)VirtualKey::T]				= rapp::KeyboardKey::KeyT;
		s_translateKey[(uint16_t)VirtualKey::U]				= rapp::KeyboardKey::KeyU;
		s_translateKey[(uint16_t)VirtualKey::V]				= rapp::KeyboardKey::KeyV;
		s_translateKey[(uint16_t)VirtualKey::W]				= rapp::KeyboardKey::KeyW;
		s_translateKey[(uint16_t)VirtualKey::X]				= rapp::KeyboardKey::KeyX;
		s_translateKey[(uint16_t)VirtualKey::Y]				= rapp::KeyboardKey::KeyY;
		s_translateKey[(uint16_t)VirtualKey::Z]				= rapp::KeyboardKey::KeyZ;
		s_translateKey[(uint16_t)VirtualKey::NumberPad0]	= rapp::KeyboardKey::NumPad0;
		s_translateKey[(uint16_t)VirtualKey::NumberPad1]	= rapp::KeyboardKey::NumPad1;
		s_translateKey[(uint16_t)VirtualKey::NumberPad2]	= rapp::KeyboardKey::NumPad2;
		s_translateKey[(uint16_t)VirtualKey::NumberPad3]	= rapp::KeyboardKey::NumPad3;
		s_translateKey[(uint16_t)VirtualKey::NumberPad4]	= rapp::KeyboardKey::NumPad4;
		s_translateKey[(uint16_t)VirtualKey::NumberPad5]	= rapp::KeyboardKey::NumPad5;
		s_translateKey[(uint16_t)VirtualKey::NumberPad6]	= rapp::KeyboardKey::NumPad6;
		s_translateKey[(uint16_t)VirtualKey::NumberPad7]	= rapp::KeyboardKey::NumPad7;
		s_translateKey[(uint16_t)VirtualKey::NumberPad8]	= rapp::KeyboardKey::NumPad8;
		s_translateKey[(uint16_t)VirtualKey::NumberPad9]	= rapp::KeyboardKey::NumPad9;
		s_translateKey[(uint16_t)VirtualKey::Add]			= rapp::KeyboardKey::Plus;
		s_translateKey[(uint16_t)VirtualKey::Subtract]		= rapp::KeyboardKey::Minus;
		s_translateKey[(uint16_t)VirtualKey::Divide]		= rapp::KeyboardKey::Slash;
		s_translateKey[(uint16_t)VirtualKey::F1 ]			= rapp::KeyboardKey::F1;
		s_translateKey[(uint16_t)VirtualKey::F2 ]			= rapp::KeyboardKey::F2;
		s_translateKey[(uint16_t)VirtualKey::F3 ]			= rapp::KeyboardKey::F3;
		s_translateKey[(uint16_t)VirtualKey::F4 ]			= rapp::KeyboardKey::F4;
		s_translateKey[(uint16_t)VirtualKey::F5 ]			= rapp::KeyboardKey::F5;
		s_translateKey[(uint16_t)VirtualKey::F6 ]			= rapp::KeyboardKey::F6;
		s_translateKey[(uint16_t)VirtualKey::F7 ]			= rapp::KeyboardKey::F7;
		s_translateKey[(uint16_t)VirtualKey::F8 ]			= rapp::KeyboardKey::F8;
		s_translateKey[(uint16_t)VirtualKey::F9 ]			= rapp::KeyboardKey::F9;
		s_translateKey[(uint16_t)VirtualKey::F10]			= rapp::KeyboardKey::F10;
		s_translateKey[(uint16_t)VirtualKey::F11]			= rapp::KeyboardKey::F11;
		s_translateKey[(uint16_t)VirtualKey::F12]			= rapp::KeyboardKey::F12;
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

				GamepadButtonss buttons = reading.Buttons;

				const GamepadButtonss changed = m_gamepadState[i].Buttons ^ buttons;
				const GamepadButtonss current = m_gamepadState[i].Buttons;

				if (GamepadButtonss::None != changed)
				{
					for (uint32_t jj=0; jj<RTM_NUM_ELEMENTS(s_gamepadRemap); ++jj)
					{
						uint16_t bit = s_gamepadRemap[jj].m_bit;
						if (bit & (uint16_t)changed)
						{
							g_eventQueue.postGamepadButtonsEvent(rapp::kDefaultWindowHandle, {(uint16_t)i}, s_gamepadRemap[jj].m_button, !!(0 == (bit & (uint16_t)current)));
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

		rapp::KeyboardKey key = s_translateKey[(uint16_t)_vk];
		if (key == rapp::KeyboardKey::None)
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

		rapp::KeyboardKey key = s_translateKey[(uint16_t)_vk];
		if (key == rapp::KeyboardKey::None)
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
