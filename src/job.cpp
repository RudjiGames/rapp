//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/src/rapp_config.h>
#include <rapp/src/job.h>

#include <atomic>
#include <thread>

namespace rapp {

struct Job
{
	struct Data
	{
		ThreadFn			m_function;
		void*				m_userData;
		Job*				m_parent;
		std::atomic<int>	m_jobCount;
	};

	Data		m_data;
	uint8_t		m_padding[RTM_CACHE_LINE_SIZE - sizeof(Data)];

	inline Job()
	{
		m_data.m_function	= 0;
		m_data.m_userData	= 0;
		m_data.m_parent		= 0;
	}

	bool isDone() const { return m_data.m_jobCount <= 0; }
};

struct JobThread : public rtm::Thread
{
	int32_t					m_index;
	std::atomic<uint32_t>	m_bottom;
	std::atomic<uint32_t>	m_top;
	std::atomic<bool>		m_active;
	rtm::Random				m_random;
	Job**					m_jobsQueue;
	Job*					m_jobsPool;
	int32_t					m_jobsAllocated;

	static uint32_t			s_jobNumThreads;
	static JobThread*		s_jobThreads;
	static Job**			s_jobQueue;
	static Job*				s_jobAllocator;
	
	JobThread()
		: m_bottom(0)
		, m_top(0)
		, m_active(false)
	{}

	virtual ~JobThread() {}

	void init(uint32_t _index, Job** _queue, Job* _allocator)
	{
		m_jobsAllocated	= 0;
		m_index			= _index;
		m_jobsQueue		= _queue;
		m_jobsPool		= _allocator;

		for (uint32_t i=0; i<_index; ++i)
			m_random.gen();
	}

	Job* newJob()
	{
		const uint32_t alloc_index = (++m_jobsAllocated) & (RAPP_JOBS_PER_THREAD - 1);
		return &m_jobsPool[alloc_index];
	}

	Job* createJob(Job* _parent, ThreadFn _function, void* _userData)
	{
		if (_parent)
			++_parent->m_data.m_jobCount;

		Job* job = newJob();
		job->m_data.m_function	= _function;
		job->m_data.m_userData	= _userData;
		job->m_data.m_parent	= _parent;
		job->m_data.m_jobCount	= 1;
		return job;
	}

	void wait(const Job* job)
	{
		while (!job->isDone())
		{
			Job* next_task = getJob();
			if (next_task)
				execute(next_task);
		}
	}

	Job* getJob()
	{
		Job* job = pop();
		if (!job)
		{
			int32_t index = m_index;
			while (index == m_index)
				index = m_random.gen() % JobThread::s_jobNumThreads;

			job = s_jobThreads[index].steal();
			if (!job)
			{
				rtm::Thread::yield();
				return 0;
			}
		}

		return job;
	}

	void execute(Job* job)
	{
		if (job->m_data.m_function)
			job->m_data.m_function(job->m_data.m_userData);
		finalize(job);
	}

	void finalize(Job* job)
	{
		const int32_t pendingJobs = --job->m_data.m_jobCount;
		if ((pendingJobs == 0) && job->m_data.m_parent)
			finalize(job->m_data.m_parent);
	}

	void push(Job* job)
	{
		uint32_t bottom = m_bottom;
		m_jobsQueue[bottom & RAPP_JOBS_QUEUE_MASK] = job;
		rtm::readWriteBarrier();
		m_bottom = bottom + 1;
	}

	Job* pop()
	{
		uint32_t bottom = m_bottom - 1;
		m_bottom = bottom;
		rtm::memoryBarrier();
		uint32_t top = m_top;
		if (top <= bottom)
		{
			Job* job = m_jobsQueue[bottom & RAPP_JOBS_QUEUE_MASK];
			if (top != bottom)
				return job;

			if (!m_top.compare_exchange_strong(top, top + 1))
				job = 0;

			m_bottom = top + 1;
			return job;
		}

		m_bottom = top;
		return 0;
	}

	Job* steal()
	{
		uint32_t top = m_top;
		rtm::readWriteBarrier();
		uint32_t bottom = m_bottom;
		if (top < bottom)
		{
			Job* job = m_jobsQueue[top & RAPP_JOBS_QUEUE_MASK];
			if (!m_top.compare_exchange_strong(top, top + 1))
				return 0;
			return job;
		}
		return 0;
	}

