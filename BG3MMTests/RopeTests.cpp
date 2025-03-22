#include "pch.h"
#include <CppUnitTest.h>
#include <filesystem>
#include <functional>

#include "Rope.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace fs = std::filesystem;

TEST_CLASS(RopeTests)
{
    static std::string ropeDOT;
public:

    TEST_CLASS_INITIALIZE(ClassInitialize)
    {
        fs::path exePath = fs::current_path();
        fs::path outPath = exePath / "rope.dot";
        ropeDOT = outPath.string();
    }

    TEST_METHOD_INITIALIZE(MethodInitilize)
    {
        // Clear the output file before each test
        if (fs::exists(ropeDOT)) {
            fs::remove(ropeDOT);
        }
    }

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
        rope.exportDOT(ropeDOT);
        
        Assert::AreEqual(std::string("Hello, Big World"), rope.str());
        Assert::IsTrue(rope.isBalanced());
    }

    TEST_METHOD(TestLargeInsertion)
    {
        Rope rope;
        std::string longText(40, 'A');
        rope.insert(0, longText);
        rope.exportDOT(ropeDOT);

        Assert::AreEqual(longText, rope.str());
    }

    TEST_METHOD(TestBalance)
    {
        Rope rope;
        rope.insert(0, "Big");
        rope.insert(3, "Top");
        rope.insert(6, "Bunny");
        rope.insert(11, "And");
        rope.insert(14, "Bruno");
        rope.exportDOT(ropeDOT);

        Assert::AreEqual(std::string("BigTopBunnyAndBruno"), rope.str());
        Assert::IsTrue(rope.isBalanced());
    }

    TEST_METHOD(TestFixInsert)
    {
        Rope rope;
        rope.insert(0, "Hello ");
        rope.exportDOT(ropeDOT);

        rope.insert(6, "World");
        rope.exportDOT(ropeDOT);

        Assert::IsTrue(rope.isBalanced());
        Assert::AreEqual(std::string("Hello World"), rope.str());
    }

    TEST_METHOD(InsertAtEveryOffset)
    {
        const std::string base = "ABCDEFG";

        for (auto i = 0u; i <= base.size(); ++i) {
            Rope rope;
            rope.insert(0, base);
            rope.exportDOT(ropeDOT);

            // Insert "x" at every possible offset from 0 to base.size()
            rope.insert(i, "x");
            rope.exportDOT(ropeDOT);

            std::string expected = base.substr(0, i) + "x" + base.substr(i);
            std::string actual = rope.str();

            Assert::AreEqual(expected, actual, L"Insert at offset failed");
        }
    }
};

// Static member variable for the DOT file path
std::string RopeTests::ropeDOT;
