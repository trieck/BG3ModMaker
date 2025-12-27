#include "pch.h"
#include "UtilityBase.h"
#include "BTree.h"

#include <CppUnitTest.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(BTreeTests)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
    }

    TEST_METHOD(Insert)
    {
        BTree<int, int> tree(5);
        tree.insert(1, 1);
        tree.insert(2, 2);
        tree.insert(3, 3);
        tree.insert(4, 4);
        tree.insert(5, 5);
        tree.insert(6, 6);

        auto it = tree.find(1);
        Assert::AreEqual(1, it.value());

        it = tree.find(2);
        Assert::AreEqual(2, it.value());

        it = tree.find(3);
        Assert::AreEqual(3, it.value());

        it = tree.find(4);
        Assert::AreEqual(4, it.value());

        it = tree.find(5);
        Assert::AreEqual(5, it.value());

        it = tree.find(6);
        Assert::AreEqual(6, it.value());

        it = tree.find(7);
        Assert::IsTrue(it == tree.end());
    }

    TEST_METHOD(StringTree)
    {
        BTree<std::string, std::string> tree(5);
        tree.insert("one", "uno");
        tree.insert("two", "dos");
        tree.insert("three", "tres");
        tree.insert("four", "cuatro");
        tree.insert("five", "cinco");
        tree.insert("six", "seis");

        auto it = tree.find("one");
        Assert::AreEqual(std::string("uno"), it.value());

        it = tree.find("two");
        Assert::AreEqual(std::string("dos"), it.value());

        it = tree.find("three");
        Assert::AreEqual(std::string("tres"), it.value());

        it = tree.find("four");
        Assert::AreEqual(std::string("cuatro"), it.value());

        it = tree.find("five");
        Assert::AreEqual(std::string("cinco"), it.value());

        it = tree.find("six");
        Assert::AreEqual(std::string("seis"), it.value());
    }

    TEST_METHOD(Iterate)
    {
        BTree<std::string, std::string> tree(3);
        tree.insert("one", "uno");
        tree.insert("two", "dos");
        tree.insert("three", "tres");
        tree.insert("four", "cuatro");
        tree.insert("five", "cinco");
        tree.insert("six", "seis");
        tree.insert("seven", "siete");
        tree.insert("eight", "ocho");
        tree.insert("nine", "nueve");
        tree.insert("ten", "diez");

        std::optional<std::string> prev;

        for (auto it = tree.begin(); it != tree.end(); ++it) {
            auto m = std::format("{} => {}\n", it.key(), it.value());
            Logger::WriteMessage(m.c_str());
            if (prev) {
                Assert::IsTrue(*prev < it.key());
            }
            prev = it.key();
        }
    }
};
