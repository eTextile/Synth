# eTextile-Synthetizer / Software / Demo
### Exploring music through textile

The eTextile-Synthetizer is designed to be used as **Standalone Audio Synthetizer** 
and also as an MIDI controleur for Audio applications sucha as **Ableton Live**, **MaxMsp**, **Puredata**, etc.
This folder is containing some demons that can be used as starting point for your own creations.

- [Ableton Live](https://www.ableton.com/en/live/ "Ableton-Live")
  - **SYNTH_MIDI.als** support
  - **SYNTH_MPE.als** support (MIDI Polyphonic Expression). This way of using MIDI allows MPE-capable devices to control multiple parameters of every note in real time for more expressive instrumental performances.

- [MaxMsp](http://msp.ucsd.edu/ "Miller Puckette")
  - **SYNTH_SLIP-OSC.maxpat**
  - **SYNTH_MIDI.maxpat**
  - **SYNTH_MPE.maxpat**

- [Pure Data](http://msp.ucsd.edu/ "Miller Puckette")
  - **SYNTH_SLIP-OSC.pd**
    - This patch depends on several third party externals available through **deken plugin** (Help -> Find externals):
	- **comport** : 
	- **mrpeach** : OSC slipenc and slipdec
	- **cyclone** : 
  - **SYNTH_MIDI.pd**
  - **SYNTH_MPE.pd**
