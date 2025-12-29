#pragma once

#include "OsiStory.h"

typedef struct OsiData
{
    const OsiStory* pStory;
    const void* pdata;
} OSIDATA, *LPOSIDATA;

enum OsiViewType
{
    OVT_FUNCTION = 0,
    OVT_GOAL = 1,
    OVT_TYPE = 2,
};
