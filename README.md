# YOLO26 Multi-Channel Demo (C++)

This is the C++ standalone implementation of the YOLO26 Object Detection & Segmentation multi-channel demo for the DEEPX M1 NPU, fully equipped with a Qt-based dynamic GUI control panel.

## Prerequisites

Ensure you have the following installed on your system:
- DEEPX `dx_app` and DXRT Base (`libdxrt.so`)

You must also install the required C++ dependencies via `apt`:
```bash
sudo apt-get update
sudo apt-get install -y qtbase5-dev libyaml-cpp-dev libopencv-dev
```

## Project Structure
- `src/` : Contains the core application, the preprocessing engine, inference multithreading workers, and Qt GUI logic.
- `build.sh` : One-click CMake build script.
- `run.sh` : One-click execution script (relies on configurations from `../yolo26_demo`).

## Build Instructions

Use the provided `build.sh` script to configure and compile the executable. 

```bash
# Standard Release Build (Default)
./build.sh

# Debug Build
./build.sh debug

# Clean Build Directory
./build.sh clean
```

## Running the Demo

The executable parses its settings (Channels, FPS mapping, RTSP/Video sources, UI configurations) dynamically from the shared YAML file.

```bash
# Run with default configuration (../yolo26_demo/demo/config/yolo26_multich.yaml)
./run.sh

# Run with a custom user configuration
./run.sh custom_config.yaml
```

> **Note**: The execution script automatically switches to the `yolo26_demo` environment directory. Ensure relative paths defined inside your YAML points properly towards `assets/...`.