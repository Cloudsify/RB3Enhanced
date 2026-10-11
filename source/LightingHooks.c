/*
    RB3Enhanced - LightingHooks.c
    Hooks for venue lighting.
*/

#include "ports.h"
#include "LightingHooks.h"

// next/prev/first lighting cues are queued in LightPreset::sManualEvents and played in order once the song reaches them
// the game seemingly only clears that queue in the Milo editor, so restarting or quitting mid-song leaves the old song's cues at the
// front of it, therefore no new cues can play until the song catches up to them
// so clear it whenever the director starts a song, before anything has been set up
// shout out to Aloquendiar from the Milohax discord for discovering the technical cause of this issue
void BandDirectorEnterHook(void *thisBandDirector)
{
#ifdef RB3E_XBOX
    LightPresetManualEventsClear((void *)PORT_LIGHTPRESET_MANUALEVENTS);
#else
    LightPresetResetEvents();
#endif
    BandDirectorEnter(thisBandDirector);
}
