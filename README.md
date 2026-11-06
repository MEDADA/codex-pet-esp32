# Codex Pet ESP32 firmware

Firmware for the Waveshare ESP32-S3-Touch-AMOLED-1.75C (466 × 466). The watch shows IDLE and WORKING animations and mirrors CodeIsland status over BLE.

This ESP-IDF project lives on `firmware/esp32`. The art team works on `main`. See [repository layout](docs/repository-layout.md) for file ownership and the asset handoff workflow.

Clone the firmware branch and load an ESP-IDF environment. The initial upload was verified with ESP-IDF commit `0d9287800812c95662921c2c5e812023939e3d58`.

```sh
git clone --branch firmware/esp32 https://github.com/MEDADA/codex-pet-esp32.git
cd codex-pet-esp32
idf.py build
python3 tools/test_pet_p0.py
```

Host tests require a C/C++ compiler and Python with Pillow. Build output goes under `build/`. Review the board's existing partition layout and back up its application before writing an image. The ESP-IDF build prints the addresses for a full installation.

To pair, open CodeIsland **Settings → Buddy**, select the watch, and accept the pairing prompt on the watch with **BOOT**.
