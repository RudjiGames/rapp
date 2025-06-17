//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_CONFIG_H
#define RTM_RAPP_CONFIG_H

#define RAPP_MAX_WINDOWS		2048

#define RAPP_TASKS_PER_THREAD	16*1024
#define RAPP_TASKS_PER_QUEUE	8*1024
#define RAPP_TASKS_QUEUE_MASK	(RAPP_TASKS_PER_QUEUE - 1)

#ifndef RAPP_WITH_RPROF
#define RAPP_WITH_RPROF			0
#endif // RAPP_WITH_RPROF

#endif // RTM_RAPP_CONFIG_H
