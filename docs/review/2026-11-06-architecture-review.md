# CodeIsland Buddy 架构评审（2026-11-06）

范围：`codeisland-pet-v11-1`（在跑的固件）+ `CodeIsland` 的 BLE 主机侧。
`codeisland-buddy-1.75c`、`xiaozhi-esp32`、`ESP32-S3-Touch-AMOLED-1.75C`、`xiaozhi-firmware`
只作为对照物，不是被评审对象。本文所有数字来自当前 `build/`（ESP-IDF 6.1.0，
target esp32s3，LVGL 9.5.0）与 `docs/development/` 下的实机日志。

---

## 0. 结论

**技术选型是对的，不需要换框架。** ESP-IDF 6.1 + LVGL 9.5 + NimBLE + Waveshare BSP
是这块板子（ESP32-S3 / 32MB Flash / 8MB PSRAM / CO5300 QSPI 466×466）上成本最低、
风险最小的组合。换成 xiaozhi-esp32 的板级抽象、ESP-Brookesia、Arduino 或 MicroPython
都不会得到更好的结果，理由见 §5。

**真正的债在工程卫生和三条边界上**：资产与分区的边界、解码与渲染线程的边界、
"这个配置到底还要不要"的边界。这三处都属于可修，不是推倒重来。

一句话优先级：**先把 3.5MB 资产搬出 app 分区，再把 JPEG 解码搬出 LVGL 线程，
然后删掉 pet 模式里已经用不到但仍在初始化的 WiFi / 音频编解码器 / FreeType。**

---

## 1. 现状（实测，不是文档描述）

### 1.1 镜像构成

| 项 | 数值 | 占比 |
| --- | --- | --- |
| app bin | 5,835,264 B（5.57 MiB） | app 分区 8 MiB 的 69.6% |
| `.flash.text` | 1,600,048 B（1.53 MiB） | 27% |
| `.flash.rodata` | 4,079,084 B（3.89 MiB） | 70% |
| 其中宠物图片 | 3,646,569 B（3.48 MiB） | `pet_idle_v3_frames.bin` 2.53MB + `pet_v41_layers.bin` 1.11MB |
| 其中中文字体 | ≈0.45 MiB | `font_buddy_cn_16/20`、`font_buddy_clock_64` |
| storage 镜像 | 8.0 MB / 12 MiB 分区 | 12 个 `.mjp` 动作包 + 128KB TTF |
| Flash 实际分配 | ≈20 MB / 32 MB | **12 MB 未进入分区表** |

**87% 的 app 镜像是静态美术资源。** 这是后面所有容量/OTA 问题的根。

### 1.2 运行时余量（实机日志）

| 指标 | 数值 | 出处 |
| --- | --- | --- |
| 内部堆空闲 | 18–65 KB | `p2/final-codeisland-boot.log` |
| 内部堆历史最低 | **6,983 B** | `two-state/pet_mem` |
| 当时最大连续块 | 13,312 B | 同上 |
| PSRAM 空闲 | 8.09 MB / 8 MB | 同上（几乎没用） |
| LVGL 渲染区域耗时 | 均值 26.9–31.4 ms，最大 67.3 ms | `two-state/trial-serial.log` |
| 实际刷新率 | 56–76 次 / 10 s ≈ 6.5 fps | 同上（目标是 12 fps） |
| 单帧 JPEG 解码 | 最大 22–37 ms | `pet_idle: updates=120 max_decode_us=` |
| 单次持锁最坏 | ≈ 37 + 67 ≈ **104 ms** | 解码与渲染同在 LVGL 线程 |

对比：`buddy_pet_ui.c:76` 取显示锁的超时是 **50 ms**，`buddy_ui_tick` 在拿不到锁时
直接 return。也就是说上面这个 104 ms 的窗口里，UI 任务必然跳过若干次 tick。这不是
理论风险，`docs/development/pet-v11-1/p2/display-lock-red.log` 就是它发生过一次的证据。

### 1.3 线程与数据流

