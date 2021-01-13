//--------------------------------------------------------------------------//
/// Copyright (c) 2018 Milos Tosic. All Rights Reserved.                   ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <bgfx_pch.h>
#include <math.h>

struct bgfxApp : public rapp::App
{
	RAPP_CLASS(bgfxApp)

	rapp::WindowHandle	m_window;
	float				m_time;

	virtual ~bgfxApp() {}

	int init(int32_t /*_argc*/, const char* const* /*_argv*/, rtmLibInterface* /*_libInterface = 0*/)
	{
		m_time = 0.0f;

		static const rapp::InputBinding bindings[] =
		{
			{ NULL, "exit", 1, { rapp::KeyboardState::Key::KeyQ,   rapp::KeyboardState::Modifier::LCtrl  }},
			{ NULL, "exit", 1, { rapp::KeyboardState::Key::KeyQ,   rapp::KeyboardState::Modifier::RCtrl  }},
			{ NULL, "hide", 1, { rapp::KeyboardState::Key::Tilde,  rapp::KeyboardState::Modifier::NoMods }},
			RAPP_INPUT_BINDING_END
		};

		rapp::inputAddBindings("bindings", bindings);
		rapp::cmdAdd("exit", cmdExit,			0, "quits application");
		rapp::cmdAdd("hide", cmdHideConsole,	0, "hides console");

		uint32_t width, height;
		rapp::windowGetDefaultSize(&width, &height);
		m_width		= width;
		m_height	= height;

		rapp::appGraphicsInit(this, m_width, m_height);

		return 0;
	}

	void suspend() {}
	void resume() {}
	void update(float _time)
	{
		m_time += _time;
	}

	void draw()
	{
		appRunOnMainThread(mainThreadFunc, this);

		rtm::Console::rgb(255, 255, 0, "Printing from app thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());

		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, (uint16_t)m_width, (uint16_t)m_height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x17, "rapp/samples/bgfx");
		bgfx::dbgTextPrintf(0, 2, 0x37, m_description);

		// debug input
		rapp::inputDgbGamePads();
		rapp::inputDgbMouse();
		rapp::inputDgbKeyboard();
		rapp::inputDgbTouch();

		// Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
	}

	void drawEyes(struct NVGcontext* vg, float x, float y, float w, float h, float mx, float my)
	{
		struct NVGpaint gloss, bg;
		float ex = w *0.23f;
		float ey = h * 0.5f;
		float lx = x + ex;
		float ly = y + ey;
		float rx = x + w - ex;
		float ry = y + ey;
		float dx,dy,d;
		float br = (ex < ey ? ex : ey) * 0.5f;
		float blink = 1 - powf(sinf(m_time*0.5f),200)*0.8f;

		bg = nvgLinearGradient(vg, x,y+h*0.25f,x+w*0.1f,y+h, nvgRGBA(220,220,220,255), nvgRGBA(128,128,128,255) );
		nvgBeginPath(vg);
		nvgEllipse(vg, lx,ly, ex,ey);
		nvgEllipse(vg, rx,ry, ex,ey);
		nvgFillPaint(vg, bg);
		nvgFill(vg);

		dx = (mx - rx) / (ex * 10);
		dy = (my - ry) / (ey * 10);
		d = sqrtf(dx*dx+dy*dy);
		if (d > 1.0f) {
			dx /= d; dy /= d;
		}
		dx *= ex*0.4f;
		dy *= ey*0.5f;
		nvgBeginPath(vg);
		nvgEllipse(vg, lx+dx,ly+dy+ey*0.25f*(1-blink), br,br*blink);
		nvgFillColor(vg, nvgRGBA(32,32,32,255) );
		nvgFill(vg);

		dx = (mx - rx) / (ex * 10);
		dy = (my - ry) / (ey * 10);
		d = sqrtf(dx*dx+dy*dy);
		if (d > 1.0f) {
			dx /= d; dy /= d;
		}
		dx *= ex*0.4f;
		dy *= ey*0.5f;
		nvgBeginPath(vg);
		nvgEllipse(vg, rx+dx,ry+dy+ey*0.25f*(1-blink), br,br*blink);
		nvgFillColor(vg, nvgRGBA(32,32,32,255) );
		nvgFill(vg);

		gloss = nvgRadialGradient(vg, lx-ex*0.25f,ly-ey*0.5f, ex*0.1f,ex*0.75f, nvgRGBA(255,255,255,128), nvgRGBA(255,255,255,0) );
		nvgBeginPath(vg);
		nvgEllipse(vg, lx,ly, ex,ey);
		nvgFillPaint(vg, gloss);
		nvgFill(vg);

		gloss = nvgRadialGradient(vg, rx-ex*0.25f,ry-ey*0.5f, ex*0.1f,ex*0.75f, nvgRGBA(255,255,255,128), nvgRGBA(255,255,255,0) );
		nvgBeginPath(vg);
		nvgEllipse(vg, rx,ry, ex,ey);
		nvgFillPaint(vg, gloss);
		nvgFill(vg);
	} 

	void drawGUI()
	{
		ImGui::SetNextWindowPos(ImVec2(10.0f, 666.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 80.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("ImGui dialog");
		ImGui::Separator();
		ImGui::Button("Button", ImVec2(180.0f,23.0f));
		ImGui::End();

		NVGcontext* ctx = (NVGcontext*)rapp::appGetNanoVGctx(this);

		rapp::MouseState ms;
		rapp::inputGetMouseState(ms);
		drawEyes(ctx, 150.0f, 150.0f, 150.0f, 100.0f, (float)ms.m_absolute[0], (float)ms.m_absolute[1]);
	}
	
	void shutDown()
	{
		rtm::Console::rgb(255, 255, 0, "Shutting down app\n");
		rapp::appGraphicsShutdown(this, m_window);
		rapp::inputRemoveBindings("bindings");
	}

	static void mainThreadFunc(void* /*_appClass*/)
	{
		rtm::Console::rgb(0, 255, 0, "Printing from main thread (ID: %u)\n", (uint32_t)rtm::Thread::getThreadID());
	}

	static int cmdExit(rapp::App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		_app->quit();
		return 0;
	}

	static int cmdHideConsole(rapp::App* _app, void* _userData, int _argc, char const* const* _argv)
	{
		RTM_UNUSED_3(_userData, _argv, _argc);
		rapp::cmdConsoleToggle(_app);
		return 0;
	}
};

RAPP_REGISTER(bgfxApp, "bgfx", "Example of bgfx based application");
