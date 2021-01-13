//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_JOB_H
#define RTM_RAPP_JOB_H

#include <rapp/inc/rapp.h>
#include <rapp/src/rapp_config.h>

namespace rapp {

	///
	void jobInit(uint32_t _numThreads = 0);

	///
	void jobShutdown();

} // namespace rapp

#endif // RTM_RAPP_JOB_H
