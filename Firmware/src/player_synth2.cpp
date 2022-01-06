/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014-2022 Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_synth2.h"

#if defined(PLAYER_SYNTH2)

// GUItool: begin automatically generated code
AudioSynthWaveform       voice_C;        //xy=2918,1860
AudioSynthNoiseWhite     voice_B;        //xy=2920,1981
AudioSynthWaveform       voice_A;        //xy=2922,1918
AudioSynthWaveformDc     voice1env;      //xy=3085,2010
AudioMixer4              voice1mix;      //xy=3098,1933
AudioSynthWaveform       lfo;            //xy=3100,2090.0001220703125
AudioSynthWaveformDc     voice1filterenv; //xy=3103,2143.0001220703125
AudioEffectMultiply      voice1_multiply_A; //xy=3317,1979
AudioMixer4              mixer1;         //xy=3337.0833282470703,2129.0833282470703
AudioFilterStateVariable voice1filter;   //xy=3545,2054
AudioFilterStateVariable delayFilter;    //xy=3617,2335
AudioMixer4              mainOutMixer;   //xy=3857,2302
AudioEffectDelay         delay1;         //xy=3903,2480
AudioOutputI2S           i2s1;           //xy=4179,2341
AudioConnection          patchCord1(voice_C, 0, voice1mix, 0);
AudioConnection          patchCord2(voice_B, 0, voice1mix, 2);
AudioConnection          patchCord3(voice_A, 0, voice1mix, 1);
AudioConnection          patchCord4(voice1env, 0, voice1_multiply_A, 1);
AudioConnection          patchCord5(voice1mix, 0, voice1_multiply_A, 0);
AudioConnection          patchCord6(lfo, 0, mixer1, 0);
AudioConnection          patchCord7(voice1filterenv, 0, mixer1, 1);
AudioConnection          patchCord8(voice1_multiply_A, 0, voice1filter, 0);
AudioConnection          patchCord9(mixer1, 0, voice1filter, 1);
AudioConnection          patchCord10(voice1filter, 0, mainOutMixer, 0);
AudioConnection          patchCord11(delayFilter, 0, mainOutMixer, 3);
AudioConnection          patchCord12(mainOutMixer, 0, i2s1, 0);
AudioConnection          patchCord13(mainOutMixer, 0, i2s1, 1);
AudioConnection          patchCord14(mainOutMixer, delay1);
AudioConnection          patchCord15(delay1, 0, delayFilter, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=4123,1942
// GUItool: end automatically generated code
#endif