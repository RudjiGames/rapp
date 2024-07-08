//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include "app_data.h"

#ifdef RAPP_WITH_BGFX
vg::Context* g_currentContext = 0;

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
		VG_CALL( vg::beginPath(g_currentContext); )
	}

	void vgMoveTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL( vg::moveTo(g_currentContext, _x, _y); )
	}

	void vgLineTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL( vg::lineTo(g_currentContext, _x, _y); )
	}

	void vgCubicTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y)
	{
		RTM_UNUSED_6(_c1x, _c1y, _c2x, _c2y, _x, _y);
		VG_CALL( vg::cubicTo(g_currentContext, _c1x, _c1y, _c2x, _c2y, _x, _y); )
	}

	void vgQuadraticTo(float _cx, float _cy, float _x, float _y)
	{
		RTM_UNUSED_4(_cx, _cy, _x, _y);
		VG_CALL( vg::quadraticTo(g_currentContext, _cx, _cy, _x, _y); )
	}

	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r)
	{
		RTM_UNUSED_5(_x1, _y1, _x2, _y2, _r);
		VG_CALL( vg::arcTo(g_currentContext, _x1, _y1, _x2, _y2, _r); )
	}

	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir)
	{
		RTM_UNUSED_6(_cx, _cy, _r, _a0, _a1, _dir);
		VG_CALL( vg::arc(g_currentContext, _cx, _cy, _r, _a0, _a1, (vg::Winding::Enum)_dir); )
	}

	void vgRect(float _x, float _y, float _w, float _h)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		VG_CALL( vg::rect(g_currentContext, _x, _y, _w, _h); )
	}

	void vgRect(float _x, float _y, float _w, float _h, float _r)
	{
		RTM_UNUSED_5(_x, _y, _w, _h, _r);
		VG_CALL( vg::roundedRect(g_currentContext, _x, _y, _w, _h, _r); )
	}

	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_rtl, _rtr, _rbr, _rbl);
		VG_CALL( vg::roundedRectVarying(g_currentContext, _x, _y, _w, _h, _rtl, _rtr, _rbr, _rbl); )
	}

	void vgCircle(float _cx, float _cy, float _radius)
	{
		RTM_UNUSED_3(_cx, _cy, _radius);
		VG_CALL( vg::circle(g_currentContext, _cx, _cy, _radius); )
	}

	void vgEllipse(float _cx, float _cy, float _rx, float _ry)
	{
		RTM_UNUSED_4(_cx, _cy, _rx, _ry);
		VG_CALL( vg::ellipse(g_currentContext, _cx, _cy, _rx, _ry); )
	}

	void vgPolyline(const float* _coords, uint32_t _numPoints)
	{
		RTM_UNUSED_2(_coords, _numPoints);
		VG_CALL(vg::polyline(g_currentContext, _coords, _numPoints));
	}

	void vgPathWinding(int _winding)
	{
		RTM_UNUSED(_winding);
		//VG_CALL( vg::SetPathWinding(g_currentContext, _winding == VG_CCW ? NVG_CCW : NVG_CW); )
	}

	void vgClosePath()
	{
		VG_CALL( vg::closePath(g_currentContext); )
	}

	void vgFill(uint32_t _color, uint32_t _flags)
	{
		RTM_UNUSED_2(_color, _flags);
		VG_CALL( vg::fillPath(g_currentContext, _color, _flags); )
	}

	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL( auto pnt = vg::createLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, _icol, _ocol); )
		VG_CALL( vg::fillPath(g_currentContext, pnt, 0); )
	}

	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_r, _f, _icol, _ocol);
		VG_CALL(auto pnt = vg::createBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, _icol, _ocol); )
		VG_CALL(vg::fillPath(g_currentContext, pnt, 0); )
	}

	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_cx, _cy, _inr, _outr, _icol, _ocol);
		VG_CALL(auto pnt = vg::createRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, _icol, _ocol); )
		VG_CALL(vg::fillPath(g_currentContext, pnt, 0); )
	}

	void vgStroke(uint32_t _color, float _width, uint32_t _flags)
	{
		RTM_UNUSED_3(_color, _width, _flags);
		VG_CALL( vg::strokePath(g_currentContext, _color, _width, _flags); )
	}

	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL(auto pnt = vg::createLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, _icol, _ocol); )
		VG_CALL(vg::strokePath(g_currentContext, pnt, _width, 0); )
	}

	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_r, _f, _icol, _ocol);
		VG_CALL(auto pnt = vg::createBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, _icol, _ocol); )
		VG_CALL(vg::strokePath(g_currentContext, pnt, _width, 0); )
	}

	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_6(_cx, _cy, _inr, _outr, _icol, _ocol);
		VG_CALL(auto pnt = vg::createRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, _icol, _ocol); )
		VG_CALL(vg::strokePath(g_currentContext, pnt, _width, 0); )
	}

} // namespace rapp
