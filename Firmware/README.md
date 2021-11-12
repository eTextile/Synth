# eTextile-Synthesizer
## Exploring music through textile

### Requirements
- **eTextile-Synthesizer PCB** & **Teensy 4.0**
- **Arduino IDE**: Arduino 1.8.16 [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Visual Studio Code**: [DOWNLOAD](https://visualstudio.microsoft.com/fr/)
- **platform.io extension**: [DOWNLOAD](https://platform.io/)
- **platform.io additional board**: Teensy 4.0

### External Libraries
All Library dependencies are automatically installed using platformio.ini (lib_deps)
Path of the lib_deps installation :
    /Synth/Firmware/.pio/libdeps/teensy40/...

### Firmware main file
    /Synth/Firmware/src/main.cpp
 
### MTP - prerequisite
  https://github.com/KurtE/MTP_Teensy.git (testing)

### Powering the eTextile-Synthesizer
- The Teensy Micro USB Type B **will not power** the eTextile-Synthesizer
- You must use the included power cable with an **5Volts AC/DC power Plug** or your laptop USB plug

### Program Synopsis
The embedded Software is implementing **image analysis algorithms** on the E256 eTextile-matrix-sensor to design a new tactile interaction vocabulary.
- **Bilinear interpolation** The 16x16 Analog pressure sensor values are interpolated with a bilinear algorithm
- **Blob tracking** The interpolated pressure matrix sensor values are analyzed with a Connected Component Labelling algorithm
- **Blob ID management** each blob is tracked in space and time using single chained linked list
- **Blob shape and movement characterisation**: blobs coordinates, size and pressure

### Benchmark
  - ADC_INPUT : 2500 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ...
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 550 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Configuring the system
The firmware is using a **platformio.ini** to select the needed fonctionnalites. Uncomment the lines that you need.

### Use the eTextile-Synthesizer as COMPUTER_INTERFACE
- [USB_MIDI]: Transmit touch coordinates via USB_MIDI
- [HARDWARE_MIDI]: Transmit touch coordinates via HARDWARE_MIDI
- [USB_OSC]: Transmit touch coordinates via USB_SLIP_OSC

### Use the eTextile-Synthesizer as STANDALONE divice
- [SYNTH_PLAYER] : Set the eTextile-Synthesizer as polyphonic synthesizer
- [GRANULAR_PLAYER] : Set the eTextile-Synthesizer as 
- [FLASH_PLAYER] : Set the eTextile-Synthesizer as 

### Mapping functionality (IN_PROGRESS)
- [MAPPING_LAYOUT]
  - [GRID_LAYOUT_MIDI_IN]
  - [GRID_LAYOUT_DEFAULT]
  - [GRID_LAYOUT_HARMONIC]
  - [GRID_LAYOUT_NOTES]
  - [GRID_LAYOUT_FREQ]

## Copyright
Except as otherwise noted, all files in the eTextile-Synthesizer project folder

    Copyright (c) 20014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthesizer project folder.

## TODO
- **Gesture Recognizer** could be a nice feature that fit the eTextile-Synthesizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)