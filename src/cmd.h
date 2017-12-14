//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_CMD_H
#define RTM_RAPP_CMD_H

namespace rapp {

	struct CmdContext
	{
		CmdContext()
			: m_maxCommandLength(0)
		{}

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

		typedef rtm_unordered_map<uint32_t, Func> CmdLookup;
		CmdLookup	m_lookup;
		uint32_t	m_maxCommandLength;
	};

	///
	void cmdInit();

	///
	void cmdShutdown();

	///
	CmdContext* cmdGetContext();

} // namespace rtm

#endif // RTM_RAPP_CMD_H
