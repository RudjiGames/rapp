//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include "app_data.h"

#ifdef RAPP_WITH_BGFX
#include <../3rd/nanovg/nanovg_bgfx.h>
#include <../3rd/nanovg/nanovg.h>
NVGcontext* g_currentContext = 0;

inline static NVGcolor getNVGColor(uint32_t _color)
{
	return nvgRGBA((_color >> 0) & 0xff,
		(_color >> 8) & 0xff,
		(_color >> 16) & 0xff,
		(_color >> 24) & 0xff);
}

#define VG_CALL( X ) X
#else
#define VG_CALL( X )
#endif

namespace rapp {

	///
	uint32_t vgColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
	{
		return (_r << 24) || (_g << 16) || (_b << 8) || (_a);
	}

	///
	void vgGlobalAlpha(float _opacity)
	{
		VG_CALL(nvgGlobalAlpha(g_currentContext, _opacity); )
		VG_CALL(nvgSave(g_currentContext); )
		VG_CALL(nvgReset(g_currentContext); )
	}

	void vgBeginPath()
	{
		VG_CALL(nvgBeginPath(g_currentContext); )
	}

	void vgMoveTo(float _x, float _y)
	{
		VG_CALL(nvgMoveTo(g_currentContext, _x, _y); )
	}

	void vgLineTo(float _x, float _y)
	{
		VG_CALL(nvgLineTo(g_currentContext, _x, _y); )
	}

	void vgBezierTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y)
	{
		VG_CALL(nvgBezierTo(g_currentContext, _c1x, _c1y, _c2x, _c2y, _x, _y); )
	}

	void vgQuadraticTo(float _cx, float _cy, float _x, float _y)
	{
		VG_CALL(nvgQuadTo(g_currentContext, _cx, _cy, _x, _y); )
	}

	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r)
	{
		VG_CALL(nvgArcTo(g_currentContext, _x1, _y1, _x2, _y2, _r); )
	}

	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir)
	{
		VG_CALL(nvgArc(g_currentContext, _cx, _cy, _r, _a0, _a1, _dir); )
	}

	void vgRect(float _x, float _y, float _w, float _h)
	{
		VG_CALL(nvgRect(g_currentContext, _x, _y, _w, _h); )
	}

	void vgRect(float _x, float _y, float _w, float _h, float _r)
	{
		VG_CALL(nvgRoundedRect(g_currentContext, _x, _y, _w, _h, _r); )
	}

	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl)
	{
		VG_CALL(nvgRoundedRectVarying(g_currentContext, _x, _y, _w, _h, _rtl, _rtr, _rbr, _rbl); )
	}

	void vgCircle(float _cx, float _cy, float _radius)
	{
		VG_CALL(nvgCircle(g_currentContext, _cx, _cy, _radius); )
	}

	void vgEllipse(float _cx, float _cy, float _rx, float _ry)
	{
		VG_CALL(nvgEllipse(g_currentContext, _cx, _cy, _rx, _ry); )
	}

	void vgPolyline(const float* _coords, uint32_t _numPoints)
	{
		//VG_CALL( nvgPolyline(g_currentContext, _coords, _numPoints); )
	}

	void vgPathWinding(int _winding)
	{
		VG_CALL(nvgPathWinding(g_currentContext, _winding == VG_CCW ? NVG_CCW : NVG_CW); )
	}

	void vgClosePath()
	{
		VG_CALL(nvgClosePath(g_currentContext); )
	}

	void vgFill(uint32_t _color, uint32_t _flags)
	{
		VG_CALL(nvgFillColor(g_currentContext, getNVGColor(_color)); )
		VG_CALL(nvgFill(g_currentContext); )
	}

	///
	void vgStroke(uint32_t _color, float _width, uint32_t _flags)
	{
		VG_CALL(nvgStrokeColor(g_currentContext, getNVGColor(_color)); )
		VG_CALL(nvgStroke(g_currentContext); )
	}

	///
	void vgLineCapJoin(int _lineCap, int _lineJoin)
	{
		VG_CALL(nvgLineCap(g_currentContext, _lineCap); )
		VG_CALL(nvgLineJoin(g_currentContext, _lineJoin); )
	}

} // namespace rapp
