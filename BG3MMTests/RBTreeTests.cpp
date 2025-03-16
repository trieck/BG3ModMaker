#include "pch.h"

#include <vector>
#include <functional>

#include <CppUnitTest.h>
#include <optional>

#include "RBTree.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(RBTreeTests)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
    }

    TEST_METHOD(TestCompilable)
    {
        // Test whether various types are allowed (compilable)
        static_assert(std::is_constructible_v<RBTree<int, int>>, "RBTree<int, int> should be valid.");
        static_assert(std::is_constructible_v<RBTree<std::string, std::string>>,
                      "RBTree<std::string, std::string> should be valid.");
        static_assert(std::is_constructible_v<RBTree<std::vector<std::string>, std::vector<std::string>>>,
                      "RBTree<std::vector<std::string>, std::vector<std::string>> should be valid.");
        static_assert(std::is_constructible_v<RBTree<std::string, std::vector<std::string>>>,
                      "RBTree<std::string, std::vector<std::string>> should be valid.");
        static_assert(std::is_constructible_v<RBTree<std::vector<std::string>, std::string>>,
                      "RBTree<std::vector<std::string>, std::string>> should be valid.");
    }

    TEST_METHOD(TestInsert)
    {
        RBTree<int, int> tree;
        tree.insert(3, 3);
        tree.insert(7, 7);
        tree.insert(5, 5);
        tree.insert(2, 2);
    }

    TEST_METHOD(TestExists)
    {
        RBTree<int, int> tree;
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
        RBTree<int, int> tree;
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
        RBTree<int, int> tree;
        Assert::IsTrue(tree.isEmpty());

        tree.insert(3, 3);
        Assert::IsFalse(tree.isEmpty());
    }

    TEST_METHOD(TestRemove)
    {
        RBTree<int, int> tree;
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
        RBTree<int, int> tree;
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
        RBTree<int, int> tree;
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

    TEST_METHOD(TestRootIsBlack)
    {
        RBTree<int, int> tree;
        tree.insert(10, 10);
        tree.insert(20, 20);
        tree.insert(30, 30);

        Assert::IsNotNull(tree.root(), L"Root should not be null.");
        Assert::IsTrue(tree.root()->color == NodeColor::BLACK, L"Root must be black.");
    }

    TEST_METHOD(TestNoRedRedViolations)
    {
        RBTree<int, int> tree;
        tree.insert(10, 10);
        tree.insert(5, 5);
        tree.insert(15, 15);
        tree.insert(1, 1);
        tree.insert(7, 7);
        tree.insert(12, 12);
        tree.insert(20, 20);

        std::function<void(decltype(tree)::PNode)> validate = [&](auto node)
        {
            if (node == nullptr) {
                return;
            }

            if (node->color == NodeColor::RED) {
                Assert::IsTrue((node->left == nullptr || node->left->color == NodeColor::BLACK) &&
                               (node->right == nullptr || node->right->color == NodeColor::BLACK) &&
                               (node->parent == nullptr || node->parent->color == NodeColor::BLACK),
                               L"Red node must not have a red parent.");
            }

            validate(node->left);
            validate(node->right);
        };

        validate(tree.root());
    }

    TEST_METHOD(TestBlackHeightConsistency)
    {
        RBTree<int, int> tree;
        tree.insert(10, 10);
        tree.insert(5, 5);
        tree.insert(15, 15);
        tree.insert(1, 1);
        tree.insert(7, 7);
        tree.insert(12, 12);
        tree.insert(20, 20);

        std::function<int(decltype(tree)::PNode)> validateBlackHeight = [&](auto node) -> int
        {
            if (node == nullptr) return 1; // Null nodes count as black

            int leftBlackHeight = validateBlackHeight(node->left);
            int rightBlackHeight = validateBlackHeight(node->right);

            // All paths must have the same black height
            Assert::AreEqual(leftBlackHeight, rightBlackHeight, L"Black height must be consistent across all paths.");

            return leftBlackHeight + (node->color == NodeColor::BLACK ? 1 : 0);
        };

        validateBlackHeight(tree.root());
    }

    TEST_METHOD(TestStringTree)
    {
        RBTree<std::string, std::string> tree;
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
