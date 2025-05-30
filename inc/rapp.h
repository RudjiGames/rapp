//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_H
#define RTM_RAPP_H

#include <stdint.h>

typedef struct _rtmLibInterface rtmLibInterface;

namespace rapp {

	struct KeyboardState
	{
		enum Key
		{
			None,
			Esc, Return, Tab, Space, Backspace, Up, Down, Left, Right,
			Insert, Delete, Home, End, PageUp, PageDown,
			Print, Plus, Minus, Equal, LeftBracket, RightBracket,
			Semicolon, Quote, Comma, Period, Slash, Backslash, Tilde,
			F1, F2, F3, F4, F5, F6, 
			F7, F8, F9, F10, F11, F12,
			NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, 
			NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
			Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

			KeyA = 65,
			KeyB, KeyC, KeyD, KeyE, KeyF, KeyG, KeyH, KeyI, KeyJ,
			KeyK, KeyL, KeyM, KeyN, KeyO, KeyP, KeyQ, KeyR, KeyS, KeyT,
			KeyU, KeyV, KeyW, KeyX, KeyY, KeyZ,

			Count
		};

		enum Modifier
		{
			NoMods	= 0x00,
			LAlt	= 0x01,
			RAlt	= 0x02,
			LCtrl	= 0x04,
			RCtrl	= 0x08,
			LShift	= 0x10,
			RShift	= 0x20,
			LMeta	= 0x40,
			RMeta	= 0x80,
		};
		
		uint8_t				m_keysPressed;
		uint8_t				m_modifiersSinceLastFrame;
		uint8_t				m_modifiers[Key::Count];
		KeyboardState::Key	m_keys[Key::Count];
	};

	struct MouseState
	{
		enum Button
		{
			None,
			Left,
			Middle,
			Right,

			Count
		};

		int32_t		m_absolute[3];
		float		m_norm[3];
		uint8_t		m_buttons[MouseState::Button::Count];
	};

	struct TouchState
	{
		static const int MAX_MULTITOUCH = 4;

		struct Touch
		{
			int32_t		m_absolute[3];
			float		m_norm[3];
		};

		Touch		m_mice[MAX_MULTITOUCH];
		int32_t		m_accelerometer[3];
	};

	struct GamepadState
	{
		enum Buttons
		{
			None		= 0,
			X			= 0x0001,
			Y			= 0x0002,
			A			= 0x0004,
			B			= 0x0008,
			Up			= 0x0010,
			Down		= 0x0020,
			Left		= 0x0040,
			Right		= 0x0080,
			LThumb		= 0x0100,
			RThumb		= 0x0200,
			Start		= 0x0400,
			Back		= 0x0800,
			LShoulder	= 0x1000,
			RShoulder	= 0x2000,
			Guide		= 0x4000,
			Connected	= 0x8000
		};

		int32_t		m_LStick[2];
		int32_t		m_RStick[2];
		uint8_t		m_LTrigger;
		uint8_t		m_RTrigger;
		uint16_t	m_buttons;
	};

	struct WindowHandle  { uint32_t idx; };
	inline bool isValid(WindowHandle _handle)  { return UINT32_MAX != _handle.idx; }
	constexpr WindowHandle kDefaultWindowHandle = { 0 };

	#define RAPP_WINDOW_FLAG_ASPECT_NONE	0x0
	#define RAPP_WINDOW_FLAG_ASPECT_RATIO	0x1
	#define RAPP_WINDOW_FLAG_FRAME			0x2
	#define RAPP_WINDOW_FLAG_RENDERING		0x4
	#define RAPP_WINDOW_FLAG_MAIN_WINDOW	0x8

	struct AppData;

	struct App
	{
		const char*		m_name;
		const char*		m_description;
		int32_t			m_exitCode;
		uint32_t		m_width;
		uint32_t		m_height;
		uint32_t		m_frameRate;
		AppData*		m_data;
		bool			m_resetView;

		App(const char* _name, const char* _description = 0);
		virtual ~App() {}

		void quit() { m_exitCode = -1; }

		virtual int		init(int32_t _argc, const char* const* _argv, rtmLibInterface* _libInterface = 0) = 0;
		virtual void	suspend()			= 0;
		virtual void	resume()			= 0;
		virtual void	update(float _time)	= 0;
		virtual void	draw(float _alpha)	= 0;
		virtual void	drawGUI() {}
		virtual void	shutDown()			= 0;
		virtual bool	isGUImode() { return true; }
	};

	typedef int(*ConsoleFn)(App* _app, void* _userData, int _argc, char const* const* _argv);
	typedef void(*ThreadFn)(void* _userData);
	typedef void(*JobFn)(void* _userData, uint32_t _start, uint32_t _end);

