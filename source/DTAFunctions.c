/*
    RB3Enhanced - DTAFunctions.c
    Custom DTA functions, usable by scripts in-game.
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "ports.h"
#include "GlobalSymbols.h"
#include "config.h"
#include "net_events.h"
#include "rb3/Data.h"
#include "rb3/SongDB.h"
#include "rb3/SongMetadata.h"
#include "rb3/BandSongMgr.h"
#include "rb3enhanced.h"
#include "net.h"
#include "version.h"

DataNode *DTAGetRB3EBuildTag(DataNode *node, DataArray *args)
{
    node->type = SYMBOL;
    node->value.string = globalSymbols.buildtag.sym;
    return node;
}

DataNode *DTAGetRB3ECommit(DataNode *node, DataArray *args)
{
    node->type = SYMBOL;
    node->value.string = globalSymbols.commit.sym;
    return node;
}

DataNode *DTAGetAPIVersion(DataNode *node, DataArray *args)
{
    /* 
        This API version should be incremented every time a DTA function gets
        added, has functionality modified, or removed, as well as every time a
        major feature is added/removed from RB3E or a new major version is
        released.

        API version history:
            0 - 29/09/2025 - Initial API version.
            1 - 10/11/2026 - Added song track and game gem queries.
    */
    node->type = INT_VALUE;
    node->value.intVal = 1;
    return node;
}

DataNode *DTAPrintToDebugger(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    switch (firstArg->type)
    {
    case FLOAT_VALUE:
        RB3E_MSG("print_debug: %f", firstArg->value.floatVal);
        break;
    case INT_VALUE:
        RB3E_MSG("print_debug: %i", firstArg->value.intVal);
        break;
    case STRING_VALUE:
        RB3E_MSG("print_debug: %s", (char *)*firstArg->value.object);
        break;
    default:
        RB3E_MSG("print_debug: %p", firstArg->value.object);
        break;
    }
    node->type = INT_VALUE;
    node->value.intVal = 1;
    return node;
}

DataNode *DTAGetMusicSpeed(DataNode *node, DataArray *args)
{
    node->type = FLOAT_VALUE;
    node->value.floatVal = config.SongSpeedMultiplier;
    return node;
}

DataNode *DTAGetTrackSpeed(DataNode *node, DataArray *args)
{
    node->type = FLOAT_VALUE;
    node->value.floatVal = config.TrackSpeedMultiplier;
    return node;
}

DataNode *DTAChangeMusicSpeed(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    switch (firstArg->type)
    {
    case FLOAT_VALUE:
        config.SongSpeedMultiplier = firstArg->value.floatVal;
        break;
    case INT_VALUE:
        config.SongSpeedMultiplier = (float)(firstArg->value.intVal);
        break;
    default:
        RB3E_MSG("Invalid type %i for rb3e_change_music_speed", firstArg->type);
        config.SongSpeedMultiplier = 1.0;
        break;
    }
    RB3E_DEBUG("Setting music speed to %.2f", config.SongSpeedMultiplier);
    node->type = INT_VALUE;
    node->value.intVal = 1;
    return node;
}

DataNode *DTAChangeTrackSpeed(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    switch (firstArg->type)
    {
    case FLOAT_VALUE:
        config.TrackSpeedMultiplier = firstArg->value.floatVal;
        break;
    case INT_VALUE:
        config.TrackSpeedMultiplier = (float)(firstArg->value.intVal);
        break;
    default:
        RB3E_MSG("Invalid type %i for rb3e_change_track_speed", firstArg->type);
        config.TrackSpeedMultiplier = 1.0;
        break;
    }
    RB3E_DEBUG("Setting track speed to %.2f", config.TrackSpeedMultiplier);
    node->type = INT_VALUE;
    node->value.intVal = 1;
    return node;
}

DataNode *DTASetVenue(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    char *venue;
    switch (firstArg->type)
    {
    case STRING_VALUE:
        venue = (char *)*firstArg->value.object;
        if (venue == NULL || strlen(venue) > 29)
        {
            RB3E_MSG("Invalid venue string value for rb3e_set_venue", NULL);
        }
        else
        {
            strncpy(config.ForcedVenue, venue, RB3E_MAX_CONFIG_LEN);
        }
        break;
    default:
        RB3E_MSG("Invalid type %i for rb3e_set_venue", firstArg->type);
        break;
    }
    node->type = INT_VALUE;
    node->value.intVal = 1;
    return node;
}

