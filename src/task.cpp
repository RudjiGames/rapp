//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/rapp_config.h>
#include <rapp/src/task_private.h>

#if RTM_COMPILER_MSVC
__pragma(warning(push))
__pragma(warning(disable:4100))
#endif

#define ENKI_CUSTOM_ALLOC_FILE_AND_LINE

#if RTM_PLATFORM_POSIX
	#include <semaphore.h>
	#include <time.h>
	#include <pthread.h>
#endif

#include "../3rd/enkiTS/src/TaskScheduler.h"

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

	class Task : public enki::ITaskSet
	{
	public:
		CompletionActionDelete	m_taskDeleter;
		TaskFn					m_function;
		void*					m_userData;
		uint32_t				m_stride;
		uint32_t				m_start;
		uint32_t				m_end;

		Task(uint32_t _numTasks, bool _delete)
			: enki::ITaskSet(_numTasks)
			, m_function(0)
			, m_userData(0)
			, m_stride(0)
			, m_start(0)
			, m_end(0)
		{
			if (_delete)
				m_taskDeleter.SetDependency(m_taskDeleter.m_Dependency, this);
		}

		void ExecuteRange(enki::TaskSetPartition _range, uint32_t _threadnum) override
		{
			RTM_UNUSED(_threadnum);
#if RAPP_WITH_RPROF
			RPROF_SCOPE("test");
#endif // RAPP_WITH_RPROF

			m_function(m_userData, _range.start, _range.end);
		}
	};

#if RAPP_WITH_RPROF
	static void rprofCallbackThreadStart(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackThreadEnd(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitNewTaskSuspendStart(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitNewTaskSuspendStop(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitTaskCompleteStart(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitTaskCompleteStop(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitTaskCompleteSuspendStart(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
	static void rprofCallbackWaitTaskCompleteSuspendStop(uint32_t _threadNum)
	{
		RTM_UNUSED(_threadNum);
	}
#endif // RAPP_WITH_RPROF

	static void* rprofAllocFunc(size_t _alignment, size_t _size, void* _userData, const char* _file, int _line)
	{
		RTM_UNUSED_3(_userData, _file, _line);
		return rtm_alloc(_size, _alignment);
	}

	static void  rprofFreeFunc(void* _ptr, size_t _size, void* _userData, const char* _file, int _line)
	{
		RTM_UNUSED_4(_size, _userData, _file, _line);
		rtm_free(_ptr);
	}

	/// 
	void taskInit()
	{
		TaskSchedulerConfig config;
#if RAPP_WITH_RPROF
		config.profilerCallbacks.threadStart						= rprofCallbackThreadStart;
		config.profilerCallbacks.threadStop							= rprofCallbackThreadEnd;
		config.profilerCallbacks.waitForNewTaskSuspendStart			= rprofCallbackWaitNewTaskSuspendStart;
		config.profilerCallbacks.waitForNewTaskSuspendStop			= rprofCallbackWaitNewTaskSuspendStop;
		config.profilerCallbacks.waitForTaskCompleteStart			= rprofCallbackWaitTaskCompleteStart;
		config.profilerCallbacks.waitForTaskCompleteStop			= rprofCallbackWaitTaskCompleteStop;
		config.profilerCallbacks.waitForTaskCompleteSuspendStart	= rprofCallbackWaitTaskCompleteSuspendStart;
		config.profilerCallbacks.waitForTaskCompleteSuspendStop		= rprofCallbackWaitTaskCompleteSuspendStop;
#endif // RAPP_WITH_RPROF

		config.customAllocator.alloc	= rprofAllocFunc;
		config.customAllocator.free		= rprofFreeFunc;
		config.customAllocator.userData = 0;

		g_TS.Initialize(config);
	}

	/// 
	void taskShutdown()
	{
		g_TS.WaitforAllAndShutdown();
	}

	/// 
	TaskHandle taskCreate(TaskFn _func, void* _userData, bool _deleteOnFinish)
	{
		return taskCreateGroup(_func, _userData, 0, 1, _deleteOnFinish);
	}

	/// 
	TaskHandle taskCreateGroup(TaskFn _func, void* _userData, uint32_t _dataStride, uint32_t _numTasks, bool _deleteOnFinish)
	{
		Task* j = new Task(_numTasks, _deleteOnFinish);
		j->m_function	= _func;
		j->m_userData	= _userData;
		j->m_stride		= _dataStride;
		j->m_start		= 0;
		j->m_end		= _numTasks;
		return { (uintptr_t)j };
	}

	/// 
	void taskDestroy(TaskHandle _task)
	{
		Task* j = (Task*)_task.idx;
		if (!j->GetIsComplete())
			g_TS.WaitforTask(j);
		delete j;
	}

	/// 
	void taskRun(TaskHandle _task)
	{
		Task* j = (Task*)_task.idx;
		g_TS.AddTaskSetToPipe(j);
	}

	/// 
	void taskWait(TaskHandle _task)
	{
		Task* j = (Task*)_task.idx;
		g_TS.WaitforTask(j);
	}

	/// 
	uint32_t taskStatus(TaskHandle _task)
	{
		Task* j = (Task*)_task.idx;
		return j->GetIsComplete() ? RAPP_TASK_STATUS_COMPLETE : RAPP_TASK_STATUS_RUNNING;
	}

} // namespace rapp
