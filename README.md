# eTextile-Synthesizer 
### Exploring music through textile

![eTextile Synthesizer](https://live.staticflickr.com/65535/48916850368_dd34d00418_c_d.jpg)

## About The project
After setting up the **E256 eTextile matrix sensor** it is now time to use it for live electronic music performances. **eTextile-Synthesizers** are handcrafted multi-touch digital interfaces based on smart textile materials and custom electronic solutions. These eTextile-synthesizers open up new possibilities for electronic music playing. The project features a fully embedded **multitouch eTextile-Synthesizers** that combines the powerful **Teensy 4.0** MCU and its audio library with the E256 eTextile matrix sensor technology. One of the main purpose of this project is to promote an intuitive music playing as well as taking benefit of textile textures diversity to form tactile cues that guide the touch. Like the popular **Eurorack modules** all eTextile-Synthesizers can connect each other via mini-jack and share them tempo as well as notes and audio streams. Thus each eTextile-Synthesizer devices have dedicated audio functionalities to generate or filter audio signals.

### Development tools & DOCs
* **Web page:** [https://synth.eTextile.org/](https://synth.eTextile.org "Project web page")
* **Repository:** [https://github.com/eTextile/synth/tree/master/](https://github.com/eTextile/synth/tree/master/ "Project repository")
* **License:** [CC-BY-SA](https://github.com/eTextile/synth/tree/master/LICENSE.txt "Project License")
* **Mail:** [contact@etextile.org](mailto:contact@etextile.org "eMail us")
* **Pictures:** [flickr.com](https://www.flickr.com/photos/maurin/albums/72157673740361510/ "Share your pictures with us")

## Project content
* **Hardware_textile:** eTextile-Synthesizer tutorial
* **Hardware_electronic:** Open Hardware electronic parts (Made with KiCad) 
* **Firmware:** eTextile-Synthesizer firmware (Compile with Arduino)
* **Softwear:** demos applications (PureData, MaxMSP, Max4Live, etc.)

## Specifications
| **Feature**             | **Description**                           |
| :---------------------- | :---------------------------------------- |
| Power-IN                | DC-5V                                     |
| Hardware resolution     | 16x16 FSRs with hardware interpolation    |
| Multi-Touch (Max points)| User defined                              |
| Points characteristics  | [ID, STATE, CX, CY, WIDTH, HEIGHT, DEPTH] |
| XY resolution           | [0:127] integer                           |
| Z resolution            | [0:127] integer                           |
| FPS                     | 420 - 500                                 |
| Audio IN                | ADC 16bits (mini-jack)                    |
| Audio OUT_1             | DAC 16bits (mini-jack)                    |
| Audio OUT_2             | Headphone (mini-jack)                     |
| Connectivity            | USB_MIDI                                  |
| Connectivity            | USB_SLIP-OSC                              |
| Connectivity            | USB_MPE (TODO)                            |
| Connectivity            | MIDI I/O (mini-jack)                      |
## Publications
- [NIME17](http://www.nime2017.org/) - Designing a Multi-Touch eTextile for Music Performances / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/NIME17-eTextile.pdf) - [Video](https://vimeo.com/217690743)
- [MOCO17](http://moco17.movementcomputing.org/) - Music Skin: Fabric Interface for Expressive Music Control / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/MOCO17-MusicSkin.pdf)

## Copyright
Except as otherwise noted, all files in the **eTextile-Synthesizer** project folder

    Copyright (c) 2014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL WARRANTIES, see LICENSE.txt included in this folder.

## Docs
- [Video made at Schmiede 2014](http://www.kobakant.at/DIY/?p=4305/)
