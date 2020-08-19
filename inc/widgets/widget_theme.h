//--------------------------------------------------------------------------//
/// Copyright (c) 2010-2016 Milos Tosic. All Rights Reserved.              ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_IMGUI_COLORS_H
#define RTM_RAPP_IMGUI_COLORS_H

#include <stdint.h>
#include <rbase/inc/cpu.h>
#include <imgui/imgui.h>
	
#define RAPP_COLOR_WIDGET_OUTLINE				IM_COL32( 96, 96,128,255)
#define RAPP_COLOR_WIDGET						IM_COL32( 32, 32, 48,255)
#define RAPP_COLOR_WIDGET_HIGHLIGHT				IM_COL32(128,128,160,255)
#define RAPP_COLOR_WIDGET_BLINK					IM_COL32( 24, 24, 32,255)
#define RAPP_COLOR_TEXT							IM_COL32(255,255,255,255)
#define RAPP_COLOR_TEXT_HIGHLIGHT				IM_COL32(128,255,255,255)
#define RAPP_COLOR_TEXT_TITLE					IM_COL32(255,255,155,255)
#define RAPP_COLOR_TEXT_SHADOW					IM_COL32(  0,  0,  0,255)
#define RAPP_COLOR_TEXT_INACTIVE				IM_COL32(192,192,192,255)
#define RAPP_COLOR_TEXT_STATUS					IM_COL32(192, 32, 32,255)

#define RAPP_WIDGET_SPACING						6
#define RAPP_WIDGET_OUTLINE						3

#define RAPP_FLASH_TIME_IN_MS					240.0f

namespace rapp {

template <typename T>
static inline T rappMax(T _v1, T _v2) { return _v1 > _v2 ? _v1 : _v2; }
template <typename T>
static inline T rappMin(T _v1, T _v2) { return _v1 < _v2 ? _v1 : _v2; } 

static void flashColor(ImU32& _drawColor, ImU32 _target, uint64_t _elapsedTime)
{
	ImVec4 target = ImColor(_target);

	float msSince = rtm::CPU::time(_elapsedTime, rtm::CPU::frequency());
	msSince = rappMin(msSince * 1000.0f, RAPP_FLASH_TIME_IN_MS);
	msSince = 1.0f - (msSince / RAPP_FLASH_TIME_IN_MS);
	
	ImVec4 col4 = ImColor(_drawColor);
	_drawColor = ImColor(	col4.x + (target.x - col4.x) * msSince,
							col4.y + (target.y - col4.y) * msSince,
							col4.z + (target.z - col4.z) * msSince,
							255.0f);
}

} // namespace rapp

#endif // RTM_RAPP_IMGUI_COLORS_H
