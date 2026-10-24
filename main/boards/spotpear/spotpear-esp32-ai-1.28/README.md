# SpotPear ESP32-AI-1.28 v1.0

Board definition for the old, non-legged **SpotPear ESP32-AI-1.28 v1.0**
with a **MAX98357A** speaker amplifier and **NP0557** I2S microphone.
The chip identities were reported from the physical board; the pin mapping
was cross-checked against the manufacturer's old-board schematic.

This is a separate hardware revision from `sp-esp32-s3-1.28-box`, which uses
an ES8311 codec, CST816D touch controller and different display pins.
Selecting that Box firmware for this board causes codec initialization to fail.

## Hardware

| Function | Signal | GPIO |
| --- | --- | --- |
| Microphone | WS / SCK / DATA | 4 / 5 / 6 |
| Speaker | BCLK / LRCLK / DIN | 15 / 16 / 7 |
| GC9A01 LCD | DC / CS / SCLK / MOSI / RESET | 10 / 13 / 14 / 17 / 18 |
| LCD backlight | Active high | 3 |
| BOOT button | Active low | 0 |

- ESP32-S3, 16 MB flash, 8 MB Octal PSRAM (80 MHz).
- GC9A01, 240 x 240, BGR, inverted colors, mirrored X, SPI mode 0 at 40 MHz.
- Reuses `NoAudioCodecSimplex`: speaker TX on I2S0 at 24 kHz and microphone RX
  on I2S1 at 16 kHz, with independent BCLK/WS lines. Both use standard I2S,
  32-bit slots and the left channel. The schematic grounds microphone L/R.
- No audio-control I2C bus or MCLK is required. The amplifier enable is not
  connected to a GPIO in the reference schematic.
- Reuses the existing GC9A01 driver, `SpiLcdDisplay`, `PwmBacklight`, and
  the button pattern from `bread-compact-wifi-lcd`.
- BOOT click toggles chat; during startup it enters Wi-Fi configuration.
  Application work is scheduled on the main task.
- Touch, camera, battery sensing, RGB LED and board-specific power control
  are not enabled. No electrical AEC reference input is provided.

The board inherits the ESP32-S3 defaults for flash/PSRAM and
`partitions/v2/16m.csv`; `config.json` does not duplicate those defaults.
Its reported type and firmware name are both `spotpear-esp32-ai-1.28`.
This identity does not imply that an official OTA feed is available for it.
Do not install firmware intended for another SpotPear revision.

## Prebuilt firmware

[Download the v2.5.3 English build](../../../../prebuilt/spotpear-esp32-ai-1.28/v2.5.3/)
with a merged image, separate images, checksums and build settings.
This newer build has passed compilation checks but is not yet hardware-tested.

## Build and flash

Activate ESP-IDF 6.0.1 or newer (the project recommends `release/v6.1`), then
run from the repository root:

```sh
python scripts/build.py spotpear/spotpear-esp32-ai-1.28 --name spotpear-esp32-ai-1.28
```

For Japanese firmware, append `--language ja-JP`. The default language follows
the project settings. For manual configuration select
`Xiaozhi Assistant -> Board Type -> SpotPear ESP32-AI-1.28 v1.0`.

After checking the serial port and backing up any existing firmware/settings:

```sh
idf.py -p PORT flash monitor
```

Expected startup messages include `SpotPear ESP32-AI-1.28 v1.0` and
`NoAudioCodec: Simplex channels created`. Check flash/PSRAM detection, LCD,
BOOT, speaker playback and microphone capture on the actual device.

## Validation

The same board source was built with Xiaozhi v2.5.0 and ESP-IDF v6.1,
flashed to an ESP32-S3 with 16 MB flash / 8 MB PSRAM, and booted into
Wi-Fi configuration without the ES8311 initialization failure. The user
confirmed working speaker and microphone on 2026-10-15.
This hardware result applies to that v2.5.0 firmware. A newer application
build needs its own hardware smoke test; it is not implied by compilation.

The port based on fork commit `39d23aee5b8682f77bbdf9cdc66799456b5ad408`
(Xiaozhi v2.5.3) was compiled and linked with ESP-IDF v6.1 on 2026-10-24.
The default English / `wn9_hiesp` build generated an app image of `0x26a5b0`
bytes, leaving 39% of the application partition free. Host tests completed
successfully (142 tests, 3 skipped); board source formatting and diff checks
also passed. This v2.5.3 firmware has not yet been flashed or hardware-tested.

## References

- [Manufacturer old-board guide](https://spotpear.com/wiki/ESP32-S3-N16R8-AI-DeepSeek-XiaoZhi-XiaGe-Qwen-DouBao-1.28-inch-LCD.html)
- [Old-board schematic, 2025-03-21](https://cdn.static.spotpear.com/uploads/picture/learn/ESP32/ESP32-S3-1.28inch-AI/ESP32S3-1.28inch-AI.pdf)
- [Original board support, firmware and hardware notes](https://github.com/tomorrow56/SpotPear-ESP32-AI-1.28)
