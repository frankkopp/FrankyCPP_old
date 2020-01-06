#!/bin/zsh
clang -Xclang -ast-dump -fno-diagnostics-color -fsyntax-only -I"src" -I"spdlog/include" -std=c++17 "$1"
