--
-- Copyright 2023 Milos Tosic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

function projectDependencies_rapp()
	return  { "rbase" }
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
	if bgfxPath == nil then
		return
	end

	files { 		
		bgfxPath .. "examples/common/imgui/**.h",
		bgfxPath .. "examples/common/imgui/**.cpp",
		bgfxPath .. "examples/common/imgui/**.inl",
		bgfxPath .. "3rdparty/dear-imgui/imgui.cpp",
		bgfxPath .. "3rdparty/dear-imgui/imgui_draw.cpp",
		bgfxPath .. "3rdparty/dear-imgui/imgui_tables.cpp",
		bgfxPath .. "3rdparty/dear-imgui/imgui_widgets.cpp",
		bgfxPath .. "3rdparty/dear-imgui/**.h",
		bgfxPath .. "3rdparty/dear-imgui/**.inl",
		bgfxPath .. "3rdparty/ib-compress/**.*",
		bgfxPath .. "examples/common/nanovg/*.h",
		bgfxPath .. "examples/common/nanovg/*.cpp"
 	}
	includedirs {
		bgfxPath .. "3rdparty/",
		bgfxPath .. "examples/",
		bgfxPath .. "examples/common/" 	
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
		buildoptions { "/wd4133 /wd4389"}
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
end

function projectAdd_rapp_bgfx()
	addProject_lib("rapp", Lib.Runtime, false, "_bgfx", true)
end
