#!/bin/bash
set -e

# Default directory expected by the app to locate video and model assets
DEMO_ROOT="../yolo26_demo"

# Check if a custom config string is provided, else use the default one
CONFIG_PATH=${1:-"demo/config/yolo26_multich.yaml"}

# Absolute paths
EXE_PATH="$(pwd)/build/yolo26_demo_cpp"

if [ ! -f "$EXE_PATH" ]; then
    echo "Error: Executable not found. Please run ./build.sh first."
    exit 1
fi

if [ ! -d "$DEMO_ROOT" ]; then
    echo "Error: Base directory $DEMO_ROOT not found. Ensure python demo is cloned alongside this cpp folder."
    exit 1
fi

# Override Wayland to prevent Qt display issues
export XDG_SESSION_TYPE=x11

echo "Starting YOLO26 Demo with config: $CONFIG_PATH"
# Switch to demo root working directory to ensure relative paths resolve correctly
cd "$DEMO_ROOT"
"$EXE_PATH" "$CONFIG_PATH"