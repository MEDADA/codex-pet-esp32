# Codex Pet ESP32 仓库结构

同一个 GitHub 仓库包含两个工作分支。美术维护 `main` 的原始资源和交接文档。固件维护 `firmware/esp32` 的 ESP-IDF 工程和板端打包资源。两个分支独立提交，固件从明确的美术提交取材，经转换和验收后提交到固件分支。

| 路径 | 内容 |
| --- | --- |
| `main/` | 固件、CodeIsland BLE 接入和板端动画 |
| `main/generated/`、`assets/fontfs/` | 构建所需的嵌入资源和 SPIFFS 素材 |
| `configs/` | Pet 和 Rem 的构建配置 |
| `tools/`、`tests/` | 素材转换、串口采样和主机回归 |
| `vendor/` | 固件引用的交接代码和测试素材 |
| `docs/` | 开发计划、架构评审和验收记录 |

固件接入时，从[美术项目索引](https://github.com/MEDADA/codex-pet-esp32/blob/main/PROJECT_INDEX.md)选定候选并记录美术提交、原始路径和 SHA-256。资源归档使用 Git LFS，在美术工作区下载后再转换为板端格式。工作流详见[协作规范](https://github.com/MEDADA/codex-pet-esp32/blob/main/docs/REPOSITORY_WORKFLOW.md)。

固件提交记录转换产物与构建配置。试烧时保留可回退的板载版本，验收记录注明实际镜像、分区、交互结果与待验证项。设备读回、串口日志和本机凭据留在本地。
