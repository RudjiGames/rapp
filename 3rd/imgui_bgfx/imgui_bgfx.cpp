/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <rapp/3rd/imgui/imgui.h>
#include <rapp/3rd/imgui/imgui_internal.h>
#include "../../inc/rapp.h"
#include <rbase/inc/console.h>

#include "imgui_bgfx.h"

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"

#include "roboto_regular.ttf.h"
#include "roboto_bold.ttf.h"
#include "robotomono_regular.ttf.h"
#include "robotomono_bold.ttf.h"
#include "segoeui_regular.ttf.h"
#include "segoeui_bold.ttf.h"
#include "icons_kenney.ttf.h"
#include "icons_font_awesome.ttf.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout)
		&& (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices));
}

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(vs_imgui_image),
	BGFX_EMBEDDED_SHADER(fs_imgui_image),

	BGFX_EMBEDDED_SHADER_END()
};

struct FontRangeMerge
{
	const void* data;
	size_t      size;
	ImWchar     ranges[3];
};

static FontRangeMerge s_fontRangeMerge[] =
{
	{ s_iconsKenneyTtf,      sizeof(s_iconsKenneyTtf),      { ICON_MIN_KI, ICON_MAX_KI, 0 } },
	{ s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), { ICON_MIN_FA, ICON_MAX_FA, 0 } },
};

static void* memAlloc(size_t _size, void* _userData);
static void memFree(void* _ptr, void* _userData);

union BGFXTexID
{
	ImTextureID ptr;
	struct
	{
		bgfx::TextureHandle	handle;
		uint8_t				flags;
		uint8_t				mip;
	} s;
};

struct OcornutImguiContext
{
	void bgfxUpdateTexture(ImTextureData* tex)
	{
		if (tex->Status == ImTextureStatus_WantCreate)
		{
			bgfx::TextureHandle th = bgfx::createTexture2D((uint16_t)tex->Width, (uint16_t)tex->Height
				, false
				, 1
				, bgfx::TextureFormat::RGBA8
				, 0
				, 0
			);

			RTM_ASSERT(bgfx::isValid(th), "");

			bgfx::updateTexture2D(th, 0, 0, 0, 0, tex->Width, tex->Height, bgfx::copy(tex->Pixels, tex->Width * tex->Height * 4));

			BGFXTexID texture;
			texture.s.handle	= th;
			texture.s.flags		= IMGUI_FLAGS_ALPHA_BLEND;
			texture.s.mip		= 0;

			tex->SetTexID(texture.ptr);
			tex->SetStatus(ImTextureStatus_OK);
		}
		else
		if (tex->Status == ImTextureStatus_WantUpdates)
		{
			BGFXTexID texture;
			texture.ptr = tex->GetTexID();

			for (ImTextureRect& rect : tex->Updates)
			{
				const bgfx::Memory* sourceData = bgfx::alloc(rect.w * rect.h * 4);
			
				uint8_t* Pixels = (uint8_t*)tex->Pixels;
				Pixels += rect.y * 4 * tex->Width + (rect.x * 4);

				for (int32_t y=0; y < rect.h; ++y)
				{
					bx::memCopy(sourceData->data + y * rect.w * 4,
						Pixels + y * 4 * tex->Width,
						rect.w * 4);
				}

				RTM_ASSERT(bgfx::isValid(texture.s.handle), "");

				bgfx::updateTexture2D(	texture.s.handle, 0, 0,
										rect.x, rect.y,
										rect.w,	rect.h,
										sourceData );
			}

			tex->SetStatus(ImTextureStatus_OK);
		}
		else
		if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
		{
			bgfx::TextureHandle th = { (uint16_t)(tex->TexID) };
			bgfx::destroy(th);

			tex->SetTexID(ImTextureID_Invalid);
			tex->SetStatus(ImTextureStatus_Destroyed);
		}
	}

