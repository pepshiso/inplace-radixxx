#include "inplace_radixxx.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <algorithm>
#include <deque>
#include <string>
#include <utility>
#include <vector>

template <typename Iterator>
bool is_sorted_(Iterator first, Iterator last)
{
    if (first == last)
        return true;
    Iterator next = first;
    ++next;
    while (next != last) {
        if (*first++ > *next++)
            return false;
    }
    return true;
}

template <typename Iterator, typename Functor>
bool is_sorted_(Iterator first, Iterator last, Functor get_key)
{
    if (first == last)
        return true;
    Iterator next = first;
    ++next;
    while (next != last)
        if (get_key(*first++) > get_key(*next++))
            return false;
    return true;
}

struct get_first {
    template <typename>
    struct result;

    template <typename Functor, typename First, typename Second>
    struct result<Functor (std::pair<First, Second>)> {
        typedef First type;
    };

    template <typename First, typename Second>
    First const& operator()(std::pair<First, Second> const& x) const {
        return x.first;
    }
};

struct get_second {
    template <typename>
    struct result;

    template <typename Functor, typename First, typename Second>
    struct result<Functor (std::pair<First, Second>)> {
        typedef Second type;
    };

    template <typename First, typename Second>
    Second const& operator()(std::pair<First, Second> const& x) const {
        return x.second;
    }
};

TEST(BoolTest, BoolTest)
{
    int n = 0;
    std::deque<bool> d;
    for (int i = 0; i < 7; ++i) {
        d.resize(n);
        for (int j = 0; j < n; ++j)
            d[j] = rand() % 2;
        inplace_radixxx::sort(d.begin(), d.end());
        EXPECT_TRUE(is_sorted_(d.begin(), d.end()));
        n = 10*n + 1;
    }
}

template <typename T>
struct BoolPairFirstTest : ::testing::Test {};

typedef ::testing::Types<std::vector<std::pair<bool, long> >,
                         std::deque<std::pair<bool, double> > >
    BoolPairFirstContainers;
TYPED_TEST_CASE(BoolPairFirstTest, BoolPairFirstContainers);

TYPED_TEST(BoolPairFirstTest, BoolPairFirstTest)
{
    typedef TypeParam Container;
    typedef typename Container::value_type ValueType;

    int n = 0;
    Container c;
    for (int i = 0; i < 7; ++i) {
        c.resize(n);
        for (int j = 0; j < n; ++j)
            c[j].first = rand() % 2;
        inplace_radixxx::sort(c.begin(), c.end(), get_first());
        EXPECT_TRUE(is_sorted_(c.begin(), c.end(), get_first()));
        n = 10*n + 1;
    }
}

template <typename T>
struct BoolPairSecondTest : ::testing::Test {};

typedef ::testing::Types<std::vector<std::pair<bool, bool> >,
                         std::deque<std::pair<std::string, bool> > >
    BoolPairSecondContainers;
TYPED_TEST_CASE(BoolPairSecondTest, BoolPairSecondContainers);

TYPED_TEST(BoolPairSecondTest, BoolPairSecondTest)
{
    typedef TypeParam Container;
    typedef typename Container::value_type ValueType;

    int n = 0;
    Container c;
    for (int i = 0; i < 7; ++i) {
        c.resize(n);
        for (int j = 0; j < n; ++j)
            c[j].first = rand() % 2;
        inplace_radixxx::sort(c.begin(), c.end(), get_second());
        EXPECT_TRUE(is_sorted_(c.begin(), c.end(), get_second()));
        n = 10*n + 1;
    }
}

template <typename T>
struct ScalarTest : ::testing::Test {};

typedef ::testing::Types<std::vector<unsigned char>, std::deque<unsigned char>,
                         std::vector<signed char>, std::deque<signed char>,
                         std::vector<char>, std::deque<char>,
                         std::vector<unsigned short>, std::deque<unsigned short>,
                         std::vector<short>, std::deque<short>,
                         std::vector<unsigned>, std::deque<unsigned>,
                         std::vector<int>, std::deque<int>,
                         std::vector<unsigned long>, std::deque<unsigned long>,
                         std::vector<float>, std::deque<float>,
                         std::vector<double>, std::deque<double> >
    ScalarTestContainers;
