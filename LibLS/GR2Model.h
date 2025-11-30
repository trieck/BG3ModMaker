#pragma once

struct GR2Vector2
{
    float x{0};
    float y{0};
};

struct GR2Vector3
{
    float x{0};
    float y{0};
    float z{0};
};

struct GR2Vector4
{
    float x{0};
    float y{0};
    float z{0};
    float w{0};
};

struct GR2U8Vector4
{
    uint8_t x{0};
    uint8_t y{0};
    uint8_t z{0};
    uint8_t w{0};
};

struct GR2U16Vector2
{
    uint16_t x{0};
    uint16_t y{0};
};

struct GR2U16Vector4
{
    uint16_t x{0};
    uint16_t y{0};
    uint16_t z{0};
    uint16_t w{0};
};

struct GR2Matrix4x4
{
    float m[4][4]{};
};

struct GR2Vertex
{
    GR2Vector3 position{};
    GR2U8Vector4 boneWeights{};
    GR2U8Vector4 boneIndices{};
    GR2U16Vector4 QTangent{};
    GR2U8Vector4 diffuseColor0{};
    GR2U16Vector2 texCoord0{};
    GR2U16Vector2 texCoord1{};
};

struct GR2Bounds
{
    GR2Vector3 min{};
    GR2Vector3 max{};
    GR2Vector3 center{};
    float radius{0};
};

struct GR2Mesh
{
    std::vector<GR2Vertex> vertices;
    std::vector<uint32_t> indices;
    GR2Bounds bounds{};
};

struct GR2Model
{
    std::vector<GR2Mesh> meshes;
    GR2Bounds bounds{};
};
