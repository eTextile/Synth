# eTextile-Synthesizer
### Exploring music through textile

![eTextile Synthesizer](https://live.staticflickr.com/65535/48916850368_dd34d00418_c_d.jpg)

## About the project
**eTextile-Synthesizers** are handcrafted multi-touch digital interfaces based on smart textile materials and custom electronic solutions. The project features a fully embedded multitouch controller that combines the powerful **Teensy 4.0** MCU with the **E256 eTextile matrix sensor** — a 16×16 FSR array with hardware interpolation up to 64×64 points.

The interface is configured via a **browser-based mapping application** that communicates with the device over USB-MIDI. Mappings (touchpad, slider, switch, knob, grid…) are defined visually, saved as JSON, and uploaded to the device.

### Links
* **Web page:** [synth.eTextile.org](https://synth.eTextile.org "Project web page")
* **Mapping app:** [github.com/eTextile/Mapping](https://github.com/eTextile/Mapping "Browser-based MIDI mapping application")
* **Repository:** [github.com/eTextile/Synth](https://github.com/eTextile/Synth "Project repository")
* **License:** [CC-BY-SA](https://github.com/eTextile/Synth/blob/master/LICENSE.txt "Project License")
* **Mail:** [contact@etextile.org](mailto:contact@etextile.org "eMail us")
* **Pictures:** [flickr.com](https://www.flickr.com/photos/maurin/albums/72157673740361510/ "Share your pictures with us")

## Project content
* **Hardware_textile:** eTextile-Synthesizer construction tutorial
* **Hardware_electronic:** Open Hardware electronics (KiCad)
* **Firmware:** eTextile-Synthesizer firmware (PlatformIO)
* **Software:** Demo applications (PureData, MaxMSP, Max4Live, etc.)

## Specifications
| **Feature**              | **Description**                            |
| :----------------------- | :----------------------------------------- |
| Power                    | DC 5V (USB)                                |
| Sensor resolution        | 16×16 FSRs                                 |
| Interpolated resolution  | 64×64                                      |
| Multi-touch              | User defined                               |
| Blob characteristics     | [ID, STATE, CX, CY, WIDTH, HEIGHT, DEPTH]  |
| XY resolution            | [0:127] integer                            |
| Z resolution             | [0:127] integer                            |
| FPS                      | 500–550                                    |
| Audio IN                 | ADC 16-bit (mini-jack)                     |
| Audio OUT 1              | DAC 16-bit (mini-jack)                     |
| Audio OUT 2              | Headphone (mini-jack)                      |
| Connectivity             | USB-MIDI                                   |
| Connectivity             | MIDI I/O (mini-jack)                       |
| Connectivity             | USB-MPE (TODO)                             |

## Publications
- [NIME17](http://www.nime2017.org/) - Designing a Multi-Touch eTextile for Music Performances / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/NIME17-eTextile.pdf) - [Video](https://vimeo.com/217690743)
- [MOCO17](http://moco17.movementcomputing.org/) - Music Skin: Fabric Interface for Expressive Music Control / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/MOCO17-MusicSkin.pdf)

## Copyright
Except as otherwise noted, all files in the **eTextile-Synthesizer** project folder

    Copyright (c) 2014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see LICENSE.txt included in this folder.

## Docs
- [Video made at Schmiede 2014](http://www.kobakant.at/DIY/?p=4305/)
