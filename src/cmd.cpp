//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/cmd.h>
#include <rapp/src/app_data.h>
#include <rapp/src/entry_p.h>

#include <time.h>	// time
#include <ctype.h>  // isspace
#include <stdint.h>
#include <stdlib.h> // size_t
#include <string.h> // strlen

namespace rapp {

	bool isSpace(char _ch)
	{
		return ' '  == _ch
			|| '\t' == _ch
			|| '\n' == _ch
			|| '\v' == _ch
			|| '\f' == _ch
			|| '\r' == _ch
			;
	}

	// Reference:
	// http://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	const char* tokenizeCommandLine(const char* _commandLine, char* _buffer, uint32_t& _bufferSize, int32_t& _argc, char* _argv[], int32_t _maxArgvs, char _term)
	{
		int32_t argc = 0;
		const char* curr = _commandLine;
		char* currOut = _buffer;
		char term = ' ';
		bool sub = false;

		enum ParserState
		{
			SkipWhitespace,
			SetTerm,
			Copy,
			Escape,
			End,
		};

		ParserState state = SkipWhitespace;

		while ('\0' != *curr
		&&     _term != *curr
		&&     argc < _maxArgvs)
		{
			switch (state)
			{
				case SkipWhitespace:
					for (; isSpace(*curr); ++curr) {}; // skip whitespace
					state = SetTerm;
					break;

				case SetTerm:
					if ('"' == *curr)
					{
						term = '"';
						++curr; // skip begining quote
					}
					else
					{
						term = ' ';
					}

					_argv[argc] = currOut;
					++argc;

					state = Copy;
					break;

				case Copy:
					if ('\\' == *curr)
					{
						state = Escape;
					}
					else if ('"' == *curr
						&&  '"' != term)
					{
						sub = !sub;
					}
					else if (isSpace(*curr) && !sub)
					{
						state = End;
					}
					else if (term != *curr || sub)
					{
						*currOut = *curr;
						++currOut;
					}
					else
					{
						state = End;
					}
					++curr;
					break;

				case Escape:
					{
						const char* start = --curr;
						for (; '\\' == *curr; ++curr) {};

						if ('"' != *curr)
						{
							int32_t count = (int32_t)(curr-start);

							curr = start;
							for (int32_t ii = 0; ii < count; ++ii)
							{
								*currOut = *curr;
								++currOut;
								++curr;
							}
						}
						else
						{
							curr = start+1;
							*currOut = *curr;
							++currOut;
							++curr;
						}
					}
					state = Copy;
					break;

				case End:
					*currOut = '\0';
					++currOut;
					state = SkipWhitespace;
					break;
			}
		}

		*currOut = '\0';
		if (0 < argc
		&&  '\0' == _argv[argc-1][0])
		{
			--argc;
		}

		_bufferSize = (uint32_t)(currOut - _buffer);
		_argc = argc;

		if ('\0' != *curr)
		{
			++curr;
		}

		return curr;
	}

void CmdContext::add(const char* _name, ConsoleFn _fn, void* _userData, const char* _description)
{
	uint32_t cmd = rtm::hashMurmur3(_name, (uint32_t)strlen(_name) );
	if (m_lookup.end() != m_lookup.find(cmd))
		remove(_name);

	//RTM_ASSERT(m_lookup.end() == m_lookup.find(cmd), "Command \"%s\" already exist!", _name);
	Func fn = { _fn, _userData, _name, _description };
	m_lookup.insert(std::make_pair(cmd, fn) );
	updateMaxLen();
}

void CmdContext::remove(const char* _name)
{
	uint32_t cmd = rtm::hashMurmur3(_name, (uint32_t)strlen(_name) );
	CmdLookup::iterator it = m_lookup.find(cmd);
	RTM_ASSERT(m_lookup.end() != it, "Command \"%s\" does not exist!", _name);
	m_lookup.erase(it);
	updateMaxLen();
}

void CmdContext::updateMaxLen()
{
	size_t len = 0;
	for (auto& cmd : m_lookup)
	{
		size_t clen = strlen(cmd.second.m_name);
		if (clen > len) len = clen;
	}
	m_maxCommandLength = (uint32_t)len;
}

bool CmdContext::exec(App* _app, const char* _cmd, int* _errorCode)
{
	rtm::Console::debug("EXEC command:    %s \n", _cmd);
	for (const char* next = _cmd; '\0' != *next; _cmd = next)
	{
		char commandLine[1024];
		uint32_t size = sizeof(commandLine);
		int argc;
		char* argv[64];
		next = tokenizeCommandLine(_cmd, commandLine, size, argc, argv, RTM_NUM_ELEMENTS(argv), '\n');
		if (argc > 0)
		{
			int err = -1;
			uint32_t cmd = rtm::hashMurmur3(argv[0], (uint32_t)strlen(argv[0]) );
			CmdLookup::iterator it = m_lookup.find(cmd);
			if (it != m_lookup.end() )
			{
				Func& fn = it->second;
				err = fn.m_fn(_app, fn.m_userData, argc, argv);
				if (_errorCode)
					*_errorCode = err;
				return true;
			}
		}
	}
	return false;
}

static CmdContext* s_cmdContext = 0;

void cmdInit()
{
	s_cmdContext = rtm_new<CmdContext>();
}

void cmdShutdown()
{
	rtm_delete<CmdContext>(s_cmdContext);
	s_cmdContext = 0;
}

CmdContext* cmdGetContext()
{
	return s_cmdContext;
}

void cmdAdd(const char* _name, ConsoleFn _fn, void* _userData, const char* _description)
{
	s_cmdContext->add(_name, _fn, _userData, _description);
}

void cmdRemove(const char* _name)
{
	s_cmdContext->remove(_name);
}

bool cmdExec(App* _app, const char* _cmd, int* _errorCode)
{
	return s_cmdContext->exec(_app, _cmd, _errorCode);
}

void cmdConsoleLog(App* _app, const char* _fmt, ...)
{
	RTM_UNUSED_2(_app, _fmt);

#if RAPP_WITH_BGFX
    char buf[2048];
    va_list args;
    va_start(args, _fmt);
    vsnprintf(buf, RTM_NUM_ELEMENTS(buf), _fmt, args);
    buf[RTM_NUM_ELEMENTS(buf)-1] = 0;
    va_end(args);

	_app->m_data->m_console->addLog(buf);
#endif // RAPP_WITH_BGFX
}

void cmdConsoleLogRGB(uint8_t _r, uint8_t _g, uint8_t _b, App* _app, const char* _fmt, ...)
{
	RTM_UNUSED_5(_r, _g, _b, _app, _fmt);

#if RAPP_WITH_BGFX
    char buf[2048];
    va_list args;
    va_start(args, _fmt);
    vsnprintf(buf, RTM_NUM_ELEMENTS(buf), _fmt, args);
    buf[RTM_NUM_ELEMENTS(buf)-1] = 0;
    va_end(args);

	_app->m_data->m_console->addLog(_r, _g, _b, buf);
#endif // RAPP_WITH_BGFX
}

void cmdConsoleToggle(App* _app)
{
	RTM_UNUSED(_app);
#if RAPP_WITH_BGFX
	_app->m_data->m_console->toggleVisibility();
#endif // RAPP_WITH_BGFX
}

float g_consoleToggleTime = 0.23f;
void cmdConsoleSetToggleTime(float _time)
{
	RTM_ASSERT(_time > 0.0f && _time < 3.0f, "Really?");
	g_consoleToggleTime = _time;
}


inline bool toBool(const char* _str)
{
	char ch = rtm::toLower(_str[0]);
	return ch == 't' ||  ch == '1';
}


bool setOrToggle(uint32_t& _flags, const char* _name, uint32_t _bit, int _first, int _argc, char const* const* _argv)
{
	if (0 == strcmp(_argv[_first], _name) )
	{
		int arg = _first+1;
		if (_argc > arg)
		{
			_flags &= ~_bit;
			_flags |= toBool(_argv[arg]) ? _bit : 0;
		}
		else
		{
			_flags ^= _bit;
		}

		return true;
	}

	return false;
}

int cmdMouseLock(App* _app, void* _userData, int _argc, char const* const* _argv)
{
	RTM_UNUSED_2(_app, _userData);

	if (_argc > 1)
	{
		inputSetMouseLock(_argc > 1 ? toBool(_argv[1]) : !inputIsMouseLocked() );
		return 0;
	}

	return 1;
}

#if RAPP_WITH_BGFX
uint32_t g_debug = BGFX_DEBUG_TEXT;
uint32_t g_reset = BGFX_RESET_VSYNC;
#else
uint32_t g_debug = 0;
uint32_t g_reset = 0;
#endif
int cmdGraphics(App* _app, void* _userData, int _argc, char const* const* _argv)
{
	RTM_UNUSED_4(_app, _userData, _argc, _argv);

#if RAPP_WITH_BGFX
	if (_argc > 1)
	{
		if (rtm::striCmp(_argv[1], "help") == 0)
		{
			cmdConsoleLog(_app, "graphics vsync       - toggle vsync on/off");
			cmdConsoleLog(_app, "graphics maxaniso    - set maximum anisotropy");
			cmdConsoleLog(_app, "graphics hmd         - HMD stereo rendering");
			cmdConsoleLog(_app, "graphics hmddbg      - HMD stereo rendering debug mode");
			cmdConsoleLog(_app, "graphics hmdrecenter - HMD calibration");
			cmdConsoleLog(_app, "graphics msaa4       - MSAA 4x mode");
			cmdConsoleLog(_app, "graphics msaa8       - MSAA 8x mode");
			cmdConsoleLog(_app, "graphics msaa16      - MSAA 16x mode");
			cmdConsoleLog(_app, "graphics flush       - flush rendering after submitting to GPU");
			cmdConsoleLog(_app, "graphics flip        - toggle flip before (default) and after rendering new frame");
			cmdConsoleLog(_app, "graphics stats       - display rendering statistics");
			cmdConsoleLog(_app, "graphics ifh         - toggle 'infinitely fast hardware', no draw calls submitted to driver");
			cmdConsoleLog(_app, "graphics text        - toggle vsync on/off");
			cmdConsoleLog(_app, "graphics wireframe   - toggle wireframe for all primitives");
			cmdConsoleLog(_app, "graphics screenshot  - take screenshot and save it to disk");
			cmdConsoleLog(_app, "graphics fullscreen  - toggle fullscreen");
			return 0;
		}

		if (setOrToggle(g_reset, "vsync",       BGFX_RESET_VSYNC,              1, _argc, _argv)
		||  setOrToggle(g_reset, "maxaniso",    BGFX_RESET_MAXANISOTROPY,      1, _argc, _argv)
		||	setOrToggle(g_reset, "msaa4",		BGFX_RESET_MSAA_X4,			   1, _argc, _argv)
		||	setOrToggle(g_reset, "msaa8",		BGFX_RESET_MSAA_X8,			   1, _argc, _argv)
		||  setOrToggle(g_reset, "msaa16",      BGFX_RESET_MSAA_X16,           1, _argc, _argv)
		||  setOrToggle(g_reset, "flush",       BGFX_RESET_FLUSH_AFTER_RENDER, 1, _argc, _argv)
		||  setOrToggle(g_reset, "flip",        BGFX_RESET_FLIP_AFTER_RENDER,  1, _argc, _argv)
			)
		{
			return 0;
		}
		else if (	setOrToggle(g_debug, "stats",     BGFX_DEBUG_STATS,     1, _argc, _argv)
				||  setOrToggle(g_debug, "ifh",       BGFX_DEBUG_IFH,       1, _argc, _argv)
				||  setOrToggle(g_debug, "text",      BGFX_DEBUG_TEXT,      1, _argc, _argv)
				||  setOrToggle(g_debug, "wireframe", BGFX_DEBUG_WIREFRAME, 1, _argc, _argv) )
		{
			//bgfx::setDebug(g_debug);
			return 0;
		}
		else if (0 == strcmp(_argv[1], "screenshot") )
		{
			bgfx::FrameBufferHandle fbh = BGFX_INVALID_HANDLE; 

			if (_argc > 2)
			{
				bgfx::requestScreenShot(fbh, _argv[2]);
			}
			else
			{
				time_t tt;
				time(&tt);

				char filePath[256];
				::snprintf(filePath, sizeof(filePath), "temp/screenshot-%" PRId64 "", (uint64_t)tt);
				bgfx::requestScreenShot(fbh, filePath);
			}

			return 0;
		}
		else if (0 == strcmp(_argv[1], "fullscreen") )
		{
			WindowHandle window = { 0 };
			windowToggleFullscreen(window);
			return 0;
		}
	}

#endif
	return 1;
}

rtm_vector<App*>& appGetRegistered();
void appSwitch(App* _app);

int cmdApp(App* _app, void* _userData, int _argc, char const* const* _argv)
{
	RTM_UNUSED_2(_app, _userData);

	if (_argc > 1)
	{
		if (rtm::striCmp(_argv[1], "help") == 0)
		{
			cmdConsoleLog(_app, "app list            - lists registered applications");
			cmdConsoleLog(_app, "app run [idx/name]  - runs application with given index or name");
			return 0;
		}

		rtm_vector<App*>& apps = appGetRegistered();

		if (rtm::striCmp(_argv[1], "list") == 0)
		{
			cmdConsoleLogRGB(127, 255, 255, _app, "Registered apps:");
			uint32_t maxNameLen = 0;
			for (uint32_t i=0; i<apps.size(); ++i)
				maxNameLen = rtm::uint32_max(maxNameLen, (uint32_t)strlen(apps[i]->m_name));

			for (uint32_t i=0; i<apps.size(); ++i)
				cmdConsoleLog(_app, "%2d) %-*s - %s",	i+1,
														maxNameLen,
														apps[i]->m_name,
														apps[i]->m_description);
			return 0;
		}

		if (rtm::striCmp(_argv[1], "run") == 0)
		{
			if (_argc != 3)
				return 1;

			uint32_t appIndex = (uint32_t)atoi(_argv[2]);
			if (appIndex < 1 || appIndex > apps.size())
			{
				// try by name
				for (uint32_t i=0; i<apps.size(); ++i)
					if (rtm::striCmp(_argv[2], apps[i]->m_name) == 0)
					{
						appSwitch(apps[i]);
						return 0;
					}
			}
			else
			{
				App* app = appGet(appIndex - 1);
				appSwitch(app);
				return 0;
			}
		}
	}
	
	return 1;
}

} // namespace rapp
