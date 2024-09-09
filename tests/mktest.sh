#!/bin/bash

# Check if a name is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <directory_name>"
    exit 1
fi

# Get the directory name from the first argument
DIR_NAME=$1

# Create the directory
mkdir -p "$DIR_NAME"

# Copy the contents of test/CMakeLists.txt to the new CMakeLists.txt file
cp "test/CMakeLists.txt" "$DIR_NAME/CMakeLists.txt"
cp "test/test.c" "$DIR_NAME/test.c"

echo "Directory '$DIR_NAME' and CMakeLists.txt file created."

