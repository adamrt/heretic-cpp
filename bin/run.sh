#!/usr/bin/env sh

rm -rf ./build/heretic \
  && cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build \
  && cmake --build build -j $(nproc) \
  && ./build/heretic
