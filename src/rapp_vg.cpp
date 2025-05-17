//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include "app_data.h"

#ifdef RAPP_WITH_BGFX

#include <../3rd/vg_renderer/include/vg/vg.h>
#include <../3rd/vg_renderer/include/vg/path.h>
#include <../3rd/vg_renderer/include/vg/stroker.h>

vg::Context*			g_currentContext	= 0;
vg::FillFlags::Enum		g_fillFlags			= vg::FillFlags::ConvexAA;
vg::StrokeFlags::Enum	g_strokeFlags		= vg::StrokeFlags::ButtRoundAA;

inline static vg::Color getVGColor(uint32_t _color)
{
	return _color;
}

#define VG_CALL( X ) X
#else
#define VG_CALL( X )
#endif

namespace rapp {

	///
	uint32_t vgColor(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
	{
		return (_r << 24) | (_g << 16) | (_b << 8) | (_a);
	}

	void vgSave()
	{
		VG_CALL(vg::pushState(g_currentContext);)
	}

	void vgRestore()
	{
		VG_CALL(vg::popState(g_currentContext);)
	}

	///
	void vgGlobalAlpha(float _opacity)
	{
		RTM_UNUSED(_opacity);
		VG_CALL(vg::setGlobalAlpha(g_currentContext, _opacity);)
	}

	void vgBeginPath()
	{
		VG_CALL(vg::beginPath(g_currentContext);)
	}

	void vgMoveTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL(vg::moveTo(g_currentContext, _x, _y);)
	}

