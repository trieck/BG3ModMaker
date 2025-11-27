#include "stdafx.h"
#include "GR2Reader.h"
#include "Timer.h"

static std::string typeToString(GR2NodeType type)
{
    switch (type) {
    case TYPE_NONE:
        return "NONE";
    case TYPE_INLINE:
        return "INLINE";
    case TYPE_REFERENCE:
        return "REFERENCE";
    case TYPE_REFERENCE_TO_ARRAY:
        return "REFERENCE_TO_ARRAY";
    case TYPE_ARRAY_OF_REFERENCES:
        return "ARRAY_OF_REFERENCES";
    case TYPE_VARIANT_REFERENCE:
        return "VARIANT_REFERENCE";
    case TYPE_REMOVED:
        return "REMOVED";
    case TYPE_REFERENCE_TO_VARIANT_ARRAY:
        return "REFERENCE_TO_VARIANT_ARRAY";
    case TYPE_STRING:
        return "STRING";
    case TYPE_TRANSFORM:
        return "TRANSFORM";
    case TYPE_REAL32:
        return "REAL32";
    case TYPE_INT8:
        return "INT8";
    case TYPE_UINT8:
        return "UINT8";
    case TYPE_BINORMAL_INT8:
        return "BINORMAL_INT8";
    case TYPE_NORMAL_UINT8:
        return "NORMAL_UINT8";
    case TYPE_INT16:
        return "INT16";
    case TYPE_UINT16:
        return "UINT16";
    case TYPE_BINORMAL_INT16:
        return "BINORMAL_INT16";
    case TYPE_NORMAL_UINT16:
        return "NORMAL_UINT16";
    case TYPE_INT32:
        return "INT32";
    case TYPE_UINT32:
        return "UINT32";
    case TYPE_REAL16:
        return "REAL16";
    case TYPE_EMPTY_REFERENCE:
        return "EMPTY_REFERENCE";
    }

    return "UNKNOWN";
}

template <typename T>
void printNumber(const std::vector<T>& values)
{
    if (values.size() == 0) {
        std::cout << "NULL";
        return;
    }

    if (values.size() == 1) {
        if constexpr (std::is_floating_point_v<T>) {
            std::cout << std::fixed << values[0];
        } else {
            std::cout << +values[0];
        }
    } else {
        std::cout << "[ ";
        for (auto i = 0u; i < values.size(); ++i) {
            if constexpr (std::is_floating_point_v<T>) {
                std::cout << std::fixed;
            }

            if constexpr (std::is_floating_point_v<T>) {
                std::cout << std::fixed << values[i];
            } else {
                std::cout << +values[i];
            }

            if (i < values.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << " ]";
    }

    if constexpr (std::is_floating_point_v<T>) {
        std::cout.unsetf(std::ios::floatfield);
    }
}

static void printTransform(const GR2ObjectInfo& info)
{
    const auto& obj = info.object;
    const auto level = info.level;

    ATLASSERT(obj->typeNode->type == TYPE_TRANSFORM);

    if (obj->data == nullptr) {
        std::cout << "NULL";
        return;
    }

    auto transformObj = std::static_pointer_cast<GRTransform>(obj);
    const auto& tr = transformObj->transform;

    std::cout << '\n';
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ');
    std::cout << std::fixed
        << "Flags: " << tr.flags << ",\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Translation: (" << tr.translation[0] << ", " << tr.translation[1] << ", " << tr.translation[2] << "),\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Rotation: (" << tr.rotation[0] << ", " << tr.rotation[1] << ", " << tr.rotation[2]
        << "),\n";
    std::cout << std::string(static_cast<size_t>(level + 1) * 3, ' ')
        << "Scale/Shear: (" << tr.scaleShear[0][0] << ", " << tr.scaleShear[0][1] << ", " << tr.scaleShear[0][2] << ", "
        << tr.scaleShear[1][0] << ", " << tr.scaleShear[1][1] << ", " << tr.scaleShear[1][2] << ", "
        << tr.scaleShear[2][0] << ", " << tr.scaleShear[2][1] << ", " << tr.scaleShear[2][2] << ")"
        << std::dec;
}

static void printValue(const GR2ObjectInfo& info)
{
    const auto& obj = info.object;

    switch (obj->typeNode->type) {
    case TYPE_STRING: {
        if (obj->data == nullptr) {
            std::cout << "NULL";
            break;
        }
        auto strObj = std::static_pointer_cast<GR2String>(obj);
        std::cout << "\"" << strObj->value << "\"";
        break;
    }
    case TYPE_INT16: {
        auto intObj = std::static_pointer_cast<GRInt16>(obj);
        printNumber(intObj->values);
        break;
    }
    case TYPE_INT32: {
        auto intObj = std::static_pointer_cast<GRInt32>(obj);
        printNumber(intObj->values);
        break;
    }
    case TYPE_REAL32: {
        auto floatObj = std::static_pointer_cast<GRFloat>(obj);
        printNumber(floatObj->values);
        break;
    }
    case TYPE_TRANSFORM: {
        printTransform(info);
        break;
    }
    default:
        if (obj->data == nullptr) {
            std::cout << "NULL";
            break;
        }
        std::cout << "0x"
            << std::setw(16) << std::setfill('0') << std::hex
            << reinterpret_cast<uint64_t>(obj->data)
            << std::dec
            << std::setfill(' ');
        break;
    }
}

static void printObject(const GR2ObjectInfo& info)
{
    std::cout << std::string(static_cast<size_t>(info.level) * 3, ' ');

    const auto& typeNode = info.object->typeNode;

    std::cout << "Name: " << info.object->name << ", type: " << typeToString(typeNode->type);
    if (typeNode->arraySize != 0) {
        std::cout << ", arraySize: " << typeNode->arraySize;
    }

    std::cout << ", Value: ";

    printValue(info);

    std::cout << "\n";
}

static void callback(const GR2ObjectInfo& info)
{
    printObject(info);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " {file-path}\n";
        return 1;
    }

    try {
        Timer timer;

        GR2Reader reader;
        reader.read(argv[1]);
        reader.traverse(callback);

        std::cout << "   Reading took: " << timer.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
