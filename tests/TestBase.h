/////////////////////////////////////////////////////////////////////////////////
// a poor man version of unit tests that mimics the style of gtest
// with much simpler logic and slimmed down assert implementation
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "Utils/Log.h"
#include "TestUtils.h"

////////////////////// Define macros ///////////////////////////////////////////

#define CLS_NAME(suite,name) T_##suite_##name

#define TEST(suite,name) class CLS_NAME(suite,name) : public BaseTest \
{\
    public:\
        CLS_NAME(suite,name)() = default;\
        ~CLS_NAME(suite,name)() = default;\
        std::string getSuite() const override {return #suite;}\
        std::string getName() const override {return #name;}\
        void runTest() override {\
            try { body(); }\
            catch (TestException& e)\
            {\
                _result = false;\
                std::cout << e.what() << std::endl;\
            }\
        }\
    private:\
        static BaseTest* ref;\
        void body();\
};\
\
BaseTest* CLS_NAME(suite,name)::ref = registerThis(new CLS_NAME(suite,name)());\
\
void CLS_NAME(suite,name)::body()

#define FAILURE(msg) throw TestException(std::string("\t") + msg)

#define ASSERT_TRUE(exp,failureMessage) if (!(exp)) FAILURE(std::string(failureMessage))

#define ASSERT_EQ(X,Y) ASSERT_TRUE( X == Y, std::string("ASSERT_EQ Failure: Expected '") + TestUtils::toString(X) + "' and '" + TestUtils::toString(Y) + "' to be equal")
////////////////////// Base classes //////////////////////////////////////////////

struct BaseTest
{
    public:
        BaseTest() = default;
        virtual ~BaseTest() = default;
        virtual std::string getSuite() const = 0;
        virtual std::string getName() const = 0;
        virtual void runTest() = 0;
        bool testResult() const { return _result; }
    protected:
        bool _result = true;
};

struct SuiteCollection {
    public:
        using Suites = std::unordered_map<std::string, std::vector< std::unique_ptr<BaseTest> > >;
        static SuiteCollection& instance()
        {
            static SuiteCollection instance;
            return instance;
        }

        static inline Suites suites{};
};

inline BaseTest* registerThis(BaseTest* cls)
{
    SuiteCollection::instance().suites[cls->getSuite()].emplace_back(cls);
    return cls; //return a reference to the pointer that now is owned by suites map
}

class TestException: public std::runtime_error
{
    public:
        explicit TestException(std::string_view message):std::runtime_error(message.data()) {}
        virtual ~TestException() noexcept = default;
};
