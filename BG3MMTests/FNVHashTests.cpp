#include "pch.h"

#include <CppUnitTest.h>
#include <cstdint>
#include <span>
#include <string_view>
#include <string>

#include "FNVHash.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace fnvhash;

TEST_CLASS(FNVHashTests)
{
public:
    TEST_METHOD(TestKnownValues)
    {
        Assert::AreEqual(hash(""), 0x811C9DC5u);
        Assert::AreEqual(hash("a"), 0xE40C292Cu);
        Assert::AreEqual(hash("foobar"), 0xBF9CF968u);
        Assert::AreEqual(hash("hello"), 0x4F9F2CABu);
        Assert::AreEqual(hash("abcdefghijklmnopqrstuvwxyz"), 0xB0BC0C82);
    }

    TEST_METHOD(TestUInt32)
    {
        uint32_t value = 0xDEADBEEF;
        uint32_t expected = fnv1a_hash(std::as_bytes(std::span(&value, 1)));
        Assert::AreEqual(hash(value), expected);
    }

    TEST_METHOD(TestString)
    {
        std::string str = "Hello, World!";
        uint32_t expected = fnv1a_hash(std::as_bytes(std::span(str.data(), str.size())));
        Assert::AreEqual(hash(str), expected);
    }

    TEST_METHOD(TestStringView)
    {
        std::string_view str = "Hello, World!";
        uint32_t expected = fnv1a_hash(std::as_bytes(std::span(str.data(), str.size())));
        Assert::AreEqual(hash(str), expected);
    }

    TEST_METHOD(TestCharArray)
    {
        const auto * str = "Hello, World!";
        uint32_t expected = fnv1a_hash(std::as_bytes(std::span(str, strlen(str))));
        Assert::AreEqual(hash(str), expected);
    }

    TEST_METHOD(TestTriviallyCopyable)
    {
        struct Trivial {
            int a;
            float b;
        };

        Trivial value{ .a= 42, .b= 3.14f };
        auto expected = fnv1a_hash(std::as_bytes(std::span(&value, 1)));
        Assert::AreEqual(hash(value), expected);
    }

    TEST_METHOD(TestHashCollisionAvoidance)
    {
        Assert::AreNotEqual(hash("foobar"), hash("foobas"));
    }

    TEST_METHOD(TestHashConsistency)
    {
        std::string str = "Hello, World!";
        uint32_t hash1 = hash(str);
        uint32_t hash2 = hash(str);
        Assert::AreEqual(hash1, hash2);
    }
};
