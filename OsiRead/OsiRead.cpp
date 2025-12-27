#include "stdafx.h"
#include "OsiReader.h"
#include "Timer.h"

std::unordered_set<const OsiNode*> visitedNodes;

void printNode(const OsiStory& story, const OsiNode& node, uint32_t level);
void printCall(const OsiStory& story, const OsiCall& call, uint32_t level);

void indent(uint32_t level)
{
    for (uint32_t i = 0; i < level; ++i) {
        std::cout << "  ";
    }
}

void printParams(const OsiStory& story, const OsiCall& call)
{
    bool first = true;
    for (const auto& param : call.parameters) {
        if (!first) {
            std::cout << ", ";
        }
        first = false;
        const auto& value = param->value;
        std::visit([&]<typename T0>(const T0& arg) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, int32_t>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, float>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "\"" << arg << "\"";
            } else {
                std::cout << "<unknown>";
            }
        }, value);
    }
}

void printDatabase(const OsiStory& story, const OsiDBNode& dbNode, uint32_t level)
{
    indent(level);
    std::cout << "[" << story.nodeTypeName(dbNode.type) << "] " << dbNode.name << "\n";

    const auto& db = story.databases[dbNode.dbRef];

    indent(level + 1);
    std::cout << "Parameters: (";
    bool first = true;
    for (const auto& typeId : db.parameters.types) {
        if (!first) {
            std::cout << ", ";
        }
        first = false;
        std::cout << story.typeName(typeId);
    }
    std::cout << ")\n";
}

void printProc(const OsiStory& story, const OsiProcNode& proc, uint32_t level)
{
    indent(level);
    std::cout << "[" << story.nodeTypeName(proc.type) << "] " << proc.name << "\n";

    indent(level + 1);
    std::cout << "RefBy:\n";
    for (const auto& entry : proc.refBy) {
        const auto& node = story.nodes[entry.nodeRef];
        printNode(story, *node, level + 2);
    }
}

void printRule(const OsiStory& story, const OsiRuleNode& rule, uint32_t level)
{
    indent(level);

    std::cout << "Calls:\n";
    for (const auto& call : rule.calls) {
        printCall(story, call, level + 1);
    }

    indent(level);
    std::cout << "Variables:\n";
    for (const auto& var : rule.variables) {
        indent(level + 1);
        std::cout << var.variableName << "\n";
    }
}

void printAdapter(const OsiAdapter& adapter, uint32_t level)
{
    indent(level);

    std::cout << "Adapter[" << adapter.index << "]: Constants: (";
    bool first = true;
    for (const auto& constValue : adapter.constants.physical) {
        if (!first) {
            std::cout << ", ";
        }
        first = false;
        std::visit([&]<typename T0>(const T0& arg) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, int32_t>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, float>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "\"" << arg << "\"";
            } else {
                std::cout << "<unknown>";
            }
        }, constValue.value);
    }
    std::cout << ")\n";
}

void printJoin(const OsiStory& story, const OsiJoinNode& joinNode, uint32_t level)
{
    indent(level);
    std::cout << "[" << story.nodeTypeName(joinNode.type) << "] " << joinNode.name << "\n";

    if (joinNode.leftParentRef != INVALID_REF) {
        const auto& leftParent = story.nodes[joinNode.leftParentRef];
        indent(level + 1);
        std::cout << "Left Parent:\n";
        printNode(story, *leftParent, level + 2);
    }
    if (joinNode.rightParentRef != INVALID_REF) {
        indent(level + 1);
        std::cout << "Right Parent:\n";
        const auto& rightParent = story.nodes[joinNode.rightParentRef];
        printNode(story, *rightParent, level + 2);
    }
    if (joinNode.leftAdapterRef != INVALID_REF) {
        const auto& leftAdapter = story.adapters[joinNode.leftAdapterRef];
        printAdapter(leftAdapter, level + 1);
    }
    if (joinNode.rightAdapterRef != INVALID_REF) {
        const auto& rightAdapter = story.adapters[joinNode.leftAdapterRef];
        printAdapter(rightAdapter, level + 1);
    }
    if (joinNode.leftDBNodeRef != INVALID_REF) {
        indent(level + 1);
        std::cout << "Left DB Node:\n";
        const auto& leftDBNode = story.nodes[joinNode.leftDBNodeRef];
        printNode(story, *leftDBNode, level + 2);
    }
    if (joinNode.rightDBNodeRef != INVALID_REF) {
        indent(level + 1);
        std::cout << "Right DB Node:\n";
        const auto& rightDBNode = story.nodes[joinNode.rightDBNodeRef];
        printNode(story, *rightDBNode, level + 2);
    }
}

void printNode(const OsiStory& story, const OsiNode& node, uint32_t level)
{
    if (visitedNodes.contains(&node)) {
        indent(level);
        std::cout << "[" << story.nodeTypeName(node.type) << "] " << node.name << "(0x" << std::hex << &node << ")" << std::dec << std::endl;
        return;
    }

    visitedNodes.insert(&node);

    switch (node.type) {
    case NT_DATABASE: {
        const auto& dbNode = static_cast<const OsiDBNode&>(node);
        printDatabase(story, dbNode, level);
    }
    break;
    case NT_PROC: {
        const auto& procNode = static_cast<const OsiProcNode&>(node);
        printProc(story, procNode, level);
    }
    break;
    case NT_RULE: {
        const auto& ruleNode = static_cast<const OsiRuleNode&>(node);
        printRule(story, ruleNode, level);
    }
    break;
    case NT_AND:
    case NT_NAND: {
        const auto& joinNode = static_cast<const OsiJoinNode&>(node);
        printJoin(story, joinNode, level);
    }
    break;
    default:
        indent(level);
        std::cout << "[" << story.nodeTypeName(node.type) << "] " << node.name << "\n";
    }
}

void printCall(const OsiStory& story, const OsiCall& call, uint32_t level)
{
    indent(level);

    std::cout << call.name << "(";
    printParams(story, call);
    std::cout << ")" << std::endl;

    auto it = story.functionNames.find(call.name);
    if (it == story.functionNames.end()) {
        indent(level);
        std::cout << "<function not found>" << std::endl;
        return;
    }

    const auto& func = it->second;
    auto nodeRef = func->nodeRef;
    if (nodeRef == INVALID_REF) {
        nodeRef = func->actionRef;
        indent(level);
        if (nodeRef != INVALID_REF) {
            std::cout << "<action node>" << std::endl;
        } else {
            std::cout << "<no node>" << std::endl;
        }
    }
    if (nodeRef == INVALID_REF) {
        return;
    }

    const auto& node = story.nodes[nodeRef];

    printNode(story, *node, level + 1);
}

void printGoals(const OsiStory& story, uint32_t level)
{
    for (const auto& goal : story.goals) {
        std::cout << "Goal[" << goal.index << "]: " << goal.name << "\n";
        indent(level + 1);
        std::cout << "Init Calls:\n";
        for (const auto& call : goal.initCalls) {
            printCall(story, call, level + 2);
        }
    }
}

void printStory(const OsiStory& story)
{
    printGoals(story, 0);
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
