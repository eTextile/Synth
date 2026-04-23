# eTextile-Synthesizer Firmware
## Build your own eTextile tactile interface to play music through textile

**Version:** 1.0.25 — **Hardware:** e256 PCB + Teensy 4.0

---

## Step-by-step User Guide

1. **Powering the eTextile-Synthesizer:**
    - Use the included power cable plugged to your laptop USB port or a **5V AC/DC power adapter**.
    - For use with the web app or a music software, also connect a **Micro USB (Type B)** cable.

2. **Load the web application** (Chrome recommended): https://mapping.etextile.org/
    - Build your custom Graphic-User-Interface using the web app features.
    - The app provides GUI modules: toggles, sliders, knobs, etc.
    - Each GUI element can be placed, resized and configured to generate **MIDI messages**.
    - Once your interface is ready, the web app generates and uploads a **config file** to the device.
    - A **long press on the LEFT_BUTTON** saves the config file to flash memory.
    - This config file is **loaded on each startup**.
    - You can also save it to your computer as an archive.

3. Use a **mini-jack TRS-A** MIDI cable to connect the device to external MIDI synthesizers.

---

## Boot Sequence

### With USB cable + power plug

1. The device is detected by the web app, which sends `[SYNC_MODE_REQUEST]`.
2. The device replies `[SYNC_MODE_DONE]`.
3. The app sends `[CONFIG_FILE_REQUEST]`.
4. The device responds with either `[FLASH_CONFIG_LOAD_DONE]` or `[FLASH_CONFIG_LOAD_FAILED]`.
    - **`FLASH_CONFIG_LOAD_DONE`**: the device sends the current config file via MIDI SysEx; the web app loads it on screen. The config can be modified and re-uploaded, overwriting the previous one.
    - **`FLASH_CONFIG_LOAD_FAILED`**: the web app prompts **NO CONFIG FILE IN THE DEVICE** — you start from scratch.

### With power plug only (no USB cable)

1. The firmware boots in `PENDING_MODE`.
2. After `PENDING_MODE_TIMEOUT` (4 s), it switches to `STANDALONE_MODE`.
3. The device loads `flash_config()`:
    - **`FLASH_CONFIG_LOAD_DONE`**: the **BUILTIN_LEDs** blink `STANDALONE_MODE`.
    - **`FLASH_CONFIG_LOAD_FAILED`**: the **BUILTIN_LEDs** blink an **ERROR_CODE**.
4. The eTextile-Synthesizer is now running with MIDI output on the mini-jack TRS-A socket.

---

## Firmware Operating Modes

| Mode | Description |
|------|-------------|
| `PENDING_MODE` | Waiting for USB connection (4 s timeout before `STANDALONE_MODE`) |
| `SYNC_MODE` | Handshake with the web app |
| `CALIBRATE_MODE` | Full matrix calibration |
| `MATRIX_RAW_MODE` | Stream raw 16×16 sensor values over USB MIDI |
| `EDIT_MODE` | Stream all blob values over USB MIDI |
| `PLAY_MODE` | Send mapping values over USB MIDI |
| `THROUGH_MODE` | Forward mapping values to hardware MIDI output |
| `STANDALONE_MODE` | Run mappings using hardware MIDI output only |
| `UPLOAD_MODE` | Receive config file from web app |
| `APPLY_MODE` | Apply received config to mappings |
| `WRITE_MODE` | Write config to flash |
| `LOAD_MODE` | Load config from flash |
| `FETCH_MODE` | Send current config file to the web app |
| `ERROR_MODE` | Unexpected behaviour — LEDs blink error code |

---

## Mapping Library (TUIs)
### Available Tactile-User-Interfaces

All TUIs are fully parametric and configured via the web app config file.

| TUI | Max instances | Max simultaneous touches |
|-----|:---:|:---:|
| `switch()` | 16 | 2 |
| `slider()` | 6 | 2 |
| `knob()` | 4 | 4 |
| `touchpad()` | 2 | 5 |
| `polygon()` | 8 | 3 |
| `grid()` | 1 | 3 |

