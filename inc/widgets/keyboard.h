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

	constexpr float RAPP_VK_BUTTON_SIZE_X	= 39.0f;
	constexpr float RAPP_VK_BUTTON_SIZE_Y	= 30.0f;
	constexpr float RAPP_VK_BUTTON_SPACING	= 3.0f;

	static void rappVirtualKeyboardClick(KeyboardState::Key _key)
	{
		ImGui::SetWindowFocus("Console");
		inputEmitKeyPress(_key);
//		ImGui::SetKeyboardFocusHere(-1); // TODO: kbd should have no focus
	}

	static void rappVirtualKeyboardLine(const char* key, KeyboardState::Key* _keyMap, float _scale)
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

		KeyboardState::Key keys1[] = {
			KeyboardState::Tilde, KeyboardState::Key1, KeyboardState::Key2, KeyboardState::Key3, KeyboardState::Key4, KeyboardState::Key5, KeyboardState::Key6,
			KeyboardState::Key7, KeyboardState::Key8, KeyboardState::Key9, KeyboardState::Key0, KeyboardState::Minus, KeyboardState::Plus };

		rappVirtualKeyboardLine("~1234567890-+", keys1, _scale);
		ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
		if (ImGui::ArrowButton("<-", ImGuiDir_Left))
			rappVirtualKeyboardClick(KeyboardState::Backspace);

		KeyboardState::Key keys2[] = {
			KeyboardState::KeyQ, KeyboardState::KeyW, KeyboardState::KeyE, KeyboardState::KeyR, KeyboardState::KeyT, KeyboardState::KeyY, KeyboardState::KeyU,
			KeyboardState::KeyI, KeyboardState::KeyO, KeyboardState::KeyP, KeyboardState::LeftBracket, KeyboardState::RightBracket, KeyboardState::Backslash };

		ImGui::Text("   ");
		ImGui::SameLine();
		rappVirtualKeyboardLine("QWERTYUIOP[]\\", keys2, _scale);

		KeyboardState::Key keys3[] = {
			KeyboardState::KeyA, KeyboardState::KeyS, KeyboardState::KeyD, KeyboardState::KeyF, KeyboardState::KeyG, KeyboardState::KeyH,
			KeyboardState::KeyJ, KeyboardState::KeyK, KeyboardState::KeyL, KeyboardState::Semicolon, KeyboardState::Quote };

		ImGui::Text("      ");
		ImGui::SameLine();
		rappVirtualKeyboardLine("ASDFGHJKL;'", keys3, _scale);
		ImGui::SameLine(0.0f, RAPP_VK_BUTTON_SPACING * _scale);
		if (ImGui::Button("Return", ImVec2(RAPP_VK_BUTTON_SIZE_X * 1.85f * _scale, RAPP_VK_BUTTON_SIZE_Y * _scale)))
			rappVirtualKeyboardClick(KeyboardState::Return);

		KeyboardState::Key keys4[] = {
			KeyboardState::KeyZ, KeyboardState::KeyX, KeyboardState::KeyC, KeyboardState::KeyV, KeyboardState::KeyB, KeyboardState::KeyN, KeyboardState::KeyM,
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
