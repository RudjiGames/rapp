--
-- Copyright 2025 Milos Tosic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

function projectDependencies_rapp()
	return  { "rbase", "enkiTS" }
end

function projectAdd_rapp() 
	addProject_lib("rapp", Lib.Runtime)
end

function projectDependencies_rapp_bgfx()
	local dependencies = {}
	if (getTargetOS() == "linux" or getTargetOS() == "freebsd") then
		table.insert(dependencies, "X11")
	end
	dependencies = mergeTwoTables(dependencies, { "bgfx" })
	dependencies = mergeTables( projectDependencies_rapp(), dependencies )
	return dependencies
end

function projectExtraConfigExecutable_rapp()
	configuration { "linux-* or freebsd" }
		buildoptions {
			"-fPIC",
		}
		links {
			"pthread",
			"X11",
			"GL",
		}
	configuration {}

	configuration { "osx" }
		linkoptions {
			"-framework CoreFoundation",
			"-framework Cocoa",
			"-framework OpenGL",
		}

	configuration {}
 end

function projectExtraConfig_rapp_bgfx()
	local bgfxPath = find3rdPartyProject("bgfx")
	local rappPath = getProjectPath("rapp")

	if bgfxPath == nil then
		return
	end

	files { 		
		bgfxPath .. "3rdparty/ib-compress/**.*",
		rappPath .. "/3rd/dear-imgui/imgui/**.h",
		rappPath .. "/3rd/dear-imgui/imgui/**.cpp",
		rappPath .. "/3rd/dear-imgui/imgui/**.inl",
		rappPath .. "/3rd/dear-imgui/imgui.cpp",
		rappPath .. "/3rd/dear-imgui/imgui_draw.cpp",
		rappPath .. "/3rd/dear-imgui/imgui_tables.cpp",
		rappPath .. "/3rd/dear-imgui/imgui_widgets.cpp",
		rappPath .. "/3rd/dear-imgui/**.h",
		rappPath .. "/3rd/dear-imgui/**.inl",
		rappPath .. "/3rd/nanovg_bgfx/**.h",
		rappPath .. "/3rd/nanovg_bgfx/**.cpp",
		rappPath .. "/3rd/nanovg_bgfx/**.c",
		rappPath .. "/3rd/vg_renderer/include/**.h",
		rappPath .. "/3rd/vg_renderer/include/**.inl",
		rappPath .. "/3rd/vg_renderer/src/**.c",
		rappPath .. "/3rd/vg_renderer/src/**.cpp",
		rappPath .. "/3rd/vg_renderer/src/**.h"
 	}
	excludes {
		rappPath .. "/3rd/vg_renderer/src/libs/stb_truetype.cpp"
	}
	includedirs {
		bgfxPath .. "3rdparty/",
		rappPath .. "/3rd/",
		rappPath .. "/3rd/vg_renderer/include"
	}
	defines { "RAPP_WITH_BGFX" }

	configuration { "debug" }
		defines { "BX_CONFIG_DEBUG=1" }
	configuration { "not debug" }
		defines { "BX_CONFIG_DEBUG=0" }
	configuration {}

 	configuration { "vs*", "windows" }
		-- 4324 - structure was padded due to alignment specifier
		-- 4389 - '==': signed/unsigned mismatch
		-- 4244 - vg_renderer: 'argument': conversion from 'float' to 'uint16_t', possible loss of data
		-- 4334 - vg_renderer: '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
		-- 4505 - unreferenced function with internal linkage has been removed
		buildoptions { "/wd4133 /wd4389 /wd4244 /wd4334 /wd4505"}
	configuration {}
end

function projectExtraConfigExecutable_rapp_bgfx()
	projectExtraConfigExecutable_rapp()
	local bgfxPath = find3rdPartyProject("bgfx")
	if bgfxPath == nil then
		return
	end
	includedirs {
		bgfxPath .. "3rdparty/",
		bgfxPath .. "examples/common/" 	
	}
	defines { "IMGUI_STB_NAMESPACE=ImStb" }
end

function projectAdd_rapp_bgfx()
	addProject_lib("rapp", Lib.Runtime, false, "_bgfx", true)
end
