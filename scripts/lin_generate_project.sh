#!/bin/bash
target="$1"

if [ -z "$target" ]; then
action="--gcc=linux-gcc-9 gmake"
fi

if [[ "$target" == "emscripten" ]]; then 
action="--gcc=asmjs gmake"
echo $action
fi

cd ../genie
../build/tools/bin/linux/genie $action
cd ../scripts
