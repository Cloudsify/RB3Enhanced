#ifndef _SONGDB_H
#define _SONGDB_H

#include "rb3/BandUser.h"
#include "rb3/GameGem.h"
#include "rb3/Vector.h"

typedef struct _SongDataTrackInfo
{
    unsigned char unknown_0x00[0x10];
    TrackType trackType;
} SongDataTrackInfo;

typedef struct _SongData
{
    unsigned char unknown_0x00[0x10];
    int mNumTracks;
    int mNumDifficulties;
#ifdef RB3E_XBOX
    unsigned char unknown_0x18[0x2c];
#else
    unsigned char unknown_0x18[0x24];
#endif
    vector mTrackInfos;
} SongData;

typedef struct _SongDB
{
    void *vtable;
    SongData *mSongData;
} SongDB;

extern GameGemList *SongDBGetGemListByDiff(SongDB *songDB, int track, int difficulty);

#endif // _SONGDB_H
