//--------------------------------------------------------------------------//
/// Copyright 2023 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <rapp_pch.h>
#include <rapp/inc/rapp.h>
#include <rapp/inc/widgets/numpad_debug.h>
#include <inttypes.h>

#ifdef RAPP_WITH_BGFX

namespace rapp {

	static inline uint32_t propertyGetSize(Property::Type _type)
	{
		switch (_type)
		{
			case Property::Uint8:
			case Property::Int8:	return 1;
			case Property::Uint16:
			case Property::Int16:	return 2;
			case Property::Uint32:
			case Property::Int32:	return 4;
			case Property::Uint64:
			case Property::Int64:	return 8;
			case Property::Float:	return sizeof(float);
			case Property::Double:	return sizeof(double);
			case Property::Bool:	return sizeof(bool);
			case Property::Color:	return 16; // 4 floats
			default:	return 0;
		};
	}

	inline void propertyCopyValue(Property::Type _type, void* _dest, void* _src)
	{
		uint32_t copySize = propertyGetSize(_type);
		if (copySize)
			memcpy(_dest, _src, copySize);
	}

	inline bool propertyIsGreater(Property::Type _type, void* _v1, void* _v2)
	{
		switch (_type)
		{
		case Property::Int8:	return  *(int8_t*)_v1 >  *(int8_t*)_v2;
		case Property::Int16:	return *(int16_t*)_v1 > *(int16_t*)_v2;
		case Property::Int32:	return *(int32_t*)_v1 > *(int32_t*)_v2;
		case Property::Int64:	return *(int64_t*)_v1 > *(int64_t*)_v2;
		case Property::Uint8:	return  *(uint8_t*)_v1 >  *(uint8_t*)_v2;
		case Property::Uint16:	return *(uint16_t*)_v1 > *(uint16_t*)_v2;
		case Property::Uint32:	return *(uint32_t*)_v1 > *(uint32_t*)_v2;
		case Property::Uint64:	return *(uint64_t*)_v1 > *(uint64_t*)_v2;
		case Property::Float:	return  *(float*)_v1 >  *(float*)_v2;
		case Property::Double:	return *(double*)_v1 > *(double*)_v2;
		};
		return false;
	}

	void propertyClamp(DebugMenu* _menu)
	{
		if (propertyIsGreater(_menu->m_type, _menu->m_minMax.m_min, _menu->m_value))
			memcpy(_menu->m_value, _menu->m_minMax.m_min, propertyGetSize(_menu->m_type));
		if (propertyIsGreater(_menu->m_type, _menu->m_value, _menu->m_minMax.m_max))
			memcpy(_menu->m_value, _menu->m_minMax.m_max, propertyGetSize(_menu->m_type));
	}

	void propertyPrintToBuffer(ImGuiDataType _type, void* _value, char _buffer[128])
	{
		switch (_type)
		{
		case ImGuiDataType_S8:		sprintf(_buffer, "%" PRId8,   *(int8_t*)_value); break;
		case ImGuiDataType_S16:		sprintf(_buffer, "%" PRId16, *(int16_t*)_value); break;
		case ImGuiDataType_S32:		sprintf(_buffer, "%" PRId32, *(int32_t*)_value); break;
		case ImGuiDataType_S64:		sprintf(_buffer, "%" PRId64, *(int64_t*)_value); break;
		case ImGuiDataType_U8:		sprintf(_buffer, "%" PRIu8,  *(uint8_t*)_value); break;
		case ImGuiDataType_U16:		sprintf(_buffer, "%" PRIu16,*(uint16_t*)_value); break;
		case ImGuiDataType_U32:		sprintf(_buffer, "%" PRIu32,*(uint32_t*)_value); break;
		case ImGuiDataType_U64:		sprintf(_buffer, "%" PRIu64,*(uint64_t*)_value); break;
		case ImGuiDataType_Float:	sprintf(_buffer, "%f",   *(float*)_value); break;
		case ImGuiDataType_Double:	sprintf(_buffer, "%lf", *(double*)_value); break;
		};
	}

	DebugMenu g_rootMenu(0, "Home");
	rapp::DebugMenu* g_currentDebugMenu = 0;

	void initializeMenus()
	{
		g_rootMenu.m_type = Property::None;
		g_currentDebugMenu = &g_rootMenu;
	}

