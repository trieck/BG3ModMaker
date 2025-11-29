#pragma once

struct Vector2
{
    float x;
    float y;
};

struct Vector3
{
    float x;
    float y;
    float z;
};

struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};

struct U8Vector4
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t w;
};

struct U16Vector2
{
    uint16_t x;
    uint16_t y;
};

struct U16Vector4
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t w;
};

struct Matrix4x4
{
    float m[4][4];
};

struct Vertex
{
    Vector3 position;
    U8Vector4 boneWeights;
    U8Vector4 boneIndices;
    U16Vector4 QTangent;
    U8Vector4 diffuseColor0;
    U16Vector2 texCoord0;
    U16Vector2 texCoord1;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct GR2Model
{
    std::vector<Mesh> meshes;
};