DataNode *DTAIsEmulator(DataNode *node, DataArray *args)
{
    node->type = INT_VALUE;
    node->value.intVal = RB3E_IsEmulator();
    return node;
}

DataNode *DTARelaunchGame(DataNode *node, DataArray *args)
{
    node->type = INT_VALUE;
    node->value.intVal = RB3E_RelaunchGame();
    return node;
}

DataNode *DTAGetSongCount(DataNode *node, DataArray *args)
{
    node->type = INT_VALUE;
    node->value.intVal = RB3E_LoadedSongCount;
    return node;
}

DataNode *DTASendModData(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    DataNode *secondArg = DataNodeEvaluate(&args->mNodes->n[2]);
    node->type = INT_VALUE;
    node->value.intVal = 0;
    // TODO(Emma): allow sending integer/float/etc second arguments? other things as pointers?
    if (firstArg->type != STRING_VALUE || secondArg->type != STRING_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_send_mod_data! 1st: %i, 2nd: %i", firstArg->type, secondArg->type);
    else
    {
        RB3E_EventModData mod_event = {0};
        char *id_tag = (char *)*firstArg->value.object;
        char *data = (char *)*secondArg->value.object;
        if (id_tag == NULL || strlen(id_tag) > sizeof(mod_event.IdentifyValue))
            RB3E_MSG("Invalid identify string value for rb3e_send_mod_data! %p", id_tag);
        else if (data == NULL || strlen(data) > sizeof(mod_event.String))
            RB3E_MSG("Invalid data string value for rb3e_send_mod_data! %p", data);
        else // assume it's fine
        {
            strncpy(mod_event.IdentifyValue, id_tag, sizeof(mod_event.IdentifyValue));
            strncpy(mod_event.String, data, sizeof(mod_event.String));
            RB3E_SendEvent(RB3E_EVENT_DX_DATA, &mod_event, sizeof(mod_event));
            node->value.intVal = 1;
        }
    }
    return node;
}

DataNode *DTAGetSongName(DataNode *node, DataArray *args)
{
    // return "Darude Sandstorm";
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    Symbol rb3e_no_song_name;
    SymbolConstruct(&rb3e_no_song_name, "rb3e_no_song_name");
    node->type = SYMBOL;
    node->value.string = rb3e_no_song_name.sym;
    if (firstArg->type != INT_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_get_song_name! 1st: %i", firstArg->type);
    else
    {
        SongMetadata *songmet = GetMetadata((BandSongMgr *)PORT_THESONGMGR, firstArg->value.intVal);
        RB3E_DEBUG("rb3e_get_song_name %i", firstArg->value.intVal);
        if (songmet == NULL) {
            RB3E_MSG("!! FAILED TO GET SONG METADATA FOR %i !!", firstArg->value.intVal);
        } else {
            Symbol titleSym;
            SymbolConstruct(&titleSym, songmet->title.buf);
            node->value.string = titleSym.sym;
        }
    }
    return node;
}

DataNode *DTAGetArtist(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    Symbol rb3e_no_artist;
    SymbolConstruct(&rb3e_no_artist, "rb3e_no_artist");
    node->type = SYMBOL;
    node->value.string = rb3e_no_artist.sym;
    if (firstArg->type != INT_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_get_artist! 1st: %i", firstArg->type);
    else
    {
        SongMetadata *songmet = GetMetadata((BandSongMgr *)PORT_THESONGMGR, firstArg->value.intVal);
        RB3E_DEBUG("rb3e_get_artist %i", firstArg->value.intVal);
        if (songmet == NULL) {
            RB3E_MSG("!! FAILED TO GET SONG METADATA FOR %i !!", firstArg->value.intVal);
        } else {
            Symbol artistSym;
            SymbolConstruct(&artistSym, songmet->artist.buf);
            node->value.string = artistSym.sym;
        }
    }
    return node;
}

DataNode *DTAGetAlbum(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    Symbol rb3e_no_album;
    SymbolConstruct(&rb3e_no_album, "rb3e_no_album");
    node->type = SYMBOL;
    node->value.string = rb3e_no_album.sym;
    if (firstArg->type != INT_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_get_album! 1st: %i", firstArg->type);
    else
    {
        SongMetadata *songmet = GetMetadata((BandSongMgr *)PORT_THESONGMGR, firstArg->value.intVal);
        RB3E_DEBUG("rb3e_get_album %i", firstArg->value.intVal);
        if (songmet == NULL) {
            RB3E_MSG("!! FAILED TO GET SONG METADATA FOR %i !!", firstArg->value.intVal);
        } else {
            Symbol albumSym;
            SymbolConstruct(&albumSym, songmet->album.buf);
            node->value.string = albumSym.sym;
        }
    }
    return node;
}

DataNode *DTAGetGenre(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    Symbol rb3e_no_genre;
    SymbolConstruct(&rb3e_no_genre, "rb3e_no_genre");
    node->type = SYMBOL;
    node->value.string = rb3e_no_genre.sym;
    if (firstArg->type != INT_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_no_genre! 1st: %i", firstArg->type);
    else
    {
        SongMetadata *songmet = GetMetadata((BandSongMgr *)PORT_THESONGMGR, firstArg->value.intVal);
        RB3E_DEBUG("rb3e_get_genre %i", firstArg->value.intVal);
        if (songmet == NULL) {
            RB3E_MSG("!! FAILED TO GET SONG METADATA FOR %i !!", firstArg->value.intVal);
        } else {
            node->value.string = songmet->genre.sym;
        }
    }
    return node;
}

DataNode *DTAGetOrigin(DataNode *node, DataArray *args)
{
    DataNode *firstArg = DataNodeEvaluate(&args->mNodes->n[1]);
    Symbol rb3e_no_origin;
    SymbolConstruct(&rb3e_no_origin, "rb3e_no_origin");
    node->type = SYMBOL;
    node->value.string = rb3e_no_origin.sym;
    if (firstArg->type != INT_VALUE)
        RB3E_MSG("Invalid types for arguments to rb3e_no_origin! 1st: %i", firstArg->type);
    else
    {
        SongMetadata *songmet = GetMetadata((BandSongMgr *)PORT_THESONGMGR, firstArg->value.intVal);
        RB3E_DEBUG("rb3e_no_origin %i", firstArg->value.intVal);
        if (songmet == NULL) {
            RB3E_MSG("!! FAILED TO GET SONG METADATA FOR %i !!", firstArg->value.intVal);
        } else {
            node->value.string = songmet->gameOrigin.sym;
        }
    }
    return node;
}

DataNode *DTADeleteSongCache(DataNode *node, DataArray *args)
{
    node->type = INT_VALUE;
    node->value.intVal = RB3E_DeleteSongCache();
    return node;
}

DataNode *DTALocalIP(DataNode *node, DataArray *args)
{
    Symbol noIpSym;
    Symbol ipSym;
    unsigned int localIP = RB3E_GetInternalIP();
    if (localIP == 0)
    {
        SymbolConstruct(&noIpSym, "(not connected)");
        node->type = SYMBOL;
        node->value.string = noIpSym.sym;
        return node;
    }
    else
    {
        char ipBuffer[24];
        unsigned char *ipParts = (unsigned char *)&localIP;
        sprintf(ipBuffer, "%i.%i.%i.%i", ipParts[0], ipParts[1], ipParts[2], ipParts[3]);
        SymbolConstruct(&ipSym, ipBuffer);
        node->type = SYMBOL;
        node->value.string = ipSym.sym;
        return node;
    }
}

static SongDB *GetLoadedSongDB(const char *functionName)
{
    SongDB *songDB = *(SongDB **)PORT_THESONGDB;
    if (songDB == NULL)
    {
        RB3E_MSG("%s: SongDB is not available", functionName);
        return NULL;
    }
    if (songDB->mSongData == NULL)
    {
        RB3E_MSG("%s: SongData is not available", functionName);
        return NULL;
    }
    return songDB;
}

static SongDataTrackInfo **GetTrackInfos(SongData *songData, int *trackInfoCount)
{
    SongDataTrackInfo **trackInfos = (SongDataTrackInfo **)VECTOR_BEGIN(songData->mTrackInfos);
    int count = VECTOR_SIZE(songData->mTrackInfos, sizeof(SongDataTrackInfo *));
    *trackInfoCount = 0;

    // if this is called outside of a song, there will be no track infos, so just bail
    if (trackInfos == NULL)
    {
        return NULL;
    }

    // if for some reason the track info count is invalid, bail
    if (count < 0 || count > 32)
    {
        return NULL;
    }

    *trackInfoCount = count;
    return trackInfos;
}

static int FindTrackByType(SongData *songData, TrackType trackType)
{
    int i;
    int trackInfoCount;
    SongDataTrackInfo **trackInfos = GetTrackInfos(songData, &trackInfoCount);

    if (trackInfos == NULL)
    {
        return -1;
    }

    for (i = 0; i < trackInfoCount; i++)
    {
        // chekc if we've found a hit and return its index
        if (trackInfos[i] != NULL && trackInfos[i]->trackType == trackType)
        {
            return i;
        }
    }

    // special fallback for 22-fret pro guitar/bass charts
    // this would otherwise break when playing on a 22-fret pro guitar
    if (trackType == kTrackRealGuitar22Fret)
    {
        return FindTrackByType(songData, (TrackType)6);
    }
    if (trackType == kTrackRealBass22Fret)
    {
        return FindTrackByType(songData, (TrackType)8);
    }

    return -1;
}

static int GameGemListCount(const GameGemList *gemList)
{
    int gemCount;

    // if we either failed to get the gem list or the gem list vector, return 0
    if (gemList == NULL || VECTOR_BEGIN(gemList->mGems) == NULL)
    {
        return 0;
    }

    gemCount = VECTOR_SIZE(gemList->mGems, sizeof(GameGem));

    // if somehow the gem count is negative or 0, return 0
    if (gemCount <= 0)
    {
        return 0;
    }

    return gemCount;
}

static GameGemList *GetGameGemListFromDTAArgs(DataArray *args, const char *functionName)
{
    DataNode *trackTypeArg;
    DataNode *difficultyArg;
    SongDB *songDB;
    SongData *songData;
    GameGemList *gemList;
    int track;
    int numDifficulties;

    if (args == NULL || args->mNodes == NULL || args->mNodeCount < 3)
    {
        RB3E_MSG("%s requires <tracktype> <difficulty>", functionName);
        return NULL;
    }

    trackTypeArg = DataNodeEvaluate(&args->mNodes->n[1]);
    difficultyArg = DataNodeEvaluate(&args->mNodes->n[2]);
    if (trackTypeArg->type != INT_VALUE || difficultyArg->type != INT_VALUE)
    {
        RB3E_MSG("Invalid argument types for %s: tracktype=%i, difficulty=%i", functionName, trackTypeArg->type, difficultyArg->type);
        return NULL;
    }

    songDB = GetLoadedSongDB(functionName);
    if (songDB == NULL)
    {
        return NULL;
    }
    songData = songDB->mSongData;

    numDifficulties = songData->mNumDifficulties;
    if (difficultyArg->value.intVal < 0 || difficultyArg->value.intVal >= numDifficulties)
    {
        RB3E_MSG("%s: difficulty %i is out of range (number of difficulties = %i)", functionName, difficultyArg->value.intVal, numDifficulties);
        return NULL;
    }

    track = FindTrackByType(songData, (TrackType)trackTypeArg->value.intVal);
    if (track < 0)
    {
        RB3E_MSG("%s: track type %i was not found", functionName, trackTypeArg->value.intVal);
        return NULL;
    }

    gemList = SongDBGetGemListByDiff(songDB, track, difficultyArg->value.intVal);
    if (gemList == NULL)
    {
        RB3E_MSG("%s: no gem list for track %i difficulty %i", functionName, track, difficultyArg->value.intVal);
        return NULL;
    }

    if (VECTOR_BEGIN(gemList->mGems) == NULL)
    {
        RB3E_MSG("%s: gem data is not available for track %i difficulty %i", functionName, track, difficultyArg->value.intVal);
        return NULL;
    }

    return gemList;
}

static GameGem *GetGameGemFromDTAArgs(DataArray *args, const char *functionName)
{
    DataNode *gemIdArg;
    GameGemList *gemList;
    int gemCount;

    if (args == NULL || args->mNodes == NULL || args->mNodeCount < 4)
    {
        RB3E_MSG("%s requires <tracktype> <difficulty> <gemid>", functionName);
        return NULL;
    }

    gemList = GetGameGemListFromDTAArgs(args, functionName);
    if (gemList == NULL)
    {
        return NULL;
    }

    gemIdArg = DataNodeEvaluate(&args->mNodes->n[3]);
    if (gemIdArg->type != INT_VALUE)
    {
        RB3E_MSG("Invalid argument type for %s: gemid=%i", functionName, gemIdArg->type);
        return NULL;
    }

    gemCount = GameGemListCount(gemList);
    if (gemIdArg->value.intVal < 0 || gemIdArg->value.intVal >= gemCount)
    {
        RB3E_MSG("%s: gem ID %i is out of range (number of gems = %i)", functionName, gemIdArg->value.intVal, gemCount);
        return NULL;
    }

    return &((GameGem *)VECTOR_BEGIN(gemList->mGems))[gemIdArg->value.intVal];
}

static int DTAArgsRequestGameGemEndTime(DataArray *args)
{
    DataNode *endTimeArg;

    if (args == NULL || args->mNodes == NULL || args->mNodeCount < 5)
    {
        return 0;
    }

    endTimeArg = DataNodeEvaluate(&args->mNodes->n[4]);
    if (endTimeArg->type != INT_VALUE)
    {
        return 0;
    }

    if (endTimeArg->value.intVal == 1)
    {
        return 1;
    }

    return 0;
}

DataNode *DTAGetGameGemTimeMs(DataNode *node, DataArray *args)
{
    GameGem *gem = GetGameGemFromDTAArgs(args, "rb3e_get_game_gem_time_ms");
    node->type = FLOAT_VALUE;
    if (gem == NULL)
    {
        node->value.floatVal = -1.0f;
    }
    else if (DTAArgsRequestGameGemEndTime(args))
    {
        node->value.floatVal = gem->milliSeconds + gem->durationMs;
    }
    else
    {
        node->value.floatVal = gem->milliSeconds;
    }
    return node;
}

DataNode *DTAGetGameGemTimeTick(DataNode *node, DataArray *args)
{
    GameGem *gem = GetGameGemFromDTAArgs(args, "rb3e_get_game_gem_time_tick");
    node->type = INT_VALUE;
    if (gem == NULL)
    {
        node->value.intVal = -1;
    }
    else if (DTAArgsRequestGameGemEndTime(args))
    {
        node->value.intVal = gem->tick + gem->durationTicks;
    }
    else
    {
        node->value.intVal = gem->tick;
    }
    return node;
}

DataNode *DTAGetGameGemCount(DataNode *node, DataArray *args)
{
    GameGemList *gemList = GetGameGemListFromDTAArgs(args, "rb3e_get_game_gem_count");
    node->type = INT_VALUE;
    if (gemList == NULL)
    {
        node->value.intVal = -1;
    }
    else
    {
        node->value.intVal = GameGemListCount(gemList);
    }
    return node;
}

DataNode *DTAGetGameGemSlots(DataNode *node, DataArray *args)
{
    GameGem *gem = GetGameGemFromDTAArgs(args, "rb3e_get_game_gem_slots");
    node->type = INT_VALUE;
    if (gem == NULL)
    {
        node->value.intVal = -1;
    }
    else
    {
        // GameGem::mSlots, one bit per lane/key/etc. the gem is on
        node->value.intVal = (int)*(unsigned int *)((unsigned char *)gem + 0xc);
    }
    return node;
}

DataNode *DTAGetTrackCount(DataNode *node, DataArray *args)
{
    SongDB *songDB = GetLoadedSongDB("rb3e_get_track_count");
    int trackCount;
    node->type = INT_VALUE;
    if (songDB == NULL)
    {
        node->value.intVal = -1;
    }
    else
    {
        GetTrackInfos(songDB->mSongData, &trackCount);
        node->value.intVal = trackCount;
    }
    return node;
}

DataNode *DTAGetTrackType(DataNode *node, DataArray *args)
{
    DataNode *trackArg;
    SongDB *songDB;
    SongDataTrackInfo **trackInfos;
    int trackCount;
    int track;
    node->type = INT_VALUE;
    node->value.intVal = -1;

    if (args->mNodeCount < 2)
    {
        RB3E_MSG("rb3e_get_track_type requires <trackindex>", NULL);
        return node;
    }
    trackArg = DataNodeEvaluate(&args->mNodes->n[1]);
    if (trackArg->type != INT_VALUE)
    {
        RB3E_MSG("Invalid type %i for rb3e_get_track_type", trackArg->type);
        return node;
    }
    track = trackArg->value.intVal;
    songDB = GetLoadedSongDB("rb3e_get_track_type");
    if (songDB == NULL)
    {
        return node;
    }
    trackInfos = GetTrackInfos(songDB->mSongData, &trackCount);
    if (track < 0 || track >= trackCount || trackInfos[track] == NULL)
    {
        RB3E_MSG("rb3e_get_track_type: track index %i is out of range (number of tracks = %i)", track, trackCount);
        return node;
    }
    node->value.intVal = trackInfos[track]->trackType;
    return node;
}

DataNode *DTAHasTrackType(DataNode *node, DataArray *args)
{
    DataNode *trackTypeArg;
    SongDB *songDB;
    node->type = INT_VALUE;
    node->value.intVal = 0;

    if (args->mNodeCount < 2)
    {
        RB3E_MSG("rb3e_has_track_type requires <tracktype>", NULL);
        return node;
    }
    trackTypeArg = DataNodeEvaluate(&args->mNodes->n[1]);
    if (trackTypeArg->type != INT_VALUE)
    {
        RB3E_MSG("Invalid type %i for rb3e_has_track_type", trackTypeArg->type);
        return node;
    }
    songDB = GetLoadedSongDB("rb3e_has_track_type");
    if (songDB == NULL)
    {
        return node;
    }

    // uses the same 22-fret fallback as the gem queries, so this is 1 whenever they would work
    node->value.intVal = FindTrackByType(songDB->mSongData, (TrackType)trackTypeArg->value.intVal) >= 0;
    return node;
}

#ifdef RB3E_XBOX
// this function is inlined on the Xbox version, so we re-create it
void DataRegisterFunc(Symbol name, DTAFunction_t func)
{
    *(DTAFunction_t *)HmxFactoryFuncAt((int *)PORT_GDATAFUNCS, &name) = func;
}
#endif

void AddDTAFunctions()
{
    DataRegisterFunc(globalSymbols.print_debug, DTAPrintToDebugger);
    DataRegisterFunc(globalSymbols.rb3e_api_version, DTAGetAPIVersion);
    DataRegisterFunc(globalSymbols.rb3e_build_tag, DTAGetRB3EBuildTag);
    DataRegisterFunc(globalSymbols.rb3e_commit, DTAGetRB3ECommit);
    DataRegisterFunc(globalSymbols.rb3e_change_music_speed, DTAChangeMusicSpeed);
    DataRegisterFunc(globalSymbols.rb3e_change_track_speed, DTAChangeTrackSpeed);
    DataRegisterFunc(globalSymbols.rb3e_get_music_speed, DTAGetMusicSpeed);
    DataRegisterFunc(globalSymbols.rb3e_get_track_speed, DTAGetTrackSpeed);
    DataRegisterFunc(globalSymbols.rb3e_set_venue, DTASetVenue);
    DataRegisterFunc(globalSymbols.rb3e_is_emulator, DTAIsEmulator);
    DataRegisterFunc(globalSymbols.rb3e_relaunch_game, DTARelaunchGame);
    DataRegisterFunc(globalSymbols.rb3e_get_song_count, DTAGetSongCount);
    DataRegisterFunc(globalSymbols.rb3e_send_event_string, DTASendModData);
    DataRegisterFunc(globalSymbols.rb3e_get_song_name, DTAGetSongName);
    DataRegisterFunc(globalSymbols.rb3e_get_artist, DTAGetArtist);
    DataRegisterFunc(globalSymbols.rb3e_get_album, DTAGetAlbum);
    DataRegisterFunc(globalSymbols.rb3e_get_origin, DTAGetOrigin);
    DataRegisterFunc(globalSymbols.rb3e_get_genre, DTAGetGenre);
    DataRegisterFunc(globalSymbols.rb3e_delete_songcache, DTADeleteSongCache);
    DataRegisterFunc(globalSymbols.rb3e_local_ip, DTALocalIP);
    DataRegisterFunc(globalSymbols.rb3e_get_game_gem_time_ms, DTAGetGameGemTimeMs);
    DataRegisterFunc(globalSymbols.rb3e_get_game_gem_time_tick, DTAGetGameGemTimeTick);
    DataRegisterFunc(globalSymbols.rb3e_get_game_gem_count, DTAGetGameGemCount);
    DataRegisterFunc(globalSymbols.rb3e_get_game_gem_slots, DTAGetGameGemSlots);
    DataRegisterFunc(globalSymbols.rb3e_get_track_count, DTAGetTrackCount);
    DataRegisterFunc(globalSymbols.rb3e_get_track_type, DTAGetTrackType);
    DataRegisterFunc(globalSymbols.rb3e_has_track_type, DTAHasTrackType);
    RB3E_MSG("Added DTA functions!", NULL);
}