```
Mac (CodeIsland/ESP32StatePublisher, 5s 心跳 + 变更即发)
  │  CoreBluetooth, .withoutResponse, ATT MTU 23 → 每包 20 字节
  ▼
NimBLE host task (core 0)  nimble_port_freertos_init
  ├─ handle_write()  → 三处重复的帧校验（buddy_ble.c / buddy_state_evidence.c / buddy_time.c）
  │                  → s_state（一把 s_lock 保护）
  │                  → 亮屏帧直接调 buddy_settings_set_brightness() → NVS 写
  ▼
buddy_ui task (core 1, prio 7, 16KB)  main.c:56
  ├─ buddy_ble_snapshot()  → 拷贝 s_state
  ├─ buddy_rtc_read() (1s) / buddy_power_read() (4s)
  ├─ bsp_display_lock(≤50ms)  ← 与 LVGL 任务竞争
  └─ pet_present() → pet_view_show() → lv_* 调用
  ▼
LVGL task (esp_lv_adapter 创建)  持锁跑 lv_timer_handler()
  └─ pet_idle_view 的 lv_timer(16ms) → jpeg_dec_open/process/close（22–37ms，持锁）
```

分层本身是清楚的：**协议/状态机（C，纯逻辑）→ 呈现（`pet_present`，纯函数）→ 视图（LVGL）→ 驱动（BSP）**。
`pet_presentation.c` 和 `buddy_state_evidence.c` 能在主机上直接单测，是整个工程里
最好的一层设计。

---

## 2. 做对的地方（不要动）

1. **`pet_present()` 是纯函数**，`pet_presentation.c:3` 一个函数吃状态吐 UI 描述符，
   不碰 LVGL。这让 11 组主机测试成为可能，也是后续换 UI 的唯一正确接缝。
2. **"不伪造在线"的状态语义**（`buddy_state_evidence.c`、`primary/current-connection`
   证据标志）。这是这类桌面挂件最容易做错的地方，你们做对了。
3. **开发板配置走 Kconfig 三档可回退**（`CONFIG_BUDDY_PET_V11` / `_ANIMATION` / `_IMU`），
   并且保留了 `configs/sdkconfig.rem` 的 Rem 回退构建。硬件项目很需要这个。
4. **UI 与 BLE 分核**（`main.c:56` 与 `CONFIG_BT_NIMBLE_PINNED_TO_CORE_0`），这个决定是对的，
   只是被 §5.2 的解码阻塞削弱了一半。
5. **显示缓冲改成内部 4 行 + 关 PSRAM 双缓冲**（`buddy_display_buffer.c`）。方向完全正确
   ——SPI LCD 的 DMA 缓冲本来就不该放 PSRAM。
6. **实机验收文化**：读回全片、SHA256、分区保持证据、拒绝无效读回。比大多数同类项目严谨。

---

## 3. 问题清单

严重度按"会不会咬人"排，不按代码美观排。

### P0-1　资产在 app 分区里，OTA 被堵死

- 事实：app 5.57 MiB / 8 MiB，`partitions.csv` 只有 `factory` + `storage`，**没有 otadata/ota_0/ota_1**。
  32MB Flash 里有 12MB 没进分区表。
- 后果：任何固件更新都必须 USB 刷写；也没有回滚槽。CodeIsland 主机侧有 Sparkle 自动更新，
  手表侧没有，两边升级节奏必然分叉。
- 根因不是"没做 OTA"，是**3.48MB 美术资源被编译进了 app**，导致双槽 OTA 看起来"装不下"。
  把资源挪到 storage 后 app ≈ 2.1 MiB，ota_0/ota_1 各 3MB 即可，剩余 ~24MB 给 storage。

### P0-2　JPEG 解码跑在 LVGL 线程里

- `pet_idle_view.c:104` 用 `lv_timer_create(advance, 16)` 注册；`advance()` 里
  `jpeg_dec_open/parse/process/close`（22–37ms）。
- `esp_lv_adapter` 的 `lvgl_worker` 是**持锁**调 `lv_timer_handler()` 的
  （`managed_components/espressif__esp_lvgl_adapter/src/adapter/esp_lv_adapter.c`）。
  所以这 22–37ms 是持锁时间，直接叠加到渲染的 27–67ms 上。
- 后果：目标 12fps 实际 6.5fps；最坏 104ms 收不到触摸和按钮；`buddy_ui_tick` 的 50ms
  取锁超时会跳过。
- `pet_motion_clip.c:58/64` 是同一问题（SPIFFS fread + JPEG 解码），只不过它发生在
  `buddy_ui` 任务持锁期间，同样阻塞 LVGL。
