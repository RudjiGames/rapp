//--------------------------------------------------------------------------//
/// Copyright 2025 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_CMD_H
#define RTM_RAPP_CMD_H

#include <rbase/inc/stringfn.h>

namespace rapp {

	struct CmdContext
	{
		CmdContext()
			: m_maxCommandLength(0)
		{
			rtm::memSet(m_lookup, 0, sizeof(m_lookup));
		}

		void add(const char* _name, ConsoleFn _fn, void* _userData, const char* _description);
		void remove(const char* _name);
		void updateMaxLen();
		bool exec(App* _app, const char* _cmd, int* _errorCode);

		struct Func
		{
			ConsoleFn	m_fn;
			void*		m_userData;
			const char*	m_name;
			const char*	m_description;
		};

		Func		m_lookup[RAPP_HASH_SIZE];
		uint32_t	m_maxCommandLength;
	};

	///
	void cmdInit();

	///
	void cmdShutdown();

	///
	CmdContext* cmdGetContext();

	int cmdMouseLock(App* _app, void* _userData, int _argc, char const* const* _argv);
	int cmdGraphics(App* _app, void* _userData, int _argc, char const* const* _argv);
	int cmdApp(App* _app, void* _userData, int _argc, char const* const* _argv);

} // namespace rtm

#endif // RTM_RAPP_CMD_H
