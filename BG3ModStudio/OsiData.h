#pragma once

#include "OsiStory.h"

typedef struct OsiData
{
    const OsiStory* pStory;
    const void* pdata;
} OSIDATA, *LPOSIDATA;

enum OsiViewType
{
    OVT_DATABASE = 0,
    OVT_ENUM,
    OVT_FUNCTION,
    OVT_GOAL,
    OVT_TYPE
};
