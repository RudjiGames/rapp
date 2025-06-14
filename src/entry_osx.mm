//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/input.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_OSX

#import <Cocoa/Cocoa.h>

#ifdef RAPP_WITH_BGFX
#include <bgfx/platform.h>
#endif

#include <rbase/inc/uint32_t.h>

@interface AppDelegate : NSObject<NSApplicationDelegate>
{
	bool terminated;
}

+ (AppDelegate *)sharedDelegate;
- (id)init;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (bool)applicationHasTerminated;

@end

@interface Window : NSObject<NSWindowDelegate>
{
	uint32_t windowCount;
}

+ (Window*)sharedDelegate;
- (id)init;
- (void)windowCreated:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowDidResize:(NSNotification*)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;

@end

namespace rapp
{
#ifdef RAPP_WITH_BGFX
	inline void osxSetNSWindow(void* _window, void* _nsgl = 0)
	{
		bgfx::PlatformData pd;
		pd.ndt		= 0;
		pd.nwh		= _window;
		pd.context	= _nsgl;
		pd.backBuffer	= 0;
		pd.backBufferDS	= 0;
		bgfx::setPlatformData(pd);
	}
#endif
	static WindowHandle s_defaultWindow = { 0 };	// TODO: Add support for more windows
	static uint8_t s_translateKey[256];

	struct MainThreadEntry
	{
		int m_argc;
		char** m_argv;

		static int32_t threadFunc(void* _userData)
		{
			CFBundleRef mainBundle = CFBundleGetMainBundle();
			if ( mainBundle != nil )
			{
				CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
				if ( resourcesURL != nil )
				{
					char path[PATH_MAX];
					if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX) )
					{
						chdir(path);
					}
					CFRelease(resourcesURL);
				}
			}

