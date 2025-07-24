#!/bin/bash
# Static analysis script for deepnote project

set -e

echo "Running clang-format check..."
# Only format source code, not tests or third-party code
find src -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i --style=file

echo "Running clang-tidy..."
if command -v clang-tidy &> /dev/null; then
    # Only analyze source code for security and quality issues
    find src -name "*.hpp" -o -name "*.cpp" | xargs clang-tidy --checks=-*,readability-*,performance-*,modernize-*,bugprone-*,clang-analyzer-*,cppcoreguidelines-*
else
    echo "clang-tidy not found, skipping..."
fi

echo "Running cppcheck..."
if command -v cppcheck &> /dev/null; then
    # Focus static analysis on source code only
    cppcheck --enable=warning,style,performance,portability,information --inline-suppr --quiet --std=c++14 src/
else
    echo "cppcheck not found, skipping..."
fi

echo "Static analysis complete!"
