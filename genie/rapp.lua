--
-- Copyright (c) 2017 Milos Tosic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

newoption {
	trigger	= "scheduler",
	value	= "Job scheduler type",
	description = "Choose type of job scheduler",
	allowed = {
		{ "none",  "Serial execution"  },
		{ "tbb",  "Intel Thread Building Blocks"  }
	}
}

if  _OPTIONS["scheduler"] == nil then
	_OPTIONS["scheduler"] = "tbb"
end


function add_scheduler_defines(definesToAppend)
	if  _OPTIONS["scheduler"] == "tbb" then
		return mergeTables(definesToAppend, {"RAPP_WITH_TBB=1"})
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
	return mergeTables( projectDependencies_rapp(), { "bx", "bimg", "bgfx" } )
end

function projectAdd_rapp_bgfx()
	addProject_lib("rapp", Lib.Runtime, false, nil, nil, nil, nil, add_scheduler_defines({ "RAPP_WITH_BGFX=1" }), "_bgfx")
end


function projectDependencyConfig_02_bgfx(_dependency)
	if _dependency == "rapp" then return {"rapp", "bgfx"} end
	return "rapp"
end

