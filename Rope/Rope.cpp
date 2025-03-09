#include "framework.h"

#include "Rope.h"
#include "RBTree.h"

struct MyData
{
    MyData() = default;

    explicit MyData(std::string v) : value(std::move(v))
    {
    }

    MyData(const MyData& other) : value(other.value)
    {
    }

    MyData(MyData&& other) noexcept : value(std::move(other.value))
    {
    }

    MyData& operator=(const MyData& other)
    {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    MyData& operator=(MyData&& other) noexcept
    {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }

    std::string value;

    bool operator <(const MyData& other) const
    {
        return value < other.value;
    }

    bool operator >(const MyData& other) const
    {
        return value > other.value;
    }
};

int main(int argc, char* argv[])
{
    RBTree<int> tree;
    tree.insert(5);
    tree.insert(3);

    auto result = tree.find(3);
    if (result != 3) {
        return -1;
    }

    result = tree.find(4);
    if (result != 0) {
        return -1;
    }

    result = tree.find(5);
    if (result != 5) {
        return -1;
    }

    tree.remove(5);
    result = tree.find(5);
    if (result != 0) {
        return -1;
    }

    RBTree<MyData> tree2;
    tree2.insert(MyData("Hello"));
    tree2.insert(MyData("World"));

    auto result2 = tree2.find(MyData("Hello"));
    if (result2.value != "Hello") {
        return -1;
    }

    result2 = tree2.find(MyData("World"));
    if (result2.value != "World") {
        return -1;
    }

    result2 = tree2.find(MyData("NotFound"));
    if (!result2.value.empty()) {
        return -1;
    }

    RBTree<std::vector<std::string>> tree3;
    tree3.insert(std::vector<std::string>{"Hello", "World"});
    tree3.insert(std::vector<std::string>{"Foo", "Bar"});

    auto result3 = tree3.find(std::vector<std::string>{"Hello", "World"});
    if (result3[0] != "Hello" || result3[1] != "World") {
        return -1;
    }

    result3 = tree3.find(std::vector<std::string>{"Foo", "Bar"});
    if (result3[0] != "Foo" || result3[1] != "Bar") {
        return -1;
    }

    result3 = tree3.find(std::vector<std::string>{"NotFound"});
    if (!result3.empty()) {
        return -1;
    }

    return 0;
}