TYPED_TEST_CASE(ScalarTest, ScalarTestContainers);

TYPED_TEST(ScalarTest, ScalarTest)
{
    typedef TypeParam Container;
    typedef typename Container::value_type ValueType;

    int n = 0;
    Container c;
    for (int i = 0; i < 7; ++i) {
        c.resize(n);
        for (int j = 0; j < n; ++j) {
            if (ValueType(-1) < 0) {
                if (rand()%2)
                    c[j] = rand();
                else
                    c[j] = -rand();
            } else
                c[j] = rand();
        }
        inplace_radixxx::sort(c.begin(), c.end());
        EXPECT_TRUE(is_sorted_(c.begin(), c.end()));
        n = 10*n + 1;
    }
}

template <typename T>
struct ScalarPairFirstTest : ::testing::Test {};

typedef ::testing::Types<std::vector<std::pair<unsigned char, std::vector<int> > >,
                         std::deque<std::pair<unsigned char, std::string> >,
                         std::vector<std::pair<signed char, int> >,
                         std::deque<std::pair<signed char, bool> >,
                         std::vector<std::pair<char, char> >,
                         std::deque<std::pair<char, std::vector<std::vector<std::vector<int> > > > >,
                         std::vector<std::pair<unsigned short, int> >,
                         std::deque<std::pair<unsigned short, short> > >
    ScalarPairFirstTestContainers;
TYPED_TEST_CASE(ScalarPairFirstTest, ScalarPairFirstTestContainers);

TYPED_TEST(ScalarPairFirstTest, ScalarPairFirstTest)
{
    typedef TypeParam Container;
    typedef typename Container::value_type ValueType;
    typedef typename ValueType::first_type FirstType;

    int n = 0;
    Container c;
    for (int i = 0; i < 7; ++i) {
        c.resize(n);
        for (int j = 0; j < n; ++j) {
            if (FirstType(-1) < 0) {
                if (rand()%2)
                    c[j].first = rand();
                else
                    c[j].first = -rand();
            } else
                c[j].first = rand();
        }
        inplace_radixxx::sort(c.begin(), c.end(), get_first());
        EXPECT_TRUE(is_sorted_(c.begin(), c.end(), get_first()));
        n = 10*n + 1;
    }
}

template <typename T>
struct remove_pointer;

template <typename T>
struct remove_pointer<T*> {
    typedef T type;
};

struct dereference {
    template <typename>
    struct result;

    template <typename Functor, typename T>
    struct result<Functor (T*)> {
        typedef T type;
    };

    template <typename T>
    T& operator()(T* p) const {
        return *p;
    }
};

template <typename T>
struct DereferencePointerToSignedTest : ::testing::Test {};

typedef ::testing::Types<std::vector<int*>, std::deque<long*> >
    DereferencePointerToSignedTestContainers;
TYPED_TEST_CASE(DereferencePointerToSignedTest, DereferencePointerToSignedTestContainers);

TYPED_TEST(DereferencePointerToSignedTest, DereferencePointerToSignedTest)
{
    typedef TypeParam PointerContainer;
    typedef typename PointerContainer::value_type Pointer;
    typedef typename remove_pointer<Pointer>::type ValueType;
    typedef std::vector<ValueType> ValueContainer;

    int n = 0;
    ValueContainer vc;
    PointerContainer pc;
    for (int i = 0; i < 7; ++i) {
        vc.resize(n), pc.resize(n);
        for (int j = 0; j < n; ++j) {
            if (rand()%2)
                vc[j] = rand();
            else
                vc[j] = -rand();
            pc[j] = &vc[j];
        }
        inplace_radixxx::sort(pc.begin(), pc.end(), dereference());
        EXPECT_TRUE(is_sorted_(pc.begin(), pc.end(), dereference()));
        n = 10*n + 1;
    }
}

struct my_pair {
    int first;
    unsigned second;

    int first_() const { return first; }
    unsigned second_() const { return second; }
};

struct get_my_pair_first {
    typedef int result_type;

