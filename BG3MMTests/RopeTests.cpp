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
        Rope rope(3);
        rope.insert(0, "Hello, World");
        rope.exportDOT(ropeDOT);

        Assert::AreEqual(std::string("Hello, World"), rope.str());
    }

    TEST_METHOD(TestFindCharacter)
    {
        Rope rope(3);
        rope.insert(0, "Hello, World");

        Assert::AreEqual('H', rope.find(0));
        Assert::AreEqual('W', rope.find(7));
        Assert::AreEqual('d', rope.find(11));
        Assert::AreEqual('\0', rope.find(20)); // Out of bounds
    }

    TEST_METHOD(TestSplitting)
    {
        Rope rope(3);
        rope.insert(0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        rope.exportDOT(ropeDOT);

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

    TEST_METHOD(TestInsertEverywhere)
    {
        const std::string base(30, 'A');

        for (auto i = 0u; i <= base.size(); ++i) {
            Rope rope;
            rope.insert(0, base);
            rope.exportDOT(ropeDOT);

            auto expected = base;
            expected.insert(i, "x");

            rope.insert(i, "x");
            rope.exportDOT(ropeDOT);

            auto actual = rope.str();
            Assert::AreEqual(expected, actual, std::wstring(L"Mismatch at offset " + std::to_wstring(i)).c_str());
        }
    }

    TEST_METHOD(TestMultipleInsertsAtSameOffset)
    {
        Rope rope;
        rope.insert(0, "Hello");

        rope.insert(3, "x");
        rope.insert(3, "y");
        rope.insert(3, "z");

        std::string expected = "Helzyxlo";
        std::string actual = rope.str();

        Assert::AreEqual(expected, actual);
    }

    TEST_METHOD(TestInsertAtSplitBoundary)
    {
        Rope rope(3);

        // Fill one leaf to capacity (if MAX_TEXT_SIZE is 3)
        rope.insert(0, "ABC"); // "ABC"

        // Insert at the split boundary
        rope.insert(3, "x"); // Expect: "ABCx"

        std::string expected = "ABCx";
        auto actual = rope.str();

        Assert::AreEqual(expected, actual);
    }

    TEST_METHOD(TestDeleteRange)
    {
        Rope rope;
        rope.insert(0, "Hello, World");
        rope.exportDOT(ropeDOT);

        // Delete "lo..W"
        rope.deleteRange(3, 8);
        rope.exportDOT(ropeDOT);

        Assert::AreEqual(std::string("Helorld"), rope.str());
    }

    TEST_METHOD(TestDeleteFromBeginning)
    {
        Rope rope;
        rope.insert(0, "Hello");
        rope.exportDOT(ropeDOT);

        // Delete "H"
        rope.deleteRange(0, 1);
        rope.exportDOT(ropeDOT);

        Assert::AreEqual(std::string("ello"), rope.str());
    }

    TEST_METHOD(TestDeleteFromEnd)
    {
        Rope rope;
        rope.insert(0, "Hello");
        rope.exportDOT(ropeDOT);

        // Delete "o"
        rope.deleteRange(4, 5);
        rope.exportDOT(ropeDOT);
        Assert::AreEqual(std::string("Hell"), rope.str());
    }

    TEST_METHOD(TestDeleteAcrossMultipleLeaves)
    {
        Rope rope(3);

        // With MAX_TEXT_SIZE = 3, this will force at least 4 leaves
        rope.insert(0, "ABCDEFGHIJK"); // length = 11

        rope.exportDOT(ropeDOT);

        // Delete "CDEFGHI" - spans multiple internal nodes/leaves
        rope.deleteRange(2, 9);

        rope.exportDOT(ropeDOT);

        std::string expected = "ABJK";
        std::string actual = rope.str();

        Assert::AreEqual(expected, actual);
    }

    TEST_METHOD(TestDeleteWholeRope)
    {
        Rope rope;
        rope.insert(0, "Complete destruction!");

        rope.exportDOT(ropeDOT);

        rope.deleteRange(0, rope.str().size());

        rope.exportDOT(ropeDOT);

        std::string actual = rope.str();
        Assert::AreEqual(std::string(""), actual);
    }

    TEST_METHOD(TestDeleteMiddleChunk)
    {
        Rope rope;
        rope.insert(0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        rope.exportDOT(ropeDOT);

        rope.deleteRange(6, 17);

        rope.exportDOT(ropeDOT);

        std::string expected = "ABCDEFRSTUVWXYZ";
        std::string actual = rope.str();

        Assert::AreEqual(expected, actual);
    }

    TEST_METHOD(TestDeleteEverywhere)
    {
        const std::string base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        for (size_t i = 0; i < base.size(); ++i) {
            Rope rope;
            rope.insert(0, base);
            rope.exportDOT(ropeDOT);

            std::string expected = base;
            expected.erase(i, 1);

            rope.deleteRange(i, i + 1);
            rope.exportDOT(ropeDOT);

            auto actual = rope.str();
            Assert::AreEqual(expected, actual, std::wstring(L"Mismatch at offset " + std::to_wstring(i)).c_str());
        }
    }

    TEST_METHOD(TestBackToBackDeletes)
    {
        Rope rope;
        rope.insert(0, "ABCDEFGHIJK");

        rope.deleteRange(2, 5); // Remove "CDE" -> "ABFGHIJK"
        rope.deleteRange(2, 4); // Remove "FG" -> "ABHIJK"

        std::string expected = "ABHIJK";
        Assert::AreEqual(expected, rope.str());
    }

    TEST_METHOD(TestDeleteToEmptyLeaf)
    {
        Rope rope(3);
        rope.insert(0, "ABCDEF");

        // If MAX_TEXT_SIZE = 3, "ABC" in one leaf, "DEF" in another
        rope.deleteRange(0, 3); // Delete "ABC"

        Assert::AreEqual(std::string("DEF"), rope.str());
    }

    TEST_METHOD(TestNoOpDelete)
    {
        Rope rope;
        rope.insert(0, "HelloWorld");

        rope.deleteRange(5, 5); // No-op
        Assert::AreEqual(std::string("HelloWorld"), rope.str());
    }

#ifdef _DEBUG
    TEST_METHOD(TestDeleteNoLeaks)
    {
        _CrtMemState before, after, diff;
        _CrtMemCheckpoint(&before);

        {
            Rope rope(3);
            rope.insert(0, "The quick brown fox jumps over the lazy dog.");
            rope.deleteRange(10, 19); // Deletes "brown fox"
        }

        _CrtMemCheckpoint(&after);
        if (_CrtMemDifference(&diff, &before, &after)) {
            _CrtMemDumpStatistics(&diff);
            _CrtDumpMemoryLeaks();
            Assert::Fail(L"Memory leak detected");
        }
    }
#endif

    TEST_METHOD(TestDontDangleOnDelete)
    {
        Rope rope(3);

        std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        rope.insert(0, input);
        rope.exportDOT(ropeDOT);

        for (size_t i = 0; i < input.size(); ++i) {
            rope.deleteRange(0, 1);
            rope.exportDOT(ropeDOT);
        }
    }
};

// Static member variable for the DOT file path
std::string RopeTests::ropeDOT;