- 正确做法：双缓冲 + 独立解码任务。LVGL 侧只做 `lv_image_set_src(&bufA)` +
  `lv_obj_invalidate`，耗时降到微秒级。8MB PSRAM 现在空着 8.09MB，
  两个 272×320 RGB565 缓冲只要 340KB。

### P0-3　pet 模式仍在初始化它已经用不到的东西

| 位置 | 仍在做的事 | 实际需要吗 |
| --- | --- | --- |
| `main.c:50` `buddy_voice_init()` | → `buddy_network_init()` → 读 NVS `rem_net`，有 SSID 就起 WiFi STA | **不需要**，pet UI 没有任何语音入口 |
| `buddy_settings.c:69` | `bsp_audio_codec_speaker_init()`（I2S TX + ES8311） | **不需要**，pet UI 不播音频 |
| `sdkconfig.defaults:49` `CONFIG_LV_USE_FREETYPE=y` + `main/CMakeLists.txt:31` 链 freetype | `lv_init()` → `lv_freetype_init()`（`lvgl/src/lv_init.c:231`） | **不需要**，`fonts/buddy_dynamic_fonts.c` 已是空实现 |
| `main/CMakeLists.txt:32` | 把 8MB `assets/fontfs` 烧进 storage | 只需要其中 12 个 `.mjp`；TTF 没有任何字体加载路径 |

最危险的是第一行：**如果这台设备当年跑 Rem 时在 NVS 里存过 SSID，那么现在每次开机都会
静默启动 WiFi**，与 BLE 共存、吃内部堆（最低已经 6.9KB），而且 pet UI 没有关掉它的入口。
这解释了为什么 `sdkconfig.defaults` 里要堆 `MSYS_2_BLOCK_COUNT=48`、`ACL_FROM_LL_COUNT=48`
这种超配。

### P0-4　两套固件树并存，且都是"真值"

- `codeisland-buddy-1.75c/`（346 个文件，**不是 git 仓库**）与 `codeisland-pet-v11-1/`（git 仓库）。
- 两者 `main/` 有 20+ 个同名文件，`buddy_ble.c`/`buddy_time.c`/`buddy_protocol.h` 已经分叉。
- `docs/development/pet-v11-1/p2/HANDOFF.md` 里"346 个基线文件逐字节哈希比对没有变化"
  这句话本身就说明：**你们在用人工哈希比对来维护一个本该由 git 提供的性质。**
- 后果：改一个协议字段要改两处；BUG 修一处漏一处；新人无法判断哪份是在跑的。

### P0-5　时钟有三份实现，其中一份是死代码

| 实现 | 位置 | 谁在用 |
| --- | --- | --- |
| `buddy_wall_time_now/date_now` | `buddy_protocol.h` 内联 | `pet_present()`（真正显示） |
| `buddy_time_*`（NVS 持久化影子时钟） | `buddy_time.c` 261 行 | **只有自己文件夹用**；UI 从不读 |
| `buddy_rtc_read`（系统时间） | `buddy_rtc.c` | UI 的 1s 轮询 |

`buddy_time.c` 唯一真实副作用是 `buddy_rtc_sync()`（settimeofday）。它的 NVS 影子状态、
`persist()`、以及 `buddy_time_now()` 全都没人读。而 `persist()` 在 `buddy_time.c:177/203`
**每分钟写一次 NVS**（新 Mac 每 5s 发一次 H:M:S，`buddy_ble.c` 的去重让它退化成每分钟一次）。
即 1440 次/天、且发生在 NimBLE host task 上。

顺带：同一个 `0xF6` 帧在 `buddy_ble.c`、`buddy_state_evidence.c`、`buddy_time.c` 里
被解析和校验了 **三遍**，三处的边界检查还不完全一致（`buddy_time.c` 允许 day≤31 但只做
`buddy_date_valid` 二次判断，`buddy_state_evidence.c` 里又内联了日期校验）。

### P1-1　BLE 被钉死在 20 字节负载

- `sdkconfig.defaults:24` `CONFIG_BT_NIMBLE_ATT_PREFERRED_MTU=23`。
- 后果：一次完整同步（agent + workspace + model + stats + subagent + preview×8 + history×10）
  ≈ 25 次 write；macOS 侧 `ESP32BridgeManager.swift:459` 只能逐包 drain 队列。
- 于是有了 `buddy_ble.c:59-60` 的 `BUDDY_WRITE_RATE_LIMIT 120` 和 `BUDDY_MBUF_FAIL_LIMIT 8`，
  以及 §P0-3 提到的 MBUF 池超配。**这是在用固件补一个协议带宽问题。**