			MainThreadEntry* self = (MainThreadEntry*)_userData;
			uint32_t result = main(self->m_argc, self->m_argv);
			[NSApp terminate:nil];
			return result;

		}
	};

	struct Context
	{
		Context()
			: m_scrollf(0.0f)
			, m_mx(0)
			, m_my(0)
			, m_scroll(0)
			, m_style(0)
			, m_exit(false)
			, m_fullscreen(false)
		{
			s_translateKey[27]             = KeyboardKey::Esc;
			s_translateKey[13]             = KeyboardKey::Return;
			s_translateKey[uint8_t('\t')]  = KeyboardKey::Tab;
			s_translateKey[127]            = KeyboardKey::Backspace;
			s_translateKey[uint8_t(' ')]   = KeyboardKey::Space;

			s_translateKey[uint8_t('+')]   =
			s_translateKey[uint8_t('=')]   = KeyboardKey::Plus;
			s_translateKey[uint8_t('_')]   =
			s_translateKey[uint8_t('-')]   = KeyboardKey::Minus;

			s_translateKey[96]             =
			s_translateKey[uint8_t('~')]   =
			s_translateKey[uint8_t('`')]   = KeyboardKey::Tilde;

			s_translateKey[uint8_t(':')]   =
			s_translateKey[uint8_t(';')]   = KeyboardKey::Semicolon;
			s_translateKey[uint8_t('"')]   =
			s_translateKey[uint8_t('\'')]  = KeyboardKey::Quote;

			s_translateKey[uint8_t('{')]   =
			s_translateKey[uint8_t('[')]   = KeyboardKey::LeftBracket;
			s_translateKey[uint8_t('}')]   =
			s_translateKey[uint8_t(']')]   = KeyboardKey::RightBracket;

			s_translateKey[uint8_t('<')]   =
			s_translateKey[uint8_t(',')]   = KeyboardKey::Comma;
			s_translateKey[uint8_t('>')]   =
			s_translateKey[uint8_t('.')]   = KeyboardKey::Period;
			s_translateKey[uint8_t('?')]   =
			s_translateKey[uint8_t('/')]   = KeyboardKey::Slash;
			s_translateKey[uint8_t('|')]   =
			s_translateKey[uint8_t('\\')]  = KeyboardKey::Backslash;

			s_translateKey[uint8_t('0')]   = KeyboardKey::Key0;
			s_translateKey[uint8_t('1')]   = KeyboardKey::Key1;
			s_translateKey[uint8_t('2')]   = KeyboardKey::Key2;
			s_translateKey[uint8_t('3')]   = KeyboardKey::Key3;
			s_translateKey[uint8_t('4')]   = KeyboardKey::Key4;
			s_translateKey[uint8_t('5')]   = KeyboardKey::Key5;
			s_translateKey[uint8_t('6')]   = KeyboardKey::Key6;
			s_translateKey[uint8_t('7')]   = KeyboardKey::Key7;
			s_translateKey[uint8_t('8')]   = KeyboardKey::Key8;
			s_translateKey[uint8_t('9')]   = KeyboardKey::Key9;

			for (char ch = 'a'; ch <= 'z'; ++ch)
			{
				s_translateKey[uint8_t(ch)]       =
				s_translateKey[uint8_t(ch - ' ')] = KeyboardKey::KeyA + (ch - 'a');
			}
		}

		NSEvent* waitEvent()
		{
			return [NSApp
				nextEventMatchingMask:NSEventMaskAny
				untilDate:[NSDate distantFuture] // wait for event
				inMode:NSDefaultRunLoopMode
				dequeue:YES
				];
		}

		NSEvent* peekEvent()
		{
			return [NSApp
				nextEventMatchingMask:NSEventMaskAny
				untilDate:[NSDate distantPast] // do not wait for event
				inMode:NSDefaultRunLoopMode
				dequeue:YES
				];
		}

		void getMousePos(int* outX, int* outY)
		{
			NSWindow* window = m_windows.getDataIndexed(0);
			NSRect originalFrame	= [window frame];
			NSPoint location		= [window mouseLocationOutsideOfEventStream];
			NSRect adjustFrame		= [window contentRectForFrameRect: originalFrame];

			int x = location.x;
			int y = (int)adjustFrame.size.height - (int)location.y;

			// clamp within the range of the window

			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x > (int)adjustFrame.size.width) x = (int)adjustFrame.size.width;
			if (y > (int)adjustFrame.size.height) y = (int)adjustFrame.size.height;

			*outX = x;
			*outY = y;
		}

		uint8_t translateModifiers(int flags)
		{
			return 0
				| ((0 != (flags & NX_DEVICELSHIFTKEYMASK ) ) ? KeyboardModifier::LShift	: 0)
				| ((0 != (flags & NX_DEVICERSHIFTKEYMASK ) ) ? KeyboardModifier::RShift	: 0)
				| ((0 != (flags & NX_DEVICELALTKEYMASK ) )   ? KeyboardModifier::LAlt	: 0)
				| ((0 != (flags & NX_DEVICERALTKEYMASK ) )   ? KeyboardModifier::RAlt	: 0)
				| ((0 != (flags & NX_DEVICELCTLKEYMASK ) )   ? KeyboardModifier::LCtrl	: 0)
				| ((0 != (flags & NX_DEVICERCTLKEYMASK ) )   ? KeyboardModifier::RCtrl	: 0)
				| ((0 != (flags & NX_DEVICELCMDKEYMASK) )    ? KeyboardModifier::LMeta	: 0)
				| ((0 != (flags & NX_DEVICERCMDKEYMASK) )    ? KeyboardModifier::RMeta	: 0)
				;
		}

		KeyboardKey::Enum handleKeyEvent(NSEvent* event, uint8_t* specialKeys, uint8_t* _pressedChar)
		{
			NSString* key = [event charactersIgnoringModifiers];
			unichar keyChar = 0;
			if ([key length] == 0)
			{
				return KeyboardKey::None;
			}

			keyChar = [key characterAtIndex:0];
			*_pressedChar = (uint8_t)keyChar;

			int keyCode = keyChar;
			*specialKeys = translateModifiers((int)[event modifierFlags]);

			// if this is a unhandled key just return None
			if (keyCode < 256)
			{
				return (KeyboardKey::Enum)s_translateKey[keyCode];
			}

			switch (keyCode)
			{
			case NSF1FunctionKey:  return KeyboardKey::F1;
			case NSF2FunctionKey:  return KeyboardKey::F2;
			case NSF3FunctionKey:  return KeyboardKey::F3;
			case NSF4FunctionKey:  return KeyboardKey::F4;
			case NSF5FunctionKey:  return KeyboardKey::F5;
			case NSF6FunctionKey:  return KeyboardKey::F6;
			case NSF7FunctionKey:  return KeyboardKey::F7;
			case NSF8FunctionKey:  return KeyboardKey::F8;
			case NSF9FunctionKey:  return KeyboardKey::F9;
			case NSF10FunctionKey: return KeyboardKey::F10;
			case NSF11FunctionKey: return KeyboardKey::F11;
			case NSF12FunctionKey: return KeyboardKey::F12;

			case NSLeftArrowFunctionKey:   return KeyboardKey::Left;
			case NSRightArrowFunctionKey:  return KeyboardKey::Right;
			case NSUpArrowFunctionKey:     return KeyboardKey::Up;
			case NSDownArrowFunctionKey:   return KeyboardKey::Down;

			case NSPageUpFunctionKey:      return KeyboardKey::PageUp;
			case NSPageDownFunctionKey:    return KeyboardKey::PageDown;
			case NSHomeFunctionKey:        return KeyboardKey::Home;
			case NSEndFunctionKey:         return KeyboardKey::End;

			case NSPrintScreenFunctionKey: return KeyboardKey::Print;
			}

			return KeyboardKey::None;
		}

		bool dispatchEvent(NSEvent* event)
		{
			if (event)
			{
				NSEventType eventType = [event type];
				uint8_t specialKeys = translateModifiers((int)[event modifierFlags]);

				switch (eventType)
				{
					case NSEventTypeMouseMoved:
					case NSEventTypeLeftMouseDragged:
					case NSEventTypeRightMouseDragged:
					case NSEventTypeOtherMouseDragged:
					{
						getMousePos(&m_mx, &m_my);
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, 0);
						break;
					}

					case NSEventTypeLeftMouseDown:
					{
						// TODO: remove!
						// Command + Left Mouse Button acts as middle! This just a temporary solution!
						// This is becase the average OSX user doesn't have middle mouse click.
						MouseState::Button mb = ([event modifierFlags] & NSEventModifierFlagCommand) ? MouseState::Button::Middle : MouseState::Button::Left;
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, mb, specialKeys, true, false);
						break;
					}

					case NSEventTypeLeftMouseUp:
					{
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Left, specialKeys, false, false);
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Middle, specialKeys, false, false); // TODO: remove!
						break;
					}

					case NSEventTypeRightMouseDown:
					{
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Right, specialKeys, true, false);
						break;
					}

					case NSEventTypeRightMouseUp:
					{
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Right, specialKeys, false, false);
						break;
					}

					case NSEventTypeOtherMouseDown:
					{
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Middle, specialKeys, true, false);
						break;
					}

					case NSEventTypeOtherMouseUp:
					{
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Middle, specialKeys, false, false);
						break;
					}

					case NSEventTypeScrollWheel:
					{
						m_scrollf += [event deltaY];

						m_scroll = (int32_t)m_scrollf;
						m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, 0);
						break;
					}

					case NSEventTypeKeyDown:
					{
						uint8_t modifiers = 0;
						uint8_t pressedChar[4];
						KeyboardKey::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);

						// Returning false means that we take care of the key (instead of the default behavior)
						if (key != KeyboardKey::None)
						{
							if (key == KeyboardKey::KeyQ && (modifiers & KeyboardModifier::RMeta) )
							{
								m_eventQueue.postExitEvent();
							}
							else
							{
								enum { ShiftMask = KeyboardModifier::LShift|KeyboardModifier::RShift };
								m_eventQueue.postCharEvent(s_defaultWindow, 1, pressedChar);
								m_eventQueue.postKeyEvent(s_defaultWindow, key, modifiers, true);
								return false;
							}
						}

						break;
					}

					case NSEventTypeKeyUp:
					{
						uint8_t modifiers  = 0;
						uint8_t pressedChar[4];
						KeyboardKey::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);

						RTM_UNUSED(pressedChar);

						if (key != KeyboardKey::None)
						{
							m_eventQueue.postKeyEvent(s_defaultWindow, key, modifiers, false);
							return false;
						}

						break;
					}
					default:
						break;
				}

				[NSApp sendEvent:event];
				[NSApp updateWindows];

				return true;
			}

			return false;
		}

		void windowDidResize()
		{
			NSWindow* window = m_windows.getDataIndexed(0);
			NSRect originalFrame = [window frame];
			NSRect rect = [window contentRectForFrameRect: originalFrame];
			uint32_t width  = uint32_t(rect.size.width);
			uint32_t height = uint32_t(rect.size.height);
			m_eventQueue.postSizeEvent({0}, width, height);

			// Make sure mouse button state is 'up' after resize.
			m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Left, 0, false, false);
			m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseState::Button::Right,0, false,  false);
		}

		int32_t run(int _argc, char** _argv)
		{
			[NSApplication sharedApplication];

			id dg = [AppDelegate sharedDelegate];
			[NSApp setDelegate:dg];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			[NSApp activateIgnoringOtherApps:YES];
			[NSApp finishLaunching];

			[[NSNotificationCenter defaultCenter]
				postNotificationName:NSApplicationWillFinishLaunchingNotification
				object:NSApp];

			[[NSNotificationCenter defaultCenter]
				postNotificationName:NSApplicationDidFinishLaunchingNotification
				object:NSApp];

			id quitMenuItem = [NSMenuItem new];
			[quitMenuItem
				initWithTitle:@"Quit"
				action:@selector(terminate:)
				keyEquivalent:@"q"];

			id appMenu = [NSMenu new];
			[appMenu addItem:quitMenuItem];

			id appMenuItem = [NSMenuItem new];
			[appMenuItem setSubmenu:appMenu];

			id menubar = [[NSMenu new] autorelease];
			[menubar addItem:appMenuItem];
			[NSApp setMainMenu:menubar];

			m_style = 0
					| NSWindowStyleMaskTitled
					| NSWindowStyleMaskClosable
					| NSWindowStyleMaskMiniaturizable
					| NSWindowStyleMaskResizable
					;

			uint32_t dWidth;
			uint32_t dHeight;
			windowGetDefaultSize(&dWidth, &dHeight);
			NSRect screenRect = [[NSScreen mainScreen] frame];
			const float centerX = (screenRect.size.width  - (float)dWidth )*0.5f;
			const float centerY = (screenRect.size.height - (float)dHeight)*0.5f;

			m_windows.allocate();
			NSRect rect = NSMakeRect(centerX, centerY, (float)dWidth, (float)dHeight);
			NSWindow* window = [[NSWindow alloc]
				initWithContentRect:rect
				styleMask:m_style
				backing:NSBackingStoreBuffered defer:NO
			];
			NSString* appName = [[NSProcessInfo processInfo] processName];
			[window setTitle:appName];
			[window makeKeyAndOrderFront:window];
			[window setAcceptsMouseMovedEvents:YES];
			[window setBackgroundColor:[NSColor blackColor]];
			[[Window sharedDelegate] windowCreated:window];

			*m_windows.getDataIndexedPtr(0) = window;
			m_windowFrame = [window frame];

#ifdef RAPP_WITH_BGFX
			osxSetNSWindow(window);
#endif
			MainThreadEntry mte;
			mte.m_argc = _argc;
			mte.m_argv = _argv;

			rtm::Thread thread;
			thread.start(mte.threadFunc, &mte);

			while (!(m_exit = [dg applicationHasTerminated]) )
			{
#ifdef RAPP_WITH_BGFX
				if (bgfx::RenderFrame::Exiting == bgfx::renderFrame() )
				{
					break;
				}
#endif
				while (dispatchEvent(peekEvent() ) )
				{
				}
			}

			m_eventQueue.postExitEvent();

#ifdef RAPP_WITH_BGFX
			while (bgfx::RenderFrame::NoContext != bgfx::renderFrame() ) {};
#endif
			thread.stop();

			return 0;
		}

		bool isValid(WindowHandle _handle)
		{
			return m_windows.isValid(_handle.idx);
		}

		EventQueue m_eventQueue;

		rtm::Data<NSWindow*, RAPP_MAX_WINDOWS, rtm::Storage::Dense>	m_windows; 

		NSRect m_windowFrame;

		float   m_scrollf;
		int32_t m_mx;
		int32_t m_my;
		int32_t m_scroll;
		int32_t m_style;
		bool    m_exit;
		bool    m_fullscreen;
	};

	static Context s_ctx;

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

	void appRunOnMainThread(ThreadFn _fn, void* _userData)
	{
		_fn(_userData);
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
		RTM_UNUSED_4(_app, _x, _y, _width);
		RTM_UNUSED_3(_height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		if (s_ctx.isValid(_handle) )
		{
			dispatch_async(dispatch_get_main_queue()
			, ^{
				[s_ctx.m_windows.getData(_handle.idx) performClose: nil];
			});
		}
	}

	void* windowGetNativeHandle(WindowHandle _handle)
	{
		if (s_ctx.m_windows.isValid(_handle.idx))
			return (void*)s_ctx.m_windows.getData(_handle.idx);
		return (void*)0;
	}
                
	void* windowGetNativeDisplayHandle()
	{
		if (s_ctx.m_windows.isValid(s_defaultWindow.idx))
		{
			NSWindow* window = s_ctx.m_windows.getData(s_defaultWindow.idx);
			NSScreen* screen = [window screen];
			return (void*)screen;
		}
		return 0;
	}

	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		if (s_ctx.isValid(_handle) )
		{
			NSWindow* window = s_ctx.m_windows.getData(_handle.idx);
			NSScreen* screen = [window screen];

			NSRect screenRect = [screen frame];
			CGFloat menuBarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];

			NSPoint position = { float(_x), screenRect.size.height - menuBarHeight - float(_y) };

			dispatch_async(dispatch_get_main_queue()
			, ^{
				[window setFrameTopLeftPoint: position];
			});
		}
	}

	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		if (s_ctx.isValid(_handle) )
		{
			NSSize size = { float(_width), float(_height) };
			dispatch_async(dispatch_get_main_queue()
			, ^{
				[s_ctx.m_windows.getData(_handle.idx) setContentSize: size];
			});
		}
	}

	void windowSetTitle(WindowHandle _handle, const char* _title)
	{
		if (s_ctx.isValid(_handle) )
		{
			NSString* title = [[NSString alloc] initWithCString:_title encoding:1];
			dispatch_async(dispatch_get_main_queue()
			, ^{
				[s_ctx.m_windows.getData(_handle.idx) setTitle: title];
			});
			[title release];
		}
	}

	void windowToggleFrame(WindowHandle _handle)
	{
		if (s_ctx.isValid(_handle) )
		{
			s_ctx.m_style ^= NSWindowStyleMaskTitled;
			dispatch_async(dispatch_get_main_queue()
			, ^{
				[s_ctx.m_windows.getData(_handle.idx) setStyleMask: s_ctx.m_style];
			});
		}
	}

	void windowToggleFullscreen(WindowHandle _handle)
	{
		if (s_ctx.isValid(_handle) )
		{
			NSWindow* window = s_ctx.m_windows.getData(_handle.idx);
			NSScreen* screen = [window screen];
			NSRect screenRect = [screen frame];

			if (!s_ctx.m_fullscreen)
			{
				s_ctx.m_style &= ~NSWindowStyleMaskTitled;
				dispatch_async(dispatch_get_main_queue()
				, ^{
					[NSMenu setMenuBarVisible: false];
					[window setStyleMask: s_ctx.m_style];
					[window setFrame:screenRect display:YES];
				});

				s_ctx.m_fullscreen = true;
			}
			else
			{
				s_ctx.m_style |= NSWindowStyleMaskTitled;
				dispatch_async(dispatch_get_main_queue()
				, ^{
					[NSMenu setMenuBarVisible: true];
					[window setStyleMask: s_ctx.m_style];
					[window setFrame:s_ctx.m_windowFrame display:YES];
				});

				s_ctx.m_fullscreen = false;
			}
		}
	}

	void windowSetMouseLock(WindowHandle _handle, bool _lock)
	{
		RTM_UNUSED_2(_handle, _lock);
	}

	void inputEmitKeyPress(KeyboardKey::Enum _key, uint8_t _modifiers)
	{
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, true);
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, false);
	}

} // namespace rapp

