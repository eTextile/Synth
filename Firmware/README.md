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

1. The device is detected by the web app, which sends a `SYNC` SysEx command.
2. The device replies `SYNC_MODE_DONE` (SysEx ACK).
3. The app switches to `MATRIX_RAW_MODE` to verify the sensor is live.
4. To load a previously saved configuration, click **FETCH CONFIG** in the web app:
    - **Success** (`LOAD_MODE_DONE`): the device sends the config JSON via SysEx; the web app loads it on screen. The config can be modified and re-uploaded, overwriting the previous one.
    - **No config found**: the web app shows **NO CONFIG FILE IN THE DEVICE** — you start from scratch.

### With power plug only (no USB cable)

1. The firmware boots in `PENDING_MODE`.
2. After `PENDING_MODE_TIMEOUT` (4 s), it switches to `PLAY_MODE`.
3. The device loads `flash_config()`:
    - **`FLASH_CONFIG_LOAD_DONE`**: the **BUILTIN_LEDs** blink `STANDALONE_MODE`.
    - **`FLASH_CONFIG_LOAD_FAILED`**: the **BUILTIN_LEDs** blink an **ERROR_CODE**.
4. The eTextile-Synthesizer is now running with MIDI output on the mini-jack TRS-A socket.

---

## Config Upload Protocol

When the web app sends a new mapping configuration, the exchange follows a strict handshake over USB MIDI SysEx:

```
CMD  format: [F0, 0x7D, 0x01, mode_value, F7]
ACK  format: [F0, 0x7D, 0x02, ack_value,  F7]
ERR  format: [F0, 0x7D, 0x03, err_code,   F7]

Web app → ALLOCATE_CONFIG (SysEx CMD)
              ← firmware sets ALLOCATE_MODE → ALLOCATE_MODE_DONE (SysEx ACK)
Web app → [F0, 0x7D, SIZE_MSB, SIZE_LSB, F7]   ← allocation size packet
              ← firmware allocates buffer → ALLOCATE_DONE (SysEx ACK)
Web app → UPLOAD_CONFIG (SysEx CMD)
              ← firmware sets UPLOAD_MODE → UPLOAD_MODE_DONE (SysEx ACK)
Web app → [F0, 0x7D, ...JSON bytes..., F7]      ← JSON config data
              ← firmware stores data in RAM → UPLOAD_DONE (SysEx ACK)
Web app → APPLY_CONFIG (SysEx CMD)
              ← firmware calls mappings_apply_config() → CONFIG_APPLY_DONE (SysEx ACK)
              ← web app returns to EDIT_MODE
              ← alert: "press LEFT BUTTON to save to flash"
User long-presses LEFT BUTTON
              ← firmware writes config to flash → WRITE_MODE_DONE (SysEx ACK)
              ← alert: "STANDALONE MODE available"
```

- **RAM apply** — automatic at every upload (APPLY_CONFIG step). The device immediately uses the new mapping.
- **Flash write** — manual, requires a physical long-press on the LEFT BUTTON. Required for the config to survive a power cycle or standalone use.

---

## Firmware Operating Modes

| Mode | Description |
|------|-------------|
| `PENDING_MODE` | Waiting for USB connection (4 s timeout before `STANDALONE_MODE`) |
| `SYNC_MODE` | Handshake with the web app |
| `CALIBRATE_MODE` | Full matrix calibration |
| `MATRIX_RAW_MODE` | Stream raw 16×16 sensor values over USB MIDI |
| `MATRIX_INTERP_MODE` | Stream interpolated 64×64 sensor values over USB MIDI |
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

| TUI | Max instances | Max simultaneous touches | Notes |
|-----|:---:|:---:|---|
| `switch()` | 16 | 2 | `tap_tempo: true` turns any switch into a MIDI Clock tap-tempo source |
| `slider()` | 6 | 2 | `steps > 0` divides the axis into note zones (ROL mode) |
| `knob()` | 4 | 4 | |
| `touchpad()` | 2 | 5 | |
| `polygon()` | 8 | 3 | |
| `grid()` | 1 | 3 | Note layout reprogrammable live via hardware MIDI input |