- macOS 默认会协商到 185+；把 preferred MTU 提到 247，配合已有的 `E8` 能力协商
  加一个"支持单包整帧"的 flag，25 包 → 7 包，洪水逻辑可以整段删掉。

### P1-2　`--wrap` 链接器劫持

- `main/CMakeLists.txt:36` 用 `-Wl,--wrap=esp_lv_adapter_register_display` 改 BSP 的私有
  默认值（BSP 里 `buffer_height=50, use_psram=true, double_buffer=true` 是硬编码的，
  `bsp_display_start_with_config()` 不暴露）。
- 能用，但：任何一次 BSP 升级改函数名/调用路径就静默失效（`__wrap_` 没人调，回到 50 行 PSRAM 双缓冲，
  表现为掉帧而不是报错）；而且它劫持的是**全局**符号。
- 更好：把 BSP 作为本地组件 vendor 进来改一行；或绕过 `bsp_display_start()`，
  自己调 `bsp_display_new()` + `esp_lv_adapter_register_display(&cfg)`（约 40 行）。
  保留 wrap 也可以，但要加一个启动期断言（读回实际 `buffer_height`）防止静默退化。

### P1-3　死代码与构建垃圾

| 内容 | 体积 | 状态 |
| --- | --- | --- |
| `main/generated/pet_v11_assets.c` | 4.3 MB / 35,542 行 | **在 CMake SRCS 里编译**，但 `nm` 确认 `pet_v11_background/open/closed` 全部被 gc-sections 丢弃 |
| `main/pet/*.c` | 8.7 MB | 不在 SRCS，从未编译 |
| `main/buddy_ui.c` + 3 个 `.bak-*` | 3,508 行 / ≈120 KB | 不在 SRCS |
| `main/generated/rem_assets.c` | 27 MB / 170,273 行 | 只在 `CONFIG_BUDDY_PET_V11=n` 时编译 |
| `restore-backup-before-stock-20261029-162226/` | 整份源码副本 | 在 git 里 |
| `assets/frames`、`assets/rem-v2`、`preview*` | 12 MB | 在 git 里 |
| `build/` 里的 `pet_idle_frames.bin.S`、`pet_trial_*.bin.S` | 3.6 MB | 旧配置残留，当前 CMake 已不再引用 |

git 里跟踪 489 个文件 / 75 MB，其中 `main/` 贡献 40 MB。每次 clean build 都要编译
那个 4.3MB 的 C 数组（生成 840KB 的 .o，然后被链接器整个丢掉）。

### P1-4　没有版本协商，60s 超时与 5s 心跳互相冗余

- 没有任何 characteristic 暴露固件版本 / 协议版本。`ESP32Protocol.swift` 里
  因此留着"legacy Mac 1.0.40 只发小时"的兼容分支（`buddy_ble.c:352` 注释）。
- 固件 60s 无写入退出 AGENT（`BUDDY_BLE_TIMEOUT_MS`），主机 5s 心跳（`minHeartbeatSeconds = 3.0`）。
  BLE 链路本身有 supervision timeout（秒级），链路断了 `BLE_GAP_EVENT_DISCONNECT` 立刻知道。
  **应用层的 60s 超时是多余的**，它换来的只是"主机还活着"的弱保证，代价是 5s 一次的常驻射频活动
  和电量。

### P1-5　没有 CI

- `codeisland-pet-v11-1` 没有 `.github/`。11 组主机测试靠 `python tools/test_pet_p0.py` 手动跑，
  且需要带 Pillow 的 Python；`tests/` 里没有 CMake/Makefile 包装。
- `-Werror` 是开着的（好事），但 `CMakeLists.txt:3` 全局 `add_compile_options(-Wno-format)`
  把格式化字符串检查关掉了——这正好会掩盖 `ESP_LOGI("%u", size_t)` 一类问题。

### P2-1　代码风格两极化，没有格式化基线

`buddy_time.c` 是正常可读的 261 行；`buddy_network.c:76` 的 `save_form()` 是一整行
（约 25 个语句）；`buddy_requests.c`、`buddy_io_contracts.h` 全是单行。
仓库里**没有 `.clang-format`**（对照：`xiaozhi-esp32/.clang-format` 是现成的）。
后果：code review 只能看逻辑不能看 diff，多人/多 agent 接手的成本被放大——
这正是这套项目已经出现过的问题（3 个 `.bak` 文件就是没有版本控制的痕迹）。

