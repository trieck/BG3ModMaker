#pragma once

#include "OsiStory.h"

typedef struct OsiData
{
    const OsiStory* pStory;
    const void* pdata;
} OSIDATA, *LPOSIDATA;

enum OsiViewType
{
    OVT_GOAL = 0,
};
