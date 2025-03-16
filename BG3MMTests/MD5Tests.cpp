#include "pch.h"
#include <CppUnitTest.h>
#include "MD5.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(MD5Tests)
{
public:
    TEST_METHOD(TestMD5Basic)
    {
        MD5 md5;
        std::string hash = md5.digestString("Hello, World");
        Assert::AreEqual(std::string("82bb413746aee42f89dea2b59614f9ef"), hash);
    }

    TEST_METHOD(TestMD5EmptyString)
    {
        MD5 md5;
        std::string hash = md5.digestString("");
        Assert::AreEqual(std::string("d41d8cd98f00b204e9800998ecf8427e"), hash);
    }

    TEST_METHOD(TestMD5Numbers)
    {
        MD5 md5;
        std::string hash = md5.digestString("1234567890");
        Assert::AreEqual(std::string("e807f1fcf82d132f9bb018ca6738a19f"), hash);
    }

    TEST_METHOD(TestMD5Consistency)
    {
        MD5 md5;
        std::string input = "ConsistencyCheck";
        std::string hash1 = md5.digestString(input);
        std::string hash2 = md5.digestString(input);
        Assert::AreEqual(hash1, hash2);
    }
};
