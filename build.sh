#!/bin/bash

# Navigate to the ics-os source directory
cd "$(dirname "$0")/ics-os" || exit 1

# Generate build.h with timestamp
if [ -d "ics-os/kernel" ]; then
    echo "const char *build_id = \"$(date)\";" > ics-os/kernel/build.h
fi

# Default target is floppy
TARGET=${1:-floppy}

# Run the build command inside the Docker container
echo "Building target: $TARGET"
docker compose run --rm --workdir /home/ics-os ics-os-build make $TARGET
