# Prebuilt firmware: SpotPear ESP32-AI-1.28 v1.0

Xiaozhi **v2.5.3** for the old, non-legged SpotPear board with **MAX98357A + NP0557**.
Built with ESP-IDF v6.1 / Xtensa GCC 15.2.0. Language: **English**;
wake word model: `wn9_hiesp` (Hi ESP).

## Download and flash

- [merged-binary.bin](merged-binary.bin): complete initial-flash image, write at **0x0**.
- [xiaozhi.bin](xiaozhi.bin): application only, mapped to **0x20000** by this partition table.
- [SHA256SUMS](SHA256SUMS): checksums for the supplied files.
- [build-info.json](build-info.json): source commit and build settings.
- `sdkconfig` and `dependencies.lock`: the configuration and resolved dependencies used for this build.

Target: **ESP32-S3 / 16 MB flash / 8 MB Octal PSRAM at 80 MHz**.
This firmware is not for the ES8311-based `sp-esp32-s3-1.28-box` revision.

With Python and esptool installed, replace `PORT` with the checked serial port.
Run from this directory:

```sh
python -m esptool --chip esp32s3 -p PORT write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 merged-binary.bin
```

Alternatively, flash the separate images with their generated offsets:

```sh
python -m esptool --chip esp32s3 -p PORT write-flash @flash_args
```

The merged image contains the bootloader, partition table, initial OTA metadata,
application and assets. Back up existing flash/settings before replacing firmware
if they need to be retained. No NVS dump or device-specific data is included.
After boot, connect to the displayed `Xiaozhi-xxxx` Wi-Fi network and open
`http://192.168.4.1` to configure Wi-Fi.

## Validation

Compilation, linking, application partition size checks, merged image generation,
and esptool image checksum validation passed. The app occupies `0x26a5b0` bytes;
39% of the app partition remains free. Host tests passed (142 tests, 3 skipped).

**Hardware-tested on 2026-10-25.** All written images passed flash verification.
Boot logs confirmed 8 MB PSRAM, simplex I2S initialization, Wi-Fi reconnection,
MQTT connection, and listening/speaking state transitions without an observed
abort or panic. The user confirmed normal operation, speaker playback and
microphone response on the physical SpotPear ESP32-AI-1.28 v1.0.

An initial no-sound report was traced by the user to a broken wire. Correcting
the wiring restored normal operation; no firmware changes were needed.
The earlier v2.5.0 build was also hardware-tested on 2026-10-15.

## Source and licensing

The firmware source is in this repository at the commit recorded in `build-info.json`.
[Board documentation](../../../main/boards/spotpear/spotpear-esp32-ai-1.28/README.md)
includes the pins, references and build command.
Xiaozhi's MIT license and the licenses of ESP-IDF and bundled dependencies apply;
see the repository LICENSE and `dependencies.lock` for the resolved components.
