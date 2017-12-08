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
	if _OPTIONS["scheduler"] ~= "tbb" then
		return { "rbase" }
	else
		return { "rbase", "tbb" }
	end
end

function projectAdd_rapp() 
	addProject_lib("rapp", Lib.Runtime, false, nil, nil, nil, nil, add_scheduler_defines())
end

function projectDependencies_rapp_bgfx()
	return mergeTables( projectDependencies_rapp(), { "bx", "bimg", "bgfx", "imgui", "nanovg" } )
end

function projectAdd_rapp_bgfx()
	local bgfxPath = find3rdPartyProject("bgfx")
	local extraFiles = {
		bgfxPath .. "examples/common/imgui/*.h",
		bgfxPath .. "examples/common/imgui/*.cpp",
		bgfxPath .. "examples/common/nanovg/*.h",
		bgfxPath .. "examples/common/nanovg/*.cpp",
	}

	local extraIncludes = {
		bgfxPath .. "3rdparty/"
	}

	addProject_lib("rapp", Lib.Runtime, false, nil, nil, 
						extraFiles, 
						extraIncludes, 
						add_scheduler_defines({ "RAPP_WITH_BGFX=1" }), "_bgfx")
end


function projectDependencyConfig_02_bgfx(_dependency)
	if _dependency == "rapp" then return {"rapp", "bgfx"} end
	return "rapp"
end