	void shutdown()
	{
		m_active = false;
	}

	static int32_t threadFunc(void* _userData)
	{
		JobThread* self = (JobThread*)_userData;

#if RTM_PLATFORM_WINDOWS
		GROUP_AFFINITY group{};
		group.Mask = (KAFFINITY)-1;
		group.Group = self->m_index & 1;
		SetThreadGroupAffinity(GetCurrentThread(), &group, 0);
#endif

		self->m_active = true;
		while (self->m_active && self->m_started)
		{
			Job* job = self->getJob();
			if (job)
				self->execute(job);
		}
		return 0;
	}
};

uint32_t	JobThread::s_jobNumThreads;
JobThread*	JobThread::s_jobThreads;
Job**		JobThread::s_jobQueue;
Job*		JobThread::s_jobAllocator;

JobHandle jobToHandle(Job* _job)
{
	return { (uint32_t)((uintptr_t)(_job - JobThread::s_jobAllocator) & 0xffffffff) };
}

Job* jobFromHandle(JobHandle _job)
{
	return _job.idx + JobThread::s_jobAllocator;
}

void jobInit(uint32_t _numThreads)
{
	if (_numThreads > 0)
		JobThread::s_jobNumThreads = _numThreads;
	else
		JobThread::s_jobNumThreads = std::thread::hardware_concurrency();

	RTM_ASSERT(JobThread::s_jobNumThreads > 1, "");

	JobThread::s_jobThreads = new JobThread[JobThread::s_jobNumThreads];

	static uint32_t queueJobs = JobThread::s_jobNumThreads * RAPP_JOBS_PER_QUEUE;
	static uint32_t totalJobs = JobThread::s_jobNumThreads * RAPP_JOBS_PER_THREAD;

	JobThread::s_jobQueue     = new Job*[queueJobs];
	JobThread::s_jobAllocator = new Job[totalJobs];

	memset(JobThread::s_jobQueue, 0, sizeof(Job*)*queueJobs);

	for (uint32_t i=0; i<JobThread::s_jobNumThreads; ++i)
		JobThread::s_jobThreads[i].init( i,	JobThread::s_jobQueue + (i*RAPP_JOBS_PER_QUEUE),
											JobThread::s_jobAllocator + (i*RAPP_JOBS_PER_THREAD));

	for (uint32_t i=1; i<JobThread::s_jobNumThreads; ++i)
		JobThread::s_jobThreads[i].start(JobThread::threadFunc, &JobThread::s_jobThreads[i]);
}

JobHandle jobCreate(ThreadFn _func, void* _userData)
{
	Job* job = JobThread::s_jobThreads[0].createJob(0, _func, _userData);
	return jobToHandle(job);
}

JobHandle jobCreateGroup(ThreadFn _func, void* _userData, uint32_t _dataStride, uint32_t _numJobs)
{
	Job* groupJob = jobFromHandle(jobCreate(0, 0));

	uint8_t* userData = (uint8_t*)_userData;
	for (uint32_t i=0; i<_numJobs; ++i)
	{
		Job* job = JobThread::s_jobThreads[0].createJob(groupJob, _func, userData + (_dataStride*i));
		JobThread::s_jobThreads[0].push(job);
	}

	return jobToHandle(groupJob);
}

JobHandle jobCreateChild(JobHandle _parent, ThreadFn _func, void* _userData)
{
	Job* job = JobThread::s_jobThreads[0].createJob(jobFromHandle(_parent), _func, _userData);
	return jobToHandle(job);
}

void jobRun(JobHandle _job)
{
	Job* job = jobFromHandle(_job);
	JobThread::s_jobThreads[0].push(job);
}

void jobWait(JobHandle _job)
{
	JobThread::s_jobThreads[0].wait(jobFromHandle(_job));
}

uint32_t jobStatus(JobHandle _job)
{
	Job* job = jobFromHandle(_job);
	return job->m_data.m_jobCount > 1 ? 1 : 0;
}

void jobShutdown()
{
	for (uint32_t i=1; i<JobThread::s_jobNumThreads; ++i)
	{
		JobThread::s_jobThreads[i].shutdown();
		JobThread::s_jobThreads[i].stop();
	}

	delete[] JobThread::s_jobThreads;
	delete[] JobThread::s_jobAllocator;
	delete[] JobThread::s_jobQueue;

	JobThread::s_jobThreads = 0;
}

} // namespace rapp
