/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#include "player_synth.h"

#if SYNTH_PLAYER
AudioOutputI2S                    i2s_OUT;
AudioSynthWaveform                wf_1;
AudioSynthWaveform                wf_2;
AudioSynthWaveform                wf_3;
AudioSynthWaveform                wf_4;
AudioSynthWaveform                wf_5;
AudioSynthWaveform                wf_6;
AudioSynthWaveform                wf_7;
AudioSynthWaveform                wf_8;
AudioSynthWaveformSineModulated   fm_1;
AudioSynthWaveformSineModulated   fm_2;
AudioSynthWaveformSineModulated   fm_3;
AudioSynthWaveformSineModulated   fm_4;
AudioSynthWaveformSineModulated   fm_5;
AudioSynthWaveformSineModulated   fm_6;
AudioSynthWaveformSineModulated   fm_7;
AudioSynthWaveformSineModulated   fm_8;
AudioEffectFade                   fade_1;
AudioEffectFade                   fade_2;
AudioEffectFade                   fade_3;
AudioEffectFade                   fade_4;
AudioEffectFade                   fade_5;
AudioEffectFade                   fade_6;
AudioEffectFade                   fade_7;
AudioEffectFade                   fade_8;
AudioMixer4                       mix_1;
AudioMixer4                       mix_2;
AudioMixer4                       mix_3;
AudioConnection                   patchCord1(wf_1, fm_1);
AudioConnection                   patchCord2(wf_2, fm_2);
AudioConnection                   patchCord3(wf_3, fm_3);
AudioConnection                   patchCord4(wf_4, fm_4);
AudioConnection                   patchCord5(wf_5, fm_5);
AudioConnection                   patchCord6(wf_6, fm_6);
AudioConnection                   patchCord7(wf_7, fm_7);
AudioConnection                   patchCord8(wf_8, fm_8);
AudioConnection                   patchCord9(fm_1, fade_1);
AudioConnection                   patchCord10(fm_2, fade_2);
AudioConnection                   patchCord11(fm_3, fade_3);
AudioConnection                   patchCord12(fm_4, fade_4);
AudioConnection                   patchCord13(fm_5, fade_5);
AudioConnection                   patchCord14(fm_6, fade_6);
AudioConnection                   patchCord15(fm_7, fade_7);
AudioConnection                   patchCord16(fm_8, fade_8);
AudioConnection                   patchCord17(fade_1, 0, mix_1, 0);
AudioConnection                   patchCord18(fade_2, 0, mix_1, 1);
AudioConnection                   patchCord19(fade_3, 0, mix_1, 2);
AudioConnection                   patchCord20(fade_4, 0, mix_1, 3);
AudioConnection                   patchCord21(fade_5, 0, mix_2, 0);
AudioConnection                   patchCord22(fade_6, 0, mix_2, 1);
AudioConnection                   patchCord23(fade_7, 0, mix_2, 2);
AudioConnection                   patchCord24(fade_8, 0, mix_2, 3);
AudioConnection                   patchCord25(mix_1, 0, mix_3, 0);
AudioConnection                   patchCord26(mix_2, 0, mix_3, 1);
AudioConnection                   patchCord28(mix_3, 0, i2s_OUT, 0);
AudioConnection                   patchCord29(mix_3, 0, i2s_OUT, 1);

synth_t allSynth[MAX_SYNTH] = {
  {&wf_1, &fm_1, &fade_1, &mix_1},
  {&wf_2, &fm_2, &fade_2, &mix_1},
  {&wf_3, &fm_3, &fade_3, &mix_1},
  {&wf_4, &fm_4, &fade_4, &mix_1},
  {&wf_5, &fm_5, &fade_5, &mix_2},
  {&wf_6, &fm_6, &fade_6, &mix_2},
  {&wf_7, &fm_7, &fade_7, &mix_2},
  {&wf_8, &fm_8, &fade_8, &mix_2}
};

void SYNTH_PLAYER_SETUP(void) {
  for (int i = 0; i < MAX_SYNTH; i++) {
    allSynth[i].fade->fadeOut(0);
    allSynth[i].wf->begin(WAVEFORM_SINE);
    allSynth[i].wf->amplitude(0.9);
    allSynth[i].fm->amplitude(0.9);
  }
  allSynth[0].mix->gain(0, 0.25);
  allSynth[0].mix->gain(1, 0.25);
  allSynth[0].mix->gain(2, 0.25);
  allSynth[0].mix->gain(3, 0.25);

  allSynth[4].mix->gain(0, 0.25);
  allSynth[4].mix->gain(1, 0.25);
  allSynth[4].mix->gain(2, 0.25);
  allSynth[4].mix->gain(3, 0.25);
}

/////////////////////////// MAKE NOISE FONCTION !
void synth_player(void) {
  //static boolean lastState[MAX_BLOBS] = {false};

  for (blob_t* blob_ptr = (blob_t *)ITERATOR_START_FROM_HEAD(&llist_blobs); blob_ptr != NULL; blob_ptr = (blob_t *)ITERATOR_NEXT(blob_ptr)) {

    if (blob_ptr->UID < MAX_SYNTH) {
      AudioNoInterrupts();
      if (blob_ptr->state) {
        if (!blob_ptr->lastState) {
          allSynth[blob_ptr->UID].wf->phase(0);
          allSynth[blob_ptr->UID].fade->fadeIn(3);
        }
        else {
          allSynth[blob_ptr->UID].wf->frequency(blob_ptr->centroid.X * 3);
          allSynth[blob_ptr->UID].fm->frequency(blob_ptr->centroid.Y * 4 + 50);
          /*
            if (blob_ptr->UID == 0) {
            allSynth[blob_ptr->UID].wf->frequency((blob_ptr->centroid.X / 4.0) + 1);
            allSynth[blob_ptr->UID].fm->frequency((blob_ptr->centroid.Y / 2.0) + 5);
            }
            else if (blob_ptr->UID == 1) {
            allSynth[blob_ptr->UID].wf->frequency(blob_ptr->centroid.X * 3);
            allSynth[blob_ptr->UID].fm->frequency(blob_ptr->centroid.Y * 4 + 50);
            }
          */
        }
      }
      else {
        allSynth[blob_ptr->UID].fade->fadeOut(500);
      }
      AudioInterrupts();
    }
  }
}
#endif
