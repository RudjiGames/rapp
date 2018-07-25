//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_APP_DATA_H
#define RTM_RAPP_APP_DATA_H

#if RAPP_WITH_BGFX

#include <rapp/src/console.h>
#include <common/nanovg/nanovg.h>

namespace rapp {

	struct AppData
	{
		Console*	m_console;
		NVGcontext*	m_nvg; 

		AppData()
			: m_console(0)
			, m_nvg(0)
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
