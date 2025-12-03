#pragma once

#include "GR2Model.h"
#include "GR2Reader.h"

class GR2ModelBuilder
{
public:
    GR2ModelBuilder() = default;
    ~GR2ModelBuilder() = default;

    GR2Model build(const char* filename, const GR2Callback& callback = {});
    GR2Model build(const ByteBuffer& buffer, const GR2Callback& callback = {});
    GR2Model build(const GR2Reader& reader);
private:
    GR2Object::Ptr findFirst(const std::vector<GR2Object::Ptr>& objs, const std::string& name);
    GR2Object::Ptr findFirstChild(const GR2Object::Ptr& obj, const std::string& name);

    uint32_t findStride(const GR2Object::Ptr& obj);
    std::vector<GR2Vertex> extractVertices(const GR2Object::Ptr& obj);
    std::vector<uint32_t> extractIndices(const GR2Object::Ptr& obj);
    GR2Mesh extractMesh(const GR2Object::Ptr& meshObj);
};
