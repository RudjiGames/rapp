//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/rapp_config.h>
#include <rapp/src/job.h>

#if RAPP_JOBS_TBB
#include <tbb/tbb.h>

namespace rapp {

void jobInit(uint32_t _numThreads)
{
}

JobHandle jobCreate(ThreadFn _func, void* _userData)
{
	return {0};
}

JobHandle jobCreateGroup(ThreadFn _func, void* _userData, uint32_t _dataStride, uint32_t _numJobs)
{
	return {0};
}

JobHandle jobCreateChild(JobHandle _parent, ThreadFn _func, void* _userData)
{
	return {0};
}

void jobRun(JobHandle _job)
{
}

void jobWait(JobHandle _job)
{
}

uint32_t jobStatus(JobHandle _job)
{
	return 0;
}

void jobShutdown()
{
}

} // namespace rapp

#endif // RAPP_JOBS_TBB
