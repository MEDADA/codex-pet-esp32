# Codex Pet ESP32 协作规范

本次整理分三步执行。先对照现有目录与交接清单，将三个候选包按版本归档。随后同步两个分支的入口及接入流程。最后从 GitHub 下载文件，核对分支、路径和 SHA-256。

| 路径 | 维护内容 |
| --- | --- |
| `shared/` | 已批准的角色母图和共用参考 |
| `idle/releases/` | IDLE 版本归档 |
| `coding/releases/` | WORKING 版本归档，兼容历史目录名 `coding` |
| `background/releases/` | 背景与光效版本归档 |
| `states/` | SUCCESS 等短反馈的设计 |
| `integration/` | 美术与固件的交接、校验值和验收资料 |
| `docs/`、`history/` | 项目规范、迁移记录和历史索引 |

美术候选使用 ASCII 版本化文件名，ZIP 等二进制走 Git LFS。每次交接记录所属模块、版本、路径、SHA-256 和已知问题。新版本另加文件，旧路径供既有交接引用。共享母图的替换按单独变更处理。

固件接入以固定的美术提交和文件校验值为输入。转换产物、构建配置、测试与实机记录归入 `firmware/esp32`。验收记录标明试烧镜像、涉及的分区、回退版本以及尚待确认的项目。美术候选、固件试烧和实机验收通过分别记录。

运行时维持 IDLE 和 WORKING 两个持续场景。Thinking 与 Coding 共用 WORKING，别名切换保持动画进度。SUCCESS 作为短反馈。完整约定见[项目索引](../PROJECT_INDEX.md)。
