#!/bin/bash

git clone https://github.com/bkaradzic/bx.git   ../../bx   ) else ( echo skipping bx     - directory exists )
git clone https://github.com/bkaradzic/bimg.git ../../bimg ) else ( echo skipping bimg   - directory exists )
git clone https://github.com/bkaradzic/bgfx.git ../../bgfx ) else ( echo skipping bgfx   - directory exists )

git clone https://github.com/RudjiGames/build.git ../../build  )  else ( echo skipping build  - directory exists )
git clone https://github.com/RudjiGames/rbase.git ../../rbase  )  else ( echo skipping rbase  - directory exists )
git clone https://github.com/RudjiGames/rprof.git ../../rprof  )  else ( echo skipping rprof  - directory exists )

git submodule update --init
