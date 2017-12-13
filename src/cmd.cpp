//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/cmd.h>
#include <rapp/src/console.h>
#include <rapp/src/entry_p.h>

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
	RTM_ASSERT(m_lookup.end() == m_lookup.find(cmd), "Command \"%s\" already exist!", _name);
	Func fn = { _fn, _userData, _name, _description };
	m_lookup.insert(std::make_pair(cmd, fn) );
}

void CmdContext::remove(const char* _name)
{
	uint32_t cmd = rtm::hashMurmur3(_name, (uint32_t)strlen(_name) );
	CmdLookup::iterator it = m_lookup.find(cmd);
	RTM_ASSERT(m_lookup.end() != it, "Command \"%s\" does not exist!", _name);
	m_lookup.erase(it);
}

bool CmdContext::exec(const char* _cmd, int* _errorCode)
{
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
				err = fn.m_fn(fn.m_userData, argc, argv);
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

bool cmdExec(const char* _cmd, int* _errorCode)
{
	return s_cmdContext->exec(_cmd, _errorCode);
}

void cmdConsoleToggle(App* _app)
{
	RTM_UNUSED(_app);
#if RAPP_WITH_BGFX
	_app->m_console->toggleVisibility();
#endif // RAPP_WITH_BGFX
}

float g_consoleToggleTime = 0.23f;
void cmdConsoleSetToggleTime(float _time)
{
	RTM_ASSERT(_time > 0.0f && _time < 3.0f, "Really?");
	g_consoleToggleTime = _time;
}

} // namespace rapp
