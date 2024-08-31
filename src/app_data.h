//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_APP_DATA_H
#define RTM_RAPP_APP_DATA_H

#ifdef RAPP_WITH_BGFX

#include <rapp/src/console.h>
#if !RAPP_WITH_VG_RENDER
#include <../3rd/nanovg_bgfx/nanovg_bgfx.h>
#include <../3rd/nanovg_bgfx/nanovg.h>
#else
#include <../3rd/vg-renderer/include/vg/vg.h>
#include <../3rd/vg-renderer/include/vg/path.h>
#include <../3rd/vg-renderer/include/vg/stroker.h>
#endif

namespace rapp {

	struct AppData
	{
		Console*		m_console;

#if !RAPP_WITH_VG_RENDER
		NVGcontext*		m_vg;
#else
		vg::Context*	m_vg;
#endif
		AppData()
			: m_console(0)
			, m_vg(0)
		{}
	};

} // namespace rtm

#else	// RAPP_WITH_BGFX

namespace rapp {

	struct AppData
	{
	};

} // namespace rtm

#endif	// RAPP_WITH_BGFX

#endif // RTM_RAPP_APP_DATA_H
