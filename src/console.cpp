//--------------------------------------------------------------------------//
/// Copyright (c) 2018 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/console.h>
#include <rapp/src/cmd.h>

#if RAPP_WITH_BGFX

#include <dear-imgui/imgui.h>

namespace rapp {

static char* strdup(const char *str)
{
	size_t len = strlen(str) + 1;
	void* buff = rtm_alloc(len);
	return (char*)memcpy(buff, (const void*)str, len);
}

Console::Console(App* _app)
	: m_app(_app)
{
	m_hide				= true;
	m_visible			= 0.0f;

	m_items				= (char**)rtm_alloc(sizeof(char*) * s_bufferHeight);
	m_itemColors		= (Color*)rtm_alloc(sizeof(Color) * s_bufferHeight);
	m_itemsStart		= 0;
	m_itemsEnd			= 0;

	memset(m_items,			0, sizeof(char*) * s_bufferHeight);
	memset(m_itemColors, 0xff, sizeof(Color) * s_bufferHeight);

    memset(m_inputBuf, 0, sizeof(m_inputBuf));
    m_historyPos = -1;

	updateCommands();
}

Console::~Console()
{
    clearLog();
    for (int i = 0; i < m_history.Size; i++)
    {
        rtm_free(m_history[i]);
    }
	rtm_free(m_items);
	rtm_free(m_itemColors);
}

void Console::clearLog()
{
	uint32_t numLogs = (m_itemsEnd - m_itemsStart) & s_bufferMask;
    for (uint32_t i=0; i<numLogs; ++i)
	{
		uint32_t index = (m_itemsStart + i) % s_bufferMask;
		rtm_free(m_items[index]);
		m_items[index] = 0;
	}

	m_itemsStart		= 0;
	m_itemsEnd			= 0;
    m_scrollToBottom	= true;
}

void Console::addLog(const char* fmt, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, RTM_NUM_ELEMENTS(buf), fmt, args);
    buf[RTM_NUM_ELEMENTS(buf)-1] = 0;
    va_end(args);

	addLog(255, 255, 255, buf);
}

void Console::addLog(uint8_t _r, uint8_t _g, uint8_t _b, const char* fmt, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, RTM_NUM_ELEMENTS(buf), fmt, args);
    buf[RTM_NUM_ELEMENTS(buf)-1] = 0;
    va_end(args);

	bool isFull = ((m_itemsEnd - m_itemsStart) & s_bufferMask) == s_bufferMask;

	if (isFull)
		m_itemsStart = (m_itemsStart + 1) & s_bufferMask;

	if (m_items[m_itemsEnd])
		rtm_free(m_items[m_itemsEnd]);

	m_items[m_itemsEnd] = strdup(buf);
	m_itemColors[m_itemsEnd].r = _r;
	m_itemColors[m_itemsEnd].g = _g;
	m_itemColors[m_itemsEnd].b = _b;
	m_itemsEnd = (m_itemsEnd + 1) & s_bufferMask;

    m_scrollToBottom = true;
}

extern float g_consoleToggleTime;

