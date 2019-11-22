#!/usr/bin/env bash
find ./src ./export -type f \( -iname \*.c -o -iname \*.cpp -o -iname \*.h -o -iname \*.hpp \) | xargs clang-format -style="{ColumnLimit : 80}" -i