### P2-2　依赖与配置卫生

| 位置 | 问题 |
| --- | --- |
| `main/idf_component.yml:8` | `espressif/esp_new_jpeg: "*"`，无版本约束（有 lock 兜底，但 manifest 层面不可复现） |
| `CMakeLists.txt:3` | 全局 `-Wno-format` |
| `sdkconfig.defaults:37` | `CONFIG_IDF_EXPERIMENTAL_FEATURES=y`，用途不明 |
| `sdkconfig.defaults:57` | `SPIRAM_MALLOC_ALWAYSINTERNAL=4096`：所有 <4KB 的分配都进内部堆，而内部堆最低只剩 6.9KB、PSRAM 空着 8MB |
| `buddy_ble.c:655` | `buddy_ble_get_state()` 返回共享 `s_state` 的裸指针且**不加锁**（当前只被死代码 `buddy_ui.c` 使用，建议直接删） |
| `buddy_ble.c:265` | 亮屏帧在 NimBLE host task 里直接触发 NVS 写（`save_u8`），可能阻塞 BLE 协议栈数十毫秒 |
| `partitions.csv` | 32MB Flash 只用了 20MB |

---

## 4. 如果只做五件事

按收益/风险比排序，每项都能独立上线、独立回退。

| # | 动作 | 收益（可验证） | 风险 | 工作量 |
| --- | --- | --- | --- | --- |
| 1 | pet 构建里删掉 `buddy_voice_init()`、`bsp_audio_codec_speaker_init()`、`CONFIG_LV_USE_FREETYPE` 及其链接 | 内部堆 +20~40KB；消除"老旧 NVS 里有 SSID 就静默开 WiFi"的隐雷 | 低（pet 模式无调用方） | 0.5 天 |
| 2 | JPEG 解码移到独立任务 + 双缓冲（PSRAM），LVGL 只换指针 | 持锁时间从 104ms 降到 <5ms；刷新率 6.5→12fps；触摸不再被吞 | 中（要处理缓冲交换与生命周期） | 2–3 天 |
| 3 | 资产外置：`pet_idle_v3_frames.bin` / `pet_v41_layers.bin` / 字体 → `storage` 分区，用已在依赖树里的 `esp_mmap_assets`（`esp_lv_fs` 已经把它拉进来了，零新增依赖） | app 5.57→≈2.1 MiB；clean build 时间大幅下降；git 瘦身 35MB | 中（LVGL 直接读 mmap 地址，需上板确认 cache 行为） | 3–5 天 |
| 4 | 分区表加 `otadata` + `ota_0/ota_1`，接 `esp_https_ota`（WiFi 按需起，只在升级时） | 首次获得可回滚的无线升级；`esp_ota_mark_app_valid_cancel_rollback()` 兜住坏固件 | 中（需要 host 侧托管固件 URL） | 3–5 天 |
| 5 | 删死代码 + 加 `.clang-format` + 把 `tools/test_pet_p0.py` 包成 `idf.py` 自定义 target + GitHub Actions（构建+主机测试+分区尺寸检查） | 每次 PR 自动挡住"镜像超 8MB""测试挂了" | 低 | 1–2 天 |

---

## 5. 与同类项目的架构对比

### 5.1 横向对比

| 维度 | 本项目 | xiaozhi-esp32 | Waveshare 官方 demo | ESP-Brookesia |
| --- | --- | --- | --- | --- |
| 定位 | 单板桌面挂件 | 多板语音助手 | 板级外设示例 | 类手机 UI 框架 |
| 板级抽象 | 直接吃 BSP，无抽象层 | `Board` 虚基类 + 50 板目录 / 150+ 板配置 | 每示例独立 | 组件化 app |
| UI | 手写 LVGL 9 单屏 | `Display` 抽象 + 表情/主题 | LVGL demo | SquareLine + app manager |
| 资产 | 编译进 app（3.48MB）+ SPIFFS `.mjp` | `assets` 分区 + `esp_mmap_assets` + 版本化 `index.json` | 示例内小图 | 内嵌 |
| 通信 | BLE GATT 自定义帧 | MQTT/WebSocket + MCP | 无 | 无 |
| OTA | **无** | `esp_https_ota` + 双槽 + 回滚 + 资产版本 | release 包手刷 | 无 |
| 状态机 | `pet_present()` 纯函数（好） | `DeviceStateMachine` 显式状态图 | 无 | app 生命周期 |
| 测试/CI | 11 组主机测试，手动跑 | GitHub Actions 全矩阵 | GitHub Actions | 有示例 CI |

