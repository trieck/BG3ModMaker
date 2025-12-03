#include "pch.h"
#include "GR2ModelBuilder.h"

GR2Model GR2ModelBuilder::build(const char* filename, const GR2Callback& callback)
{
    GR2Reader reader;
    reader.load(filename, callback);
    return build(reader);
}

GR2Model GR2ModelBuilder::build(const ByteBuffer& buffer, const GR2Callback& callback)
{
    GR2Reader reader;
    reader.load(buffer, callback);
    return build(reader);
}

GR2Model GR2ModelBuilder::build(const GR2Reader& reader)
{
    GR2Model model;

    const auto& rootObjects = reader.rootObjects();

    auto models = findFirst(rootObjects, "Models");
    if (models == nullptr) {
        return model; // No Models node found
    }

    auto meshBindings = findFirstChild(models, "MeshBindings");
    if (meshBindings == nullptr) {
        return model; // No MeshBindings node found
    }

    for (const auto& child : meshBindings->children) {
        if (child->name != "Mesh") {
            continue;
        }

        model.meshes.emplace_back(extractMesh(child));
    }

    // Calculate overall model bounds
    if (!model.meshes.empty()) {
        bool firstBounds = true;

        for (const auto& mesh : model.meshes) {
            if (mesh.vertices.empty()) {
                continue;
            }

            if (firstBounds) {
                model.bounds.min = mesh.bounds.min;
                model.bounds.max = mesh.bounds.max;
                firstBounds = false;
            } else {
                model.bounds.min.x = std::min(model.bounds.min.x, mesh.bounds.min.x);
                model.bounds.min.y = std::min(model.bounds.min.y, mesh.bounds.min.y);
                model.bounds.min.z = std::min(model.bounds.min.z, mesh.bounds.min.z);

                model.bounds.max.x = std::max(model.bounds.max.x, mesh.bounds.max.x);
                model.bounds.max.y = std::max(model.bounds.max.y, mesh.bounds.max.y);
                model.bounds.max.z = std::max(model.bounds.max.z, mesh.bounds.max.z);
            }
        }

        // Calculate overall center
        model.bounds.center.x = (model.bounds.min.x + model.bounds.max.x) * 0.5f;
        model.bounds.center.y = (model.bounds.min.y + model.bounds.max.y) * 0.5f;
        model.bounds.center.z = (model.bounds.min.z + model.bounds.max.z) * 0.5f;

        // Calculate overall radius
        auto dx = model.bounds.max.x - model.bounds.min.x;
        auto dy = model.bounds.max.y - model.bounds.min.y;
        auto dz = model.bounds.max.z - model.bounds.min.z;
        model.bounds.radius = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.5f;
    } else {
        model.bounds.min = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        model.bounds.max = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        model.bounds.center = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        model.bounds.radius = 0.0f;
    }

    return model;
}

GR2Object::Ptr GR2ModelBuilder::findFirst(const std::vector<GR2Object::Ptr>& objs, const std::string& name)
{
    for (const auto& obj : objs) {
        if (obj->name == name) {
            return obj;
        }
    }

    for (const auto& obj : objs) {
        auto child = findFirstChild(obj, name);
        if (child != nullptr) {
            return child;
        }
    }

    return nullptr;
}

GR2Object::Ptr GR2ModelBuilder::findFirstChild(const GR2Object::Ptr& obj, const std::string& name)
{
    for (const auto& child : obj->children) {
        if (child->name == name) {
            return child;
        }

        auto o = findFirstChild(child, name);
        if (o != nullptr) {
            return o;
        }
    }

    return nullptr;
}

uint32_t GR2ModelBuilder::findStride(const GR2Object::Ptr& obj)
{
    auto stride = 0u;

    for (auto i = 1u; i < obj->children.size(); ++i) {
        if (obj->children[i]->name == obj->children[0]->name) {
            stride = static_cast<int>(i);
            break;
        }
    }

    return stride;
}

