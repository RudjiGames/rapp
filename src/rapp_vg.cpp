//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include "app_data.h"

#ifdef RAPP_WITH_BGFX
NVGcontext* g_currentContext = 0;

inline static NVGcolor toNVGColor(uint32_t _color)
{
	return nvgRGBA( (_color >>  0) & 0xff,
					(_color >>  8) & 0xff,
					(_color >> 16) & 0xff,
					(_color >> 24) & 0xff);
}

#define VG_CALL( X ) X
#else
#define VG_CALL( X )
#endif

namespace rapp {

	static float gs_opacity = 1.0f;
	
	void vgGlobalAlpha(float _opacity)
	{
		RTM_ASSERT((_opacity >= 0.0f) && (_opacity <= 1.0f), "Invalid opacity value!");
		gs_opacity = _opacity;
	}

	uint32_t vgColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
	{
		return  (uint32_t(_r) <<  0) |
				(uint32_t(_g) <<  8) |
				(uint32_t(_b) << 16) |
				(uint32_t(_a) << 24);
	}

	void vgBeginPath()
	{
		VG_CALL( nvgBeginPath(g_currentContext); )
	}

	void vgMoveTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL( nvgMoveTo(g_currentContext, _x, _y); )
	}

	void vgLineTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL( nvgLineTo(g_currentContext, _x, _y); )
	}

	void vgCubicTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y)
	{
		RTM_UNUSED_6(_c1x, _c1y, _c2x, _c2y, _x, _y);
		VG_CALL( nvgBezierTo(g_currentContext, _c1x, _c1y, _c2x, _c2y, _x, _y); )
	}

	void vgQuadraticTo(float _cx, float _cy, float _x, float _y)
	{
		RTM_UNUSED_4(_cx, _cy, _x, _y);
		VG_CALL( nvgQuadTo(g_currentContext, _cx, _cy, _x, _y); )
	}

	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r)
	{
		RTM_UNUSED_5(_x1, _y1, _x2, _y2, _r);
		VG_CALL( nvgArcTo(g_currentContext, _x1, _y1, _x2, _y2, _r); )
	}

	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir)
	{
		RTM_UNUSED_6(_cx, _cy, _r, _a0, _a1, _dir);
		VG_CALL( nvgArc(g_currentContext, _cx, _cy, _r, _a0, _a1, _dir); )
	}

	void vgRect(float _x, float _y, float _w, float _h)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		VG_CALL( nvgRect(g_currentContext, _x, _y, _w, _h); )
	}

	void vgRect(float _x, float _y, float _w, float _h, float _r)
	{
		RTM_UNUSED_5(_x, _y, _w, _h, _r);
		VG_CALL( nvgRoundedRect(g_currentContext, _x, _y, _w, _h, _r); )
	}

	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_rtl, _rtr, _rbr, _rbl);
		VG_CALL( nvgRoundedRectVarying(g_currentContext, _x, _y, _w, _h, _rtl, _rtr, _rbr, _rbl); )
	}

	void vgCircle(float _cx, float _cy, float _radius)
	{
		RTM_UNUSED_3(_cx, _cy, _radius);
		VG_CALL( nvgCircle(g_currentContext, _cx, _cy, _radius); )
	}

	void vgEllipse(float _cx, float _cy, float _rx, float _ry)
	{
		RTM_UNUSED_4(_cx, _cy, _rx, _ry);
		VG_CALL( nvgEllipse(g_currentContext, _cx, _cy, _rx, _ry); )
	}

	void vgPolyline(const float* _coords, uint32_t _numPoints)
	{
		RTM_UNUSED_2(_coords, _numPoints);
		VG_CALL( for (uint32_t i=0; i<_numPoints; ++i) )
		VG_CALL(	nvgLineTo(g_currentContext, _coords[i*2+0], _coords[i*2+1]); )
	}

	void vgPathWinding(int _winding)
	{
		RTM_UNUSED(_winding);
		VG_CALL( nvgPathWinding(g_currentContext, _winding == VG_CCW ? NVG_CCW : NVG_CW); )
	}

	void vgClosePath()
	{
		VG_CALL( nvgClosePath(g_currentContext); )
	}

	void vgFill(uint32_t _color, uint32_t _flags)
	{
		RTM_UNUSED_2(_color, _flags);
		VG_CALL( nvgFillColor(g_currentContext, toNVGColor(_color)); )
		VG_CALL( nvgFill(g_currentContext); )
	}

	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL( auto pnt = nvgLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL( nvgFillPaint(g_currentContext, pnt); )
		VG_CALL( nvgFill(g_currentContext); )
	}

	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_r, _f, _icol, _ocol);
		VG_CALL(auto pnt = nvgBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL(nvgFillPaint(g_currentContext, pnt); )
		VG_CALL(nvgFill(g_currentContext); )
	}

	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_cx, _cy, _inr, _outr, _icol, _ocol);
		VG_CALL(auto pnt = nvgRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL(nvgFillPaint(g_currentContext, pnt); )
		VG_CALL(nvgFill(g_currentContext); )
	}

	void vgStroke(uint32_t _color, float _width, uint32_t _flags)
	{
		RTM_UNUSED_3(_color, _width, _flags);
		VG_CALL( nvgStrokeColor(g_currentContext, toNVGColor(_color)); )
		VG_CALL( nvgStroke(g_currentContext); )
	}

	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL(auto pnt = nvgLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL(nvgStrokePaint(g_currentContext, pnt); )
		VG_CALL(nvgStroke(g_currentContext); )
	}

	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_r, _f, _icol, _ocol);
		VG_CALL(auto pnt = nvgBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL(nvgStrokePaint(g_currentContext, pnt); )
		VG_CALL(nvgStroke(g_currentContext); )
	}

	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_cx, _cy, _inr, _outr, _icol, _ocol);
		VG_CALL(auto pnt = nvgRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, toNVGColor(_icol), toNVGColor(_ocol)); )
		VG_CALL(nvgStrokePaint(g_currentContext, pnt); )
		VG_CALL(nvgStroke(g_currentContext); )
	}

} // namespace rapp
