#include "TestBase.h"
#include "Utils/FixedRuntimeArray.h"

TEST(FixedRuntimeArray, sanityTest1)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};
    std::array<int,5> ref = {1,2,3,4,5};

    ASSERT_TRUE( array.size() == ref.size(), "size mismatch between FixedRuntimeArray and expected array");

    auto j = ref.begin();
    for (auto i=array.begin();  i!=array.end(); ++i)
    {
        ASSERT_TRUE( *j == *i , "failed value match between FixedRuntimeArray and expected array");
        ++j;
    }
}


TEST(FixedRuntimeArray, sanityTest2)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};

    std::array<int,5> ref = {1,2,3,4,5};

    ASSERT_TRUE( array.size() == ref.size(), "size mismatch between FixedRuntimeArray and expected array");

    auto j = ref.begin();
    for (auto a : array)
    {
        ASSERT_TRUE( *j == a, "failed value match between FixedRuntimeArray and expected array");
        ++j;
    }
}

TEST(FixedRuntimeArray, sanityTest3)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};

    std::array<int,5> ref = {1,2,3,4,5};

    ASSERT_TRUE( array.size() == ref.size(), "size mismatch between FixedRuntimeArray and expected array");

    auto j = ref.begin();
    for (const auto& a : array)
    {
        ASSERT_TRUE( *j == a, "failed value match between FixedRuntimeArray and expected array");
        ++j;
    }
}

TEST(FixedRuntimeArray, constIterator)
{
    const wal::FixedRuntimeArray array = {1,2,3,4,5};

    std::array<int,5> ref = {1,2,3,4,5};

    ASSERT_TRUE( array.size() == ref.size(), "size mismatch between FixedRuntimeArray and expected array");

    auto j = ref.begin();
    for (const auto& a : array)
    {
        ASSERT_TRUE( *j == a, "failed value match between FixedRuntimeArray and expected array");
        ++j;
    }
}


TEST(FixedRuntimeArray, copyToArray)
{
    constexpr size_t size = 5;
    wal::FixedRuntimeArray array1 = {1,2,3,4,5};
    wal::FixedRuntimeArray array2(size,0);

    auto aIt = array1.begin();
    std::copy_n(aIt, size, array2.begin());

    std::array<int,size> ref = {1,2,3,4,5};

    auto j = ref.begin();
    for (auto a : array1)
    {
        ASSERT_TRUE( *j == a, "failed value match on original array");
        ++j;
    }

    j = ref.begin();
    for (auto a : array2)
    {
        ASSERT_TRUE( *j == a, "failed value match on copied array");
        ++j;
    }
}

TEST(FixedRuntimeArray, exceedArraySizeTest)
{
    constexpr size_t size = 2;
    constexpr int dval = 123;
    wal::FixedRuntimeArray array(size,dval);

    try
    {
        auto a = array.begin();
        for (int i=0; i<size*2; ++i)
        {
            int val = *a;
            ++a;
            ASSERT_TRUE( val == dval, "value is mismatch");
        }    
    }
    catch(const std::out_of_range& e)
    {
        return;
    }

    FAILURE( "didn't get expecetd out of range exception" );
}


TEST(FixedRuntimeArray, copyCtorTest)
{
    constexpr size_t size = 5;
    wal::FixedRuntimeArray array1 = {1,2,3,4,5};
    wal::FixedRuntimeArray copy(array1);

    std::array<int,size> ref = {1,2,3,4,5};

    auto j = ref.begin();
    for (auto a : array1)
    {
        ASSERT_TRUE( *j == a, "failed value match on original array");
        ++j;
    }

    j = ref.begin();
    for (auto a : copy)
    {
        ASSERT_TRUE( *j == a, "failed value match on copied array");
        ++j;
    }
}


TEST(FixedRuntimeArray, moveCtorTest)
{
    constexpr size_t size = 5;
    wal::FixedRuntimeArray array1 = {1,2,3,4,5};
    wal::FixedRuntimeArray copy(std::move(array1));

    std::array<int,size> ref = {1,2,3,4,5};

    auto j = ref.begin();
    for (auto a : array1)
    {
        ASSERT_TRUE( *j == a, "failed value match on original array");
        ++j;
    }

    j = ref.begin();
    for (auto a : copy)
    {
        ASSERT_TRUE( *j == a, "failed value match on copied array");
        ++j;
    }
}


TEST(FixedRuntimeArray, randomAccessOperator)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};

    auto it = array.begin();
    for (int i=0;i<array.size();++i)
    {
        ASSERT_TRUE(it[i] == array[i], "value missmatch with operator[]");
    }
    ASSERT_TRUE(it == array.begin(), "using operator[] changed the iterator");
}

TEST(FixedRuntimeArray, additionToIteratorOperator)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};

    auto it = array.begin();
    it+=1;
    ASSERT_TRUE(*it == array[1], "mismatch between operator+= and value at array offset 1");
    it+=2;
    ASSERT_TRUE(*it == array[3], "mismatch between operator+= and value at array offset 3");
    it+=1;
    ASSERT_TRUE(*it == array[4], "mismatch between operator+= and value at array offset 4");
}

TEST(FixedRuntimeArray, additionOperator)
{
    wal::FixedRuntimeArray array = {1,2,3,4,5};

    auto it = array.begin();
    ASSERT_TRUE(*(it+1) == array[1], "mismatch between operator+= and value at array offset 1");
    ASSERT_TRUE(*(it+3) == array[3], "mismatch between operator+= and value at array offset 3");
    ASSERT_TRUE(*(it+4) == array[4], "mismatch between operator+= and value at array offset 4");
    ASSERT_TRUE(it == array.begin(), "using operator+ changed the iterator");
}