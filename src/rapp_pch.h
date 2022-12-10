//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp/inc/rapp.h>
#include <rapp/src/rapp_config.h>

#define RBASE_NAMESPACE rapp
//#define RTM_DEFINE_STL_TYPES
#define RAPP_HASH_SIZE	32768
#define RAPP_HASH_MASK	32767
#define RAPP_MAX_APPS	64

#include <rbase/inc/platform.h>

#include <rbase/inc/atomic.h>
#include <rbase/inc/itc.h>
#include <rbase/inc/console.h>
#include <rbase/inc/cpu.h>
#include <rbase/inc/containers.h>
#include <rbase/inc/datastore.h>
#include <rbase/inc/handlepool.h>
#include <rbase/inc/hash.h>
#include <rbase/inc/libassert.h>
#include <rbase/inc/libhandler.h>
#include <rbase/inc/mutex.h>
#include <rbase/inc/path.h>
#include <rbase/inc/queue.h>
#include <rbase/inc/random.h>
#include <rbase/inc/stringfn.h>
#include <rbase/inc/thread.h>
#include <rbase/inc/uint32_t.h>
