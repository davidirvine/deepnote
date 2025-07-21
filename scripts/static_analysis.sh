#!/bin/bash
# Static analysis script for deepnote project

set -e

echo "Running clang-format check..."
find src test -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i --style=file

echo "Running clang-tidy..."
if command -v clang-tidy &> /dev/null; then
    find src -name "*.hpp" | xargs clang-tidy --checks=-*,readability-*,performance-*,modernize-*,bugprone-*
else
    echo "clang-tidy not found, skipping..."
fi

echo "Running cppcheck..."
if command -v cppcheck &> /dev/null; then
    cppcheck --enable=warning,style,performance,portability --inline-suppr --quiet src/
else
    echo "cppcheck not found, skipping..."
fi

echo "Static analysis complete!"
