<img height="81" src="https://rudji.com/img/lib/rapp.svg"/>

[![Build status](https://ci.appveyor.com/api/projects/status/5ai0w0mu6ay2wcme?svg=true)](https://ci.appveyor.com/project/milostosic/rapp-6qu5f)
[![License](https://img.shields.io/badge/license-BSD--2%20clause-blue.svg)](https://github.com/RudjiGames/rapp/blob/master/LICENSE)

**rapp** is a library that provides cross-platform application entry point and additional functionality.
rapp is **heavily** based on [bgfx](https://github.com/bkaradzic/bgfx) examples entry point code that can be found [here](https://github.com/bkaradzic/bgfx/tree/master/examples/common/entry). Graphics code is still [bgfx](https://github.com/bkaradzic/bgfx) based but..  

...dependecy on [bx](https://github.com/bkaradzic/bx) and [bgfx](https://github.com/bkaradzic/bgfx) has been removed by aggressively copying code and by a special feature of [build](https://github.com/RudjiGames/build) system that allows to have the same library in a 'solution' with different configurations.  
To clarify, in the screenshot below first sample (command line) links against **rapp** and [rbase](https://github.com/RudjiGames/rbase) while the second one (graphics) links against **rapp_bgfx**, [rbase](https://github.com/RudjiGames/rbase), [bx](https://github.com/bkaradzic/bx), [bimg](https://github.com/bkaradzic/bimg) and [bgfx](https://github.com/bkaradzic/bgfx) - this is all automated.   

<img src="https://github.com/RudjiGames/rapp/blob/master/img/rapp_configs.png" width=142 height=153>

Thanks to multiple configurations per project feature of [build](https://github.com/RudjiGames/build), script taking care of this is very short and can be seen [here](https://github.com/RudjiGames/rapp/blob/master/genie/rapp.lua), basically it just enables a library wide define.

Features
======

**rapp** currently has the following functionality:
* Applications written as classes with init/shutdown/suspend/resume functionality
* Command line (tools, unit tests, etc.) or graphics applications (games, etc.)
* Custom commands
* Input controllers (mouse, keyboard, gamepad, etc.) with input binding callbacks and debug visualizations
* Ability to run code on main/message loop thread
* Job scheduler with job stealing for fine grained parallelism
* [ImGui](https://github.com/ocornut/imgui) and [NanoVG](https://github.com/memononen/nanovg) integration
* Quake like console - [ImGui](https://github.com/ocornut/imgui) based
* Built-in [**rprof**](https://github.com/RudjiGames/rprof) CPU profiler
* Multiple applications in one binary
* Window functions

[**rprof**](https://github.com/RudjiGames/rprof) CPU profiler is an optional dependency and needs to be explicitly requested when generating project files, for example:

      GENie --with-rprof vs2022

Here's a screenshot of a [bgfx](https://github.com/bkaradzic/bgfx) sample showing input debugging, [ImGui](https://github.com/ocornut/imgui) dialog and Quake like console as well as [NanoVG](https://github.com/memononen/nanovg) shape (eyes):  
<img src="https://github.com/RudjiGames/rapp/blob/master/img/input_debug.png">

Platform support
======

|                  | Input (KMG) | Threading | Console | Graphics |
|------------------|-------------|-----------|---------|----------|
| **Windows**      | ✓✓✓        |  ✓        | ✓      |    ✓     |
| **Xbox One**     | ✓✓✓        |  ✓        | ✓      |    ✓     |
| **PlayStation 4**| ✓✓✓        |  ✓        | ✓      |    ✓     |
| **Linux**        | ✓✓X         |  ✓        | ✓      |    ✓     |
| **Android**      | XXX         |  ✓        | ?      |    ✓     |
| **OSX**          | ✓✓X         |  ✓        | ✓      |    ✓     |
| **Emscripten**   | ✓✓✓         |  X        | ✓      |    ✓     |

✓ - Working  
X - Not yet implemented  
? - Not supported  

Platforms with partial implementations or not tested: **iOS, UWP, FreeBSD**  
Input (KMG) stands for Keyboard, Mouse and Gamepad  

Source Code
======

You can get the latest source code by cloning it from github:

      git clone https://github.com/RudjiGames/rapp.git 
	  
Build and dependencies
======

There's quite a few dependencies and they can be fetched manually or using a [batch file](https://github.com/RudjiGames/rapp/blob/master/scripts/fetch_dependencies.bat).
For the list of dependencies please refer to the batch file.

Once dependencies are cloned, [GENie](https://github.com/bkaradzic/genie) project generator tool can be used to generate project files/solution. [This](https://github.com/RudjiGames/rapp/blob/master/scripts/generate_project.bat) batch file is an example of generating a solution and project files for VS2022.

License (BSD 2-clause)
======

<a href="http://opensource.org/licenses/BSD-2-Clause" target="_blank">
<img align="right" src="https://opensource.org/wp-content/uploads/2022/10/osi-badge-dark.svg" width="100" height="137">
</a>

	Copyright 2025 Milos Tosic. All rights reserved.
	
	https://github.com/RudjiGames/rapp
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
	   1. Redistributions of source code must retain the above copyright notice,
	      this list of conditions and the following disclaimer.
	
	   2. Redistributions in binary form must reproduce the above copyright
	      notice, this list of conditions and the following disclaimer in the
	      documentation and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
	EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
