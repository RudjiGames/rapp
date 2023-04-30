//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp/inc/rapp.h>
#include <rapp/src/rapp_config.h>

//#define RTM_DEFINE_STL_TYPES
#define RAPP_HASH_SIZE	32768
#define RAPP_HASH_MASK	32767
#define RAPP_MAX_APPS	64

#define RBASE_NAMESPACE rapp
#include <rbase/inc/platform.h>
#include <rbase/inc/libassert.h>
#include <rbase/inc/libhandler.h>
#include <rbase/inc/atomic.h>
#include <rbase/inc/itc.h>
#include <rbase/inc/datastore.h>
#include <rbase/inc/cpu.h>
#include <rbase/inc/hash.h>
#include <rbase/inc/mutex.h>
#include <rbase/inc/path.h>
#include <rbase/inc/queue.h>

#ifdef RAPP_WITH_RPROF
#include <rprof/inc/rprof.h>
#endif // RAPP_WITH_RPROF
