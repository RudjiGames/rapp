//--------------------------------------------------------------------------//
/// Copyright (c) 2010-2016 Milos Tosic. All Rights Reserved.              ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_KEYBOARD_H
#define RTM_RAPP_KEYBOARD_H

#include <stdint.h>

#if RAPP_WITH_BGFX

#include <rapp/inc/rapp.h>
#include <rapp/inc/widgets/widget_theme.h>

namespace rapp {

	constexpr float BUTTON_SIZE_X	= 39.0f;
	constexpr float BUTTON_SIZE_Y	= 30.0f;
	constexpr float BUTTON_SPACING	= 3.0f;

	static void rappKeyboardLine(const char* key, KeyboardState::Key* _keyMap, float _scale)
    {
        size_t num = rtm::strLen(key);
        for (size_t i=0; i<num; i++)
        {
			char key_label[] = "X";
			key_label[0] = *key;
			if (ImGui::Button(key_label, ImVec2(BUTTON_SIZE_X * _scale, BUTTON_SIZE_Y * _scale)))
				inputEmitKeyPress(_keyMap[i]);
			key++;
			if (i<num-1)
				ImGui::SameLine(0.0f, BUTTON_SPACING * _scale);
        }
    };

	static int rappVirtualKeyboard()
	{
		ImGui::Begin("Virtual Keyboard", 0, ImGuiWindowFlags_NoResize);

		float scale = 1.0f;

		KeyboardState::Key keys1[] = {
			KeyboardState::Tilde, KeyboardState::Key1, KeyboardState::Key2, KeyboardState::Key3, KeyboardState::Key4, KeyboardState::Key5, KeyboardState::Key6,
			KeyboardState::Key7, KeyboardState::Key8, KeyboardState::Key9, KeyboardState::Key0, KeyboardState::Minus, KeyboardState::Plus };

		rappKeyboardLine("~1234567890-+", keys1, scale);
		ImGui::SameLine(0.0f, BUTTON_SPACING * scale);
		if (ImGui::ArrowButton("<-", ImGuiDir_Left))
			inputEmitKeyPress(KeyboardState::Backspace);

		KeyboardState::Key keys2[] = {
			KeyboardState::KeyQ, KeyboardState::KeyW, KeyboardState::KeyE, KeyboardState::KeyR, KeyboardState::KeyT, KeyboardState::KeyY, KeyboardState::KeyU,
			KeyboardState::KeyI, KeyboardState::KeyO, KeyboardState::KeyP, KeyboardState::LeftBracket, KeyboardState::RightBracket, KeyboardState::Backslash };

		ImGui::Text("   ");
		ImGui::SameLine();
		rappKeyboardLine("qwertyuiop[]\\", keys2, scale);

		KeyboardState::Key keys3[] = {
			KeyboardState::KeyA, KeyboardState::KeyS, KeyboardState::KeyD, KeyboardState::KeyF, KeyboardState::KeyG, KeyboardState::KeyH,
			KeyboardState::KeyJ, KeyboardState::KeyK, KeyboardState::KeyL, KeyboardState::Semicolon, KeyboardState::Quote };

		ImGui::Text("      ");
		ImGui::SameLine();
		rappKeyboardLine("asdfghjkl;'", keys3, scale);
		ImGui::SameLine(0.0f, BUTTON_SPACING * scale);
		if (ImGui::Button("Return", ImVec2(BUTTON_SIZE_X * 1.85f * scale, BUTTON_SIZE_Y * scale)))
			inputEmitKeyPress(KeyboardState::Return);

		KeyboardState::Key keys4[] = {
			KeyboardState::KeyZ, KeyboardState::KeyX, KeyboardState::KeyC, KeyboardState::KeyV, KeyboardState::KeyB, KeyboardState::KeyN, KeyboardState::KeyM,
			KeyboardState::Period, KeyboardState::Comma, KeyboardState::Slash };

		ImGui::Text("         ");
		ImGui::SameLine();
		rappKeyboardLine("zxcvbnm,.?", keys4, scale);
		ImGui::Text("                                          ");
		ImGui::SameLine(0.0f, BUTTON_SPACING * scale);
		if (ImGui::Button("---------", ImVec2(BUTTON_SIZE_X * 6.0f * scale, BUTTON_SIZE_Y * scale)))
			inputEmitKeyPress(KeyboardState::Space);

		ImGui::End();
		return 0;
	}

} // namespace rapp

#endif // RAPP_WITH_BGFX

#endif // RTM_RAPP_KEYBOARD_H