	void vgLineTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL(vg::lineTo(g_currentContext, _x, _y);)
	}

	void vgBezierTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y)
	{
		RTM_UNUSED_6(_c1x, _c1y, _c2x, _c2y, _x, _y);
		VG_CALL(vg::cubicTo(g_currentContext, _c1x, _c1y, _c2x, _c2y, _x, _y);)
	}

	void vgQuadraticTo(float _cx, float _cy, float _x, float _y)
	{
		RTM_UNUSED_4(_cx, _cy, _x, _y);
		VG_CALL(vg::quadraticTo(g_currentContext, _cx, _cy, _x, _y);)
	}

	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r)
	{
		RTM_UNUSED_5(_x1, _y1, _x2, _y2, _r);
		VG_CALL(vg::arcTo(g_currentContext, _x1, _y1, _x2, _y2, _r);)
	}

	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir)
	{
		RTM_UNUSED_6(_cx, _cy, _r, _a0, _a1, _dir);
		VG_CALL(vg::arc(g_currentContext, _cx, _cy, _r, _a0, _a1, _dir ? vg::Winding::CW : vg::Winding::CCW);)
	}

	void vgRect(float _x, float _y, float _w, float _h)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		VG_CALL(vg::rect(g_currentContext, _x, _y, _w, _h);)
	}

	void vgRect(float _x, float _y, float _w, float _h, float _r)
	{
		RTM_UNUSED_5(_x, _y, _w, _h, _r);
		VG_CALL(vg::roundedRect(g_currentContext, _x, _y, _w, _h, _r);)
	}

	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_rtl, _rtr, _rbr, _rbl);
		VG_CALL(vg::roundedRectVarying(g_currentContext, _x, _y, _w, _h, _rtl, _rtr, _rbr, _rbl);)
	}

	void vgCircle(float _cx, float _cy, float _radius)
	{
		RTM_UNUSED_3(_cx, _cy, _radius);
		VG_CALL(vg::circle(g_currentContext, _cx, _cy, _radius);)
	}

	void vgEllipse(float _cx, float _cy, float _rx, float _ry)
	{
		RTM_UNUSED_4(_cx, _cy, _rx, _ry);
		VG_CALL(vg::ellipse(g_currentContext, _cx, _cy, _rx, _ry);)
	}

	void vgPolyline(const float* _coords, uint32_t _numPoints)
	{
		RTM_UNUSED_2(_coords, _numPoints);
		VG_CALL(vg::polyline(g_currentContext, _coords, _numPoints);)
	}

	void vgPathWinding(int _winding)
	{
		RTM_UNUSED(_winding);
		VG_CALL(g_fillFlags = _winding == VG_CCW ? vg::FillFlags::ConvexAA : vg::FillFlags::ConcaveNonZeroAA);
	}

	void vgClosePath()
	{
		VG_CALL(vg::closePath(g_currentContext);)
	}

	void vgFill(uint32_t _color, uint32_t _flags)
	{
		RTM_UNUSED_2(_color, _flags);
		VG_CALL(vg::fillPath(g_currentContext, getVGColor(_color), g_fillFlags);)
	}

	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL(vg::GradientHandle gradient = vg::createLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::fillPath(g_currentContext, gradient, g_fillFlags);)
	}

	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_x, _y, _w, _h, _r, _f);
		RTM_UNUSED_2(_icol, _ocol);
		VG_CALL(vg::GradientHandle gradient = vg::createBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::fillPath(g_currentContext, gradient, g_fillFlags);)
	}

	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_4(_cx, _cy, _inr, _outr);
		RTM_UNUSED_2(_icol, _ocol);
		VG_CALL(vg::GradientHandle gradient = vg::createRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::fillPath(g_currentContext, gradient, g_fillFlags);)
	}

	void vgStroke(uint32_t _color, float _width, uint32_t _flags)
	{
		RTM_UNUSED_3(_color, _width, _flags);
		VG_CALL(vg::strokePath(g_currentContext, getVGColor(_color), _width, g_strokeFlags);)
	}

	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_4(_sx, _sy, _ex, _ey);
		RTM_UNUSED_3(_icol, _ocol, _width);
		VG_CALL(vg::GradientHandle gradient = vg::createLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::strokePath(g_currentContext, gradient, _width, g_strokeFlags);)
	}

	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_6(_x, _y, _w, _h, _r, _f);
		RTM_UNUSED_3(_icol, _ocol, _width);
		VG_CALL(vg::GradientHandle gradient = vg::createBoxGradient(g_currentContext, _x, _y, _w, _h, _r, _f, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::strokePath(g_currentContext, gradient, _width, g_strokeFlags);)
	}

	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_4(_cx, _cy, _inr, _outr);
		RTM_UNUSED_3(_icol, _ocol, _width);
		VG_CALL(vg::GradientHandle gradient = vg::createRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, getVGColor(_icol), getVGColor(_ocol));)
		VG_CALL(vg::strokePath(g_currentContext, gradient, _width, g_strokeFlags);)
	}

	void vgLineCapJoin(int _lineCap, int _lineJoin)
	{
		RTM_UNUSED_2(_lineCap, _lineJoin);
		VG_CALL(g_strokeFlags = (vg::StrokeFlags::Enum)VG_STROKE_FLAGS(_lineCap, _lineJoin, 1);)
	}

	void vgTransformPoint(float* dx, float* dy, const float* t, float sx, float sy)
	{
		*dx = sx * t[0] + sy * t[2] + t[4];
		*dy = sx * t[1] + sy * t[3] + t[5];
	}

	void vgTransformScale(float* _xform, float _scaleX, float _scaleY)
	{
		_xform[0] = _scaleX;
		_xform[1] = 0.0f;
		_xform[2] = 0.0f;
		_xform[3] = _scaleY;
		_xform[4] = 0.0f;
		_xform[5] = 0.0f;
	}

	void vgTransformTranslate(float* _xform, float _translateX, float _translateY)
	{
		_xform[0] = 1.0f;
		_xform[1] = 0.0f;
		_xform[2] = 0.0f;
		_xform[3] = 1.0f;
		_xform[4] = _translateX;
		_xform[5] = _translateY;
	}

	void vgTransformTranslateScale(float* _xform, float _translateX, float _translateY, float _scaleX, float _scaleY)
	{
		_xform[0] = _scaleX;
		_xform[1] = 0.0f;
		_xform[2] = 0.0f;
		_xform[3] = _scaleY;
		_xform[4] = _translateX;
		_xform[5] = _translateY;
	}

	void vgTransformRotate(float* _xform, float _angleRadian)
	{
		float cs = cosf(_angleRadian);
		float sn = sinf(_angleRadian);
		_xform[0] = cs;
		_xform[1] = sn;
		_xform[2] = -sn;
		_xform[3] = cs;
		_xform[4] = 0.0f;
		_xform[5] = 0.0f;
	}

	void vgTransformMultiply(float* _xform, const float* _left, const float* _right)
	{
		float t0 = _left[0] * _right[0] + _left[1] * _right[2];
		float t2 = _left[2] * _right[0] + _left[3] * _right[2];
		float t4 = _left[4] * _right[0] + _left[5] * _right[2] + _right[4];
		_xform[1] = _left[0] * _right[1] + _left[1] * _right[3];
		_xform[3] = _left[2] * _right[1] + _left[3] * _right[3];
		_xform[5] = _left[4] * _right[1] + _left[5] * _right[3] + _right[5];
		_xform[0] = t0;
		_xform[2] = t2;
		_xform[4] = t4;
	}

} // namespace rapp
