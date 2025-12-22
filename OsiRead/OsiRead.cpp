#include "stdafx.h"
#include "OsiReader.h"
#include "Timer.h"

void printTypes(const OsiStory& story)
{
    std::cout << "Types:\n\n";

    std::vector<uint8_t> indices;
    indices.reserve(story.types.size());

    for (const auto& index : story.types | std::views::keys) {
        indices.emplace_back(index);
    }

    std::ranges::sort(indices);

    for (auto index : indices) {
        const auto& type = story.types.at(index);

        std::cout << "Type " << static_cast<int>(index) << ": " << type.name;

        if (story.isAlias(index)) {
            auto aliasIndex = story.typeAliases.at(index);
            std::cout << " (alias of Type " << static_cast<int>(aliasIndex)
                << ": " << story.types.at(aliasIndex).name << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

void printEnums(const OsiStory& story)
{
    std::cout << "\nEnums:\n\n";

    // Collect and sort enum type IDs
    std::vector<uint16_t> enumTypeIds;
    enumTypeIds.reserve(story.enums.size());

    for (const auto& typeId : story.enums | std::views::keys) {
        enumTypeIds.emplace_back(typeId);
    }

    std::ranges::sort(enumTypeIds);

    for (auto typeId : enumTypeIds) {
        const auto& osiEnum = story.enums.at(typeId);

        // Resolve enum name from type table
        std::string enumName = "<unknown>";
        auto it = story.types.find(static_cast<uint8_t>(typeId));
        if (it != story.types.end()) {
            enumName = it->second.name;
        }

        std::cout << "Enum " << enumName
            << " (Type " << static_cast<int>(typeId) << "):\n";

        // Collect and sort enum elements by numeric value
        std::vector<std::pair<std::string, uint64_t>> elements;
        elements.reserve(osiEnum.elements.size());

        for (const auto& [name, value] : osiEnum.elements) {
            elements.emplace_back(name, value);
        }

        std::ranges::sort(elements,
                          [](const auto& a, const auto& b) {
                              return a.second < b.second;
                          });

        for (const auto& [name, value] : elements) {
            std::cout << "  " << name << " = " << value << "\n";
        }

        std::cout << "\n";
    }
}

void printFunctions(const OsiStory& story)
{
    std::cout << "\nFunctions:\n\n";

    for (const auto& func : story.functions) {
        const auto& sig = func.name;
        std::cout << sig.name << "(";
        for (auto i = 0u; i < sig.parameters.types.size(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            if (sig.isOutParam(i)) {
                std::cout << "out ";
            }
            auto typeId = sig.parameters.types[i];
            std::cout << story.typeName(typeId);
        }
        std::cout << ")\n";
    }
}

void printRule(const OsiStory& story, const OsiRuleNode* rule)
{
    std::cout << "  Line: " << rule->line << "\n";
    std::cout << "  Is Query: " << (rule->isQuery ? "Yes" : "No") << "\n";
    std::cout << "  Variables:\n";
    for (const auto& var : rule->variables) {
        std::cout << "    " << var.variableName
            << " : " << story.typeName(var.type)
            << "\n";
    }
    std::cout << "  Calls:\n";
    for (const auto& call : rule->calls) {
        std::cout << "    " << (call.negate ? "NOT " : "") << call.name << "(";
        for (auto i = 0u; i < call.parameters.size(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            const auto& param = call.parameters[i];
            auto typeName = story.typeName(param->type);
            std::cout << typeName << " ";

            if (param->isValid()) {
                switch (param->value.index()) {
                case 0: // int32_t
                    std::cout << std::get<int32_t>(param->value);
                    break;
                case 1: // int64_t
                    std::cout << std::get<int64_t>(param->value);
                    break;
                case 2: // float
                    std::cout << std::get<float>(param->value);
                    break;
                case 3: // string
                    std::cout << "\"" << std::get<std::string>(param->value) << "\"";
                    break;
                default:
                    std::cout << "<unknown>";
                    break;
                }
            } else {
                std::cout << "<invalid>";
            }
        }
        std::cout << ")\n";
    }
}

void printNodes(const OsiStory& story)
{
    std::cout << "\nNodes:\n\n";

    for (const auto& node : story.nodes) {
        auto nodeId = node->index;

        std::cout
            << "Node " << nodeId
            << " Type=" << story.nodeTypeName(node->type);
        if (!node->name.empty()) {
            std::cout << " Name=\"" << node->name << "\"";
        }
        std::cout << "\n";
        if (node->type == NT_RULE) {
            auto rule = dynamic_cast<OsiRuleNode*>(node.get());
            printRule(story, rule);
        }
    }
}

void printStory(const OsiStory& story)
{
    printTypes(story);
    printEnums(story);
    printFunctions(story);
    printNodes(story);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " {file-path}\n";
        return 1;
    }

    Timer timer;

    try {
        OsiReader reader;
        reader.readFile(argv[1]);
        std::cout << "   Reading took: " << timer.str() << std::endl;

        printStory(reader.story());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
