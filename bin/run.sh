#!/usr/bin/env sh


OS=$(uname)

if [[ "$OS" == "Linux" ]]; then
  CORES=$(nproc)
elif [[ "$OS" == "Darwin" ]]; then
  CORES=$(sysctl -n hw.ncpu)
else
    echo "Unsupported OS: $OS"
fi

rm -rf ./build/heretic \
  && cmake -S . -DCMAKE_BUILD_TYPE=Debug -B build \
  && cmake --build build -j $CORES \
  && ./build/heretic
