#include "pch.h"

#include <filesystem>
#include <CppUnitTest.h>
#include "Localization.h"

namespace fs = std::filesystem;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(LocaTests)
{
private:
    LocaResource m_resource;

public:
    TEST_METHOD_INITIALIZE(Setup)
    {
        fs::path exePath = fs::current_path();
        fs::path locaPath = exePath / "loca.xml";

        m_resource = LocaXmlReader::Read(locaPath.string());
        Assert::IsTrue(m_resource.entries.size() >= 24, L"Failed to load localization XML");
    }

    TEST_METHOD(TestFirstEntry)
    {
        Assert::AreEqual(std::string("h5eec0cb422f845c8a1abe70393d59838"), m_resource.entries[0].key);

    }

    TEST_METHOD(TestWriteLocalization)
    {
        fs::path exePath = fs::current_path();
        
        fs::path outPath = exePath / "loca.loca";

        LocaWriter::Write(outPath.string(), m_resource);
    }
};

