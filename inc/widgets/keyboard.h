//--------------------------------------------------------------------------//
/// Copyright (c) 2010-2016 Milos Tosic. All Rights Reserved.              ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_KEYBOARD_H
#define RTM_RAPP_KEYBOARD_H

#include <stdint.h>

#if RAPP_WITH_BGFX

#include <imgui/imgui.h>
#include <rapp/inc/widgets/widget_theme.h>

namespace rapp {

	static void rappKeyboardLine(const char* key)
    {
        size_t num = rtm::strLen(key);
        for (size_t i=0; i<num; i++)
        {
                char key_label[] = "X";
                key_label[0] = *key;
                if (ImGui::Button(key_label,ImVec2(46,32)))
                {
					// add key
                }
                key++;
                if (i<num-1)
                ImGui::SameLine();
        }
    };

	static int rappDrawKeyboard(uint32_t keys[8])
	{
		ImGui::Begin("Virtual Keyboard");

		rappKeyboardLine("~1234567890-+");
		ImGui::SameLine();
		if (ImGui::ArrowButton("<-", ImGuiDir_Left))
        {
             // add key
        }
		ImGui::Text("   ");
		ImGui::SameLine();
		rappKeyboardLine("qwertyuiop[]\\");

		ImGui::Text("      ");
		ImGui::SameLine();
		rappKeyboardLine("asdfghjkl;'");
		ImGui::SameLine();
		if (ImGui::Button("Return", ImVec2(92, 32)))
        {
             // add key
        }

		ImGui::Text("         ");
		ImGui::SameLine();
		rappKeyboardLine("zxcvbnm,.?");
		ImGui::Text("                                    ");ImGui::SameLine();
		if (ImGui::Button("---------", ImVec2(230, 32)))
        {
             // add key
        }

		ImGui::End();
		return 0;
	}

} // namespace rapp

#endif // RAPP_WITH_BGFX

#endif // RTM_RAPP_KEYBOARD_H
