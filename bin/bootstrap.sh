#!/usr/bin/env sh

# download the sokol-shdc shader compiler
wget -q https://github.com/floooh/sokol-tools-bin/raw/master/bin/linux/sokol-shdc
chmod +x sokol-shdc

# compile the shader
./sokol-shdc -i src/shader.glsl -o src/shader.glsl.h -l glsl430

# get the submodules
git submodule update --init --recursive
