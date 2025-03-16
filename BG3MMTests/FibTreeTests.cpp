#include "pch.h"

#include <vector>
#include <functional>
#include <CppUnitTest.h>
#include <optional>

#include "FibTree.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(FibTreeTests)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
    }

    TEST_METHOD(TestCompilable)
    {
        // Test whether various types are allowed (compilable)
        static_assert(std::is_constructible_v<FibTree<int, int>>, "FibTree<int, int> should be valid.");
        static_assert(std::is_constructible_v<FibTree<std::string, std::string>>,
            "FibTree<std::string, std::string> should be valid.");
        static_assert(std::is_constructible_v<FibTree<std::vector<std::string>, std::vector<std::string>>>,
            "FibTree<std::vector<std::string>, std::vector<std::string>> should be valid.");
        static_assert(std::is_constructible_v<FibTree<std::string, std::vector<std::string>>>,
            "FibTree<std::string, std::vector<std::string>> should be valid.");
        static_assert(std::is_constructible_v<FibTree<std::vector<std::string>, std::string>>,
            "FibTree<std::vector<std::string>, std::string>> should be valid.");
    }

    TEST_METHOD(TestInsert)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);
    }

    TEST_METHOD(TestExists)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);

        Assert::IsTrue(tree.exists(3));
        Assert::IsTrue(tree.exists(7));
        Assert::IsTrue(tree.exists(5));
        Assert::IsTrue(tree.exists(2));
        Assert::IsFalse(tree.exists(4));
        Assert::IsFalse(tree.exists(8));
        Assert::IsFalse(tree.exists(1));
    }

    TEST_METHOD(TestFind)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);

        int value = 0;
        Assert::IsTrue(tree.find(3, value));
        Assert::AreEqual(value, 3);
        Assert::IsTrue(tree.find(7, value));
        Assert::AreEqual(value, 7);
        Assert::IsTrue(tree.find(5, value));
        Assert::AreEqual(value, 5);
        Assert::IsTrue(tree.find(2, value));
        Assert::AreEqual(value, 2);
        Assert::IsFalse(tree.find(4, value));
    }

    TEST_METHOD(TestIsEmpty)
    {
        FibTree<int, int> tree;
        Assert::IsTrue(tree.isEmpty());

        tree.insert(3, 3);
        Assert::IsFalse(tree.isEmpty());
    }

    TEST_METHOD(TestRemove)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);

        Assert::IsTrue(tree.exists(3));
        tree.remove(3);
        Assert::IsFalse(tree.exists(3));

        Assert::IsTrue(tree.exists(5));
        tree.remove(5);
        Assert::IsFalse(tree.exists(5));

        Assert::IsTrue(tree.exists(7));
        tree.remove(7);
        Assert::IsFalse(tree.exists(7));

        Assert::IsTrue(tree.exists(2));
        tree.remove(2);
        Assert::IsFalse(tree.exists(2));

        Assert::IsTrue(tree.isEmpty());
    }

    TEST_METHOD(TestRemoveAll)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);
        Assert::IsFalse(tree.isEmpty());
        tree.removeAll();
        Assert::IsTrue(tree.isEmpty());
    }

    TEST_METHOD(TestInOrder)
    {
        FibTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);

        std::optional<int> lastKey;

        tree.traverse(
            [&](int k, int v)
            {
                if (lastKey.has_value()) {
                    Assert::IsTrue(lastKey.value() < k);
                }

                lastKey = k;
            }
        );
    }
    
    TEST_METHOD(TestStringTree)
    {
        FibTree<std::string, std::string> tree;
        tree.insert("apple", "fruit");
        tree.insert("carrot", "vegetable");
        tree.insert("banana", "fruit");
        tree.insert("broccoli", "vegetable");
        Assert::IsTrue(tree.exists("apple"));
        Assert::IsTrue(tree.exists("carrot"));
        Assert::IsTrue(tree.exists("banana"));
        Assert::IsTrue(tree.exists("broccoli"));
        std::optional<std::string> lastKey;
        tree.traverse(
            [&](const std::string& k, const std::string& v)
            {
                if (lastKey.has_value()) {
                    Assert::IsTrue(lastKey.value() < k);
                }
                lastKey = k;
            }
        );
    }
};
