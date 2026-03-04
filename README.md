# YOLO26 多路演示（C++）

这是 YOLO26 目标检测与分割多路演示的 C++ 独立实现，面向 DEEPX M1 NPU，并提供基于 Qt 的动态 GUI 控制面板。

## 环境依赖

请先确保系统中已安装：
- DEEPX `dx_app` 和 DXRT Base（`libdxrt.so`）

并通过 `apt` 安装 C++ 依赖：
```bash
sudo apt-get update
sudo apt-get install -y qtbase5-dev libyaml-cpp-dev libopencv-dev
```

默认 `config/default.yaml` 中的模型与视频路径指向 `assets/...`。
- `src/`：核心应用代码（预处理引擎、推理多线程、Qt GUI 逻辑）
- `config/default.yaml`：项目内运行配置（模型路径 + 通道配置）
- `build.sh`：一键 CMake 编译脚本
- `run.sh`：一键运行脚本（默认读取 `config/default.yaml`）

## 编译方法

使用 `build.sh` 完成配置与编译：

```bash
# 标准 Release 编译（默认）
./build.sh

# Debug 编译
./build.sh debug

# 清理构建目录
./build.sh clean
```

## 运行方法

程序启动时会从仓库内的 YAML 文件读取运行参数。

```bash
# 使用默认配置（config/default.yaml）
./run.sh

# 使用自定义配置
./run.sh custom_config.yaml
```

## YAML 配置说明（简要）

默认配置文件：`config/default.yaml`

支持的关键字段：
- `model`：模型文件路径（字符串）
- `channels`：通道列表（数组）
  - `name`：通道名
  - `type`：`video` / `rtsp` / `camera`
  - `source`：当 `type` 为 `video/rtsp` 时是字符串；当 `type` 为 `camera` 时是整数设备号
  - `enabled`：是否启用该通道
  - `max_fps`：该通道最大帧率

示例：

```yaml
model: "assets/models/yolo26s-1.dxnn"
channels:
  - name: "ch1"
    type: "video"
    source: "assets/videos/cctv-city-road.mov"
    enabled: true
    max_fps: 25
```