std::vector<GR2Vertex> GR2ModelBuilder::extractVertices(const GR2Object::Ptr& obj)
{
    std::vector<GR2Vertex> vertices;

    auto stride = findStride(obj);

    const auto vertexCount = obj->children.size() / stride;

    for (auto i = 0u; i < vertexCount; ++i) {
        GR2Vertex v{};
        for (auto j = 0u; j < stride; ++j) {
            const auto& attr = obj->children[i * stride + j];
            if (attr->name == "Position") {
                const auto posObj = static_pointer_cast<GR2Float>(attr);
                if (posObj && posObj->values.size() >= 3) {
                    v.position.x = posObj->values[0];
                    v.position.y = posObj->values[1];
                    v.position.z = posObj->values[2];
                }
            } else if (attr->name == "BoneWeights") {
                const auto boneObj = std::static_pointer_cast<GR2UInt8>(attr);
                if (boneObj && boneObj->values.size() >= 4) {
                    v.boneWeights.x = boneObj->values[0];
                    v.boneWeights.y = boneObj->values[1];
                    v.boneWeights.z = boneObj->values[2];
                    v.boneWeights.w = boneObj->values[3];
                }
            } else if (attr->name == "BoneIndices") {
                const auto boneObj = std::static_pointer_cast<GR2UInt8>(attr);
                if (boneObj && boneObj->values.size() >= 4) {
                    v.boneIndices.x = boneObj->values[0];
                    v.boneIndices.y = boneObj->values[1];
                    v.boneIndices.z = boneObj->values[2];
                    v.boneIndices.w = boneObj->values[3];
                }
            } else if (attr->name == "QTangent") {
                const auto tanObj = std::static_pointer_cast<GR2UInt16>(attr);
                if (tanObj && tanObj->values.size() >= 4) {
                    v.QTangent.x = tanObj->values[0];
                    v.QTangent.y = tanObj->values[1];
                    v.QTangent.z = tanObj->values[2];
                    v.QTangent.w = tanObj->values[3];
                }
            } else if (attr->name == "DiffuseColor0") {
                const auto colObj = std::static_pointer_cast<GR2UInt8>(attr);
                if (colObj && colObj->values.size() >= 4) {
                    v.diffuseColor0.x = colObj->values[0];
                    v.diffuseColor0.y = colObj->values[1];
                    v.diffuseColor0.z = colObj->values[2];
                    v.diffuseColor0.w = colObj->values[3];
                }
            } else if (attr->name == "TextureCoordinates0") {
                const auto textObj = std::static_pointer_cast<GR2UInt16>(attr);
                if (textObj && textObj->values.size() >= 2) {
                    v.texCoord0.x = textObj->values[0];
                    v.texCoord0.y = textObj->values[1];
                }
            } else if (attr->name == "TextureCoordinates1") {
                const auto textObj = std::static_pointer_cast<GR2UInt16>(attr);
                if (textObj && textObj->values.size() >= 2) {
                    v.texCoord1.x = textObj->values[0];
                    v.texCoord1.y = textObj->values[1];
                }
            }
        }

        vertices.emplace_back(v);
    }

    return vertices;
}

std::vector<uint32_t> GR2ModelBuilder::extractIndices(const GR2Object::Ptr& obj)
{
    std::vector<uint32_t> indices;

    GR2Object::Ptr indexObj;
    if ((indexObj = findFirstChild(obj, "Indices16")) != nullptr) {
        if (auto indicesArray = std::static_pointer_cast<GR2ArrayReference>(indexObj)) {
            indices.reserve(indicesArray->children.size());

            for (const auto& child : indicesArray->children) {
                if (auto index = std::static_pointer_cast<GR2Int16>(child)) {
                    if (!index->values.empty()) {
                        indices.emplace_back(index->values[0]);
                    }
                }
            }
        }
    } else if ((indexObj = findFirstChild(obj, "Indices")) != nullptr) {
        if (auto indicesArray = std::static_pointer_cast<GR2ArrayReference>(indexObj)) {
            indices.reserve(indicesArray->children.size());

            for (const auto& child : indicesArray->children) {
                if (auto index = std::static_pointer_cast<GR2Int32>(child)) {
                    if (!index->values.empty()) {
                        indices.emplace_back(index->values[0]);
                    }
                }
            }
        }
    }

    return indices;
}

GR2Mesh GR2ModelBuilder::extractMesh(const GR2Object::Ptr& meshObj)
{
    GR2Mesh mesh;

    auto vertexDataRef = findFirstChild(meshObj, "PrimaryVertexData");
    if (!vertexDataRef) {
        return mesh;
    }

    auto verticesObj = findFirstChild(vertexDataRef, "Vertices");
    if (!verticesObj) {
        return mesh;
    }

    mesh.vertices = extractVertices(verticesObj);

    auto topObj = findFirstChild(meshObj, "PrimaryTopology");
    if (!topObj) {
        return mesh;
    }

    mesh.indices = extractIndices(topObj);

    // Calculate bounding box
    if (!mesh.vertices.empty()) {
        mesh.bounds.min = mesh.vertices[0].position;
        mesh.bounds.max = mesh.vertices[0].position;

        for (const auto& vertex : mesh.vertices) {
            mesh.bounds.min.x = std::min(mesh.bounds.min.x, vertex.position.x);
            mesh.bounds.min.y = std::min(mesh.bounds.min.y, vertex.position.y);
            mesh.bounds.min.z = std::min(mesh.bounds.min.z, vertex.position.z);

            mesh.bounds.max.x = std::max(mesh.bounds.max.x, vertex.position.x);
            mesh.bounds.max.y = std::max(mesh.bounds.max.y, vertex.position.y);
            mesh.bounds.max.z = std::max(mesh.bounds.max.z, vertex.position.z);
        }

        // Calculate center
        mesh.bounds.center.x = (mesh.bounds.min.x + mesh.bounds.max.x) * 0.5f;
        mesh.bounds.center.y = (mesh.bounds.min.y + mesh.bounds.max.y) * 0.5f;
        mesh.bounds.center.z = (mesh.bounds.min.z + mesh.bounds.max.z) * 0.5f;

        // Calculate radius (diagonal length / 2)
        auto dx = mesh.bounds.max.x - mesh.bounds.min.x;
        auto dy = mesh.bounds.max.y - mesh.bounds.min.y;
        auto dz = mesh.bounds.max.z - mesh.bounds.min.z;
        mesh.bounds.radius = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.5f;
    } else {
        // Empty mesh - set default bounds
        mesh.bounds.min = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        mesh.bounds.max = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        mesh.bounds.center = GR2Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f};
        mesh.bounds.radius = 0.0f;
    }

    return mesh;
}
