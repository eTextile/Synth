# eTextile-Synthetizer / Software / Demo
## Exploring music through textile
### eTextile-Synthetizer -> Ableton Live 
#### MIDI InstantMappings

With the file [UserConfiguration.txt](./UserConfiguration.txt) provided in this directory, you can create your own Instant Mappings.

Note: Since this mechanism relies on the scripts added for Live 6, you can solely map all the values provided by the eTextile-Synthetizer Surface. You can neither add functions nor can you change the banks of device parameters.

1. Create a directory in 'User Remote Scripts'. The name of the directory will show up in the list of Control Surfaces in Live. Naming: The name of the directory should not start with '_' or '.'.

2. Copy the the file [UserConfiguration.txt](./UserConfiguration.txt) into your directory. The name of the file needs to remain unchanged.

3. Edit the copy to adapt it to your needs. The comments in the file will tell you what to do.

# TODO
- **SYNTH_MIDI.als**
- **SYNTH_MPE.als** (MIDI Polyphonic Expression).
This way of using MIDI allows MPE-capable devices to control multiple parameters of every note in real time for more expressive instrumental performances.

