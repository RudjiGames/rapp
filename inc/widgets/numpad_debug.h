//--------------------------------------------------------------------------//
/// Copyright (c) 2010-2016 Milos Tosic. All Rights Reserved.              ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#ifndef RTM_RAPP_DEBUG_H
#define RTM_RAPP_DEBUG_H

#include <stdint.h>

#if RAPP_WITH_BGFX

#include <imgui/imgui.h>
#include <rapp/inc/widgets/widget_theme.h>

#define RAPP_DEBUG_WITH_STD_STRING	1

#if RAPP_DEBUG_WITH_STD_STRING
#include <string>
#endif // RAPP_DEBUG_WITH_STD_STRING

// TODO:
// 1. custom types undo
// 2. keyboard navigation
// 3. simplify macros?

namespace rapp {

	struct Property
	{
		enum Type : uint32_t
		{
			None,
			Int8,
			Int16,
			Int32,
			Int64,
			Uint8,
			Uint16,
			Uint32,
			Uint64,
			Float,
			Double,
			Bool,
			Color,
#if RAPP_DEBUG_WITH_STD_STRING
			StdString,
#endif // RAPP_DEBUG_WITH_STD_STRING
			Custom,
		};

		enum Edit
		{
			Editing,
			Accept,
			Cancel
		};
	};

	//	Example usage:
	//	--------------------------
	//
	//	struct CustomProp {};													// some custom type
	//	void CustomDfltFn(void*){}												// custom default value setter
	//	void CustomEditorFn(rapp::DebugMenu*){}									// custom type editor
	//	void CustomPreviewFn(rapp::DebugMenu*, ImDrawList*, ImVec2, ImVec2){}	// custom type preview
	//
	//	struct Graphics
	//	{
	//		float		m_clear[4];
	//		bool		m_hdr;
	//		float		m_minZ;
	//		uint16_t	m_num16;
	//		CustomProp	m_custom;
	//	};
	//	
	//	Graphics g_Graphics;
	//	
	//	RAPP_DEBUG_MENU_ROOT(Graphics, "Graphics")
	//		RAPP_DEBUG_MENU_PROPERTY(Background,	Graphics, rapp::Property::Color,	"Background color", &g_Graphics.m_clear, PAPP_ARRAY_ARG({ 0.46f, 0.23f, 0.69f, 1.0f }))
	//		RAPP_DEBUG_MENU_PROP_VEC(Vector4,		Graphics, rapp::Property::Float,	"float4 vec", 4)
	//			RAPP_DEBUG_MENU_PROP_VEC_ELEMENT_RNG(R, Vector4, "R", &g_Graphics.m_vector[0], 0.2f, 0.0f, 1.0f)
	//			RAPP_DEBUG_MENU_PROP_VEC_ELEMENT_RNG(G, Vector4, "G", &g_Graphics.m_vector[1], 0.4f, 0.0f, 1.0f)
	//			RAPP_DEBUG_MENU_PROP_VEC_ELEMENT_RNG(B, Vector4, "B", &g_Graphics.m_vector[2], 0.6f, 0.0f, 1.0f)
	//			RAPP_DEBUG_MENU_PROP_VEC_ELEMENT_RNG(A, Vector4, "A", &g_Graphics.m_vector[3], 1.0f, 0.0f, 1.0f)
	//		RAPP_DEBUG_MENU_PROP_RNG(MinZ,			Graphics, rapp::Property::Float,	"Min Z",	&g_Graphics.m_minZ, 0.46f, 0.1f, 0.89f)
	//		RAPP_DEBUG_MENU_PROP_RNG(Num16,			Graphics, rapp::Property::Uint16,	"Num 16",	&g_Graphics.m_num16, 123, 89, 189)
	//		RAPP_DEBUG_MENU_PROP_CUS(Custom,		Graphics, rapp::Property::Custom,	"Custom",	&g_Graphics.m_custom, CustomDfltFn, CustomEditorFn, CustomPreviewFn)

	// Arrays must be guarded
	#define PAPP_ARRAY_ARG(...) __VA_ARGS__

	// Declares a root debug menu
	#define RAPP_DEBUG_MENU_ROOT(_name, _label)																		\
		rapp::DebugMenu		rapp##_name(0, _label);

	// Declares a debug menu
	#define RAPP_DEBUG_MENU(_name, _parentName, _label)																\
		rapp::DebugMenu		rapp##_name(&rapp##_parentName, _label);

	// Declares a vector property with specified element count
	#define RAPP_DEBUG_MENU_PROP_VEC(_name, _parentName, _type, _label, _count)										\
		rapp::DebugMenu		rapp##_name(&rapp##_parentName, _label, _count, _type);

