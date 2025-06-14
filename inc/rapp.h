//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_H
#define RTM_RAPP_H

#include <stdint.h>

typedef struct _rtmLibInterface rtmLibInterface;

namespace rapp {

	struct WindowHandle  { uint32_t idx; };
	inline bool isValid(WindowHandle _handle)  { return UINT32_MAX != _handle.idx; }
	constexpr WindowHandle kDefaultWindowHandle = { 0 };

	#define RAPP_WINDOW_FLAG_ASPECT_NONE	0x00
	#define RAPP_WINDOW_FLAG_ASPECT_RATIO	0x01
	#define RAPP_WINDOW_FLAG_FRAME			0x02
	#define RAPP_WINDOW_FLAG_RENDERING		0x04
	#define RAPP_WINDOW_FLAG_MAIN_WINDOW	0x08
	#define RAPP_WINDOW_FLAG_DPI_AWARE		0x10

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
	typedef void(*TaskFn)(void* _userData, uint32_t _start, uint32_t _end);

	// ------------------------------------------------
	/// Initialization functions
	// ------------------------------------------------

	/// Initializes rapp library.
	///
	/// @param[in] _libInterface   : Optional pointer to rtm library interface (alloc, log)
	///
	/// @returns true on success
	void init(rtmLibInterface* _libInterface = 0);

	/// Shuts down rapp library and release resources.
	void shutDown();

	// ------------------------------------------------
	/// Application functions

	/// Returns application at provided index.
	///
	/// @param[in] _index          : Index of the application to get.
	///
	/// @returns pointer to application or nullptr if index is invalid
	App* appGet(uint32_t _index);

	/// Returns number of registered applications.
	///
	/// @returns Number of registered applications.
	uint32_t appGetCount();

	/// Sets application update rate, in frames per second.
	///
	/// @param[in] _app            : Application to change update FPS on.
	/// @param[in] _fps            : Application updates per second to run.
	void appSetUpdateFrameRate(App* _app, int32_t fps);

	/// Runs application with command line arguments.
	///
	/// @param[in] _app            : Application to run.
	/// @param[in] _argc           : Arguments count.
	/// @param[in] _argv           : Arguments list.
	int appRun(App* _app, int _argc, const char* const* _argv);

	/// Runs a function on main thread
	///
	/// @param[in] _fn             : Function to run.
	/// @param[in] _userData       : User data for function argument.
	void appRunOnMainThread(ThreadFn _fn, void* _userData);

	/// Initializes application graphics rendering and creates a main window.
	///
	/// @param[in] _app            : Application to initialize rendering for.
	/// @param[in] _width          : Width of the main window.
	/// @param[in] _height         : Height of the main window.
	/// @param[in] _mainwindowFlags: Optional, additional window craetion flags.
	WindowHandle appGraphicsInit(App* _app, uint32_t _width, uint32_t _height, uint32_t _mainwindowFlags = RAPP_WINDOW_FLAG_ASPECT_RATIO);

	/// Shuts down application rendering and releases related resources.
	///
	/// @param[in] _app            : Application to shut down rendering for.
	/// @param[in] _mainWindow     : Main window handle.
	void appGraphicsShutdown(App* _app, WindowHandle _mainWindow);

	// ------------------------------------------------
	/// Vector rendering functions
	// ------------------------------------------------

	#define VG_WINDING_CCW		0
	#define VG_WINDING_CW		1

	#define VG_LINE_CAP_BUTT	0
	#define VG_LINE_CAP_ROUND	1
	#define VG_LINE_CAP_SQUARE	2

	#define VG_LINE_JOIN_MITER	0
	#define VG_LINE_JOIN_ROUND	1
	#define VG_LINE_JOIN_BEVEL	2

