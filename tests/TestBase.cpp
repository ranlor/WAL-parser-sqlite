#include "TestBase.h"
#include <iostream>
#include <iomanip>

void divider(int len = 100, char c = '-')
{
    std::cout << std::setw(len) << std::setfill(c) << "" << std::endl;
}

int main(int argc, char** argv)
{
    static const std::string prefix{"[~]"};
    int totalSuiteFails = 0;
    int totalTestFails = 0;
    for (const  auto& e : SuiteCollection::instance().suites )
    {
        std::string suite = e.first;
        int failedTests = 0;
        int testsCount = e.second.size();
        for (const auto& test : e.second )
        {
            std::string fullname = suite+"."+test->getName();
            std::cout << prefix << "Runing [" << fullname << "]" << std::endl;
            try 
            {
                test->runTest();
                std::cout << prefix <<  "  " << (test->testResult() ? "!Passed!" : "*Failed*") << std::endl;
                if (!test->testResult()) { failedTests++; }
            }
            catch (std::exception& e)
            {
                std::cout << prefix << "  " << "Failed. Exception thrown in test: " << e.what() << std::endl;
                failedTests++;
            }
        }
        divider();
        std::cout << prefix << " " << suite << 
            " Failed " << failedTests << "/" << testsCount <<
            " Passed " << (testsCount-failedTests) << "/" << testsCount <<
        std::endl;
        divider();
        totalTestFails += failedTests;
        if (failedTests > 0) { totalSuiteFails++;}
    }
    divider(100,'=');
    std::cout << prefix << 
        " Failed " << totalTestFails << " tests in " << totalSuiteFails << " Suites" <<
    std::endl;
    divider(100,'=');
    return 0;
}