//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/rapp_config.h>
#include <rapp/src/job.h>

#if RTM_COMPILER_MSVC
__pragma(warning(push))
__pragma(warning(disable:4100))
#endif

#include "../3rd/enkiTS/src/LockLessMultiReadPipe.h"
#include "../3rd/enkiTS/src/TaskScheduler.h"
#include "../3rd/enkiTS/src/TaskScheduler.cpp"
#include "../3rd/enkiTS/src/TaskScheduler_c.h"
#include "../3rd/enkiTS/src/TaskScheduler_c.cpp"

#if RTM_COMPILER_MSVC
__pragma(warning(pop))
#endif

#include <atomic>
#include <thread>

using namespace enki;

namespace rapp {

	enki::TaskScheduler g_TS;

	struct CompletionActionDelete : ICompletable
	{
		Dependency    m_Dependency;
		void OnDependenciesComplete(TaskScheduler* pTaskScheduler_, uint32_t threadNum_)
		{
			ICompletable::OnDependenciesComplete(pTaskScheduler_, threadNum_);
			delete m_Dependency.GetDependencyTask();
		}
	};

	class Job : public enki::ITaskSet
	{
	public:
		JobFn					m_function;
		void*					m_userData;
		uint32_t				m_stride;
		uint32_t				m_start;
		uint32_t				m_end;
		CompletionActionDelete	m_taskDeleter;


		Job(uint32_t _numJobs, bool _delete)
			: enki::ITaskSet(_numJobs)
			, m_function(0)
			, m_userData(0)
			, m_stride(0)
			, m_start(0)
			, m_end(0)
		{
			if (_delete)
				m_taskDeleter.SetDependency(m_taskDeleter.m_Dependency, this);
		}

		void ExecuteRange(enki::TaskSetPartition _range, uint32_t /*_threadnum*/) override
		{
#ifdef RAPP_WITH_RPROF
			RPROF_SCOPE("test");
#endif // RAPP_WITH_RPROF

			m_function(m_userData, _range.start, _range.end);
		}
	};

	void jobInit()
	{
		g_TS.Initialize();
	}

	void jobShutdown()
	{
		g_TS.WaitforAllAndShutdown();
	}

	JobHandle jobCreate(JobFn _func, void* _userData, bool _deleteOnFinish)
	{
		return jobCreateGroup(_func, _userData, 0, 1, _deleteOnFinish);
	}

	JobHandle jobCreateGroup(JobFn _func, void* _userData, uint32_t _dataStride, uint32_t _numJobs, bool _deleteOnFinish)
	{
		Job* j = new Job(_numJobs, _deleteOnFinish);
		j->m_function	= _func;
		j->m_userData	= _userData;
		j->m_stride		= _dataStride;
		j->m_start		= 0;
		j->m_end		= _numJobs;
		return { (uintptr_t)j };
	}

	void jobDestroy(JobHandle _job)
	{
		Job* j = (Job*)_job.idx;
		if (!j->GetIsComplete())
			g_TS.WaitforTask(j);
		delete j;
	}

	void jobRun(JobHandle _job)
	{
		Job* j = (Job*)_job.idx;
		g_TS.AddTaskSetToPipe(j);
	}

	void jobWait(JobHandle _job)
	{
		Job* j = (Job*)_job.idx;
		g_TS.WaitforTask(j);
	}

	uint32_t jobStatus(JobHandle _job)
	{
		Job* j = (Job*)_job.idx;
		return j->GetIsComplete() ? 1 : 0;
	}

} // namespace rapp
