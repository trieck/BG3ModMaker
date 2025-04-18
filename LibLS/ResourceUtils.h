#pragma once

#include "Resource.h"

class ResourceUtils
{
public:
    static Resource::Ptr loadResource(const char* filename, ResourceFormat format);
    static void saveResource(const char* filename, const Resource::Ptr& resource, ResourceFormat format);
};

