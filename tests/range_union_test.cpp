#define BOOST_TEST_MODULE RANGE_UNION_TEST_MODULE
#include <boost/test/included/unit_test.hpp>
#include <numeric>
#include <queue>

#include <boost/range/iterator_range.hpp>

#include <boost/mpl/list.hpp>

#include "../range_union.hpp"

using namespace tyti;

class if_not_pod
{
public:
    template <typename T>
    static auto deref(const T &i)
    {
        if constexpr (std::is_pod_v<T>)
            return i;
        else
            return *i;
    }
    template <typename T, typename ConT>
    static auto get_begin(ConT &cont)
    {
        if constexpr (std::is_pod_v<T>)
            return static_cast<typename ConT::value_type>(0);
        else
            return cont.begin();
    }
};

template <typename R>
void check_values(const range_union<R> &iu, const std::vector<int> &vals)
{
    // c++03
    std::vector<int>::const_iterator checkIterC03 = vals.begin();
    typename range_union<R>::iterator endIter = iu.end();

    // check increment
    for (typename range_union<R>::iterator iter = iu.begin(); iter != endIter; ++iter)
        BOOST_REQUIRE_EQUAL(if_not_pod::deref(*iter), *(checkIterC03++));

    BOOST_REQUIRE_MESSAGE(checkIterC03 == vals.end(),
                          std::string("Not enough values in interval. Has to be more.\nCount of missing values: ") + std::to_string(std::distance(checkIterC03, vals.cend())));

    // check decrement
    // --checkIterC03;
    // for (typename range_union<R>::iterator iter = --iu.end(); iter != endIter; --iter)
    //     BOOST_REQUIRE_EQUAL(if_not_pod::deref(*iter), *(checkIterC03--));

    // c++11
    auto checkIterC11 = std::cbegin(vals);
    for (const auto &value : iu)
    {
        BOOST_REQUIRE_EQUAL(if_not_pod::deref(value), *(checkIterC11++));
    }
}
template <class rangeT>
void check_std_operators()
{
    typedef typename rangeT::iterator iterator_class;
    typedef rangeT range;
    range_union<range> iu;
    BOOST_REQUIRE(iu.empty());

    std::vector<int> numbers(25);
    std::iota(numbers.begin(), numbers.end(), 0);
    auto begin = if_not_pod::get_begin<iterator_class, decltype(numbers)>(numbers);

    std::vector<int> compare_container;

    iu += range(begin + 5, begin + 10);
    BOOST_REQUIRE(!iu.empty());
    compare_container = {5, 6, 7, 8, 9};
    check_values(iu, compare_container);

    iu += range(begin + 15, begin + 20);
    BOOST_REQUIRE(iu.size() == 2);
    compare_container = {5, 6, 7, 8, 9, 15, 16, 17, 18, 19};
    check_values(iu, compare_container);

    iu += range(begin + 14, begin + 16);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("expand front failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19};
    check_values(iu, compare_container);

    range_union<range> iu2;
    iu2 += iu;
    check_values(iu2, compare_container);
    iu2 -= iu;
    BOOST_REQUIRE(iu2.empty());
    iu2 += iu;

    iu += range(begin + 9, begin + 11);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19};
    check_values(iu, compare_container);

    iu += range(begin + 23, begin + 25);
    BOOST_REQUIRE_MESSAGE(iu.size() == 3, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 23, 24};
    check_values(iu, compare_container);

    iu += range(begin + 21, begin + 22);
    BOOST_REQUIRE_MESSAGE(iu.size() == 4, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 21, 23, 24};
    check_values(iu, compare_container);

    iu += range(begin + 6, begin + 23);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("union failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    check_values(iu, compare_container);

    iu += range(begin + 10, begin + 20);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("expand middle failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    check_values(iu, compare_container);

    iu2 += iu;
    check_values(iu2, compare_container);

    iu -= range(begin + 10, begin + 15);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("erase middle failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    check_values(iu, compare_container);

    iu -= range(begin + 10, begin + 25);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("erase end failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = {5, 6, 7, 8, 9};
    check_values(iu, compare_container);

    iu -= range(begin, begin + 6);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("erase end failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = {6, 7, 8, 9};
    check_values(iu, compare_container);
}

typedef boost::mpl::list<
    pair_range<int>,
    pair_range<char>,
    pair_range<unsigned int>,
    // pair_range<float>, //creates tons of warning, you should fix that
    pair_range<std::vector<int>::iterator>,
    boost::iterator_range<std::vector<int>::iterator>>
    test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(range_union_operators, T, test_types)
{
    check_std_operators<T>();
}

BOOST_AUTO_TEST_CASE(Disjunct)
{
    using range = pair_range<int>;
    range_union<range> iu;
    BOOST_REQUIRE(iu.empty());
    iu += range(0, 17);
    iu += range(20, 25);

    BOOST_CHECK(iu.disjunct(range(18, 20)));

    BOOST_CHECK(!iu.disjunct(range(17, 20)));
    BOOST_CHECK(!iu.disjunct(range(18, 21)));
}

BOOST_AUTO_TEST_CASE(IteratorTest)
{
    using range = pair_range<int>;
    range_union<range> iu;
    BOOST_REQUIRE(iu.empty());
    iu += range(0, 17);
    iu += range(20, 25);

    auto it = iu.begin();
    auto it2 = it;
    while (it != iu.end() || it2 != iu.end())
    {
        BOOST_REQUIRE_EQUAL(*it, *it2);
        ++it;
        it2++;
    }
    BOOST_REQUIRE(it == it2); // equals numbers.end()

    --it;
    it2--;
    BOOST_REQUIRE(*it == *it2);
    while (it != iu.end() || it2 != iu.end())
    {
        BOOST_REQUIRE_EQUAL(*it, *it2);
        ++it;
        it2++;
    }
    BOOST_REQUIRE(it == it2); // equals numbers.end()
}