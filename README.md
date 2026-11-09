# Codex Pet ESP32

面向 Waveshare ESP32-S3 466×466 圆形 AMOLED 表盘。美术资源和 ESP-IDF 固件在同一仓库协作。

| 分支 | 用途 | 入口 |
| --- | --- | --- |
| `main` | 原始美术、候选包和交接记录 | [项目索引](PROJECT_INDEX.md) |
| `firmware/esp32` | 固件、板端素材、构建和实机验收 | [固件 README](https://github.com/MEDADA/codex-pet-esp32/tree/firmware/esp32) |

从[候选资源索引](integration/ASSET_IMPORT_MANIFEST_20261109.md)下载当前版本并核对 SHA-256。Git 克隆时先安装 Git LFS，再执行 `git lfs pull` 获取二进制内容。新增版本的目录、状态和交接流程见[协作规范](docs/REPOSITORY_WORKFLOW.md)。