### 5.2 该借什么、不该借什么

**不要借 xiaozhi 的板级抽象。** 它服务 150+ 块板配置，用虚基类 + 每板一个目录是合理代价；
你只有一块板、一个产品，抽象只会增加一层间接。你们"直接吃 BSP"的选择是对的。

**该借 xiaozhi 的两件事：**
1. **资产分区 + `esp_mmap_assets` + 资产版本号**（`main/assets.cc`）。这正是解决 P0-1 的标准答案，
   而且你们已经把这个库链接进来了（`esp_lvgl_adapter` → `esp_lv_fs` → `esp_mmap_assets`）。
2. **OTA 双槽 + `esp_ota_mark_app_valid_cancel_rollback()` + 首次启动自检**
   （`main/ota.cc:261-276`）。配 32MB Flash 完全放得下。

**不要借 ESP-Brookesia。** 它是给"多 app、可切换桌面"的产品用的；你只有一个全屏场景，
引入 app manager 只会把 30ms 的渲染预算分给调度器。Waveshare 的 03 示例自己也承认
要保 40KB 的 PSRAM 任务栈并逐版本回归。

**不要换 Arduino / MicroPython。** 你们已经用上了 LVGL 9.5、双核亲和、NimBLE 精细配置、
PPA 相关组件；这些在 Arduino 上不可控，在 MicroPython 上性能差一个量级。

本板的 ESP32-S3 无 PPA。当前工程使用的 ESP-IDF 仅在 SoC 声明 `SOC_PPA_SUPPORTED` 时编译 PPA 驱动，ESP32-P4 声明了该能力。本板的动画优化优先缩短 LVGL 持锁时间，并继续沿用已验证的内部 DMA 缓冲。

### 5.3 选型评估结论

| 选择 | 评价 | 理由 |
| --- | --- | --- |
| ESP-IDF 6.1（非 Arduino） | ✅ 正确 | 需要 NimBLE 精细调参、双核亲和、PSRAM/Flash 模式、OTA |
| LVGL 9.5（当初拒绝降级 8） | ✅ 正确 | BSP 与 adapter 都是 9.x；降级会牵动整个已验证的显示/触控链 |
| NimBLE（非 Bluedroid） | ✅ 正确 | 更小、可配置项更细、单连接场景足够 |
| Waveshare BSP 托管组件 | ✅ 基本正确，⚠️ 可调项缺失 | 省掉 CO5300/CST9217/AXP2101 全部驱动工作；代价是 `buffer_height` 被写死，于是有了 wrap hack |
| `esp_lv_adapter` | ✅ 正确，⚠️ 必须遵守它的线程模型 | 它给的锁/任务模型是"LVGL 只在它的任务里跑"，当前代码在 timer 回调里做重活违反了这一点 |
| SPIFFS（12MB storage） | ⚠️ 建议换 | IDF 6 下 SPIFFS 已是遗留选择；`.mjp` 是顺序读，`esp_mmap_assets`（mmap 直读）比 `fseek/fread` 快一个量级且省 RAM |
| C + 少量 C++ | ✅ 可以 | C++ 只用在 IMU 跟踪器（模板/`isfinite`），没有异常/RTTI 负担；代价是构建里混了两种语言 |
| 无 OTA | ❌ 该补 | 见 P0-1 |

---

## 6. 建议的目标结构

```
codeisland-buddy/                 # 单一仓库，删掉 codeisland-buddy-1.75c 的副本
├─ main/
│  ├─ app/            main.c, buddy_pet_ui.c            # 启动 + 80ms tick
│  ├─ ble/            buddy_ble.c, buddy_protocol.h, buddy_requests.c
│  ├─ domain/         pet_presentation.c, buddy_state_evidence.c, buddy_time.c(合并后)
│  ├─ ui/             pet_view_lvgl9.c, pet_idle_view.c, pet_motion_clip.c
│  ├─ media/          decoder_task.c   ← 新增：唯一持有 JPEG 解码器的地方
│  ├─ drivers/        buddy_button.c, buddy_power.c, buddy_rtc.c, buddy_imu6.c
│  └─ net/            buddy_network.c, buddy_voice.c   ← 只在 CONFIG_BUDDY_VOICE 时编译
├─ assets/            *.png 源 + 生成脚本（产物不入库）
├─ partitions.csv     nvs | otadata | phy | ota_0(3M) | ota_1(3M) | assets(25M)
└─ tools/             test runner, asset builder, CI 脚本
```

