//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_CONSOLE_H
#define RTM_RAPP_CONSOLE_H

#if RAPP_WITH_BGFX

#include <dear-imgui/imgui.h>
#include <rapp/src/cmd.h>

namespace rapp {

struct Console
{
	static const uint32_t	s_bufferHeight	= (1 << 12);
	static const uint32_t	s_bufferMask	= s_bufferHeight - 1;

	struct Color { uint8_t r, g, b; };

	App*						m_app;
	char						m_inputBuf[256];
	char**						m_items;
	Color*						m_itemColors;
	uint32_t					m_itemsStart;
	uint32_t					m_itemsEnd;
	bool						m_scrollToBottom;
	ImVector<char*>				m_history;
	int							m_historyPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<CmdContext::Func>	m_commands;
	bool						m_hide;
	float						m_visible;

	Console(App* _app);
	~Console();

	void clearLog();
	void addLog(const char* fmt, ...);
	void addLog(uint8_t _r, uint8_t _g, uint8_t _b, const char* fmt, ...);
	void draw();
	void execCommand(const char* command_line);
    int  textEditCallback(ImGuiInputTextCallbackData* data);
	void toggleVisibility();
	void updateCommands();

	static int textEditCallbackStub(ImGuiInputTextCallbackData* data);
};

} // namespace rapp

#endif // RAPP_WITH_BGFX

#endif // RTM_RAPP_CONSOLE_H
