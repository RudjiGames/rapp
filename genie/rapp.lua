--
-- Copyright (c) 2017 Milos Tosic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

function projectDependencies_rapp()
	return { "rbase" }
end

function projectAdd_rapp() 
	addProject_lib("rapp")
end

function projectDependencies_rapp_bgfx()
	return { "rbase", "bx", "bimg", "bgfx" }
end

function projectAdd_rapp_bgfx()
	addProject_lib("rapp", Lib.Runtime, false, nil, nil, nil, nil, "RAPP_WITH_BGFX=1", "_bgfx")
end

function projectDependencyConfig_02_bgfx(_dependency)
	if _dependency == "rapp" then return {"rapp", "bgfx"} end
	return "rapp"
end

