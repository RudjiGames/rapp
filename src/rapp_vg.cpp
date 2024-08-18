//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include "app_data.h"

#ifdef RAPP_WITH_BGFX
#include <../3rd/nanovg_bgfx/nanovg_bgfx.h>
#include <../3rd/nanovg_bgfx/nanovg.h>
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

	void vgSave()
	{
		VG_CALL(nvgSave(g_currentContext);)
	}

	void vgRestore()
	{
		VG_CALL(nvgRestore(g_currentContext);)
	}

	///
	void vgGlobalAlpha(float _opacity)
	{
		RTM_UNUSED(_opacity);
		VG_CALL(nvgGlobalAlpha(g_currentContext, _opacity);)
	}

	void vgBeginPath()
	{
		VG_CALL(nvgBeginPath(g_currentContext);)
	}

	void vgMoveTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL(nvgMoveTo(g_currentContext, _x, _y);)
	}

	void vgLineTo(float _x, float _y)
	{
		RTM_UNUSED_2(_x, _y);
		VG_CALL(nvgLineTo(g_currentContext, _x, _y);)
	}

	void vgBezierTo(float _c1x, float _c1y, float _c2x, float _c2y, float _x, float _y)
	{
		RTM_UNUSED_6(_c1x, _c1y, _c2x, _c2y, _x, _y);
		VG_CALL(nvgBezierTo(g_currentContext, _c1x, _c1y, _c2x, _c2y, _x, _y);)
	}

	void vgQuadraticTo(float _cx, float _cy, float _x, float _y)
	{
		RTM_UNUSED_4(_cx, _cy, _x, _y);
		VG_CALL(nvgQuadTo(g_currentContext, _cx, _cy, _x, _y);)
	}

	void vgArcTo(float _x1, float _y1, float _x2, float _y2, float _r)
	{
		RTM_UNUSED_5(_x1, _y1, _x2, _y2, _r);
		VG_CALL(nvgArcTo(g_currentContext, _x1, _y1, _x2, _y2, _r);)
	}

	void vgArc(float _cx, float _cy, float _r, float _a0, float _a1, int _dir)
	{
		RTM_UNUSED_6(_cx, _cy, _r, _a0, _a1, _dir);
		VG_CALL(nvgArc(g_currentContext, _cx, _cy, _r, _a0, _a1, _dir);)
	}

	void vgRect(float _x, float _y, float _w, float _h)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		VG_CALL(nvgRect(g_currentContext, _x, _y, _w, _h);)
	}

	void vgRect(float _x, float _y, float _w, float _h, float _r)
	{
		RTM_UNUSED_5(_x, _y, _w, _h, _r);
		VG_CALL(nvgRoundedRect(g_currentContext, _x, _y, _w, _h, _r);)
	}

	void vgRoundedRectVarying(float _x, float _y, float _w, float _h, float _rtl, float _rtr, float _rbr, float _rbl)
	{
		RTM_UNUSED_4(_x, _y, _w, _h);
		RTM_UNUSED_4(_rtl, _rtr, _rbr, _rbl);
		VG_CALL(nvgRoundedRectVarying(g_currentContext, _x, _y, _w, _h, _rtl, _rtr, _rbr, _rbl);)
	}

	void vgCircle(float _cx, float _cy, float _radius)
	{
		RTM_UNUSED_3(_cx, _cy, _radius);
		VG_CALL(nvgCircle(g_currentContext, _cx, _cy, _radius);)
	}

	void vgEllipse(float _cx, float _cy, float _rx, float _ry)
	{
		RTM_UNUSED_4(_cx, _cy, _rx, _ry);
		VG_CALL(nvgEllipse(g_currentContext, _cx, _cy, _rx, _ry);)
	}

	void vgPolyline(const float* _coords, uint32_t _numPoints)
	{
		RTM_UNUSED_2(_coords, _numPoints);
		for (uint32_t i=0; i<_numPoints; ++i)
		{
			VG_CALL(nvgLineTo(g_currentContext, _coords[i*2+0], _coords[i*2+1]);)
		}
	}

	void vgPathWinding(int _winding)
	{
		RTM_UNUSED(_winding);
		VG_CALL(nvgPathWinding(g_currentContext, _winding == VG_CCW ? NVG_CCW : NVG_CW);)
	}

	void vgClosePath()
	{
		VG_CALL(nvgClosePath(g_currentContext);)
	}

	void vgFill(uint32_t _color, uint32_t _flags)
	{
		RTM_UNUSED_2(_color, _flags);
		VG_CALL(nvgFillColor(g_currentContext, getNVGColor(_color));)
		VG_CALL(nvgFill(g_currentContext);)
	}

	void vgFillLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_sx, _sy, _ex, _ey, _icol, _ocol);
		VG_CALL(NVGpaint grad = nvgLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, getNVGColor(_icol), getNVGColor(_ocol)));
		VG_CALL(nvgFillPaint(g_currentContext, grad));
		VG_CALL(nvgFill(g_currentContext);)
	}

	void vgFillBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_6(_x, _y, _w, _h, _r, _f);
		RTM_UNUSED_2(_icol, _ocol);
	}

	void vgFillRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol)
	{
		RTM_UNUSED_4(_cx, _cy, _inr, _outr);
		RTM_UNUSED_2(_icol, _ocol);
		VG_CALL(NVGpaint grad = nvgRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, getNVGColor(_icol), getNVGColor(_ocol)));
		VG_CALL(nvgFillPaint(g_currentContext, grad));
		VG_CALL(nvgFill(g_currentContext);)
	}

	void vgStroke(uint32_t _color, float _width, uint32_t _flags)
	{
		RTM_UNUSED_3(_color, _width, _flags);
		VG_CALL(nvgStrokeWidth(g_currentContext, _width));
		VG_CALL(nvgStrokeColor(g_currentContext, getNVGColor(_color));)
		VG_CALL(nvgStroke(g_currentContext);)
	}

	void vgStrokeLinearGradient(float _sx, float _sy, float _ex, float _ey, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_4(_sx, _sy, _ex, _ey);
		RTM_UNUSED_3(_icol, _ocol, _width);
		VG_CALL(nvgStrokeWidth(g_currentContext, _width));
		VG_CALL(NVGpaint grad = nvgLinearGradient(g_currentContext, _sx, _sy, _ex, _ey, getNVGColor(_icol), getNVGColor(_ocol)));
		VG_CALL(nvgStrokePaint(g_currentContext, grad));
		VG_CALL(nvgStroke(g_currentContext);)
	}

	void vgStrokeBoxGradient(float _x, float _y, float _w, float _h, float _r, float _f, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_6(_x, _y, _w, _h, _r, _f);
		RTM_UNUSED_3(_icol, _ocol, _width);
	}

	void vgStrokeRadialGradient(float _cx, float _cy, float _inr, float _outr, uint32_t _icol, uint32_t _ocol, float _width)
	{
		RTM_UNUSED_4(_cx, _cy, _inr, _outr);
		RTM_UNUSED_3(_icol, _ocol, _width);
		VG_CALL(nvgStrokeWidth(g_currentContext, _width));
		VG_CALL(NVGpaint grad = nvgRadialGradient(g_currentContext, _cx, _cy, _inr, _outr, getNVGColor(_icol), getNVGColor(_ocol)));
		VG_CALL(nvgStrokePaint(g_currentContext, grad));
		VG_CALL(nvgStroke(g_currentContext);)
	}

	void vgLineCapJoin(int _lineCap, int _lineJoin)
	{
		RTM_UNUSED_2(_lineCap, _lineJoin);
		VG_CALL(nvgLineCap(g_currentContext, _lineCap);)
		VG_CALL(nvgLineJoin(g_currentContext, _lineJoin);)
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