> **grid()** is inspired by the [Omnichord](https://en.wikipedia.org/wiki/Omnichord). It maps a rectangular zone of the textile surface to a grid of MIDI notes. The layout can be updated live from a MIDI keyboard connected to the hardware MIDI input.

> **slider()** — when `steps > 0`, the slider is divided into `steps` equal zones along its axis. Entering a zone sends a NoteOn (this mode requires `press = NoteOn`). The note for each zone is set at configuration time or updated live from a MIDI keyboard via `populate` (see below).

### Populate modes (`slider`, `grid`, `knob`, `polygon`, `switch`)

TUIs that support `steps` can have their note layout reprogrammed live by a MIDI keyboard connected to the **hardware MIDI input** (TRS-A). The `populate` parameter controls how incoming notes are distributed across the step zones:

| Mode | Behaviour |
|------|-----------|
| `OFF` | Keyboard input ignored (default) |
| `AS_PLAYED` | Notes fill zones in order of arrival, wrapping around |
| `UP` | Held notes sorted lowest→highest and assigned to zones 0..N |
| `DOWN` | Held notes sorted highest→lowest and assigned to zones 0..N |
| `OCTAVE` | First note sets zone 0; each subsequent zone adds one semitone (`base + i`) |
| `PING_PONG` | Each new note is assigned to the next zone in a back-and-forth sequence |

The `receive_chan` parameter selects which MIDI channel on the hardware input is used for populate. Up to `MAX_SLIDER_STEPS` (24) zones are supported per slider.

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
ADC scan (16×16) → Bilinear interpolation (64×64) → Blob detection → Velocity → Mapping → MIDI out
```

1. **Force image acquisition** — 16×16 matrix scanned via synchronous dual analog reads.
2. **Bilinear interpolation** — sensor image upscaled to 64×64.
3. **Touch thresholding** — adjustable threshold via rotary encoder.
4. **Blob segmentation** — scanline flood fill (SFF) + connected-component labeling (CCL).
5. **Blob tracking** — proximity-based ID persistence across frames (up to 16 simultaneous blobs).
6. **Blob characterisation** — centroid (X, Y, Z), bounding box (W, H).
7. **Velocity & attack detection** — per-blob velocity computed every frame; attack peak captured to derive NoteOn velocity (see below).
8. **Mapping** — TUI dispatch to USB MIDI or hardware MIDI output.

### Velocity & Attack Detection

Compiled in when `BLOB_VELOCITY` is defined. Runs after blob tracking, once per frame.

#### Continuous velocity (XY + Z)

Two quantities are updated for every `PRESENT` blob at most every `VELOCITY_MIN_INTERVAL_MS`:

| Field | Description |
|-------|-------------|
| `velocity.xy` | Smoothed lateral speed — Euclidean distance of centroid displacement / Δt (units/s) |
| `velocity.z` | Smoothed vertical speed — signed Δz / Δt (>0 pressing, <0 releasing, units/s) |

Both are filtered with an **exponential moving average** (EMA, α = `VELOCITY_EMA_ALPHA`) to suppress frame-to-frame sensor noise without a ring buffer.

#### Attack detection (NoteOn velocity)

A fixed-delay approach would add a constant latency on every note — perceptible to musicians. Instead, the firmware uses **peak-drop detection**:

```
NEW → arm window (attack_z = 0, note_on_z_pending = true)
  ↓  each PRESENT frame:
  ├─ |velocity.z| rising  → update attack_z (running peak)
  ├─ |velocity.z| < peak × VELOCITY_ATTACK_DROP
  │   AND age ≥ VELOCITY_ATTACK_MIN_MS → attack_done = true  ← impact over
  └─ age ≥ VELOCITY_ATTACK_MAX_MS      → attack_done = true  ← hard deadline
```

When `attack_done` is set, the MIDI layer reads `attack_z` to send the deferred NoteOn with a velocity proportional to the impact peak. `attack_z` is preserved through `RELEASED` so the MIDI layer can still use it for NoteOff.

#### Constants (`include/config.h`)

| Constant | Default | Description |
|----------|---------|-------------|
| `VELOCITY_MIN_INTERVAL_MS` | 10 ms | Minimum time between velocity updates |
| `VELOCITY_EMA_ALPHA` | 0.4 | EMA factor: 0 = frozen, 1 = raw (no smoothing) |
| `VELOCITY_ATTACK_MIN_MS` | 5 ms | Guard time before peak-drop detection activates |
| `VELOCITY_ATTACK_MAX_MS` | 80 ms | Hard deadline — forces `attack_done` if peak never drops |
| `VELOCITY_ATTACK_DROP` | 0.5 | Ratio: `attack_done` when `\|vz\| < peak × ratio` |
| `VELOCITY_ATTACK_Z_MAX` | 1500 | Max expected `\|velocity.z\|` in units/s — tune to calibrate MIDI range |
| `VELOCITY_XY_MAX` | 200 | Max expected lateral velocity in units/s — used for ROL slider NoteOn velocity |

---

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

### SysEx Packet Types

All control traffic between the web app and the firmware uses SysEx with device ID `0x7D`:

| Byte 2 (`pkt_type`) | Direction | Purpose |
|---------------------|-----------|---------|
| `0x01` (`PKT_CMD`) | web → firmware | Mode switch command |
| `0x02` (`PKT_ACK`) | firmware → web | Acknowledgment |
| `0x03` (`PKT_ERR`) | firmware → web | Error notification |
| `0x04` (`PKT_PARAM`) | firmware → web | Parameter value (threshold, levels) |

### MIDI Channels (user mappings)

Standard MIDI channels are used for user-defined TUI mappings only. All mode control, configuration upload, and device feedback use SysEx (no MIDI channel).

### Key Source Files

| File | Role |
|------|------|
| [src/main.cpp](src/main.cpp) | Setup and main loop — per-mode dispatch table at top of `loop()` |
| [src/scan.cpp](src/scan.cpp) | ADC matrix acquisition |
| [src/interp.cpp](src/interp.cpp) | Bilinear interpolation |
| [src/blob.cpp](src/blob.cpp) | Blob detection and tracking |
| [src/mapping.cpp](src/mapping.cpp) | TUI dispatch engine — blob lifecycle → MIDI out |
| [src/usb_midi_io.cpp](src/usb_midi_io.cpp) | USB MIDI I/O |
| [src/hardware_midi_io.cpp](src/hardware_midi_io.cpp) | Hardware MIDI I/O |
| [src/midi_tap_tempo.cpp](src/midi_tap_tempo.cpp) | Tap-tempo MIDI Clock generator (24 PPQN) |
| [src/config.cpp](src/config.cpp) | Hardware init, flash config |
| [include/config.h](include/config.h) | All project constants |
| [include/mapping.h](include/mapping.h) | Touch types (`touch_press_t`, `touch_linear_t`, `touch_planar_t`) and `common_t` vtable |

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
