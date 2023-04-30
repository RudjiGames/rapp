//--------------------------------------------------------------------------//
/// Copyright (c) 2018 Milos Tosic. All Rights Reserved.                   ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#define RBASE_NAMESPACE rapp
#include <rbase/inc/platform.h>
#include <rbase/inc/console.h>
#include <rbase/inc/libassert.h>

#define RAPP_WITH_BGFX 1
#include <rapp/inc/rapp.h>

#ifdef RAPP_WITH_RPROF
#include <rprof/inc/rprof.h>
#include <rprof/inc/rprof_imgui.h>
#endif // RAPP_WITH_RPROF