	void render(ImDrawData* _drawData)
	{
		if (_drawData->Textures != nullptr)
			for (ImTextureData* tex : *_drawData->Textures)
				if (tex->Status != ImTextureStatus_OK)
					bgfxUpdateTexture(tex);

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(_drawData->DisplaySize.x * _drawData->FramebufferScale.x);
		int fb_height = (int)(_drawData->DisplaySize.y * _drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = _drawData->DisplayPos.x;
			float y = _drawData->DisplayPos.y;
			float width = _drawData->DisplaySize.x;
			float height = _drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_viewId, NULL, ortho);
			bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );
		}

		const ImVec2 clipPos   = _drawData->DisplayPos;       // (0,0) unless using multi-viewports
		const ImVec2 clipScale = _drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

			if (!checkAvailTransientBuffers(numVertices, m_layout, numIndices) )
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
			bgfx::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

			bgfx::Encoder* encoder = bgfx::begin();

			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_MSAA
						;

					BGFXTexID textureHandle;
					textureHandle.ptr = cmd->GetTexID();
					bgfx::ProgramHandle program = m_program;

					if (0 != cmd->GetTexID())
					{
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->GetTexID() };
						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
							? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							: BGFX_STATE_NONE
							;
						textureHandle.s.handle	= texture.s.handle;
						textureHandle.s.flags	= texture.s.flags;
						textureHandle.s.mip		= texture.s.mip;
						if (0 != texture.s.mip)
						{
							const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
							bgfx::setUniform(u_imageLodEnabled, lodEnabled);
							program = m_imageProgram;
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (cmd->ClipRect.x - clipPos.x) * clipScale.x;
					clipRect.y = (cmd->ClipRect.y - clipPos.y) * clipScale.y;
					clipRect.z = (cmd->ClipRect.z - clipPos.x) * clipScale.x;
					clipRect.w = (cmd->ClipRect.w - clipPos.y) * clipScale.y;

					if (clipRect.x <  fb_width
					&&  clipRect.y <  fb_height
					&&  clipRect.z >= 0.0f
					&&  clipRect.w >= 0.0f)
					{
						const uint16_t xx = uint16_t(bx::max(clipRect.x, 0.0f) );
						const uint16_t yy = uint16_t(bx::max(clipRect.y, 0.0f) );
						encoder->setScissor(xx, yy
								, uint16_t(bx::min(clipRect.z, 65535.0f)-xx)
								, uint16_t(bx::min(clipRect.w, 65535.0f)-yy)
								);

						encoder->setState(state);
						encoder->setTexture(0, s_tex, textureHandle.s.handle);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(m_viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}

	void create(float _fontSize, bx::AllocatorI* _allocator)
	{
		IMGUI_CHECKVERSION();

		m_allocator = _allocator;

		if (NULL == _allocator)
		{
			static bx::DefaultAllocator allocator;
			m_allocator = &allocator;
		}

		m_viewId = 255;
		m_lastScroll = 0;
		m_last = bx::getHPCounter();

		ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

		m_imgui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.DeltaTime   = 1.0f / 60.0f;
		io.IniFilename = NULL;

		rapp::uiSetStyle(RAPP_STYLE_RUDJI);

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

		for (int32_t ii=0; ii<(int32_t)rapp::KeyboardKey::Count; ++ii)
		{
			m_keyMap[ii] = ImGuiKey_None;
		}

		m_keyMap[rapp::KeyboardKey::Esc]          = ImGuiKey_Escape;
		m_keyMap[rapp::KeyboardKey::Return]       = ImGuiKey_Enter;
		m_keyMap[rapp::KeyboardKey::Tab]          = ImGuiKey_Tab;
		m_keyMap[rapp::KeyboardKey::Space]        = ImGuiKey_Space;
		m_keyMap[rapp::KeyboardKey::Backspace]    = ImGuiKey_Backspace;
		m_keyMap[rapp::KeyboardKey::Up]           = ImGuiKey_UpArrow;
		m_keyMap[rapp::KeyboardKey::Down]         = ImGuiKey_DownArrow;
		m_keyMap[rapp::KeyboardKey::Left]         = ImGuiKey_LeftArrow;
		m_keyMap[rapp::KeyboardKey::Right]        = ImGuiKey_RightArrow;
		m_keyMap[rapp::KeyboardKey::Insert]       = ImGuiKey_Insert;
		m_keyMap[rapp::KeyboardKey::Delete]       = ImGuiKey_Delete;
		m_keyMap[rapp::KeyboardKey::Home]         = ImGuiKey_Home;
		m_keyMap[rapp::KeyboardKey::End]          = ImGuiKey_End;
		m_keyMap[rapp::KeyboardKey::PageUp]       = ImGuiKey_PageUp;
		m_keyMap[rapp::KeyboardKey::PageDown]     = ImGuiKey_PageDown;
		m_keyMap[rapp::KeyboardKey::Print]        = ImGuiKey_PrintScreen;
		m_keyMap[rapp::KeyboardKey::Plus]         = ImGuiKey_Equal;
		m_keyMap[rapp::KeyboardKey::Minus]        = ImGuiKey_Minus;
		m_keyMap[rapp::KeyboardKey::LeftBracket]  = ImGuiKey_LeftBracket;
		m_keyMap[rapp::KeyboardKey::RightBracket] = ImGuiKey_RightBracket;
		m_keyMap[rapp::KeyboardKey::Semicolon]    = ImGuiKey_Semicolon;
		m_keyMap[rapp::KeyboardKey::Quote]        = ImGuiKey_Apostrophe;
		m_keyMap[rapp::KeyboardKey::Comma]        = ImGuiKey_Comma;
		m_keyMap[rapp::KeyboardKey::Period]       = ImGuiKey_Period;
		m_keyMap[rapp::KeyboardKey::Slash]        = ImGuiKey_Slash;
		m_keyMap[rapp::KeyboardKey::Backslash]    = ImGuiKey_Backslash;
		m_keyMap[rapp::KeyboardKey::Tilde]        = ImGuiKey_GraveAccent;
		m_keyMap[rapp::KeyboardKey::F1]           = ImGuiKey_F1;
		m_keyMap[rapp::KeyboardKey::F2]           = ImGuiKey_F2;
		m_keyMap[rapp::KeyboardKey::F3]           = ImGuiKey_F3;
		m_keyMap[rapp::KeyboardKey::F4]           = ImGuiKey_F4;
		m_keyMap[rapp::KeyboardKey::F5]           = ImGuiKey_F5;
		m_keyMap[rapp::KeyboardKey::F6]           = ImGuiKey_F6;
		m_keyMap[rapp::KeyboardKey::F7]           = ImGuiKey_F7;
		m_keyMap[rapp::KeyboardKey::F8]           = ImGuiKey_F8;
		m_keyMap[rapp::KeyboardKey::F9]           = ImGuiKey_F9;
		m_keyMap[rapp::KeyboardKey::F10]          = ImGuiKey_F10;
		m_keyMap[rapp::KeyboardKey::F11]          = ImGuiKey_F11;
		m_keyMap[rapp::KeyboardKey::F12]          = ImGuiKey_F12;
		m_keyMap[rapp::KeyboardKey::NumPad0]      = ImGuiKey_Keypad0;
		m_keyMap[rapp::KeyboardKey::NumPad1]      = ImGuiKey_Keypad1;
		m_keyMap[rapp::KeyboardKey::NumPad2]      = ImGuiKey_Keypad2;
		m_keyMap[rapp::KeyboardKey::NumPad3]      = ImGuiKey_Keypad3;
		m_keyMap[rapp::KeyboardKey::NumPad4]      = ImGuiKey_Keypad4;
		m_keyMap[rapp::KeyboardKey::NumPad5]      = ImGuiKey_Keypad5;
		m_keyMap[rapp::KeyboardKey::NumPad6]      = ImGuiKey_Keypad6;
		m_keyMap[rapp::KeyboardKey::NumPad7]      = ImGuiKey_Keypad7;
		m_keyMap[rapp::KeyboardKey::NumPad8]      = ImGuiKey_Keypad8;
		m_keyMap[rapp::KeyboardKey::NumPad9]      = ImGuiKey_Keypad9;
		m_keyMap[rapp::KeyboardKey::Key0]         = ImGuiKey_0;
		m_keyMap[rapp::KeyboardKey::Key1]         = ImGuiKey_1;
		m_keyMap[rapp::KeyboardKey::Key2]         = ImGuiKey_2;
		m_keyMap[rapp::KeyboardKey::Key3]         = ImGuiKey_3;
		m_keyMap[rapp::KeyboardKey::Key4]         = ImGuiKey_4;
		m_keyMap[rapp::KeyboardKey::Key5]         = ImGuiKey_5;
		m_keyMap[rapp::KeyboardKey::Key6]         = ImGuiKey_6;
		m_keyMap[rapp::KeyboardKey::Key7]         = ImGuiKey_7;
		m_keyMap[rapp::KeyboardKey::Key8]         = ImGuiKey_8;
		m_keyMap[rapp::KeyboardKey::Key9]         = ImGuiKey_9;
		m_keyMap[rapp::KeyboardKey::KeyA]         = ImGuiKey_A;
		m_keyMap[rapp::KeyboardKey::KeyB]         = ImGuiKey_B;
		m_keyMap[rapp::KeyboardKey::KeyC]         = ImGuiKey_C;
		m_keyMap[rapp::KeyboardKey::KeyD]         = ImGuiKey_D;
		m_keyMap[rapp::KeyboardKey::KeyE]         = ImGuiKey_E;
		m_keyMap[rapp::KeyboardKey::KeyF]         = ImGuiKey_F;
		m_keyMap[rapp::KeyboardKey::KeyG]         = ImGuiKey_G;
		m_keyMap[rapp::KeyboardKey::KeyH]         = ImGuiKey_H;
		m_keyMap[rapp::KeyboardKey::KeyI]         = ImGuiKey_I;
		m_keyMap[rapp::KeyboardKey::KeyJ]         = ImGuiKey_J;
		m_keyMap[rapp::KeyboardKey::KeyK]         = ImGuiKey_K;
		m_keyMap[rapp::KeyboardKey::KeyL]         = ImGuiKey_L;
		m_keyMap[rapp::KeyboardKey::KeyM]         = ImGuiKey_M;
		m_keyMap[rapp::KeyboardKey::KeyN]         = ImGuiKey_N;
		m_keyMap[rapp::KeyboardKey::KeyO]         = ImGuiKey_O;
		m_keyMap[rapp::KeyboardKey::KeyP]         = ImGuiKey_P;
		m_keyMap[rapp::KeyboardKey::KeyQ]         = ImGuiKey_Q;
		m_keyMap[rapp::KeyboardKey::KeyR]         = ImGuiKey_R;
		m_keyMap[rapp::KeyboardKey::KeyS]         = ImGuiKey_S;
		m_keyMap[rapp::KeyboardKey::KeyT]         = ImGuiKey_T;
		m_keyMap[rapp::KeyboardKey::KeyU]         = ImGuiKey_U;
		m_keyMap[rapp::KeyboardKey::KeyV]         = ImGuiKey_V;
		m_keyMap[rapp::KeyboardKey::KeyW]         = ImGuiKey_W;
		m_keyMap[rapp::KeyboardKey::KeyX]         = ImGuiKey_X;
		m_keyMap[rapp::KeyboardKey::KeyY]         = ImGuiKey_Y;
		m_keyMap[rapp::KeyboardKey::KeyZ]         = ImGuiKey_Z;

		io.ConfigFlags |= 0
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_NavEnableKeyboard
			;

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		m_program = bgfx::createProgram(
			  bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
			, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
			, true
			);

		u_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		m_imageProgram = bgfx::createProgram(
			  bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
			, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
			, true
			);

		m_layout
			.begin()
			.add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		// Load fonts and icons

		addFontWithIcons(ImGui::Font::RobotoMono,		(void*)s_robotoMonoRegularTtf,	sizeof(s_robotoMonoRegularTtf));
		addFontWithIcons(ImGui::Font::RobotoMonoBold,	(void*)s_robotoMonoBoldTtf,		sizeof(s_robotoMonoBoldTtf));
		addFontWithIcons(ImGui::Font::Roboto,			(void*)s_robotoRegularTtf,		sizeof(s_robotoRegularTtf));
		addFontWithIcons(ImGui::Font::RobotoBold,		(void*)s_robotoBoldTtf,			sizeof(s_robotoBoldTtf));
		addFontWithIcons(ImGui::Font::SegoeUI,			(void*)s_segoeuiTtf,			sizeof(s_segoeuiTtf));
		addFontWithIcons(ImGui::Font::SegoeUIBold,		(void*)s_segoeuiBoldTtf,		sizeof(s_segoeuiBoldTtf));

		io.Fonts->Build();
	}

	void addFontWithIcons(ImGui::Font::Enum _font, void* _data, size_t _dataSize)
	{
		ImGuiIO& io = ImGui::GetIO();

		m_font[_font] = io.Fonts->AddFontDefault();

		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;
		config.MergeMode			= true;
		io.Fonts->AddFontFromMemoryCompressedTTF(_data, int(_dataSize), 30.0f, &config);

		config.GlyphMinAdvanceX		= 15.0f;
		for (uint32_t ii = 0; ii < BX_COUNTOF(s_fontRangeMerge); ++ii)
		{
			const FontRangeMerge& frm = s_fontRangeMerge[ii];
			io.Fonts->AddFontFromMemoryTTF((void*)frm.data, (int)frm.size, 15.0f, &config, frm.ranges);
		}
	}

	void destroy()
	{
		ImGui::DestroyContext(m_imgui);

		bgfx::destroy(s_tex);
		bgfx::destroy(u_imageLodEnabled);
		bgfx::destroy(m_imageProgram);
		bgfx::destroy(m_program);

		m_allocator = NULL;
	}

	void beginFrame(
		  int32_t _mx
		, int32_t _my
		, uint8_t _button
		, int32_t _scroll
		, int _width
		, int _height
		, int _inputChar
		, bgfx::ViewId _viewId
		)
	{
		m_viewId = _viewId;

		ImGuiIO& io = ImGui::GetIO();
		if (_inputChar >= 0)
		{
			io.AddInputCharacter(_inputChar);
		}

		io.DisplaySize = ImVec2( (float)_width, (float)_height);

		const int64_t now = bx::getHPCounter();
		const int64_t frameTime = now - m_last;
		m_last = now;
		const double freq = double(bx::getHPFrequency() );
		io.DeltaTime = float(frameTime/freq);

		io.AddMousePosEvent( (float)_mx, (float)_my);
		io.AddMouseButtonEvent(ImGuiMouseButton_Left,   0 != (_button & IMGUI_MBUT_LEFT  ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Right,  0 != (_button & IMGUI_MBUT_RIGHT ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, 0 != (_button & IMGUI_MBUT_MIDDLE) );
		io.AddMouseWheelEvent(0.0f, (float)(_scroll - m_lastScroll) );
		m_lastScroll = _scroll;

		uint8_t modifiers = rapp::inputGetModifiersState();
		io.AddKeyEvent(ImGuiMod_Shift, 0 != (modifiers & (rapp::KeyboardModifier::LShift | rapp::KeyboardModifier::RShift) ) );
		io.AddKeyEvent(ImGuiMod_Ctrl,  0 != (modifiers & (rapp::KeyboardModifier::LCtrl  | rapp::KeyboardModifier::RCtrl ) ) );
		io.AddKeyEvent(ImGuiMod_Alt,   0 != (modifiers & (rapp::KeyboardModifier::LAlt   | rapp::KeyboardModifier::RAlt  ) ) );
		io.AddKeyEvent(ImGuiMod_Super, 0 != (modifiers & (rapp::KeyboardModifier::LMeta  | rapp::KeyboardModifier::RMeta ) ) );
		for (int32_t ii = 0; ii < (int32_t)rapp::KeyboardKey::Count; ++ii)
		{
			io.AddKeyEvent(m_keyMap[ii], rapp::inputGetKeyState(rapp::KeyboardKey::Enum(ii) ) );
			io.SetKeyEventNativeData(m_keyMap[ii], 0, 0, ii);
		}

		ImGui::NewFrame();
	}

	void endFrame()
	{
		ImGui::Render();
		render(ImGui::GetDrawData());
	}

	ImGuiContext*       m_imgui;
	bx::AllocatorI*     m_allocator;
	bgfx::VertexLayout  m_layout;
	bgfx::ProgramHandle m_program;
	bgfx::ProgramHandle m_imageProgram;
	bgfx::UniformHandle s_tex;
	bgfx::UniformHandle u_imageLodEnabled;
	ImFont* m_font[ImGui::Font::Count];
	int64_t m_last;
	int32_t m_lastScroll;
	bgfx::ViewId m_viewId;
	ImGuiKey m_keyMap[(int)rapp::KeyboardKey::Count];
};

static OcornutImguiContext s_ctx;

static void* memAlloc(size_t _size, void* _userData)
{
	BX_UNUSED(_userData);
	return bx::alloc(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BX_UNUSED(_userData);
	bx::free(s_ctx.m_allocator, _ptr);
}

void imguiCreate(float _fontSize, bx::AllocatorI* _allocator)
{
	s_ctx.create(_fontSize, _allocator);
}

void imguiDestroy()
{
	s_ctx.destroy();
}

void imguiBeginFrame(int32_t _mx, int32_t _my, uint8_t _button, int32_t _scroll, uint16_t _width, uint16_t _height, int _inputChar, bgfx::ViewId _viewId)
{
	s_ctx.beginFrame(_mx, _my, _button, _scroll, _width, _height, _inputChar, _viewId);
}

void imguiEndFrame()
{
	s_ctx.endFrame();
}

namespace ImGui
{
	void PushFont(Font::Enum _font, float _size)
	{
		PushFont(s_ctx.m_font[_font], _size);
	}

	void PushEnabled(bool _enabled)
	{
		extern void PushItemFlag(int option, bool enabled);
		PushItemFlag(ImGuiItemFlags_Disabled, !_enabled);
		PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (_enabled ? 1.0f : 0.5f) );
	}

	void PopEnabled()
	{
		extern void PopItemFlag();
		PopItemFlag();
		PopStyleVar();
	}

} // namespace ImGui

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: 'int rect_width_compare(const void*, const void*)' defined but not used
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
#define STBTT_malloc(_size, _userData) memAlloc(_size, _userData)
#define STBTT_free(_ptr, _userData) memFree(_ptr, _userData)
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#include <stb/stb_truetype.h>
BX_PRAGMA_DIAGNOSTIC_POP();