关键约束（写进 README，比写进文档更有用）：

1. **LVGL 线程只做 LVGL**：任何解码/文件 IO/网络都不允许出现在 `lv_timer` 或
   `bsp_display_lock` 区间内。
2. **app 分区不含位图**：图片一律走 `assets` 分区，`lv_image_dsc_t` 的 `data` 指向 mmap 地址。
3. **一个帧只解析一次**：`0xF6` 等帧在 `ble/` 层校验后，domain 层只消费结果。
4. **协议有版本**：新增只读 characteristic 暴露 `fw_version` / `proto_version`，
   主机侧据此分支，而不是靠"旧 Mac 只发小时"的注释。

---

## 7. 附：本文数字的复现方法

```bash
# 镜像构成
size -A build/codeisland_buddy_1_75c.bin 2>/dev/null || \
  python3 -c "print(open('build/codeisland_buddy_1_75c.bin','rb').read(1))"
grep -E "^\.flash\.(text|rodata)" build/codeisland_buddy_1_75c.map

# 哪些资产真的被链接进去了
nm -S --size-sort build/codeisland_buddy_1_75c.elf | grep -E "pet_(v11|v41|idle)"

# 实测数字
grep -h "pet_render\|pet_idle\|pet_mem" docs/development/pet-v11-1/**/*.log
```

---

## 8. 修复记录（2026-11-06 第一轮）

原则：每步可独立回退，改完立即用真实 ESP-IDF 交叉编译 + 主机测试验证。
**没有烧写、没有复位设备**，所有结论都只到"编译、链接、分区检查、主机测试"为止。

### 8.1 结果

| 指标 | 基线 | 修复后 | 变化 |
| --- | --- | --- | --- |
| app bin | 5,835,264 B（0x590a00） | 4,793,232 B（0x492390） | **−1,042,032 B（−17.9%）** |
| app 分区余量 | 30% | 43% | +13 个百分点 |
| `.flash.text` | 1,600,048 | 782,760 | −817,288 |
| `.flash.rodata` | 4,079,084 | 3,884,932 | −194,152 |
| `.dram0.data` | 23,807 | 19,311 | −4,496 |
| `.dram0.bss` | 21,696 | 7,040 | −14,656 |
| `.iram0.text` | 125,763 | 104,783 | −20,980 |
| `main/` 目录 | 48 MB | 35 MB | −13 MB |
| 主机测试 | 11 组 C + 1 C++ + 2 Python | 同样全绿 | — |

文本段几乎腰斩，主因是 FreeType（含 LVGL freetype 模块）从未被使用却一直链接在镜像里。

### 8.2 改动清单

| # | 改动 | 文件 | 说明 |
| --- | --- | --- | --- |
| 1 | 新增 `CONFIG_BUDDY_VOICE_BRIDGE`（pet 默认 n / Rem 默认 y） | `main/Kconfig.projbuild`、`main/CMakeLists.txt` | `buddy_voice.c`、`buddy_network.c` 只在需要时编译 |
| 2 | pet 模式不再初始化语音/网络 | `main/main.c` | 消除"老 NVS 里有 SSID 就静默开 WiFi"的隐雷 |
| 3 | pet 模式不再初始化 ES8311/I2S | `main/buddy_settings.c` | `CONFIG_BUDDY_VOICE_BRIDGE=n` 时跳过 `bsp_audio_codec_speaker_init()` |
| 4 | 关闭 FreeType 并移除强制链接 | 5 个 sdkconfig + `main/CMakeLists.txt` | `fonts/buddy_dynamic_fonts.c` 早已是空实现 |
| 5 | 删除 NVS 影子时钟，时间同步收敛到 `buddy_rtc_apply_hint()` | 删 `buddy_time.c/.h`；改 `buddy_rtc.c/.h`、`buddy_ble.c`、`main.c` | 261 行死代码 + 每分钟一次 NVS 写消失；行为不变（UI 本来就只读系统时钟） |
| 6 | 删除 `buddy_ble_get_state()` | `buddy_ble.c/.h` | 返回共享 `s_state` 裸指针且不加锁，唯一调用方是死代码 |
| 7 | 亮度 NVS 写从 NimBLE host task 挪到 UI task | `buddy_settings.c/.h`、两个 UI 的 tick | `buddy_settings_persist()` 由 80ms tick 冲刷 |
| 8 | 删除未编译/未引用的源码 | `main/pet/*`（8.7MB）、`main/buddy_ui.c` 及 3 个 `.bak`（3,508 行） | 全部在 git 中，可用 `git checkout -- <path>` 恢复 |
| 9 | 停止全局 `-Wno-format` | `CMakeLists.txt` | 暴露并修掉了 2 处 `snprintf` 截断隐患（`clock[6]`→`clock[8]`） |
| 10 | `esp_new_jpeg` 从 `"*"` 改为 `1.0.2` | `main/idf_component.yml` | 与 `dependencies.lock` 一致，manifest 层可复现 |

