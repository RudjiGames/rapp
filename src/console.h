//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#if RAPP_WITH_BGFX

#include <ocornut-imgui/imgui.h>

namespace rapp {

struct Console
{
	char					m_inputBuf[256];
	char**					m_items;
	uint32_t				m_itemsStart;
	uint32_t				m_itemsEnd;
	static const uint32_t	s_bufferHeight	= (1 << 12);
	static const uint32_t	s_bufferMask	= s_bufferHeight - 1;

	bool					m_scrollToBottom;
	ImVector<char*>			m_history;
	int						m_historyPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<const char*>	m_commands;
	bool					m_hide;
	float					m_visible;

	Console();
	~Console();

	void clearLog();
	void addLog(const char* fmt, ...) IM_FMTARGS(2);
	void draw(App* _app);
	void execCommand(const char* command_line);
    int  textEditCallback(ImGuiTextEditCallbackData* data);
	void toggleVisibility();
	void updateCommands();

	static int textEditCallbackStub(ImGuiTextEditCallbackData* data);
};

} // namespace rapp

#endif // RAPP_WITH_BGFX