	// Declares a vector property element
	#define RAPP_DEBUG_MENU_PROP_VEC_ELEMENT(_name, _parentName, _label, _pointer, _defaultValue)					\
		auto rapp##_name##__LINE__ = _defaultValue;																	\
		rapp::DebugMenu	rapp##_name(&rapp##_parentName, _label, rapp##_parentName.m_type, _pointer, &rapp##_name##__LINE__);

	// Declares a vector property element with range
	#define RAPP_DEBUG_MENU_PROP_VEC_ELEMENT_RNG(_name, _parentName, _label, _pointer, _defaultValue, _min, _max)	\
		auto rapp##_name##__LINE__ = _defaultValue;																	\
		auto rapp##_name##_min_##__LINE__ = _min;																	\
		auto rapp##_name##_max_##__LINE__ = _max;																	\
		rapp::DebugMenu	rapp##_name(&rapp##_parentName, _label, rapp##_parentName.m_type, _pointer, &rapp##_name##__LINE__, &rapp##_name##_min_##__LINE__, &rapp##_name##_max_##__LINE__);

	// Declares a property, each property belongs to a menu
	#define RAPP_DEBUG_MENU_PROPERTY(_name, _parentName, _type, _label, _pointer, _defaultValue)					\
		auto rapp##_name##__LINE__ = _defaultValue;																	\
		rapp::DebugMenu	rapp##_name(&rapp##_parentName, _label, _type, _pointer, &rapp##_name##__LINE__);

	// Declares a ranged property (has minimum and maximum)
	#define RAPP_DEBUG_MENU_PROP_RNG(_name, _parentName, _type, _label, _pointer, _defaultValue, _min, _max)		\
		auto rapp##_name##__LINE__ = _defaultValue;																	\
		auto rapp##_name##_min_##__LINE__ = _min;																	\
		auto rapp##_name##_max_##__LINE__ = _max;																	\
		rapp::DebugMenu		rapp##_name(&rapp##_parentName, _label, _type, _pointer, &rapp##_name##__LINE__, &rapp##_name##_min_##__LINE__, &rapp##_name##_max_##__LINE__);

	// Declares a custom property
	#define RAPP_DEBUG_MENU_PROP_CUS(_name, _parentName, _type, _label, _pointer, _setDefaultFn, _editFn, _viewFn)	\
		rapp::DebugMenu	rapp##_name(&rapp##_parentName, _label, _type, _pointer, 0, 0, 0, _setDefaultFn, _editFn, _viewFn);

#if RAPP_DEBUG_WITH_STD_STRING
	#define RAPP_DEBUG_MENU_PROP_STR(_name, _parentName, _type, _label, _pointer)									\
		RAPP_DEBUG_MENU_PROP_CUS(_name, _parentName, _type, _label, _pointer, rapp::stringSetDefaultFn, rapp::stringEditorFn, rapp::stringPreviewFn)
#endif // RAPP_DEBUG_WITH_STD_STRING

	// Forward declaration of the debug menu drawing function
	void rappDebugMenu();

	struct DebugMenu
	{
		typedef void (*CustomDfltFn)(void* _pointer);
		typedef void (*CustomEditFn)(void* _pointer);
		typedef void (*CustomPrevFn)(void* _pointer, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size);

		static const int MAX_VARIABLES		= 8;
		static const int MAX_VARIABLE_SIZE	= sizeof(double);
		static const int MAX_STORAGE_SIZE	= sizeof(double);

		struct MinMax
		{
			uint8_t				m_min[MAX_STORAGE_SIZE];
			uint8_t				m_max[MAX_STORAGE_SIZE];
		};

		DebugMenu*				m_parent;
		uint32_t				m_index;
		uint32_t				m_numChildren;
		union
		{
			DebugMenu*			m_children[8];
			MinMax				m_minMax;
		};
		const char*				m_label;
		Property::Type			m_type;
		uint32_t				m_width;
		void*					m_value;
		CustomEditFn			m_customEditor;
		CustomPrevFn			m_customPreview;
		bool					m_minmaxSet;

		DebugMenu(DebugMenu* _parent = 0, const char* _label = 0, uint32_t _width = 1, Property::Type _type = Property::None);

		DebugMenu(DebugMenu* _parent, const char* _label, Property::Type _type, void* _pointer
						, void* _defaultValue	= 0
						, void* _minValue		= 0
						, void* _maxValue		= 0
						, CustomDfltFn	_customDefault	= 0
						, CustomEditFn	_customEditor	= 0
						, CustomPrevFn	_customPreview	= 0);
	};

#if RAPP_DEBUG_WITH_STD_STRING
	void stringSetDefaultFn(void* _var);
	void stringEditorFn(void* _var);
	void stringPreviewFn(void* _var, ImDrawList* _drawList, ImVec2 _minXY, ImVec2 _size);
#endif // RAPP_DEBUG_WITH_STD_STRING

} // namespace rapp

#endif // RAPP_WITH_BGFX

#endif // RTM_RAPP_DEBUG_H
