#include "pch.h"
#include <CppUnitTest.h>
#include <functional>
#include <iostream>

#include "Rope.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(RopeTests)
{
public:
    TEST_METHOD(TestBasicInsertion)
    {
        Rope rope;
        rope.insert(0, "Hello, World");

        Assert::AreEqual(std::string("Hello, World"), rope.str());
    }

    TEST_METHOD(TestFindCharacter)
    {
        Rope rope;
        rope.insert(0, "Hello, World");

        rope.printTree(std::cout);

        Assert::AreEqual('H', rope.find(0));
        Assert::AreEqual('W', rope.find(7));
        Assert::AreEqual('d', rope.find(11));
        Assert::AreEqual('\0', rope.find(20)); // Out of bounds
    }

    TEST_METHOD(TestSplitting)
    {
        Rope rope;
        rope.insert(0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        Assert::AreEqual(std::string("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), rope.str());
    }

    TEST_METHOD(TestInsertAtDifferentPositions)
    {
        Rope rope;
        rope.insert(0, "World");
        rope.insert(0, "Hello, ");
        rope.insert(7, "Big ");
        Assert::AreEqual(std::string("Hello, Big World"), rope.str());
    }

    TEST_METHOD(TestLargeInsertion)
    {
        Rope rope;
        std::string longText(5000, 'A');
        rope.insert(0, longText);

        Assert::AreEqual(longText, rope.str());
    }
};
