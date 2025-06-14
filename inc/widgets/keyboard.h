//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_KEYBOARD_H
#define RTM_RAPP_KEYBOARD_H

#include <stdint.h>

#ifdef RAPP_WITH_BGFX

#include <rapp/inc/rapp.h>
#include <rapp/inc/widgets/widget_theme.h>

namespace rapp {

	constexpr float RAPP_VK_BUTTON_SIZE_X	= 39.0f;
	constexpr float RAPP_VK_BUTTON_SIZE_Y	= 30.0f;
	constexpr float RAPP_VK_BUTTON_SPACING	= 3.0f;

	static void rappVirtualKeyboardClick(KeyboardKey _key)
	{
		ImGui::SetWindowFocus("Console");
		inputEmitKeyPress(_key);
//		ImGui::SetKeyboardFocusHere(-1); // TODO: kbd should have no focus
	}

	static void rappVirtualKeyboardLine(const char* key, KeyboardKey* _keyMap, float _scale)
    {
        size_t num = rtm::strLen(key);
        for (size_t i=0; i<num; i++)
        {
			char key_label[] = "X";
			key_label[0] = *key;
			if (ImGui::Button(key_label, ImVec2(RAPP_VK_BUTTON_SIZE_X * _scale, RAPP_VK_BUTTON_SIZE_Y * _scale)))
				rappVirtualKeyboardClick(_keyMap[i]);
			key++;
			if (i<num-1)
				ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
        }
    };

	static void rappVirtualKeyboard(float _scale = 1.0f)
	{
		ImGui::Begin("Virtual Keyboard", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		KeyboardKey keys1[] = {
			KeyboardState::Tilde, KeyboardKey::Key1, KeyboardKey::Key2, KeyboardKey::Key3, KeyboardKey::Key4, KeyboardKey::Key5, KeyboardKey::Key6,
			KeyboardKey::Key7, KeyboardKey::Key8, KeyboardKey::Key9, KeyboardKey::Key0, KeyboardState::Minus, KeyboardState::Plus };

		rappVirtualKeyboardLine("~1234567890-+", keys1, _scale);
		ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
		if (ImGui::ArrowButton("<-", ImGuiDir_Left))
			rappVirtualKeyboardClick(KeyboardState::Backspace);

		KeyboardKey keys2[] = {
			KeyboardKey::KeyQ, KeyboardKey::KeyW, KeyboardKey::KeyE, KeyboardKey::KeyR, KeyboardKey::KeyT, KeyboardKey::KeyY, KeyboardKey::KeyU,
			KeyboardKey::KeyI, KeyboardKey::KeyO, KeyboardKey::KeyP, KeyboardState::LeftBracket, KeyboardState::RightBracket, KeyboardState::Backslash };

		ImGui::Text("   ");
		ImGui::SameLine();
		rappVirtualKeyboardLine("QWERTYUIOP[]\\", keys2, _scale);

		KeyboardKey keys3[] = {
			KeyboardKey::KeyA, KeyboardKey::KeyS, KeyboardKey::KeyD, KeyboardKey::KeyF, KeyboardKey::KeyG, KeyboardKey::KeyH,
			KeyboardKey::KeyJ, KeyboardKey::KeyK, KeyboardKey::KeyL, KeyboardState::Semicolon, KeyboardState::Quote };

		ImGui::Text("      ");
		ImGui::SameLine();
		rappVirtualKeyboardLine("ASDFGHJKL;'", keys3, _scale);
		ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
		if (ImGui::Button("Return", ImVec2(RAPP_VK_BUTTON_SIZE_X * 1.85f * _scale, RAPP_VK_BUTTON_SIZE_Y * _scale)))
			rappVirtualKeyboardClick(KeyboardState::Return);

		KeyboardKey keys4[] = {
			KeyboardKey::KeyZ, KeyboardKey::KeyX, KeyboardKey::KeyC, KeyboardKey::KeyV, KeyboardKey::KeyB, KeyboardKey::KeyN, KeyboardKey::KeyM,
			KeyboardState::Period, KeyboardState::Comma, KeyboardState::Slash };

		ImGui::Text("         ");
		ImGui::SameLine();
		rappVirtualKeyboardLine("ZXCVBNM,.?", keys4, _scale);
		ImGui::Text("                                          ");
		ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
		if (ImGui::Button("---------", ImVec2(RAPP_VK_BUTTON_SIZE_X * 6.0f * _scale, RAPP_VK_BUTTON_SIZE_Y * _scale)))
			rappVirtualKeyboardClick(KeyboardState::Space);

		ImGui::End();
	}

} // namespace rapp

#endif // RAPP_WITH_BGFX

#endif // RTM_RAPP_KEYBOARD_H