    result_type operator()(my_pair const& p) const {
        return p.first;
    }
    result_type operator()(my_pair const* p) const {
        return p->first;
    }
};

struct get_my_pair_second {
    typedef unsigned result_type;

    result_type operator()(my_pair const& p) const {
        return p.second;
    }
    result_type operator()(my_pair const* p) const {
        return p->second;
    }
};

TEST(MemberPointerTest, Function)
{
    int n = 0;
    std::vector<my_pair> v;
    std::vector<my_pair*> vp;
    for (int i = 0; i < 7; ++i) {
        v.resize(n);
        vp.resize(n);
        for (int j = 0; j < n; ++j)
            if (rand()%2)
                v[j].first = rand();
            else
                v[j].first = -rand();
        inplace_radixxx::sort(v.begin(), v.end(), &my_pair::first_);
        EXPECT_TRUE(is_sorted_(v.begin(), v.end(), get_my_pair_first()));

        for (int j = 0; j < n; ++j) {
            if (rand()%2)
                v[j].first = rand();
            else
                v[j].first = -rand();
            vp[j] = &v[j];
        }
        inplace_radixxx::sort(vp.begin(), vp.end(), &my_pair::first_);
        EXPECT_TRUE(is_sorted_(vp.begin(), vp.end(), get_my_pair_first()));

        for (int j = 0; j < n; ++j)
            v[j].second = rand();
        inplace_radixxx::sort(v.begin(), v.end(), &my_pair::second_);
        EXPECT_TRUE(is_sorted_(v.begin(), v.end(), get_my_pair_second()));

        inplace_radixxx::sort(vp.begin(), vp.end(), &my_pair::second_);
        EXPECT_TRUE(is_sorted_(vp.begin(), vp.end(), get_my_pair_second()));

        n = 10*n + 1;
    }
}

TEST(MemberPointerTest, Object)
{
    int n = 0;
    std::vector<my_pair> v;
    std::vector<my_pair*> vp;
    for (int i = 0; i < 7; ++i) {
        v.resize(n);
        vp.resize(n);
        for (int j = 0; j < n; ++j)
            if (rand()%2)
                v[j].first = rand();
            else
                v[j].first = -rand();
        inplace_radixxx::sort(v.begin(), v.end(), &my_pair::first);
        EXPECT_TRUE(is_sorted_(v.begin(), v.end(), get_my_pair_first()));

        for (int j = 0; j < n; ++j) {
            if (rand()%2)
                v[j].first = rand();
            else
                v[j].first = -rand();
            vp[j] = &v[j];
        }
        inplace_radixxx::sort(vp.begin(), vp.end(), &my_pair::first);
        EXPECT_TRUE(is_sorted_(vp.begin(), vp.end(), get_my_pair_first()));

        for (int j = 0; j < n; ++j)
            v[j].second = rand();
        inplace_radixxx::sort(v.begin(), v.end(), &my_pair::second);
        EXPECT_TRUE(is_sorted_(v.begin(), v.end(), get_my_pair_second()));

        inplace_radixxx::sort(vp.begin(), vp.end(), &my_pair::second);
        EXPECT_TRUE(is_sorted_(vp.begin(), vp.end(), get_my_pair_second()));

        n = 10*n + 1;
    }
}

TEST(ReverseSortTest, NoFunctor)
{
    std::vector<int> v(1024 * 1024);
    for (std::size_t i = 0; i < v.size(); ++i)
        if (rand()%2)
            v[i] = rand();
        else
            v[i] = -rand();
    inplace_radixxx::rsort(v.rbegin(), v.rend());
    is_sorted_(v.begin(), v.end());
}

TEST(ReverseSortTest, WithFunctor)
{
    std::vector<std::pair<int, int> > v(1024 * 1024);
    for (std::size_t i = 0; i < v.size(); ++i)
        if (rand()%2)
            v[i].second = rand();
        else
            v[i].second = -rand();
    inplace_radixxx::rsort(v.rbegin(), v.rend(), &std::pair<int, int>::second);
    is_sorted_(v.begin(), v.end(), get_second());
}
// get_second