	// ------------------------------------------------
	/// Initialization functions
	// ------------------------------------------------

	/// 
	void init(rtmLibInterface* _libInterface = 0);

	/// 
	void shutDown();

	// ------------------------------------------------
	/// Application functions
	// ------------------------------------------------

	///
	App* appGet(uint32_t _index);

	///
	uint32_t appGetCount();

	// 
	void appSetUpdateFrameRate(App* _app, int32_t fps);

	///
	int appRun(App* _app, int _argc, const char* const* _argv);

	///
	void appRunOnMainThread(ThreadFn _fn, void* _userData);

	///
	WindowHandle appGraphicsInit(App* _app, uint32_t _width, uint32_t _height);

	///
	void appGraphicsShutdown(App* _app, WindowHandle _mainWindow);

	// ------------------------------------------------
	/// Vector rendering functions
	// ------------------------------------------------

	#define VG_CCW	0
	#define VG_CW	1

	#define VG_LINE_CAP_BUTT	0
	#define VG_LINE_CAP_ROUND	1
	#define VG_LINE_CAP_SQUARE	2

	#define VG_LINE_JOIN_MITER	0
	#define VG_LINE_JOIN_ROUND	1
	#define VG_LINE_JOIN_BEVEL	2

	///
	uint32_t vgColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255);

	///
	void vgSave();

	///
	void vgRestore();

	///
	void vgGlobalAlpha(float _opacity);

	///
	void vgBeginPath();

	///
	void vgMoveTo(float _x, float _y);

	///
	void vgLineTo(float _x, float _y);

	///
	void vgBezierTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y);

	///
	void vgQuadraticTo(float _cx, float _cy, float _x, float _y);

	///
	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r);

	///
	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir);

	///
	void vgRect(float _x, float _y, float _w, float _h);

	///
	void vgRect(float _x, float _y, float _w, float _h, float _r);

	///
	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl);

	///
	void vgCircle(float _cx, float _cy, float _radius);

	///
	void vgEllipse(float _cx, float _cy, float _rx, float _ry);

	///
	void vgPolyline(const float* _coords, uint32_t _numPoints);

	///
	void vgPathWinding(int _winding);

	///
	void vgClosePath();

	///
	void vgFill(uint32_t _color, uint32_t _flags);

	///
	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol);

	///
	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol);

	///
	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol);

	///
	void vgStroke(uint32_t _color, float _width, uint32_t _flags);

	///
	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol, float _width);

	///
	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol, float _width);

	///
	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol, float _width);

	///
	void vgLineCapJoin(int _lineCap, int _lineJoin);

	///
	void vgTransformPoint(float* dx, float* dy, const float* t, float sx, float sy);

	///
	void vgTransformScale(float* _xform, float _scaleX, float _scaleY);

	///
	void vgTransformTranslate(float* _xform, float _translateX, float _translateY);

	///
	void vgTransformTranslateScale(float* _xform, float _translateX, float _translateY, float _scaleX, float _scaleY);

	///
	void vgTransformRotate(float* _xform, float _angleRadian);

	///
	void vgTransformMultiply(float* _xform, const float* _left, const float* _right);

	// ------------------------------------------------
	/// Debug output functions
	// ------------------------------------------------

	///
	void dbgPrintf(const char* _format, ...);

	///
	void dbgPrintfData(const void* _data, uint32_t _size, const char* _format, ...);

	// ------------------------------------------------
	/// Custom command functions
	// ------------------------------------------------

	///
	void cmdAdd(const char* _name, ConsoleFn _fn, void* _userData = 0, const char* _description = "");

	///
	void cmdRemove(const char* _name);

	///
	bool cmdExec(App* _app, const char* _cmd, int* _errorCode);

	///
	void cmdConsoleLog(App* _app, const char* _fmt, ...);

	///
	void cmdConsoleLogRGB(uint8_t _r, uint8_t _g, uint8_t _b, App* _app, const char* _fmt, ...);

	///
	void cmdConsoleToggle(App* _app);

	///
	void cmdConsoleSetToggleTime(float _time);

	// ------------------------------------------------
	/// Window functions
	// ------------------------------------------------

	/// 
	void windowGetDefaultSize(uint32_t* _width, uint32_t* _height);

	/// 
	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags = RAPP_WINDOW_FLAG_ASPECT_NONE, const char* _title = "");

	/// 
	void windowDestroy(WindowHandle _handle);

	/// 
	void* windowGetNativeHandle(WindowHandle _handle);

	/// 
	void* windowGetNativeDisplayHandle();

	/// 
	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y);

	/// 
	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height);

	/// 
	void windowSetTitle(WindowHandle _handle, const char* _title);

	/// 
	void windowToggleFrame(WindowHandle _handle);

	/// 
	void windowToggleFullscreen(WindowHandle _handle = {0});

	/// 
	void windowSetMouseLock(WindowHandle _handle, bool _lock);

	// ------------------------------------------------
	/// Job system types and functions
	// ------------------------------------------------

	struct JobHandle { uintptr_t  idx; };

	///
	JobHandle jobCreate(JobFn  _func, void* _userData = 0, bool _deleteOnFinish = true);

	///
	JobHandle jobCreateGroup(JobFn _func, void* _userData, uint32_t _dataStride, uint32_t _numJobs, bool _deleteOnFinish = true);

	///
	void jobDestroy(JobHandle _job);

	///
	void jobRun(JobHandle _job);

	/// 
	void jobWait(JobHandle _job);

	///
	uint32_t jobStatus(JobHandle _job);

	// ------------------------------------------------
	/// Input functions
	// ------------------------------------------------

	struct InputBinding;

	typedef void(*InputBindingFn)(const void* _userData, const InputBinding* _binding);

	struct InputBindingKeyboard
	{
		KeyboardState::Key		m_key;
		uint8_t					m_modifiers;
	};

	struct InputBindingMouse
	{
		MouseState::Button		m_button;		// MouseState::Button::None for movement binding
		uint8_t					m_modifiers;
	};

	struct InputBindingGamepad
	{
		enum Stick
		{
			None,
			LeftStick,
			LeftTrigger,
			RightStick,
			RightTrigger
		};

		GamepadState::Buttons	m_button;		// GamepadState::Button::None for stick/trigger event
		uint8_t					m_gamepadIndex;
		Stick					m_stick;
	};

	struct InputBindingTouch
	{
	};

	struct InputBinding
	{
		enum Enum
		{
			Gamepad,
			Mouse,
			Keyboard,
			Touch,

			Count
		};

		InputBinding::Enum		m_type;
		InputBindingFn			m_fn;
		const void*				m_userData;
		uint8_t					m_flags;

		union
		{
			InputBindingKeyboard	m_bindingKeyboard;
			InputBindingMouse		m_bindingMouse;
			InputBindingGamepad		m_bindingGamepad;
			InputBindingTouch		m_bindingTouch;
		};

		InputBinding(int);
		InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingKeyboard& _kb);
		InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingMouse& _mouse);
		InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingGamepad& _gp);
		InputBinding(InputBindingFn _fn, const void* _userData, uint8_t _flag, const InputBindingTouch& _touch);
	};

	#define RAPP_INPUT_BINDING_END		{ 0 }

	///
	void inputAddBindings(const char* _name, const InputBinding* _bindings);

	///
	void inputRemoveBindings(const char* _name);

	/// 
	void inputGetGamePadState(int _index, GamepadState& _gps);

	/// 
	void inputGetMouseState(MouseState& _ms);

	/// 
	void inputSetMouseLock(bool _lock);

	/// 
	bool inputGetMouseLock();

	/// 
	void inputGetKeyboardState(KeyboardState& _ks);

	///
	bool inputGetKeyState(KeyboardState::Key _key, uint8_t* _modifiers = 0);

	/// 
	void inputEmitKeyPress(KeyboardState::Key _key, uint8_t _modifiers = 0);

	/// 
	void inputDgbGamePads(int _maxGamepads = 4);

	/// 
	void inputDgbMouse();

	/// 
	void inputDgbKeyboard();

	/// 
	void inputDgbTouch();

} // namespace rapp

#ifdef RAPP_WITH_BGFX
#include <bgfx/bgfx.h>
#include <dear-imgui/imgui.h>
#endif

#define RAPP_CLASS(_appClass)												\
	_appClass(const char* _name, const char* _description = 0)				\
		: App(_name, _description) {}										\
	virtual ~_appClass() {}

#define RAPP_INSTANCE(_appClass)											\
	s_ ## _appClass ## _app

#define RAPP_REGISTER(_appClass, _name, _description)						\
	_appClass RAPP_INSTANCE(_appClass) (_name, _description);

#define RAPP_DBG_STRINGIZE(_x) RAPP_DBG_STRINGIZE_(_x)
#define RAPP_DBG_STRINGIZE_(_x) #_x
#define RAPP_DBG_FILE_LINE_LITERAL "" __FILE__ "(" RAPP_DBG_STRINGIZE(__LINE__) "): "
#define RAPP_DBG(_format, ...) rapp::dbgPrintf(RAPP_DBG_FILE_LINE_LITERAL "" _format "\n", ##__VA_ARGS__)

#endif // RTM_RAPP_H
