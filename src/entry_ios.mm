//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/input.h>
#include <rapp/src/entry_p.h>

#if RTM_PLATFORM_IOS

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/CAEAGLLayer.h>

#ifdef RAPP_WITH_BGFX
#include <bgfxplatform.h>
#endif

namespace rapp
{
	struct MainThreadEntry
	{
		int m_argc;
		char** m_argv;

		static int32_t threadFunc(void* _userData);
	};

	static WindowHandle s_defaultWindow = { 0 };

	struct Context
	{
		Context(uint32_t _width, uint32_t _height)
		{
			static const char* argv[1] = { "ios" };
			m_mte.m_argc = 1;
			m_mte.m_argv = const_cast<char**>(argv);

			m_eventQueue.postSizeEvent(s_defaultWindow, _width, _height);

#ifdef RAPP_WITH_BGFX
			// Prevent render thread creation.
			bgfx::renderFrame();
#endif

			m_thread.start(MainThreadEntry::threadFunc, &m_mte);
		}

		~Context()
		{
			m_thread.stop();
		}

		MainThreadEntry m_mte;
		rtm::Thread m_thread;

		EventQueue m_eventQueue;
	};

	static Context* s_ctx;

	int32_t MainThreadEntry::threadFunc(void* _userData)
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
		int32_t result = main(self->m_argc, self->m_argv);
		return result;
	}

	const Event* poll()
	{
		return s_ctx->m_eventQueue.poll();
	}

	const Event* poll(WindowHandle _handle)
	{
		return s_ctx->m_eventQueue.poll(_handle);
	}

	void release(const Event* _event)
	{
		s_ctx->m_eventQueue.release(_event);
	}

	void appRunOnMainThread(App::threadFn _fn, void* _userData)
	{
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
		RTM_UNUSED_6(_x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void windowDestroy(WindowHandle _handle)
	{
		RTM_UNUSED(_handle);
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
		RTM_UNUSED(_handle, _lock);
	}

	void inputEmitKeyPress(KeyboardState::Key _key, uint8_t _modifiers)
	{
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, true);
		s_ctx.m_eventQueue.postKeyEvent(rapp::kDefaultWindowHandle, _key, _modifiers, false);
	}

} // namespace rapp

using namespace rapp;

@interface View : UIView
{
	CADisplayLink* m_displayLink;
}

@end

@implementation View

+ (Class)layerClass
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];

	if (nil == self)
	{
		return nil;
	}

#ifdef RAPP_WITH_BGFX
	CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
	bgfx::iosSetEaglLayer(layer);
#endif
	return self;
}

- (void)start
{
	if (nil == m_displayLink)
	{
		m_displayLink = [self.window.screen displayLinkWithTarget:self selector:@selector(renderFrame)];
		//[m_displayLink setFrameInterval:1];
		//[m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		//		[m_displayLink addToRunLoop:[NSRunLoop currentRunLoop]];
		[m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
	}
}

- (void)stop
{
	if (nil != m_displayLink)
	{
		[m_displayLink invalidate];
		m_displayLink = nil;
	}
}

- (void)renderFrame
{
#ifdef RAPP_WITH_BGFX
	bgfx::renderFrame();
#endif
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	RTM_UNUSED(touches);
	UITouch *touch = [[event allTouches] anyObject];
	CGPoint touchLocation = [touch locationInView:self];

	s_ctx->m_eventQueue.postMouseEvent(s_defaultWindow, touchLocation.x, touchLocation.y, 0);
	s_ctx->m_eventQueue.postMouseEvent(s_defaultWindow, touchLocation.x, touchLocation.y, 0, MouseButton::Left, true);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	RTM_UNUSED(touches);
	UITouch *touch = [[event allTouches] anyObject];
	CGPoint touchLocation = [touch locationInView:self];
	s_ctx->m_eventQueue.postMouseEvent(s_defaultWindow, touchLocation.x, touchLocation.y, 0, MouseButton::Left, false);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	RTM_UNUSED(touches);
	UITouch *touch = [[event allTouches] anyObject];
	CGPoint touchLocation = [touch locationInView:self];
	s_ctx->m_eventQueue.postMouseEvent(s_defaultWindow, touchLocation.x, touchLocation.y, 0);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	RTM_UNUSED(touches);
	UITouch *touch = [[event allTouches] anyObject];
	CGPoint touchLocation = [touch locationInView:self];
	s_ctx->m_eventQueue.postMouseEvent(s_defaultWindow, touchLocation.x, touchLocation.y, 0, MouseButton::Left, false);
}

@end

@interface AppDelegate : UIResponder<UIApplicationDelegate>
{
	UIWindow* m_window;
	View* m_view;
}

@property (nonatomic, retain) UIWindow* m_window;
@property (nonatomic, retain) View* m_view;

@end

@implementation AppDelegate

@synthesize m_window;
@synthesize m_view;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	RTM_UNUSED(application, launchOptions);

	CGRect rect = [ [UIScreen mainScreen] bounds];
	m_window = [ [UIWindow alloc] initWithFrame: rect];
	m_view = [ [View alloc] initWithFrame: rect];

	[m_window addSubview: m_view];
	[m_window makeKeyAndVisible];

	//float scaleFactor = [[UIScreen mainScreen] scale]; // should use this, but ui is too small on ipad retina
	float scaleFactor = 1.0f;
	[m_view setContentScaleFactor: scaleFactor ];

	s_ctx = new Context((uint32_t)(scaleFactor*rect.size.width), (uint32_t)(scaleFactor*rect.size.height));
	return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	RTM_UNUSED(application);
	[m_view stop];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	RTM_UNUSED(application);
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	RTM_UNUSED(application);
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	RTM_UNUSED(application);
	[m_view start];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	RTM_UNUSED(application);
	[m_view stop];
}

- (void)dealloc
{
	[m_window release];
	[m_view release];
	[super dealloc];
}

@end

int main(int _argc, char* _argv[])
{
	NSAutoreleasePool* pool = [ [NSAutoreleasePool alloc] init];
	int exitCode = UIApplicationMain(_argc, _argv, @"UIApplication", NSStringFromClass([AppDelegate class]) );
	[pool release];
	return exitCode;
}

#endif // RTM_PLATFORM_IOS
