#!/bin/bash

set -eou pipefail

OS=$(uname)

if [[ "$OS" == "Linux" ]]; then
    OSPATH=linux
elif [[ "$OS" == "Darwin" ]]; then
    OSPATH=osx_arm64/
else
    echo "Unsupported OS: $OS"
fi

# download the sokol-shdc shader compiler
wget -q https://github.com/floooh/sokol-tools-bin/raw/master/bin/${OSPATH}/sokol-shdc
chmod +x sokol-shdc

# compile the shader
if [[ "$OS" == "Linux" ]]; then
    ./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl430
elif [[ "$OS" == "Darwin" ]]; then
    ./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl410
else
    echo "Unsupported OS: $OS"
fi

# get the submodules
git submodule update --init --recursive
