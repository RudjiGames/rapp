//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

namespace rapp {

	///
	void uiSetStyle(int _style, float _alpha)
	{
		RTM_UNUSED(_style);

#ifdef RAPP_WITH_BGFX
		ImGuiStyle& style = ImGui::GetStyle();

		switch (_style)
		{
		case RAPP_STYLE_LIGHT:
		case RAPP_STYLE_DARK:
			{
				if (_style == RAPP_STYLE_LIGHT)
				{
					ImGui::StyleColorsLight(&style);
				}
				else
				{
					ImGui::StyleColorsDark(&style);
				}

				style.Alpha			= 1.0f;
				style.FrameRounding = 3.0f;
			}
			break;

		default:
		case RAPP_STYLE_RUDJI:
			{
				style.Alpha			= 1.0f;
				style.FrameRounding = 3.0f;

				//   8 = 0.031f
				//  13 = 0.050f
				//  21 = 0.082f
				//  34 = 0.133f
				//  55 = 0.215f
				//  89 = 0.349f
				// 144 = 0.564f
				// 233 = 0.913f

				#define RUDJI_COL(_C)				ImVec4(_C, 1.0f)
				#define RUDJI_ALPHA(_C, _A)			ImVec4(_C.x, _C.y, _C.z, _A)

				#define RUDJI_COL_WHITE				ImVec4(1.000f, 1.000f, 1.000f, 1.00f)
				#define RUDJI_COL_BLACK				ImVec4(0.000f, 0.000f, 0.000f, 1.00f)
				#define RUDJI_COL_PURPLE			ImVec4(0.082f, 0.082f, 0.133f, 1.00f)
				#define RUDJI_COL_PURPLE_DARK		ImVec4(0.050f, 0.050f, 0.082f, 1.00f)
				#define RUDJI_COL_PURPLE_LIGHT		ImVec4(0.349f, 0.349f, 0.564f, 1.00f)
				#define RUDJI_COL_BLUE_LIGHT		ImVec4(0.215f, 0.215f, 0.564f, 1.00f)
				#define RUDJI_COL_GREEN				ImVec4(0.133f, 0.564f, 0.349f, 1.00f)
				#define RUDJI_COL_GREEN_DARK		ImVec4(0.133f, 0.349f, 0.215f, 1.00f)
				#define RUDJI_COL_RED				ImVec4(0.564f, 0.215f, 0.215f, 1.00f)
				#define RUDJI_COL_RED_DARK			ImVec4(0.349f, 0.082f, 0.082f, 1.00f)
				#define RUDJI_COL_CYAN_DARK			ImVec4(0.133f, 0.349f, 0.349f, .60f)
				#define RUDJI_COL_CYAN				ImVec4(0.215f, 0.564f, 0.564f, 1.00f)
				#define RUDJI_COL_CYAN_LIGHT		ImVec4(0.349f, 0.913f, 0.913f, 1.00f)
				#define RUDJI_COL_PINK				ImVec4(0.564f, 0.349f, 0.564f, 1.00f)

				ImGui::StyleColorsDark(&style);

				style.Colors[ImGuiCol_Text]						= RUDJI_COL_WHITE;
				style.Colors[ImGuiCol_TextDisabled]				= RUDJI_ALPHA(RUDJI_COL_WHITE,  0.6f);
				style.Colors[ImGuiCol_WindowBg]					= RUDJI_ALPHA(RUDJI_COL_PURPLE, 0.93f);
				style.Colors[ImGuiCol_ChildBg]					= RUDJI_COL_PURPLE_DARK;
				style.Colors[ImGuiCol_PopupBg]					= RUDJI_COL_PURPLE;
				style.Colors[ImGuiCol_Border]					= RUDJI_ALPHA(RUDJI_COL_PURPLE_LIGHT, 0.36f);
				style.Colors[ImGuiCol_BorderShadow]				= RUDJI_ALPHA(RUDJI_COL_PURPLE_LIGHT, 0.36f);
				style.Colors[ImGuiCol_FrameBg]					= RUDJI_COL_BLACK;
				style.Colors[ImGuiCol_FrameBgHovered]			= RUDJI_COL_PURPLE_DARK;
				style.Colors[ImGuiCol_FrameBgActive]			= RUDJI_COL_PURPLE;
				style.Colors[ImGuiCol_TitleBg]					= RUDJI_COL_PURPLE_DARK;
				style.Colors[ImGuiCol_TitleBgActive]			= RUDJI_COL_BLACK;
				style.Colors[ImGuiCol_TitleBgCollapsed]			= RUDJI_COL_PURPLE;
				style.Colors[ImGuiCol_MenuBarBg]				= RUDJI_COL_PURPLE_DARK;
				style.Colors[ImGuiCol_ScrollbarBg]				= RUDJI_COL_BLACK;
				style.Colors[ImGuiCol_ScrollbarGrab]			= RUDJI_COL_PURPLE;
				style.Colors[ImGuiCol_ScrollbarGrabHovered]		= RUDJI_COL_CYAN_DARK;
				style.Colors[ImGuiCol_ScrollbarGrabActive]		= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_CheckMark]				= RUDJI_COL_GREEN;
				style.Colors[ImGuiCol_SliderGrab]				= RUDJI_COL_CYAN_DARK;
				style.Colors[ImGuiCol_SliderGrabActive]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_Button]					= RUDJI_COL_CYAN_DARK;
				style.Colors[ImGuiCol_ButtonHovered]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_ButtonActive]				= RUDJI_COL_CYAN_LIGHT;
				style.Colors[ImGuiCol_Header]					= RUDJI_ALPHA(RUDJI_COL_CYAN_DARK, 0.5f);
				style.Colors[ImGuiCol_HeaderHovered]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_HeaderActive]				= RUDJI_COL_CYAN_LIGHT;
				style.Colors[ImGuiCol_Separator]				= RUDJI_COL_CYAN_DARK;
				style.Colors[ImGuiCol_SeparatorHovered]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_SeparatorActive]			= RUDJI_COL_CYAN_LIGHT;
				style.Colors[ImGuiCol_ResizeGrip]				= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_ResizeGripHovered]		= RUDJI_COL_GREEN_DARK;
				style.Colors[ImGuiCol_ResizeGripActive]			= RUDJI_COL_GREEN;
				style.Colors[ImGuiCol_InputTextCursor]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_TabHovered]				= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_Tab]						= RUDJI_COL_BLACK;
				style.Colors[ImGuiCol_TabSelected]				= RUDJI_COL_CYAN_DARK;
	//			style.Colors[ImGuiCol_TabSelectedOverline]		= ;
	//			style.Colors[ImGuiCol_TabDimmed]				= ;
	//			style.Colors[ImGuiCol_TabDimmedSelected]		= ;
	//			style.Colors[ImGuiCol_TabDimmedSelectedOverline]= ;
	//			style.Colors[ImGuiCol_DockingPreview]			= ;
	//			style.Colors[ImGuiCol_DockingEmptyBg]			= ;
				style.Colors[ImGuiCol_PlotLines]				= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_PlotLinesHovered]			= RUDJI_COL_CYAN_LIGHT;
				style.Colors[ImGuiCol_PlotHistogram]			= RUDJI_COL_CYAN;
				style.Colors[ImGuiCol_PlotHistogramHovered]		= RUDJI_COL_CYAN_LIGHT;
	//			style.Colors[ImGuiCol_TableHeaderBg]			= ;
	//			style.Colors[ImGuiCol_TableBorderStrong]		= ;
	//			style.Colors[ImGuiCol_TableBorderLight]			= ;
	//			style.Colors[ImGuiCol_TableRowBg]				= ;
	//			style.Colors[ImGuiCol_TableRowBgAlt]			= ;
				style.Colors[ImGuiCol_TextLink]					= RUDJI_COL_CYAN_LIGHT;
				style.Colors[ImGuiCol_TextSelectedBg]			= RUDJI_COL_CYAN_DARK;
	//			style.Colors[ImGuiCol_TreeLines]				= ;
	//			style.Colors[ImGuiCol_DragDropTarget]			= ;
	//			style.Colors[ImGuiCol_NavCursor]				= ;
	//			style.Colors[ImGuiCol_NavWindowingHighlight]	= ;
	//			style.Colors[ImGuiCol_NavWindowingDimBg]		= ;
	//			style.Colors[ImGuiCol_ModalWindowDimBg]			= ;
			}
			break;
		};

		for (int i=0; i<ImGuiCol_COUNT; i++)
		{
			ImVec4& col = style.Colors[i];
			if (col.w < 1.00f)
			{
				col.w *= _alpha;
			}
		}
#endif
	}

	/// 
	void uiSetScale(float _scale)
	{
#ifdef RAPP_WITH_BGFX
		ImGuiStyle& style = ImGui::GetStyle();
		float sc = ImGui::GetWindowDpiScale();
		sc = 1.0f;
#endif // RAPP_WITH_BGFX
	}

} // namespace rapp