@implementation AppDelegate

+ (AppDelegate *)sharedDelegate
{
	static id delegate = [AppDelegate new];
	return delegate;
}

- (id)init
{
	self = [super init];

	if (nil == self)
	{
		return nil;
	}

	self->terminated = false;
	return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	RTM_UNUSED(sender);
	self->terminated = true;
	return NSTerminateCancel;
}

- (bool)applicationHasTerminated
{
	return self->terminated;
}

@end

@implementation Window

+ (Window*)sharedDelegate
{
    static id windowDelegate = [Window new];
    return windowDelegate;
}

- (id)init
{
    self = [super init];
    if (nil == self)
    {
        return nil;
    }
    
    self->windowCount = 0;
    return self;
}

- (void)windowCreated:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:self];
    
    assert(self->windowCount < ~0u);
    self->windowCount += 1;
}

- (void)windowWillClose:(NSNotification*)notification
{
    RTM_UNUSED(notification);
}

- (BOOL)windowShouldClose:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:nil];
    
    assert(self->windowCount);
    self->windowCount -= 1;
    
    if (self->windowCount == 0)
    {
        [NSApp terminate:self];
        return false;
    }
    
    return true;
}

- (void)windowDidResize:(NSNotification*)notification
{
    RTM_UNUSED(notification);
    using namespace rapp;
    s_ctx.windowDidResize();
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    RTM_UNUSED(notification);
    using namespace rapp;
    //s_ctx.windowDidResize();
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    RTM_UNUSED(notification);
    using namespace rapp;
    //s_ctx.windowDidResize();
}
@end

int main(int _argc, char** _argv)
{
	using namespace rapp;
	return s_ctx.run(_argc, _argv);
}

#endif // RTM_PLATFORM_OSX
