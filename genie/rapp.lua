--
-- Copyright (c) 2018 Milos Tosic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

newoption {
	trigger	= "scheduler",
	value	= "Job scheduler type",
	description = "Choose type of job scheduler",
	allowed = {
		{ "rapp", "Built-in job scheduler" },
		{ "tbb",  "Intel Thread Building Blocks" }
	}
}

if  _OPTIONS["scheduler"] == nil then
	_OPTIONS["scheduler"] = "rapp"
end

function add_scheduler_defines(definesToAppend)
	if  _OPTIONS["scheduler"] == "rapp" then
		return mergeTables(definesToAppend, {"RAPP_JOBS_INTERNAL=1"})
	elseif  _OPTIONS["scheduler"] == "tbb" then
		return mergeTables(definesToAppend, {"RAPP_JOBS_TBB=1"})
	end
	return definesToAppend
end

function projectDependencies_rapp()
	local dependencies = { "rbase" }
	if _OPTIONS["scheduler"] == "tbb" then
		table.insert(dependencies, "tbb")
	end
	if (getTargetOS() == "linux" or getTargetOS() == "freebsd") then
		table.insert(dependencies, "X11")
	end
	return dependencies
end

function projectExtraConfig_rapp()
	defines { add_scheduler_defines() }
end

function projectAdd_rapp() 
	addProject_lib("rapp", Lib.Runtime )
end

function projectDependencies_rapp_bgfx()
	return mergeTables( projectDependencies_rapp(), { "bgfx" } )
end

function projectExtraConfigExecutable_rapp()
	configuration { "linux-* or freebsd" }
		links {
			"pthread",
		}

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
	defines { add_scheduler_defines({ "RAPP_WITH_BGFX=1" }) }
end

function projectExtraConfigExecutable_rapp_bgfx()
	local bgfxPath = find3rdPartyProject("bgfx")
	includedirs {
		bgfxPath .. "3rdparty/",
		bgfxPath .. "examples/common/" 	
	}
end

function projectAdd_rapp_bgfx()
	addProject_lib("rapp", Lib.Runtime, false, "_bgfx", true)
end