> **grid()** is inspired by the [Omnichord](https://en.wikipedia.org/wiki/Omnichord). It maps a rectangular zone of the textile surface to a grid of MIDI notes. The layout can be updated live from a MIDI keyboard connected to the hardware MIDI input.

---

## Development Setup

### Frameworks & Tools

- **Hardware:** eTextile-Synthesizer PCB (e256) + Teensy 4.0 @ 600 MHz
- **Build system:** [PlatformIO](https://platformio.org/) (VS Code extension recommended)
- **Teensy platform:** install via PlatformIO Advanced Installation:
  ```
  https://github.com/nicolas-thill/platform-teensy-e256.git
  ```

### Build Environments

| Environment | USB mode | Description |
|-------------|----------|-------------|
| `USB_MIDI` (default) | MIDI only | Production build |
| `USB_MIDI_SERIAL` | MIDI + Serial | Debug build |

### Library Dependencies

All dependencies are automatically installed by PlatformIO via `lib_deps` in [platformio.ini](platformio.ini):

- [ArduinoJson](https://arduinojson.org/) >= 7.0.4
- [Encoder](https://github.com/PaulStoffregen/Encoder)

Installation path: `.pio/libdeps/teensy40/`

### Debug Flags (`USB_MIDI_SERIAL` environment only)

Uncomment the desired flags in [platformio.ini](platformio.ini):

| Flag | Description |
|------|-------------|
| `DEBUG_FPS` | Print frames per second |
| `DEBUG_ENCODER` | Print encoder value |
| `DEBUG_BUTTONS` | Print button states |
| `DEBUG_MODES` | Print mode changes |
| `DEBUG_LEVELS` | Print level changes |
| `DEBUG_ADC` | Print 16×16 raw analog values |
| `DEBUG_INTERP` | Print 64×64 interpolated values |
| `DEBUG_BITMAP` | Print 64×64 binary image (threshold applied) |
| `DEBUG_FIND_BLOBS` | Print low-level blob detection values |
| `DEBUG_BLOBS` | Print blob tracking values |
| `DEBUG_MIDI_IO` | Print MIDI I/O traffic |
| `DEBUG_CONFIG` | Print config parsing |
| `DEBUG_MAPPINGS_SWITCHS` | Print switch GUI values |
| `DEBUG_MAPPINGS_SLIDERS` | Print slider GUI values |
| `DEBUG_MAPPINGS_KNOBS` | Print knob GUI values |
| `DEBUG_MAPPINGS_TOUCHPADS` | Print touchpad GUI values |
| `DEBUG_MAPPINGS_POLYGONS` | Print polygon GUI values |

---

## Firmware Architecture

### Signal Processing Pipeline

```
ADC scan (16×16) → Bilinear interpolation (64×64) → Blob detection → Mapping → MIDI out
```

1. **Force image acquisition** — 16×16 matrix scanned via synchronous dual analog reads.
2. **Bilinear interpolation** — sensor image upscaled to 64×64.
3. **Touch thresholding** — adjustable threshold via rotary encoder.
4. **Blob segmentation** — scanline flood fill (SFF) + connected-component labeling (CCL).
5. **Blob tracking** — proximity-based ID persistence across frames (up to 16 simultaneous blobs).
6. **Blob characterisation** — centroid (X, Y, Z), bounding box (W, H).
7. **Mapping** — TUI dispatch to USB MIDI or hardware MIDI output.

### Blob Lifecycle

```
FREE → NEW → PRESENT → MISSING → RELEASED → FREE
```

| State | Description |
|-------|-------------|
| `FREE` | Available in the pool, no active mapping |
| `NEW` | First frame the blob is detected |
| `PRESENT` | Blob actively tracked |
| `MISSING` | Not detected for < 50 ms (debounce) |
| `RELEASED` | Not detected for 50–500 ms (triggers note-off) |

### MIDI Channels

| Channel | Role |
|---------|------|
| 1 | MIDI input |
| 3 | Threshold / signal levels |
| 4 | Operating modes |
| 5 | Verbosity / acknowledgments |
| 6 | Error codes |

### Key Source Files

| File | Role |
|------|------|
| [src/main.cpp](src/main.cpp) | Setup and main loop |
| [src/scan.cpp](src/scan.cpp) | ADC matrix acquisition |
| [src/interp.cpp](src/interp.cpp) | Bilinear interpolation |
| [src/blob.cpp](src/blob.cpp) | Blob detection and tracking |
| [src/mapping.cpp](src/mapping.cpp) | TUI dispatch engine |
| [src/usb_midi_io.cpp](src/usb_midi_io.cpp) | USB MIDI I/O |
| [src/hardware_midi_io.cpp](src/hardware_midi_io.cpp) | Hardware MIDI I/O |
| [src/config.cpp](src/config.cpp) | Hardware init, flash config |
| [include/config.h](include/config.h) | All project constants |

### Benchmark

| Pipeline stage | FPS |
|----------------|-----|
| ADC scan only | ~2500 |
| + Bilinear interpolation | — |
| + Blob tracking | — |
| + Mapping lib | ~900 |

---

## Copyright

Except as otherwise noted, all files in the eTextile-Synthesizer project folder:

    Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>

This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International.
For information on usage, redistribution, and a DISCLAIMER OF ALL WARRANTIES, see [LICENSE](../LICENSE).

---

## Roadmap

- **Gesture Recognizer** — pattern recognition on the tactile surface.
- **MIDI MPE** — continuous per-note pitch and pressure control after note-on ([MPE spec](https://www.midi.org/midi-articles/midi-polyphonic-expression-mpe)).
- **Embedded synthesizer** integration:
    - [MicroDexed](https://www.parasitstudio.de/) — open source DX7-style FM synth
    - [Dexed](https://github.com/asb2m10/dexed) — FM plugin synth