	DebugMenu::DebugMenu(DebugMenu* _parent, const char* _label, uint32_t _width, Property::Type _type)
		: m_parent(_parent)
		, m_index(0)
		, m_numChildren(0)
		, m_label(_label)
		, m_type(_type)
		, m_width(_width)
		, m_customEditor(0)
		, m_customPreview(0)
		, m_minmaxSet(false)
	{
		static int initialized = 0;
		if (initialized++ == 1)
		{
			initializeMenus();
		}

		if (!_parent)
			m_parent = g_currentDebugMenu;

		for (int i=0; i<8; ++i)
			m_children[i] = 0;

		if (m_width > 1)
			if (!((_type >= Property::Int8) && (_type <= Property::Double)))
			{
				RTM_BREAK;
			}

		if (m_parent)
		{
			if (m_parent->m_numChildren == 8)	// ignore more than 8 child menus
				return;
			m_index = m_parent->m_numChildren;
			if (m_parent)
				m_parent->m_children[m_parent->m_numChildren++] = this;
		}
		else
			m_index = 0;
	}

	DebugMenu::DebugMenu(DebugMenu* _parent, const char* _label, Property::Type _type, void* _pointer
						, void* _defaultValue
						, void* _minValue
						, void* _maxValue
						, CustomDfltFn	_customDefault
						, CustomEditFn	_customEditor
						, CustomPrevFn	_customPreview)
		: m_parent(_parent)
		, m_index(0)
		, m_numChildren(0)
		, m_label(_label)
		, m_type(_type)
		, m_width(1)
		, m_value(_pointer)
		, m_customEditor(_customEditor)
		, m_customPreview(_customPreview)
		, m_minmaxSet(false)
	{
		if (m_parent->m_numChildren == 8)	// ignore more than 8 child menus
			return;
		for (int i=0; i<8; ++i)
			m_children[i] = 0;
		m_index = m_parent->m_numChildren;
		m_parent->m_children[m_parent->m_numChildren++] = this;

		if (Property::Color == _type)
		{
			_defaultValue = (float*)*(float**)_defaultValue;
			if (_minValue && _maxValue)
			{
				_minValue = (float*)*(float**)_minValue;
				_maxValue = (float*)*(float**)_maxValue;
			}
		}

		if (_defaultValue)
			propertyCopyValue(_type, m_value, _defaultValue);

		if (_customDefault)
			_customDefault(m_value);

		if (_minValue && _minValue)
		{
			propertyCopyValue(_type, m_minMax.m_min, _minValue);
			propertyCopyValue(_type, m_minMax.m_max, _maxValue);
			m_minmaxSet = true;
		}
	}

	ImGuiDataType rappToImGuiScalarType(Property::Type _type)
	{
		switch (_type)
		{
		case Property::Int8:	return ImGuiDataType_S8;
		case Property::Int16:	return ImGuiDataType_S16;
		case Property::Int32:	return ImGuiDataType_S32;
		case Property::Int64:	return ImGuiDataType_S64;
		case Property::Uint8:	return ImGuiDataType_U8;
		case Property::Uint16:	return ImGuiDataType_U16;
		case Property::Uint32:	return ImGuiDataType_U32;
		case Property::Uint64:	return ImGuiDataType_U64;
		case Property::Float:	return ImGuiDataType_Float;
		case Property::Double:	return ImGuiDataType_Double;
		};
		RTM_BREAK;
		return ImGuiDataType_COUNT;
	}

	void rappPrintValueType(ImGuiDataType _type, void* _value)
	{
		char text[128];
		propertyPrintToBuffer(_type, _value, text);
		ImGui::TextColored(ImColor(RAPP_COLOR_TEXT), text);
	}

	void rappAddCentered_text(const char* _text, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size)
	{
		float fontSizeNumber	= ImGui::GetFontSize() * 1.35f;
		ImVec2 text_size(1000.0f, 1000.0f);
		while (text_size.y > _size.y)
		{
			fontSizeNumber *= 0.9f;
			text_size = ImGui::GetFont()->CalcTextSizeA(fontSizeNumber, FLT_MAX, 0.0f, _text);
		}
		_drawList->PushClipRect(_minXY, ImVec2(_minXY.x + _size.x, _minXY.y + _size.y));
		ImVec2 textPos(_minXY.x + (_size.x - text_size.x)/2, _minXY.y + (_size.y - text_size.y)/2);
		_drawList->AddText(0, fontSizeNumber, textPos,  RAPP_COLOR_TEXT, _text);
		_drawList->PopClipRect();
	}