	/// Crates a 32bit color out of 8bit color components.
	///
	/// @param[in] _r              : Red color component.
	/// @param[in] _g              : Green color component.
	/// @param[in] _b              : Blue color component.
	/// @param[in] _a              : Alpha color component, defaults to 255 (1.0f).
	///
	/// @returns color in 32bit format (RGBA)
	uint32_t vgColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255);

	/// Saves vector graphics context state.
	void vgSave();

	/// Restores vector graphics context state.
	void vgRestore();

	/// Sets global alpha to provided value.
	///
	/// @param[in] _opacity        : Opacity/alpha value to set.
	void vgGlobalAlpha(float _opacity);

	/// Begins a new vector path
	void vgBeginPath();

	/// Moves draw cursor to provided coordinates.
	///
	/// @param[in] _x              : X axis coordinate.
	/// @param[in] _y              : Y axis coordinate. 
	void vgMoveTo(float _x, float _y);

	/// 
	///
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	void vgLineTo(float _x, float _y);

	/// 
	///
	/// @param[in] _c1x            : 
	/// @param[in] _c1y            : 
	/// @param[in] _c2x            : 
	/// @param[in] _c2y            : 
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	void vgBezierTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y);

	/// 
	///
	/// @param[in] _cx             : 
	/// @param[in] _cy             : 
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	void vgQuadraticTo(float _cx, float _cy, float _x, float _y);

	/// 
	///
	/// @param[in] _x1             : 
	/// @param[in] _y1             : 
	/// @param[in] _x2             : 
	/// @param[in] _y2             : 
	/// @param[in] _r              : 
	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r);

	/// 
	///
	/// @param[in] _cx             : 
	/// @param[in] _cy             : 
	/// @param[in] _r              : 
	/// @param[in] _a0             : 
	/// @param[in] _a1             : 
	/// @param[in] _dir            : 
	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir);

	/// 
	///
	/// @param[in] _x              : Rect position, X coordinate.
	/// @param[in] _y              : Rect position, Y coordinate.
	/// @param[in] _w              : Rect width.
	/// @param[in] _h              : Rect height.
	void vgRect(float _x, float _y, float _w, float _h);

	/// 
	///
	/// @param[in] _x              : Rect position, X coordinate.
	/// @param[in] _y              : Rect position, Y coordinate.
	/// @param[in] _w              : Rect width.
	/// @param[in] _h              : Rect height.
	/// @param[in] _r              : Rounding radius in rectangle corners.
	void vgRect(float _x, float _y, float _w, float _h, float _r);

	/// 
	///
	/// @param[in] _x              : Rect position, X coordinate.
	/// @param[in] _y              : Rect position, Y coordinate.
	/// @param[in] _w              : Rect width.
	/// @param[in] _h              : Rect height.
	/// @param[in] _rtl            : Rounding radius in top left corner.
	/// @param[in] _rtr            : Rounding radius in top right corner.
	/// @param[in] _rbr            : Rounding radius in bottom right corner.
	/// @param[in] _rbl            : Rounding radius in bottom left corner.
	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl);

	/// 
	///
	/// @param[in] _cx             : Circle position, X coordinate.
	/// @param[in] _cy             : Circle position, Y coordinate.
	/// @param[in] _radius         : Circle radius.
	void vgCircle(float _cx, float _cy, float _radius);

	/// 
	///
	/// @param[in] _cx             : Ellipse center, X coordinate.
	/// @param[in] _cy             : Ellipse center, Y coordinate.
	/// @param[in] _rx             : Ellipse radius along X axis.
	/// @param[in] _ry             : Ellipse radius along Y axis.
	void vgEllipse(float _cx, float _cy, float _rx, float _ry);

	/// 
	/// @param[in] _coords         : 
	/// @param[in] _numPoints      : 
	void vgPolyline(const float* _coords, uint32_t _numPoints);

	/// Sets path winding.
	///
	/// @param[in] _winding        : Winding direction, either VG_WINDING_CW or VG_WINDING_CCW.
	void vgPathWinding(int _winding);

	/// Closes an open vector path.
	void vgClosePath();

	/// 
	///
	/// @param[in] _color          : 
	/// @param[in] _flags          : 
	void vgFill(uint32_t _color, uint32_t _flags);

	/// 
	///
	/// @param[in] _sx             : 
	/// @param[in] _sy             : 
	/// @param[in] _ex             : 
	/// @param[in] _ey             : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol);

	/// 
	///
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	/// @param[in] _w              : 
	/// @param[in] _h              : 
	/// @param[in] _r              : 
	/// @param[in] _f              : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol);

	/// 
	///
	/// @param[in] _cx             : 
	/// @param[in] _cy             : 
	/// @param[in] _inr            : 
	/// @param[in] _outr           : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol);

	/// 
	///
	/// @param[in] _color          : 
	/// @param[in] _width          : 
	/// @param[in] _flags          : 
	void vgStroke(uint32_t _color, float _width, uint32_t _flags);

	/// 
	///
	/// @param[in] _sx             : 
	/// @param[in] _sy             : 
	/// @param[in] _ex             : 
	/// @param[in] _ey             : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	/// @param[in] _width          : 
	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol, float _width);

	/// 
	///
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	/// @param[in] _w              : 
	/// @param[in] _h              : 
	/// @param[in] _r              : 
	/// @param[in] _f              : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	/// @param[in] _width          : 
	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol, float _width);

	/// 
	///
	/// @param[in] _cx             : 
	/// @param[in] _cy             : 
	/// @param[in] _inr            : 
	/// @param[in] _outr           : 
	/// @param[in] _icol           : 
	/// @param[in] _ocol           : 
	/// @param[in] _width          : 
	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol, float _width);

	/// 
	///
	/// @param[in] _lineCap        : 
	/// @param[in] _lineJoin       : 
	void vgLineCapJoin(int _lineCap, int _lineJoin);

	/// 
	///
	/// @param[in] _dx             : 
	/// @param[in] _dy             : 
	/// @param[in] _t              : 
	/// @param[in] _sx             : 
	/// @param[in] _sy             : 
	void vgTransformPoint(float* dx, float* dy, const float* t, float sx, float sy);

	/// 
	///
	/// @param[in] _xform          : 
	/// @param[in] _scaleX         : 
	/// @param[in] _scaleY         : 
	void vgTransformScale(float* _xform, float _scaleX, float _scaleY);

	/// 
	///
	/// @param[in] _xform          : 
	/// @param[in] _translateX     : 
	/// @param[in] _translateY     : 
	void vgTransformTranslate(float* _xform, float _translateX, float _translateY);

	/// 
	///
	/// @param[in] _xform          : 
	/// @param[in] _translateX     : 
	/// @param[in] _translateY     : 
	/// @param[in] _scaleX         : 
	/// @param[in] _scaleY         : 
	void vgTransformTranslateScale(float* _xform, float _translateX, float _translateY, float _scaleX, float _scaleY);

	/// 
	///
	/// @param[in] _xform          : 
	/// @param[in] _angleRadian    : 
	void vgTransformRotate(float* _xform, float _angleRadian);

	/// 
	///
	/// @param[in] _xform          : 
	/// @param[in] _left           : 
	/// @param[in] _right          : 
	void vgTransformMultiply(float* _xform, const float* _left, const float* _right);

	// ------------------------------------------------
	/// Debug output functions

	///	Prints formatted text to debug output
	///
	/// @param[in] _format         : Text format.
	/// @param[in] va_args         : Text formatting arguments.
	void dbgPrintf(const char* _format, ...);

	///	Prints a data block to debug output.
	///
	/// @param[in] _data           : Pointer to data block.
	/// @param[in] _size           : Size of the data block.
	/// @param[in] _format         : Text format.
	/// @param[in] va_args         : Text formatting arguments.
	void dbgPrintfData(const void* _data, uint32_t _size, const char* _format, ...);

	// ------------------------------------------------
	/// Custom command functions
	// ------------------------------------------------

	/// Adds/registers a custom command.
	///
	/// @param[in] _name           : Command name.
	/// @param[in] _fn             : Command callback.
	/// @param[in] _userData       : Callback user data.
	/// @param[in] _description    : Command description.
	void cmdAdd(const char* _name, ConsoleFn _fn, void* _userData = 0, const char* _description = "");

	/// Removes/unregisters a custom command.
	///
	/// @param[in] _name           : Name of command to unregister.
	void cmdRemove(const char* _name);

	/// Runs a custom command in the context of an application.
	///
	/// @param[in] _app            : Application context.
	/// @param[in] _cmd            : Command to run.
	/// @param[in,out] _errorCode  : Optional pointer to error code result.
	bool cmdExec(App* _app, const char* _cmd, int* _errorCode);

	/// Outputs formatted text to application console.
	///
	/// @param[in] _app            : Application to output text for.
	/// @param[in] _fmt            : Text format.
	/// @param[in] va_args         : Variable arguments
	void cmdConsoleLog(App* _app, const char* _fmt, ...);

	/// Outputs colored formatted text to application console.
	///
	/// @param[in] _r              : Red color component.
	/// @param[in] _g              : Green color component.
	/// @param[in] _b              : Blue color component.
	/// @param[in] _app            : Application to output text for.
	/// @param[in] _fmt            : Text format.
	/// @param[in] va_args         : Variable arguments
	void cmdConsoleLogRGB(uint8_t _r, uint8_t _g, uint8_t _b, App* _app, const char* _fmt, ...);

	///	Toggles console visibility status.
	///
	/// @param[in] _app            : Application to toggle console for.
	void cmdConsoleToggle(App* _app);

	/// Sets time for console toggle transition.
	///
	/// @param[in] _time           : Time, in seconds.
	void cmdConsoleSetToggleTime(float _time);

	// ------------------------------------------------
	/// Window functions
	// ------------------------------------------------

	/// Get default application window size.
	///
	/// @param[in,out] _width      : Pointer to variable to store window width in.
	/// @param[in,out] _heught     : Pointer to variable to store window height in.
	void windowGetDefaultSize(uint32_t* _width, uint32_t* _height);

	/// Creates an application window.
	///
	/// @param[in] _app            : Application to create window for.
	/// @param[in] _x              : Window position X coordinate.
	/// @param[in] _y              : Window position Y coordinate.
	/// @param[in] _width          : Window width.
	/// @param[in] _heught         : Window height.
	/// @param[in] _flags          : Creation flags.
	/// @param[in] _title          : Window title.
	WindowHandle windowCreate(App* _app, int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags = RAPP_WINDOW_FLAG_ASPECT_NONE, const char* _title = "");

	/// Destroys a window and release related resources.
	///
	/// @param[in] _handle         : Handle to window to destroy.
	void windowDestroy(WindowHandle _handle);

	/// Returns native OS window handle.
	///
	/// @param[in] _handle         : Handle to window to get a native handle of.
	///
	/// @returns Native OS window handle.
	void* windowGetNativeHandle(WindowHandle _handle);

	/// Returns native OS display handle.
	///
	/// @returns Native display handle.
	void* windowGetNativeDisplayHandle();

	/// Sets window position.
	///
	/// @param[in] _handle         : Handle to window.
	/// @param[in] _x              : 
	/// @param[in] _y              : 
	void windowSetPos(WindowHandle _handle, int32_t _x, int32_t _y);

	/// Sets window size.
	///
	/// @param[in] _handle         : Handle to window.
	/// @param[in] _width          : 
	/// @param[in] _height         : 
	void windowSetSize(WindowHandle _handle, uint32_t _width, uint32_t _height);

	/// Sets window title.
	///
	/// @param[in] _handle         : Handle to window.
	/// @param[in] _title          : New window title.
	void windowSetTitle(WindowHandle _handle, const char* _title);

	/// Toggles window frame.
	///
	/// @param[in] _handle         : Handle to window.
	void windowToggleFrame(WindowHandle _handle);

	/// Toggles window full screen mode.
	///
	/// @param[in] _handle         : Handle to window.
	void windowToggleFullscreen(WindowHandle _handle = {0});

	/// Locks mouse cursor to window.
	///
	/// @param[in] _handle         : Handle to window to lock mouse to.
	/// @param[in] _lock           : Lock status.
	void windowSetMouseLock(WindowHandle _handle, bool _lock);

	// ------------------------------------------------
	/// Task system types and functions
	// ------------------------------------------------

	#define RAPP_TASK_STATUS_PENDING	0
	#define RAPP_TASK_STATUS_RUNNING	1
	#define RAPP_TASK_STATUS_COMPLETE	2

	struct TaskHandle { uintptr_t  idx; };

	/// Creates a task to run in a task system.
	/// 
	/// @param[in] _func           : Function to run.
	/// @param[in] _userData       : User data to provide to function the call as argument.
	/// @param[in] _delteOnFinish  : Whether to delete the task once it is finished.
	///
	/// @returns Handle for the created task.
	TaskHandle taskCreate(TaskFn  _func, void* _userData = 0, bool _deleteOnFinish = true);

	/// Creates a task group for linearly data stored.
	/// 
	/// @param[in] _func           : Function to run on each of the data blocks (_dataStride size)
	/// @param[in] _userData       : User data to process.
	/// @param[in] _dataStride     : Stride between two blocks of data.
	/// @param[in] _numTasks       : Number of tasks to run, implicitly defining data size.
	/// @param[in] _delteOnFinish  : Whether to delete the task once it is finished.
	///
	/// @returns Handle for the created task.
	TaskHandle taskCreateGroup(TaskFn _func, void* _userData, uint32_t _dataStride, uint32_t _numTasks, bool _deleteOnFinish = true);

	/// Destroys a task.
	/// 
	/// @param[in] _task           : Handle for the taks to destroy.
	void taskDestroy(TaskHandle _task);

	/// Schedules a task for running.
	/// 
	/// @param[in] _task           : Task to run.
	void taskRun(TaskHandle _task);

	/// Wait on task to finish.
	/// 
	/// @param[in] _task           : Task to wait on.
	void taskWait(TaskHandle _task);

	/// Returns current status of the task.
	/// 
	/// @param[in] _task           : Task to get status of.
	///
	/// @returns Status of the task.
	uint32_t taskStatus(TaskHandle _task);

	// ------------------------------------------------
	/// Input functions
	// ------------------------------------------------

	struct KeyboardKey
	{
		enum Enum
		{
			None,

			Esc, Return, Tab, Space, Backspace, Up, Down, Left,
			Right, Insert, Delete, Home, End, PageUp, PageDown,
			Print, Plus, Minus, Equal, LeftBracket, RightBracket,
			Semicolon, Quote, Comma, Period, Slash, Backslash, Tilde,

			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

			NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
			NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,

			Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

			KeyA = 65, KeyB, KeyC, KeyD, KeyE, KeyF, KeyG, KeyH, KeyI,
			KeyJ, KeyK, KeyL, KeyM, KeyN, KeyO, KeyP, KeyQ, KeyR, KeyS,
			KeyT, KeyU, KeyV, KeyW, KeyX, KeyY, KeyZ,

			Count
		};
	};

	struct KeyboardModifier
	{
		enum Enum
		{
			None		= 0x00,
			LAlt		= 0x01,
			RAlt		= 0x02,
			LCtrl		= 0x04,
			RCtrl		= 0x08,
			LShift		= 0x10,
			RShift		= 0x20,
			LMeta		= 0x40,
			RMeta		= 0x80,
		};
	};

	struct MouseButton
	{
		enum Enum
		{
			None	= 0,
			Left,
			Middle,
			Right,

			Count
		};
	};

	struct GamepadButton
	{
		enum Enum
		{
			None		= 0x0000,
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
	};

	struct GamepadStick
	{
		enum Enum
		{
			NoStick		= 0,
			LeftStick,
			LeftTrigger,
			RightStick,
			RightTrigger
		};
	};

	struct KeyboardState
	{
		uint8_t				m_keysPressed;
		uint8_t				m_modifiersSinceLastFrame;
		uint8_t				m_modifiers[KeyboardKey::Count];
		KeyboardKey::Enum	m_keys[KeyboardKey::Count];
	};

	struct MouseState
	{
		int32_t				m_absolute[3];
		float				m_norm[3];
		uint8_t				m_buttons[MouseButton::Count];
	};

	struct TouchState
	{
		static const int MAX_MULTITOUCH = 4;

		struct Touch
		{
			int32_t			m_absolute[3];
			float			m_norm[3];
		};

		Touch				m_mice[MAX_MULTITOUCH];
		int32_t				m_accelerometer[3];
	};

	struct GamepadState
	{
		int32_t				m_LStick[2];
		int32_t				m_RStick[2];
		uint8_t				m_LTrigger;
		uint8_t				m_RTrigger;
		uint16_t			m_buttons;
	};

	struct InputBindingKeyboard
	{
		KeyboardKey::Enum	m_key;
		uint8_t				m_modifiers;
	};

	struct InputBindingMouse
	{
		MouseButton::Enum	m_button;		// MouseButton::None for movement binding
		uint8_t				m_modifiers;
	};

	struct InputBindingGamepad
	{
		GamepadButton::Enum	m_button;		// GamepadState::Button::None for stick/trigger event
		uint8_t				m_gamepadIndex;
		GamepadStick::Enum	m_stick;
	};

	struct InputBindingTouch
	{
	};

	struct InputBinding
	{
		typedef void(*InputBindingFn)(const void* _userData, const InputBinding* _binding);

		uint32_t			m_flags;
		InputBindingFn		m_fn;
		const void*			m_userData;

		union
		{
			InputBindingKeyboard	m_bindingKeyboard;
			InputBindingMouse		m_bindingMouse;
			InputBindingGamepad		m_bindingGamepad;
			InputBindingTouch		m_bindingTouch;
		};

		InputBinding(int);
		InputBinding(InputBindingFn _fn, const void* _userData, uint32_t _flag, const InputBindingKeyboard& _kb);
		InputBinding(InputBindingFn _fn, const void* _userData, uint32_t _flag, const InputBindingMouse& _mouse);
		InputBinding(InputBindingFn _fn, const void* _userData, uint32_t _flag, const InputBindingGamepad& _gp);
		InputBinding(InputBindingFn _fn, const void* _userData, uint32_t _flag, const InputBindingTouch& _touch);
	};

	#define RAPP_INPUT_BINDING_END		{ 0 }

	/// Adds input bindings to binding table.
	/// 
	/// @param[in] _name           : Name of the binding.
	/// @param[in] _bindings       : Array of input bindings, ending with nullptr.
	void inputAddBindings(const char* _name, const InputBinding* _bindings);

	/// Removes an input binding.
	/// 
	/// @param[in] _name           : Binding name.
	void inputRemoveBindings(const char* _name);

	/// Retrieves state of a game pad.
	/// 
	/// @param[in] _index          : Index of the game pad to get state for.
	/// @param[in,out] _gps        : Game pad state structure reference.
	void inputGetGamePadState(int _index, GamepadState& _gps);

	/// Retrieves state of the mouse.
	/// 
	/// @param[in,out] _ms         : Mouse state structure reference.
	void inputGetMouseState(MouseState& _ms);

	/// Sets mouse lock state.
	/// 
	/// @param[in] _lock           : Lock state to set.
	void inputSetMouseLock(bool _lock);

	/// Gets current mouse lock state.
	bool inputGetMouseLock();

	/// Retrieves state of the keyboard.
	/// 
	/// @param[in,out] _ks         : Keyboard state reference.
	void inputGetKeyboardState(KeyboardState& _ks);

	///	Retrieves state of a key.
	///
	/// @param[in] _key            : Key to get state for.
	/// @param[in,out] _modifiers  : Optional pointer to store modifiers to.
	bool inputGetKeyState(KeyboardKey::Enum _key, uint8_t* _modifiers = 0);

	/// Returns keyboard modifiers state.
	///
	/// @returns Returns keyboard modifiers state, as uint8.
	uint8_t inputGetModifiersState();

	/// Emits a key press.
	///
	/// @param[in] _key            : Key press to emit.
	/// @param[in] _modifiers      : Modifiers for the key.
	void inputEmitKeyPress(KeyboardKey::Enum _key, uint8_t _modifiers = 0);

	/// Draws game pad debugging information
	///
	/// @param[in] _maxGamepads    : Maximum number of game pads to draw info for.
	void inputDgbGamePads(int _maxGamepads = 4);

	/// Draws mouse debugging information
	void inputDgbMouse();

	/// Draws keyboard debugging information
	void inputDgbKeyboard();

	/// Draws touch pad debugging information
	void inputDgbTouch();

} // namespace rapp

#ifdef RAPP_WITH_BGFX
#include <bgfx/bgfx.h>
#include <imgui/imgui.h>
#endif

#define RAPP_CLASS(_appClass)																	\
	_appClass(const char* _name, const char* _description = 0)	: App(_name, _description) {}	\
	virtual ~_appClass() {}

#define RAPP_INSTANCE(_appClass)																\
	s_ ## _appClass ## _app

#define RAPP_REGISTER(_appClass, _name, _description)											\
	_appClass RAPP_INSTANCE(_appClass) (_name, _description);

#define RAPP_DBG_STRINGIZE(_x)		RAPP_DBG_STRINGIZE_(_x)
#define RAPP_DBG_STRINGIZE_(_x)		#_x
#define RAPP_DBG_FILE_LINE_LITERAL	"" __FILE__ "(" RAPP_DBG_STRINGIZE(__LINE__) "): "
#define RAPP_DBG(_format, ...)		rapp::dbgPrintf(RAPP_DBG_FILE_LINE_LITERAL "" _format "\n", ##__VA_ARGS__)

#endif // RTM_RAPP_H
