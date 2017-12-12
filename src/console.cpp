//--------------------------------------------------------------------------//
/// Copyright (c) 2017 by Milos Tosic. All Rights Reserved.                ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>

#include <rapp/inc/rapp.h>
#include <rapp/src/console.h>
#include <rapp/src/cmd.h>

#if RAPP_WITH_BGFX

namespace rapp {

static char* Strdup(const char *str)
{
	size_t len = strlen(str) + 1;
	void* buff = rtm_alloc(len);
	return (char*)memcpy(buff, (const void*)str, len);
}

Console::Console()
{
	m_hide		= false;
	m_visible	= 1.0f;

    clearLog();
    memset(m_inputBuf, 0, sizeof(m_inputBuf));
    m_historyPos = -1;

	updateCommands();
}

Console::~Console()
{
    clearLog();
    for (int i = 0; i < m_history.Size; i++)
        rtm_free(m_history[i]);
}

void Console::clearLog()
{
    for (int i=0; i<m_items.Size; i++)
        rtm_free(m_items[i]);
    m_items.clear();
    m_scrollToBottom = true;
}

void Console::addLog(const char* fmt, ...)
{
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf)-1] = 0;
    va_end(args);
    m_items.push_back(Strdup(buf));
    m_scrollToBottom = true;
}

extern float g_consoleToggleTime;

void Console::draw(App* _app)
{
	static float lastDrawTime = rtm::CPU::time();

	float currDrawTime = rtm::CPU::time();
	float delta = (currDrawTime - lastDrawTime);

	if (m_hide && m_visible > 0.0f)
	{
		m_visible -= delta/g_consoleToggleTime;
		m_visible = m_visible < 0.0f ? 0.0f : m_visible;
	}

	if (!m_hide && m_visible < 1.0f)
	{
		m_visible += delta/g_consoleToggleTime;
		m_visible = m_visible > 1.0f ? 1.0f : m_visible;
	}

	lastDrawTime = currDrawTime;

	float posY = -(1.0f - m_visible) * float(_app->m_height/2);
	ImGui::SetNextWindowPos(ImVec2(float(_app->m_width/5), posY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(float(_app->m_width*6/10), float(_app->m_height/2)), ImGuiCond_Always);
	bool p_open;
    if (!ImGui::Begin("", &p_open,	ImGuiWindowFlags_NoTitleBar |
									ImGuiWindowFlags_NoResize	|
									ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

	bool copy_to_clipboard = false;

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetStyle().ItemSpacing.y - ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear")) clearLog();
        if (ImGui::Selectable("Copy")) copy_to_clipboard = true;
	    if (ImGui::Selectable("Scroll to bottom")) m_scrollToBottom = true;
        ImGui::EndPopup();
    }

    // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
    // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
    // To use the clipper we could replace the 'for (int i = 0; i < m_items.Size; i++)' loop with:
    //     ImGuiListClipper clipper(m_items.Size);
    //     while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
    // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
    // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
    // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
    if (copy_to_clipboard)
        ImGui::LogToClipboard();

    for (int i = 0; i < m_items.Size; i++)
    {
        const char* item = m_items[i];
        ImVec4 col = ImVec4(1.0f,1.0f,1.0f,1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
        if (rtm::stristr(item, "[ERROR]")) col = ImColor(1.0f,0.4f,0.4f,1.0f);
        else if (rtm::strncmp(item, "$> ", 2) == 0) col = ImColor(0.58f,1.0f,0.58f,1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(item);
        ImGui::PopStyleColor();
    }

	if (copy_to_clipboard)
        ImGui::LogFinish();

    if (m_scrollToBottom)
        ImGui::SetScrollHere();

    m_scrollToBottom = false;
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    // Command-line
	if (m_visible == 0.0f || m_visible == 1.0f)
    if (ImGui::InputText("<$", m_inputBuf, IM_ARRAYSIZE(m_inputBuf), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory, &textEditCallbackStub, (void*)this))
    {
        char* input_end = m_inputBuf+strlen(m_inputBuf);
        while (input_end > m_inputBuf && input_end[-1] == ' ') { input_end--; } *input_end = 0;
        if (m_inputBuf[0])
            execCommand(m_inputBuf);
        strcpy(m_inputBuf, "");
    }

    // Demonstrate keeping auto focus on the input box
    if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    ImGui::End();
}

void Console::execCommand(const char* command_line)
{
    addLog("$> %s\n", command_line);

    // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
    m_historyPos = -1;
    for (int i = m_history.Size-1; i >= 0; i--)
        if (rtm::strincmp(m_history[i], command_line) == 0)
        {
            rtm_free(m_history[i]);
            m_history.erase(m_history.begin() + i);
            break;
        }
    m_history.push_back(Strdup(command_line));

    // Process command
    if (rtm::strincmp(command_line, "CLEAR") == 0)
    {
        clearLog();
    }
    else
	if (rtm::strincmp(command_line, "HELP") == 0)
    {
        addLog("Available commands:");
        for (int i = 0; i < m_commands.Size; i++)
            addLog("- %s", m_commands[i]);
    }
    else if (rtm::strincmp(command_line, "HISTORY") == 0)
    {
        int first = m_history.Size - 10;
        for (int i = first > 0 ? first : 0; i < m_history.Size; i++)
            addLog("%3d: %s\n", i, m_history[i]);
    }
    else
    {
		int error;
		if (!cmdExec(command_line, &error))
	        addLog("Unknown command: '%s'\n", command_line);
		else
		{
			if (error)
		        addLog("Command '%s' returned error code: %d\n", command_line, error);
		}
    }
}

int Console::textEditCallbackStub(ImGuiTextEditCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
{
    Console* console = (Console*)data->UserData;
    return console->textEditCallback(data);
}

int Console::textEditCallback(ImGuiTextEditCallbackData* data)
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
            ImVector<const char*> candidates;
            for (int i = 0; i < m_commands.Size; i++)
                if (rtm::strincmp(m_commands[i], word_start, (int)(word_end-word_start)) == 0)
                    candidates.push_back(m_commands[i]);

            if (candidates.Size == 0)
            {
                // No match
                addLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
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
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                addLog("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
                    addLog("- %s\n", candidates[i]);
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
    m_commands.push_back("clear");
    m_commands.push_back("history");

	CmdContext* context = cmdGetContext();

	if (!context) return;

	for (auto& i : context->m_lookup)
	{
		m_commands.push_back(i.second.m_name);
	}
}

} // namespace rapp

#endif // RAPP_WITH_BGFX
