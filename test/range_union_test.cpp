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
    template<typename T>
    static typename T::value_type deref(const T& i, const std::false_type& ){ return *i; }
    template<typename T>
    static T deref(const T& i, const std::true_type& ){ return i; }
    
    template<typename ConT>
    static typename ConT::iterator get_begin(ConT& c, std::false_type& ){ return c.begin(); }
    template<typename ConT>
    static typename ConT::value_type get_begin(ConT& c, std::true_type& ){ return static_cast<ConT::value_type>(0); }
public:
    template<typename T>
    static auto deref(const T& i) -> decltype(deref(i,typename std::is_pod<T>::type())) { return deref(i, typename std::is_pod<T>::type()); }
    template<typename T, typename ConT>
    static auto get_begin(ConT& cont) -> decltype(get_begin(cont, typename std::is_pod<T>::type())) { return get_begin(cont, typename std::is_pod<T>::type()); }
};

template<typename R>
void check_values(const range_union< R >& iu, const std::vector<int>& vals)
{
    //c++03
    std::vector<int>::const_iterator checkIterC03 = vals.begin();
    typename range_union< R >::iterator endIter = iu.end();
    for (typename range_union< R >::iterator iter = iu.begin(); iter != endIter; ++iter)
        BOOST_REQUIRE_EQUAL(if_not_pod::deref(*iter), *(checkIterC03++));
    
    BOOST_REQUIRE_MESSAGE(checkIterC03 == vals.end(), 
        std::string("Too less values in interval. Has to be more.\nCount of missing values: ") 
        + std::to_string(std::distance(checkIterC03, vals.cend())));
    
    //c++11
    auto checkIterC11 = std::cbegin(vals);
    for (const auto& value : iu)
    {
        BOOST_REQUIRE_EQUAL(if_not_pod::deref(value), *(checkIterC11++));
    }
        
}
template< class rangeT>
void check_std_operators()
{
    typedef typename  rangeT::iterator iterator_class;
    typedef rangeT range;
    range_union< range > iu;
    BOOST_REQUIRE(iu.empty());

    std::vector<int> numbers(25);
    std::iota(numbers.begin(), numbers.end(), 0);
    auto begin = if_not_pod::get_begin<iterator_class>(numbers);    

    std::vector<int> compare_container;

    iu += range(begin + 5, begin + 10);
    BOOST_REQUIRE(!iu.empty());
    compare_container = { 5, 6, 7, 8, 9 };
    check_values(iu, compare_container);

    iu += range(begin + 15, begin + 20);
    BOOST_REQUIRE(iu.size() == 2);
    compare_container = { 5, 6, 7, 8, 9, 15, 16, 17, 18, 19 };
    check_values(iu, compare_container);

    iu += range(begin + 14, begin + 16);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("expand front failed. actual size is : ") + std::to_string(iu.size()) );
    compare_container = { 5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19 };
    check_values(iu, compare_container);

    range_union< range > iu2;
    iu2 += iu;
    check_values(iu2, compare_container);
    iu2 -= iu;
    BOOST_REQUIRE(iu2.empty());
    iu2 += iu;

    iu += range(begin + 9, begin + 11);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()) );
    compare_container = { 5, 6, 7, 8, 9, 10 , 14, 15, 16, 17, 18, 19 };
    check_values(iu, compare_container);

    iu += range(begin + 23, begin + 25);
    BOOST_REQUIRE_MESSAGE(iu.size() == 3, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = { 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 23, 24 };
    check_values(iu, compare_container);

    iu += range(begin + 21, begin + 22);
    BOOST_REQUIRE_MESSAGE(iu.size() == 4, std::string("expand end failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = { 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 21, 23, 24 };
    check_values(iu, compare_container);

    iu += range(begin + 6, begin + 23);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("union failed. actual size is: ") + std::to_string(iu.size()));
    compare_container = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
    check_values(iu, compare_container);

    iu += range(begin + 10, begin + 20);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("expand middle failed. actual size is : ") + std::to_string(iu.size()) );
    compare_container = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
    check_values(iu, compare_container);

    iu2 += iu;
    check_values(iu2, compare_container);

    iu -= range(begin + 10, begin + 15);
    BOOST_REQUIRE_MESSAGE(iu.size() == 2, std::string("erase middle failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = { 5, 6, 7, 8, 9, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
    check_values(iu, compare_container);

    iu -= range(begin + 10, begin + 25);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("erase end failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = { 5, 6, 7, 8, 9 };
    check_values(iu, compare_container);

    iu -= range(begin , begin + 6);
    BOOST_REQUIRE_MESSAGE(iu.size() == 1, std::string("erase end failed. actual size is : ") + std::to_string(iu.size()));
    compare_container = { 6, 7, 8, 9 };
    check_values(iu, compare_container);
}

typedef boost::mpl::list<
    pair_range<int>,
    pair_range<char>,
    pair_range<unsigned int>,
    //pair_range<float>,
    pair_range<std::vector<int>::iterator>,
    boost::iterator_range<std::vector<int>::iterator>
> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(range_union_operators, T, test_types)
{
    check_std_operators<T>();
}

//BOOST_AUTO_TEST_CASE(range_union_operators)
//{
//    check_std_operators<
//    check_std_operators<pair_range, int>();
//    check_std_operators<pair_range, float>();
//    check_std_operators<pair_range, std::vector<int>::iterator>();
//    check_std_operators<boost::iterator_range, std::vector<int>::iterator>();
//}