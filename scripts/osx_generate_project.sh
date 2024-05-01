#!/bin/bash
target="$1"

if [ -z "$target" ]; then
action="--gcc=osx gmake"
fi

if [[ "$target" == "emscripten" ]]; then 
action="--gcc=asmjs gmake"
echo $action
fi

cd ../genie
../build/tools/bin/darwin/genie $action
cd ../scripts
