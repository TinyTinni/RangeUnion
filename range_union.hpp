#ifndef __TYTI_RANGE_UNION_HPP__
#define __TYTI_RANGE_UNION_HPP__

#include <utility>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <functional>

#include <vector>
#include <list>


//#include <boost/range/iterator_range.hpp> // compatible with

namespace tyti
{
    template<typename RangeT, typename ContainerT>
    class range_union;
}


template<typename T, template<class,class> class C>
tyti::range_union<T,C> operator+(tyti::range_union<T,C> lhs, const typename T& rhs)
{
    return lhs += rhs;
}
template<typename T, template<class, class> class C>
tyti::range_union<T,C> operator-(tyti::range_union<T,C> lhs, const typename T& rhs)
{
    return lhs -= rhs;
}


namespace tyti
{

    template<typename T>
    class pair_range : public std::pair<T, T>
    {
    public:
        typedef T iterator; //forward iterator
        pair_range(){}
        pair_range(const iterator& begin, const iterator& end) :std::pair<T, T>(begin, end) {}
        const T& begin() const { return first; }
        const T& end() const { return second; }
    };

    template<typename RangeT, class ContainerT = std::vector<RangeT, std::allocator<RangeT> > >
    class range_union
    {
        class less_or_disjunct
        {
        public:
            bool operator()(const RangeT& lhs, const RangeT& rhs)
            {
                return lhs.end() < rhs.begin();
            }
        };
        //typedef std::vector<RangeT> container_type;
        typedef ContainerT container_type;
        container_type m_ranges;


    public:
        typedef RangeT range_type;
        typedef typename container_type::iterator ranges_iterator;
        typedef typename container_type::const_iterator const_ranges_iterator;

        class iterator : public std::iterator<std::forward_iterator_tag, typename range_type::iterator>
        {
            const range_union<range_type>& m_interval;
            const_ranges_iterator m_currentRange;
            typename range_type::iterator m_current;

            bool equal(const iterator& rhs) const { return (/*(&m_interval == &rhs.m_interval) && (m_currentRange == rhs.m_currentRange) &&*/ (m_current == rhs.m_current)); }

        public:
            iterator(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                m_interval(interval),
                m_currentRange(currentRange),
                m_current((currentRange != interval.ranges_end()) ? currentRange->begin() : interval.ranges_back().end())
            {
            }

            iterator& operator++()
            {
                if (m_currentRange == m_interval.ranges_end())
                    return *this;

                typename range_type::iterator next = m_current;
                const_ranges_iterator nextRange = m_currentRange;
                ++nextRange;
                ++next;
                if ((nextRange != m_interval.ranges_end() && next != m_currentRange->end()) || //not the last range, skip end iter
                    nextRange == m_interval.ranges_end())//last range, so iterate over all elements
                {
                    m_current = next;
                }
                else if (nextRange != m_interval.ranges_end()) //current range has finished and there exists one more range -> go to next range
                {
                    m_current = nextRange->begin();
                    m_currentRange = nextRange;
                }

                return *this;
            }

            bool operator==(const iterator& rhs) const { return equal(rhs); }
            bool operator!=(const iterator& rhs) const { return !equal(rhs); }
            typename range_type::iterator& operator*() { return m_current; }
            const typename range_type::iterator& operator*() const { return m_current; }
        };



        range_union& operator+=(const range_type& rhs)
        {
            ranges_iterator lw_it = std::lower_bound(m_ranges.begin(), m_ranges.end(), rhs, less_or_disjunct());
            if (lw_it == m_ranges.end() || lw_it->begin() > rhs.end()) //all are less |or| lw_it is greater than rhs _and_ disjunct
            {
                m_ranges.insert(lw_it, rhs);
                return *this;
            }
            //lw_it and rhs are not disjunct
            ranges_iterator up_it = std::upper_bound(std::next(lw_it), m_ranges.end(), rhs, less_or_disjunct());
            //overwrite first non disjunct range
            using namespace std; //use algorithm min/max or, if defined before, windows macros of min/max
            *lw_it = range_type(min(rhs.begin(), lw_it->begin()), max(rhs.end(), std::prev(up_it)->end()));
            //delete the rest
            m_ranges.erase(std::next(lw_it), up_it);
            return *this;
        }

        range_union& operator-=(const range_type& rhs)
        {
            std::pair<ranges_iterator, ranges_iterator> contiguous = std::equal_range(m_ranges.begin(), m_ranges.end(), rhs, less_or_disjunct());
            if (contiguous.first != contiguous.second)
            {
                typename range_type::iterator first = contiguous.first->begin();
                typename range_type::iterator last = std::prev(contiguous.second)->end();

                //range to delete
                std::pair<ranges_iterator, ranges_iterator> deleteRange =
                    std::make_pair((first < rhs.begin()) ? std::next(contiguous.first) : contiguous.first, contiguous.second);

                if (first < rhs.begin())//reuse first block?
                    *contiguous.first = range_type(contiguous.first->begin(), rhs.begin());

                ranges_iterator it = m_ranges.erase(deleteRange.first, deleteRange.second);

                if (last > rhs.end())
                    m_ranges.insert(it, range_type(rhs.end(), last));//m_ranges.emplace(rhs.end(), last);
            }
            return *this;
        }

private:
    template <class T, class K> struct minus : std::binary_function <T, K, T> {
        T operator() (const T& x, const K& y) const { return x - y; }
    };

    template <class T, class K> struct plus : std::binary_function <T, K, T> {
        T operator() (const T& x, const K& y) const { return x + y; }
    };

public:
        range_union& operator-=(const range_union& rhs)
        {
            return *this = std::accumulate(rhs.ranges_begin(), rhs.ranges_end(), *this, minus<range_union, range_type>()); // todo: std::ref
        }

        range_union& operator+=(const range_union& rhs)
        {
            return *this = std::accumulate(rhs.ranges_begin(), rhs.ranges_end(), *this, plus<range_union, range_type>()); // todo: std::ref
        }

        iterator begin() const { return iterator(*this, ranges_begin()); }
        iterator end() const { return iterator(*this, ranges_end()); }

        const_ranges_iterator ranges_begin() const { return m_ranges.begin(); }
        const_ranges_iterator ranges_end() const { return m_ranges.end(); }
        range_type ranges_front() const { return *m_ranges.begin(); }
        range_type ranges_back() const { return *m_ranges.rbegin(); }

        size_t size() { return m_ranges.size(); }
        bool empty() { return m_ranges.empty(); }

        // return true, if the given range is disjunct with all the ranges saved in the union. otherwise false
        bool disjunct(const range_type& r) { std::pair<ranges_iterator, ranges_iterator> ov = m_ranges.equal_range(rhs); return ov.first == ov.second; }

    };
}

#endif //__TYTI_RANGE_UNION_HPP__