验证命令与结果：

```bash
source ~/esp/esp-idf-jh/export.sh
idf.py build                      # pet：绿，0x492390，43% 余量
python3 tools/test_pet_p0.py      # 主机测试：11 组 C + 1 C++ + 2 Python 全绿
idf.py -B build-rem -DSDKCONFIG=configs/sdkconfig.rem build   # Rem 回退：绿，0x6f7f60，13% 余量
```

Rem 回退配置也从 7,779,264 B 降到 7,307,104 B（同样去掉了 FreeType）。
它只剩 13% 余量，正好说明第 3 项（资产外置）对两条构建路径都是必需的，不只是 pet。

### 8.3 下一步（未做，需要实机验证）

1. **解码移出 LVGL 线程**（`pet_idle_view.c` / `pet_motion_clip.c`）。这是剩余收益最大的一项，
   但它改变时序行为，必须在设备上验证触摸响应和动画流畅度，不能只靠编译通过。
2. **资产外置 + mmap**（app 5.57→约 2.1 MiB）。需要重新生成资产分区镜像，
   上板前应先在 `configs/sdkconfig.pet-static` 上做一次可回退的对照。
3. **分区表加 otadata/ota_0/ota_1**。改分区表会改变现有烧写偏移，
   必须与设备读回的分区一起核对后再执行。
4. **CI**：把 `idf.py build` + `tools/test_pet_p0.py` + 镜像大小预算做成一条命令。

### 8.4 实机试烧与后续优先级

- 试烧前读回原应用，备份位于 `docs/development/pet-v11-1/integrated-20261105/device-backup/arch-round1-previous-app.bin`。两次写入均限于 `0x10000` 应用镜像。首次写入前后读回的分区表和 NVS 逐字节一致。
- 首次试烧日志停在 BSP 的背光开启提示，未到 BLE 初始化。该命令与已启动的 LVGL 刷屏共用 SPI 面板 IO。修订版删除重复的启动亮度写入，保留 UI 初始化时持锁恢复亮度。BLE 亮度请求由 UI 持同一显示锁应用，释放锁后写 NVS。待保存标记原子交换，写入失败后限频重试。
- 修订版为 4,793,536 B，应用分区剩余约 43%。构建和 `tools/test_pet_p0.py` 通过，烧录写入哈希校验通过，启动日志中的 ELF SHA256 前缀为 `026ca85bd`。上板日志见 `docs/development/pet-v11-1/integrated-20261105/arch-round1-serial.log`。
- 下一轮优先把 JPEG 解码及读盘移出 LVGL 线程，比较同一动画的显示锁超时、帧刷新和触摸响应，继续沿用四行内部 DMA 缓冲。资产外置和 OTA 分区调整各自独立验证，资产迁移时保留可回退的应用和素材组合。

50 秒串口采样确认 CodeIsland 自动重连、时钟同步、IDLE 切入 THINKING 并播放 238 帧动画。THINKING 持续约 43 秒，期间未见 SPI/DMA 提交错误、JPEG 解码错误或重启。仍有 4 次 LVGL 锁超时，动画继续刷新。与上一版工作动画采样相比，内部堆低水位由 43,251 B 升至 90,559 B。热读盘平均约 4.6 ms、JPEG 平均约 19.6 ms，两项耗时基本持平。以上为串口结果，屏幕撕裂需目视确认。