void Console::draw()
{
	static float lastDrawTime = rtm::CPU::time();

	float currDrawTime = rtm::CPU::time();
	float delta = (currDrawTime - lastDrawTime);
	if (delta < 0.006f)
		delta = 0.006f;
    
	if (m_hide && (m_visible > 0.0f))
	{
		m_visible -= delta/g_consoleToggleTime;
		m_visible = m_visible < 0.0f ? 0.0f : m_visible;
	}

	if (!m_hide && m_visible < 1.0f)
	{
		m_visible += delta/g_consoleToggleTime;
		m_visible = m_visible > 1.0f ? 1.0f : m_visible;
	}

	if (m_visible == 0.0f)
		return;
    
	lastDrawTime = currDrawTime;

	float posY = -(1.0f - m_visible) * float(m_app->m_height/2);
	ImGui::SetNextWindowPos(ImVec2(float(m_app->m_width/5), posY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(float(m_app->m_width*6/10), float(m_app->m_height/2)), ImGuiCond_Always);
	bool p_open;
    if (!ImGui::Begin("Console", &p_open,	ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoResize	|
											ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

	bool copy_to_clipboard = false;

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetStyle().ItemSpacing.y - ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear")) clearLog();
        if (ImGui::Selectable("Copy")) copy_to_clipboard = true;
	    if (ImGui::Selectable("Scroll to bottom")) m_scrollToBottom = true;
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
	ImGui::PushFont(ImGui::Font::Mono);

    if (copy_to_clipboard)
    {
        ImGui::LogToClipboard();
    }

	uint32_t numLogs = (m_itemsEnd - m_itemsStart) & s_bufferMask;
	ImGuiListClipper clipper;
	clipper.Begin(numLogs);
	while (clipper.Step())
	for (int i=clipper.DisplayStart; i<clipper.DisplayEnd; i++)
	{
		uint32_t index = (m_itemsStart + i) & s_bufferMask;
		const char* item = m_items[index];
		ImVec4 col = ImColor(	float(m_itemColors[index].r)/255.0f,
								float(m_itemColors[index].g)/255.0f,
								float(m_itemColors[index].b)/255.0f,
								1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}

	clipper.End();

	if (copy_to_clipboard)
        ImGui::LogFinish();

    if (m_scrollToBottom)
        ImGui::SetScrollHereY();

    m_scrollToBottom = false;
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    // Command-line
	if (m_visible == 1.0f)
	{
		ImGui::Text("$>"); 
		ImGui::SameLine(); 
		ImGui::SetNextItemWidth(-1);

		if (ImGui::InputText("##", m_inputBuf, RTM_NUM_ELEMENTS(m_inputBuf), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory, &textEditCallbackStub, (void*)this))
		{
			char* input_end = m_inputBuf+strlen(m_inputBuf);
			while (input_end > m_inputBuf && input_end[-1] == ' ') { input_end--; } *input_end = 0;
			if (m_inputBuf[0])
				execCommand(m_inputBuf);
			strcpy(m_inputBuf, "");
		}
	}

    // Demonstrate keeping auto focus on the input box
    if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    ImGui::End();
}

void Console::execCommand(const char* command_line)
{
    addLog(147, 255, 147, "$> %s\n", command_line);

    // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
    m_historyPos = -1;
    for (int i = m_history.Size-1; i >= 0; i--)
        if (rtm::striCmp(m_history[i], command_line) == 0)
        {
            rtm_free(m_history[i]);
            m_history.erase(m_history.begin() + i);
            break;
        }
    m_history.push_back(strdup(command_line));

    // Process command
    if (rtm::striCmp(command_line, "CLEAR") == 0)
    {
        clearLog();
    }
    else
	if (rtm::striCmp(command_line, "HELP") == 0)
    {
        addLog("Available commands:");
        for (int i = 0; i < m_commands.Size; i++)
		{
			addLog("> %-*s - %s",	cmdGetContext()->m_maxCommandLength + 3,
									m_commands[i].m_name,
									m_commands[i].m_description);
		}
    }
    else if (rtm::striCmp(command_line, "HISTORY") == 0)
    {
        int first = m_history.Size - 10;
        for (int i = first > 0 ? first : 0; i < m_history.Size; i++)
		{
		    addLog(77, 175, 77, "%3d: %s\n", i, m_history[i]);
		}
    }
    else
    {
		int error;
		if (!cmdExec(m_app, command_line, &error))
	        addLog("Unknown command: '%s'\n", command_line);
		else
		{
			if (error)
		        addLog("Command '%s' returned error code: %d\n", command_line, error);
		}
    }
}

int Console::textEditCallbackStub(ImGuiInputTextCallbackData* data)
{
    Console* console = (Console*)data->UserData;
    return console->textEditCallback(data);
}

int Console::textEditCallback(ImGuiInputTextCallbackData* data)
{
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            ImVector<CmdContext::Func*> candidates;
            for (int i = 0; i < m_commands.Size; i++)
                if (rtm::striCmp(m_commands[i].m_name, word_start, (int)(word_end-word_start)) == 0)
                    candidates.push_back(&m_commands[i]);

            if (candidates.Size == 0)
            {
                // No match
                addLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                data->InsertChars(data->CursorPos, candidates[0]->m_name);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = rtm::toUpper(candidates[i]->m_name[match_len]);
                        else if (c == 0 || c != rtm::toUpper(candidates[i]->m_name[match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0]->m_name, candidates[0]->m_name + match_len);
                }

                // List matches
                addLog("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
					addLog("> %-*s - %s",	cmdGetContext()->m_maxCommandLength + 3,
											candidates[i]->m_name,
											candidates[i]->m_description);
            }

            break;
        }

    case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
            const int prev_history_pos = m_historyPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (m_historyPos == -1)
                    m_historyPos = m_history.Size - 1;
                else if (m_historyPos > 0)
                    m_historyPos--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (m_historyPos != -1)
                    if (++m_historyPos >= m_history.Size)
                        m_historyPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != m_historyPos)
            {
                data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (m_historyPos >= 0) ? m_history[m_historyPos] : "");
                data->BufDirty = true;
            }
        }
    }
    return 0;
}

void Console::toggleVisibility()
{
	m_hide = !m_hide;
}

void Console::updateCommands()
{
	m_commands.clear();
	m_commands.push_back({0, 0, "clear",	"Clears the console buffer"});
	m_commands.push_back({0, 0, "history",	"Lists history of entered commands"});

	CmdContext* context = cmdGetContext();

	if (!context) return;

	for (int i=0; i<RAPP_HASH_SIZE; ++i)
		if (context->m_lookup[i].m_fn)
			m_commands.push_back(context->m_lookup[i]);
}

} // namespace rapp

#endif // RAPP_WITH_BGFX
