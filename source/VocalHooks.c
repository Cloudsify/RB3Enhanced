/*
    RB3Enhanced - VocalHooks.c
    Hooks for vocals.
*/

#include "VocalHooks.h"

// fixes the "talky bug" where talkies would become impossible due to mVoice/FullBandEnergy being uninitialized
// and happening to contain NaN or other invalid values that can't be normalized over time back to 0.0
void VoiceBeatResetHook(void *thisVoiceBeat)
{
    VoiceBeatReset(thisVoiceBeat);
    *(double *)((char *)thisVoiceBeat + 0x128) = 0.0; // mVoiceEnergy
    *(double *)((char *)thisVoiceBeat + 0x130) = 0.0; // mFullBandEnergy
}
