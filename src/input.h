//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_INPUT_H
#define RTM_RAPP_INPUT_H

#include <rapp/inc/rapp.h>
#include <rapp/src/rapp_config.h>

namespace rapp {

	const char* getName(KeyboardState::Key _key);

	struct GamepadHandle { uint16_t idx; };
	inline bool isValid(GamepadHandle _handle) { return UINT16_MAX != _handle.idx; }

	struct GamepadAxis
	{
		enum Enum
		{
			LeftX,
			LeftY,
			LeftZ,
			RightX,
			RightY,
			RightZ,

			Count
		};
	};

	///
	void inputInit();

	///
	void inputShutdown();

	///
	bool inputProcess(App* _app);

	///
	void inputSetKeyState(KeyboardState::Key _key, uint8_t _modifiers, bool _down);

	///
	uint8_t inputGetModifiersState();

	/// Adds single UTF-8 encoded character into input buffer.
	void inputChar(uint8_t _len, const uint8_t _char[4]);

	/// Returns single UTF-8 encoded character from input buffer.
	void inputGetChar(uint8_t _char[4]);

	/// Flush internal input buffer.
	void inputCharFlush();

	///
	void inputSetMouseResolution(uint16_t _width, uint16_t _height);

	///
	void inputSetMousePos(int32_t _mx, int32_t _my, int32_t _mz);

	///
	void inputResetMouseMovement();

	///
	void inputResetGamepadAxisMovement();

	///
	void inputSetMouseButtonState(MouseState::Button _button, uint8_t _state);

	///
	void inputGetMouse(float _mouse[3]);

	///
	bool inputIsMouseLocked();

	///
	void inputSetGamepadConnected(GamepadHandle _handle, bool _connected);

	///
	void inputSetGamepadButtonState(GamepadHandle _handle, GamepadState::Buttons _button, bool _pressed);

	///
	void inputSetGamepadAxis(GamepadHandle _handle, GamepadAxis::Enum _axis, int32_t _value);

	///
	int32_t inputGetGamepadAxis(GamepadHandle _handle, GamepadAxis::Enum _axis);

} // namespace rapp

#endif // RTM_RAPP_INPUT_H
