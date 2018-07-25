//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_CONFIG_H
#define RTM_RAPP_CONFIG_H

#define RAPP_MAX_WINDOWS		2048

#define RAPP_JOBS_PER_THREAD	16*1024
#define RAPP_JOBS_PER_QUEUE		8*1024
#define RAPP_JOBS_QUEUE_MASK	(RAPP_JOBS_PER_QUEUE - 1)

#ifndef RAPP_WITH_BGFX
#define RAPP_WITH_BGFX		0
#endif

#endif // RTM_RAPP_CONFIG_H