	static int cmdKey(rapp::App* /*_app*/, void* /*_userData*/, int /*_argc*/, char const* const* /*_argv*/)
	{
		return 0;
	}

	void rappDebugAddBindings()
	{
		static bool inputBindingsRegistered = false;
		if (inputBindingsRegistered)
			return;
		inputBindingsRegistered = true;

		static const rapp::InputBinding bindings[] =
		{
			{ 0, "num0",	1, { rapp::KeyboardState::Key::NumPad0,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num1",	1, { rapp::KeyboardState::Key::NumPad1,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num2",	1, { rapp::KeyboardState::Key::NumPad2,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num3",	1, { rapp::KeyboardState::Key::NumPad3,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num4",	1, { rapp::KeyboardState::Key::NumPad4,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num5",	1, { rapp::KeyboardState::Key::NumPad5,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num6",	1, { rapp::KeyboardState::Key::NumPad6,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num7",	1, { rapp::KeyboardState::Key::NumPad7,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num8",	1, { rapp::KeyboardState::Key::NumPad8,	rapp::KeyboardState::Modifier::NoMods }},
			{ 0, "num9",	1, { rapp::KeyboardState::Key::NumPad9,	rapp::KeyboardState::Modifier::NoMods }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("debug_bindings", bindings);
		rapp::cmdAdd("num0",	cmdKey, (void*)KeyboardState::Key::NumPad0,	"");
		rapp::cmdAdd("num1",	cmdKey, (void*)KeyboardState::Key::NumPad1,	"");
		rapp::cmdAdd("num2",	cmdKey, (void*)KeyboardState::Key::NumPad2,	"");
		rapp::cmdAdd("num3",	cmdKey, (void*)KeyboardState::Key::NumPad3,	"");
		rapp::cmdAdd("num4",	cmdKey, (void*)KeyboardState::Key::NumPad4,	"");
		rapp::cmdAdd("num5",	cmdKey, (void*)KeyboardState::Key::NumPad5,	"");
		rapp::cmdAdd("num6",	cmdKey, (void*)KeyboardState::Key::NumPad6,	"");
		rapp::cmdAdd("num7",	cmdKey, (void*)KeyboardState::Key::NumPad7,	"");
		rapp::cmdAdd("num8",	cmdKey, (void*)KeyboardState::Key::NumPad8,	"");
		rapp::cmdAdd("num9",	cmdKey, (void*)KeyboardState::Key::NumPad9,	"");
	}

	static void rappPropertyTypeEditor_scalar(DebugMenu* _menu, ImGuiDataType _type, int _cnt = -1)
	{
		if (!_menu->m_value)
		{
			for (uint32_t i=0; i<_menu->m_width; ++i)
				rappPropertyTypeEditor_scalar(_menu->m_children[i], rappToImGuiScalarType(_menu->m_type), i);
		}
		else
		{
			if (_menu->m_minmaxSet)
			{
				char name1[6]; strcpy(name1, "##00"); name1[3] = (char)('0' + _cnt);
				char name2[6]; strcpy(name2, "##10"); name2[3] = (char)('0' + _cnt);
				ImGui::InputScalar(name1, _type, _menu->m_value);
				propertyClamp(_menu);
				ImGui::SliderScalar(name2, _type, _menu->m_value, _menu->m_minMax.m_min, _menu->m_minMax.m_max);
				ImGui::TextColored(ImColor(RAPP_COLOR_TEXT_HIGHLIGHT), "Min:");
				ImGui::SameLine();
				rappPrintValueType(_type, _menu->m_minMax.m_min);
				ImGui::SameLine();
				ImGui::TextColored(ImColor(RAPP_COLOR_TEXT_HIGHLIGHT), "Max:");
				ImGui::SameLine();
				rappPrintValueType(_type, _menu->m_minMax.m_max);
			}
			else
				ImGui::InputScalar("", _type, _menu->m_value);
		}
	}

	static void rappPropertyPreview_scalar(DebugMenu* _menu, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size, ImGuiDataType _type)
	{
		char text[128];
		char textFull[128*4] = "";

		if (!_menu->m_value)
		{
			for (uint32_t i=0; i<_menu->m_width; ++i)
			{
				propertyPrintToBuffer(_type, _menu->m_children[i]->m_value, text);
				strcat(textFull, text);
				if (i != _menu->m_width)
					strcat(textFull, "\n");
			}
		}
		else
			propertyPrintToBuffer(_type, _menu->m_value, textFull);

		rappAddCentered_text(textFull, _drawList, _minXY, _size);
	}
	
	static void rappPropertyTypeEditor_Bool(DebugMenu* _menu)
	{
	    bool* valuePtr = (bool*)_menu->m_value;
		int b = *valuePtr ? 1 : 0;
		ImGui::RadioButton("False", &b, 0);
		ImGui::SameLine();
		ImGui::RadioButton("True", &b, 1);
		*valuePtr = b == 1 ? true : false;
	}

	static void rappPropertyPreview_Bool(DebugMenu* _menu, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size)
	{
		bool value = *(bool*)_menu->m_value;
		rappAddCentered_text(value ? "True" : "False", _drawList, _minXY, _size);
	}
	
	static void rappPropertyTypeEditor_Color(DebugMenu* _menu)
	{
		float* valuesPtr = (float*)_menu->m_value;
		float col[4] = {0};
		for (int i=0; i<4; ++i) col[i] = valuesPtr[i];
		ImGui::ColorPicker4("", col, ImGuiColorEditFlags_PickerHueWheel);
		for (int i=0; i<4; ++i) valuesPtr[i] = col[i];
	}

	static void rappPropertyPreview_Color(DebugMenu* _menu, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size)
	{
		float* rgb = (float*)_menu->m_value;
		ImU32 color = ImColor(ImVec4(rgb[0], rgb[1], rgb[2], rgb[3]));
		float extent = _size.x < _size.y ? _size.x : _size.y;
		ImVec2 rectPos(_minXY.x + (_size.x- extent)/2, _minXY.y + (_size.y- extent)/2);
		ImVec2 rectSize(rectPos.x + extent, rectPos.y + extent);
		_drawList->AddRectFilled(rectPos, rectSize, RAPP_COLOR_WIDGET_OUTLINE);
		rectPos.x += RAPP_WIDGET_OUTLINE;	rectPos.y += RAPP_WIDGET_OUTLINE;
		rectSize.x -= RAPP_WIDGET_OUTLINE;	rectSize.y -= RAPP_WIDGET_OUTLINE;
		_drawList->AddRectFilled(rectPos, rectSize, color);
	}

	static Property::Edit rappPropertyTypeEditorDispath(DebugMenu* _menu, ImVec2 _minXY, ImVec2 _size)
	{
		ImGui::OpenPopup("##propEditor");
		ImGui::SetNextWindowPos(_minXY);
		ImGui::SetNextWindowSize(_size,  ImGuiCond_Always);
		if (ImGui::BeginPopup("##propEditor", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 maxXY(_minXY.x+_size.x, _minXY.y+_size.y);
			draw_list->AddRectFilled(_minXY, maxXY, RAPP_COLOR_WIDGET_OUTLINE);
			_minXY = ImVec2(_minXY.x + RAPP_WIDGET_OUTLINE*3, _minXY.y + RAPP_WIDGET_OUTLINE*2);
			maxXY = ImVec2(maxXY.x - (RAPP_WIDGET_OUTLINE*3), maxXY.y - (RAPP_WIDGET_OUTLINE*2));
			draw_list->AddRectFilled(_minXY, maxXY, RAPP_COLOR_WIDGET);

			ImGui::TextColored(ImColor(RAPP_COLOR_TEXT_TITLE), "  %s", _menu->m_label);
			ImGui::Separator();
			ImGui::BeginChild("##propEditorInner",ImVec2(),false,ImGuiWindowFlags_NoScrollbar);
			switch (_menu->m_type)
			{
				case Property::Int8:	
				case Property::Int16:	
				case Property::Int32:	
				case Property::Int64:	
				case Property::Uint8:	
				case Property::Uint16:	
				case Property::Uint32:	
				case Property::Uint64:	
				case Property::Float:	
				case Property::Double:	rappPropertyTypeEditor_scalar(_menu, rappToImGuiScalarType(_menu->m_type));	break;
				case Property::Bool:	rappPropertyTypeEditor_Bool(_menu);		break;
				case Property::Color:	rappPropertyTypeEditor_Color(_menu);	break;
#if RAPP_DEBUG_WITH_STD_STRING
				case Property::StdString:
#endif // RAPP_DEBUG_WITH_STD_STRING
				case Property::Custom:	_menu->m_customEditor(_menu);			break;
				default:
					RTM_BREAK;
					break;
			};
			ImGui::EndChild();
			ImGui::EndPopup();
		}

		if (rapp::inputGetKeyState(rapp::KeyboardState::Key::Esc))
			return Property::Cancel;

		if (rapp::inputGetKeyState(rapp::KeyboardState::Key::Return)	||
			rapp::inputGetKeyState(rapp::KeyboardState::Key::NumPad5))
			return Property::Accept;

		return Property::Editing;
	}

	static bool rappPropertyTypePreviewDispath(DebugMenu* _menu, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size)
	{
		if (!_menu)
			return false;

		if (_menu->m_parent && (_menu->m_parent->m_width > 1))
			return false;

		switch (_menu->m_type)
		{
			case Property::Int8:	
			case Property::Int16:	
			case Property::Int32:	
			case Property::Int64:	
			case Property::Uint8:	
			case Property::Uint16:	
			case Property::Uint32:	
			case Property::Uint64:	
			case Property::Float:	
			case Property::Double:	rappPropertyPreview_scalar(_menu, _drawList, _minXY, _size, rappToImGuiScalarType(_menu->m_type)); return true;
			case Property::Bool:	rappPropertyPreview_Bool(_menu, _drawList, _minXY, _size); return true;
			case Property::Color:	rappPropertyPreview_Color(_menu, _drawList, _minXY, _size); return true;
#if RAPP_DEBUG_WITH_STD_STRING
			case Property::StdString:
#endif // RAPP_DEBUG_WITH_STD_STRING
			case Property::Custom:	if (_menu->m_customPreview)
									{
										_menu->m_customPreview(_menu, _drawList, _minXY, _size);
										return true;
									}
									else
										return false;
			default:
				return false;
		};
	}

#if RAPP_DEBUG_WITH_STD_STRING
	void stringSetDefaultFn(void* _var)
	{
		*(std::string*)_var = "";
	}

	void stringEditorFn(void* _var)
	{
		std::string* var = (std::string*)_var;
		char tempString[4096];
		strcpy(tempString, var->c_str());
		ImGui::InputText("", tempString, 4096);
		*var = tempString;
	}

	void stringPreviewFn(void* _var, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size)
	{
		std::string* value = (std::string*)_var;
		rapp::rappAddCentered_text(value->c_str(), _drawList, _minXY, _size);
	}
#endif // RAPP_DEBUG_WITH_STD_STRING

	static bool rappRoundedButton(DebugMenu* _menu, ImDrawList* _drawList, ImVec2& _position, ImVec2& _size, int _button, const char* _label, bool _editingValue)
	{
		static uint64_t s_lastHoverTime[9] = {0};
		static uint64_t s_lastClickTime[9] = {0};
		static bool		s_hovered[9] = {false};
	
		ImVec2 maxC(_position.x + _size.x, _position.y + _size.y);
		_drawList->AddRectFilled(_position, maxC, RAPP_COLOR_WIDGET_OUTLINE, RAPP_WIDGET_OUTLINE * 3);

		_position.x += RAPP_WIDGET_OUTLINE;
		_position.y += RAPP_WIDGET_OUTLINE;
		maxC.x -= RAPP_WIDGET_OUTLINE;
		maxC.y -= RAPP_WIDGET_OUTLINE;

		ImU32 drawColor = RAPP_COLOR_WIDGET;

		ImVec2 mousePos(ImGui::GetIO().MousePos);
		uint64_t currTime = rtm::CPU::clock();
		bool hover = false;

		if (ImGui::IsMouseHoveringRect(_position, ImVec2(maxC.x, maxC.y)))
		{
			if (!s_hovered[_button])
				s_lastHoverTime[_button] = currTime;

			if (!_editingValue)
			flashColorElapsed(drawColor, RAPP_COLOR_WIDGET_BLINK, currTime - s_lastHoverTime[_button]);
			hover = true;
		}

		if (hover && ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			s_lastClickTime[_button] = currTime;

		if (s_hovered[_button] && (hover == false))
			s_lastHoverTime[_button] = 0;

		s_hovered[_button] = hover;

		uint64_t deltaTime = currTime - s_lastClickTime[_button];

		if (!_editingValue)
		flashColorElapsed(drawColor, RAPP_COLOR_WIDGET_HIGHLIGHT, deltaTime);
		_drawList->AddRectFilled(_position, maxC, drawColor, RAPP_WIDGET_OUTLINE * 3);

		ImU32 numberColor = _button == 5 ? RAPP_COLOR_WIDGET : RAPP_COLOR_TEXT_STATUS;
		if (!_editingValue)
		flashColorElapsed(numberColor, _button == 5 ? RAPP_COLOR_TEXT_SHADOW : RAPP_COLOR_TEXT, deltaTime*2);

		char FK[] = "0";
		FK[0] = (char)('0' + _button);
		static float fontSizeNumber	= ImGui::GetFontSize() * 1.66f;
		static float fontSizeLabel	= ImGui::GetFontSize() * 1.11f;
		ImVec2 text_size = ImGui::GetFont()->CalcTextSizeA(fontSizeNumber, FLT_MAX, 0.0f, FK);
		ImVec2 center = ImVec2(_position.x + _size.x/2, _position.y + _size.y/ 2);
		ImVec2 text_pos = ImVec2(center.x - text_size.x/2, center.y - text_size.y);
		ImVec2 previewPosition(center.x - _size.x/2 + 6, center.y - _size.y/2 + 6);
		ImVec2 previewSize(_size.x - 12, _size.y/2 - 12);
		bool previewDrawn = rappPropertyTypePreviewDispath(_menu, _drawList, previewPosition, previewSize);
		if (!previewDrawn)
			_drawList->AddText(0, fontSizeNumber, text_pos, numberColor, FK);

		text_size = ImGui::GetFont()->CalcTextSizeA(fontSizeLabel, FLT_MAX, 0.0f, _label);
		text_pos = ImVec2(center.x - text_size.x/2, center.y+4);

		drawColor = RAPP_COLOR_TEXT_HIGHLIGHT;
		flashColorElapsed(drawColor, IM_COL32_WHITE, deltaTime);

		if (!_editingValue)
			_drawList->AddText(0, fontSizeLabel, text_pos, drawColor, _label);

		return s_lastClickTime[_button] == currTime;
	}

	void rappDebugMenu()
	{
		rappDebugAddBindings();

		DebugMenu* ret = g_currentDebugMenu;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));

		ImVec2 screenRes = ImGui::GetIO().DisplaySize;
		ImVec2 buttonSize = ImVec2(128.0f,128.0f);
		screenRes.x -= (buttonSize.x + RAPP_WIDGET_SPACING) * 3;
		screenRes.y -= (buttonSize.y + RAPP_WIDGET_SPACING) * 3;

		ImGui::SetNextWindowPos(screenRes);

		static uint32_t remapIndex[] = {5, 6, 5, 4, 7, 5, 3, 0, 1, 2 };
		static uint32_t remapIndexChild[] = {7,8,9,6,3,2,1,4};

		bool editingValue = ((g_currentDebugMenu->m_numChildren == 0) && (g_currentDebugMenu->m_type != Property::None) ||
							 (g_currentDebugMenu->m_width > 1));

		static bool oldEditingValue = editingValue;

		ImGui::SetNextWindowSize(ImVec2(buttonSize.x * 3 + RAPP_WIDGET_SPACING * 3,buttonSize.y * 3 + RAPP_WIDGET_SPACING * 3));
		if (ImGui::Begin("##rappDebugMenu", 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			const ImVec2 drag_delta = ImVec2(ImGui::GetIO().MousePos.x - screenRes.x, ImGui::GetIO().MousePos.y - screenRes.y);
			const float drag_dist2 = drag_delta.x*drag_delta.x + drag_delta.y*drag_delta.y;

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->PushClipRectFullScreen();

			uint32_t	editIndex = 0xffffffff;

			static bool oldPressed[9] = {false};
			for (uint32_t x=0; x<3; ++x)
			for (uint32_t y=0; y<3; ++y)
			{
				ImVec2 minC(screenRes.x + buttonSize.x * x + RAPP_WIDGET_SPACING * x, screenRes.y + buttonSize.y * y + RAPP_WIDGET_SPACING * y);

				uint32_t displayIndex = 9 - (y*3 + (2-x));
				uint32_t childIndex = remapIndex[displayIndex];
				DebugMenu* child = g_currentDebugMenu->m_numChildren > childIndex ? g_currentDebugMenu->m_children[childIndex] : 0;

				bool validButton = false;
				const char* label = "";
				if (childIndex < (int)g_currentDebugMenu->m_numChildren)
				{
					validButton = true;
					label = child->m_label;
				}

				if ((displayIndex == 5) && (g_currentDebugMenu->m_parent))
					label = g_currentDebugMenu->m_parent->m_label;

				if (editingValue && (displayIndex != 5) && (g_currentDebugMenu->m_index == childIndex))
				{
					editIndex	= childIndex;
					label		= g_currentDebugMenu->m_label;
				}

	bool keyPressed = rapp::inputGetKeyState(rapp::KeyboardState::Key(rapp::KeyboardState::Key::NumPad0 + displayIndex));

				if (rappRoundedButton(child, draw_list, minC, buttonSize, displayIndex, label, editingValue) && !editingValue)
				{
					if ((5 == displayIndex) && g_currentDebugMenu->m_parent) // 5, go up a level
						ret = g_currentDebugMenu->m_parent;
					else
					if (childIndex < (int)g_currentDebugMenu->m_numChildren)
						ret = child;
				}

	keyPressed = oldPressed[displayIndex];
			}

			draw_list->PopClipRect();

			ImGui::End();

			static uint8_t		origValue[DebugMenu::MAX_STORAGE_SIZE][DebugMenu::MAX_VARIABLES]; // for undo
			static DebugMenu*	editedMenu = 0;
			static bool			undoNeeded = false;
			if (editingValue && (oldEditingValue != editingValue)) // started to edit
			{
				editedMenu = g_currentDebugMenu;
				if (g_currentDebugMenu->m_width > 1)
				{
					for (uint32_t i=0; i<g_currentDebugMenu->m_width; ++i)
						propertyCopyValue(g_currentDebugMenu->m_type, origValue[i], g_currentDebugMenu->m_children[i]->m_value);
				}
				else
					propertyCopyValue(g_currentDebugMenu->m_type, origValue, g_currentDebugMenu->m_value);
			}

			if (!editingValue && (oldEditingValue != editingValue)) // finished edit
			{
				if (undoNeeded)
				{
					if (editedMenu->m_width > 1)
					{
						for (uint32_t i=0; i<editedMenu->m_width; ++i)
							propertyCopyValue(editedMenu->m_type, editedMenu->m_children[i]->m_value, origValue[i]);
					}
					else
						propertyCopyValue(editedMenu->m_type, editedMenu->m_value, origValue);
				}
				editedMenu = 0;
				undoNeeded = false;
			}

			Property::Edit editRet = Property::Editing;
			if (editingValue && g_currentDebugMenu->m_index == editIndex)
				editRet = rappPropertyTypeEditorDispath(g_currentDebugMenu, ImVec2(screenRes.x + RAPP_WIDGET_SPACING * 2, screenRes.y + RAPP_WIDGET_SPACING * 2),
																			ImVec2(buttonSize.x * 3 - RAPP_WIDGET_SPACING * 2, buttonSize.y * 3 - RAPP_WIDGET_SPACING * 2));

			if (editRet == Property::Cancel)
				undoNeeded = true;

			if (editRet != Property::Editing)
				ret = g_currentDebugMenu->m_parent;

			oldEditingValue = editingValue;
		}
		ImGui::PopStyleColor(2);
		g_currentDebugMenu = ret;
	}

} // namespace rapp

#endif // RAPP_WITH_BGFX
