#include "pch.h"
#include "UtilityBase.h"
#include "GR2Reader.h"

#include <CppUnitTest.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(GR2Tests)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
    }

    TEST_METHOD(Read)
    {
        GR2Reader reader;
        try {
            reader.read("D:\\tmp\\EF_GobboC.GR2");
            reader.traverse();
        } catch (const std::exception& ex) {
            CString what(ex.what());
            Assert::Fail(what);
        }
    }
};
