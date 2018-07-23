--
-- Copyright (c) 2017 Milos Tosic. All rights reserved.
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

function projectAdd_rapp() 
	addProject_lib("rapp", Lib.Runtime, false, nil, nil, nil, add_scheduler_defines())
end

function projectDependencies_rapp_bgfx()
	return mergeTables( projectDependencies_rapp(), { "bx", "bimg", "bgfx" } )
end

function projectExtraConfig_rapp()
	configuration { "linux-* or freebsd" }
		links {
			"pthread",
		}

	configuration {}
 end

function projectAdd_rapp_bgfx()
	local bgfxPath = find3rdPartyProject("bgfx")
	local extraFiles = {
		bgfxPath .. "examples/common/imgui/**.h",
		bgfxPath .. "examples/common/imgui/**.cpp",
		bgfxPath .. "examples/common/imgui/**.inl",
		bgfxPath .. "3rdparty/dear-imgui/**.*",
		bgfxPath .. "3rdparty/ib-compress/**.*",
		bgfxPath .. "examples/common/nanovg/*.h",
		bgfxPath .. "examples/common/nanovg/*.cpp"
	}

	local extraIncludes = {
		bgfxPath .. "3rdparty/",
		bgfxPath .. "examples/"
	}

	addProject_lib("rapp", Lib.Runtime, false, nil,  
						extraFiles, 
						extraIncludes, 
						add_scheduler_defines({ "RAPP_WITH_BGFX=1" }), "_bgfx")
end

function projectDependencyConfig_03_bgfx(_dependency)
	if _dependency == "rapp" then return {"rapp", "bgfx"} end
	return _dependency
end

function projectDependencyConfig_04_multi_app(_dependency)
	if _dependency == "rapp" then return {"rapp", "bgfx"} end
	return _dependency
end

