#ifndef __TYTI_RANGE_UNION_HPP__
#define __TYTI_RANGE_UNION_HPP__

#include <utility>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <functional>
#include <numeric>

#include <vector>


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
    /** \brief Simple Example of a range
     shows the requirement of a range
     */
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

    /** \brief range_union class
    @param RangeT range type. Requirement are shown in pair_range
    @param ContainerT container which will be used. Use vector, if you want max performance for iteration. 
        Use list, if you want max performance when you insert/delete a lot(+=/-= operators).
    */
    template<typename RangeT, class ContainerT = std::vector<RangeT, std::allocator<RangeT> > >
    class range_union
    {
        ///////////////////////////////////////////////////////////////////////////////
        // common types
        ///////////////////////////////////////////////////////////////////////////////
    public:
        typedef RangeT range_type;
        typedef ContainerT container_type;
        typedef typename container_type::iterator ranges_iterator;
        typedef typename container_type::const_iterator const_ranges_iterator;


    private:
        ///////////////////////////////////////////////////////////////////////////////
        // members
        ///////////////////////////////////////////////////////////////////////////////
        container_type m_ranges;

        ///////////////////////////////////////////////////////////////////////////////
        // private details
        ///////////////////////////////////////////////////////////////////////////////
        struct less_or_disjunct
        {
            bool operator()(const RangeT& lhs, const RangeT& rhs)
            {
                return lhs.end() < rhs.begin();
            }
        };
        
        // iterator
        template<typename tagT>
        class base_iteratorT : public base_iteratorT<std::bidirectional_iterator_tag>
        {
        public:
            base_iteratorT(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                base_iteratorT<std::bidirectional_iterator_tag>(interval, currentRange)
            {}
            base_iteratorT(const base_iteratorT<std::bidirectional_iterator_tag>& it) :
                base_iteratorT<std::bidirectional_iterator_tag>(it)
            {}
        };

        template<>
        class base_iteratorT<std::forward_iterator_tag>
        {
        protected:
            const range_union<range_type>& m_interval;
            const_ranges_iterator m_currentRange;
            typename range_type::iterator m_current;

            bool equal(const base_iteratorT& rhs) const { return m_current == rhs.m_current; }

        public:
            base_iteratorT(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                m_interval(interval),
                m_currentRange(currentRange),
                m_current((currentRange != interval.ranges_end()) ? currentRange->begin() : interval.ranges_back().end())
            {
            }

            base_iteratorT& operator++()
            {
                if (m_currentRange == m_interval.ranges_end())
                    return *this;

                const_ranges_iterator nextRange = m_currentRange;
                ++nextRange;
                ++m_current;
                if (m_current == m_currentRange->end() && nextRange != m_interval.ranges_end()) //skip endIter if current range is not the last range
                {
                    m_current = nextRange->begin();
                    m_currentRange = nextRange;
                }

                return *this;
            }
            base_iteratorT operator++(int)
            {
                base_iteratorT tmp(*this);
                ++(*this);
                return tmp;
            }

            bool operator==(const base_iteratorT& rhs) const { return equal(rhs); }
            bool operator!=(const base_iteratorT& rhs) const { return !equal(rhs); }
            const typename range_type::iterator& operator*() const { return m_current; }
        };

        template<>
        class base_iteratorT<std::bidirectional_iterator_tag> : public base_iteratorT<std::forward_iterator_tag>
        {
        public:
            base_iteratorT(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                base_iteratorT<std::forward_iterator_tag>(interval, currentRange)
            {}
            base_iteratorT& operator--()
            {
                if (m_currentRange == m_interval.ranges_end())
                    return *this;

                const_ranges_iterator prevRange = m_currentRange;
                --prevRange;
                if (m_current == m_currentRange->begin())
                {
                    m_current = m_currentRange->end();
                    if (prevRange != m_interval.ranges_begin()) // if its not the first range, continue
                    {
                        --m_current;
                        m_currentRange = prevRange;
                    }
                }
                else
                    --m_current;

                return *this;
            }
            base_iteratorT operator--(int)
            {
                base_iteratorT tmp(*this);
                --(*this);
                return tmp;
            }
        };

        template<typename iter, bool isSpecT>
        class iteratorT : public std::iterator<typename std::iterator_traits<iter>::iterator_category, iter>, public base_iteratorT<typename std::iterator_traits<iter>::iterator_category>
        {
        public:
            iteratorT(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                base_iteratorT<typename std::iterator_traits<iter>::iterator_category>(interval, currentRange) {}
            iteratorT(const base_iteratorT<typename std::iterator_traits<iter>::iterator_category>& it) :
                base_iteratorT<typename std::iterator_traits<iter>::iterator_category>(it) {}
        };

        //for plain types like int, unsigned int, etc.
        template<typename iter>
        class iteratorT<iter, true> : public std::iterator<std::bidirectional_iterator_tag, iter>, public base_iteratorT<std::bidirectional_iterator_tag>
        {
        public:
            iteratorT(const range_union<range_type>& interval, const const_ranges_iterator& currentRange) :
                base_iteratorT<std::bidirectional_iterator_tag>(interval, currentRange) {}
            iteratorT(const base_iteratorT<std::bidirectional_iterator_tag>& it) :
                base_iteratorT<std::bidirectional_iterator_tag>(it) {}
        };


    public:
        ///////////////////////////////////////////////////////////////////////////////
        // public interface
        ///////////////////////////////////////////////////////////////////////////////

        typedef iteratorT<
            typename range_type::iterator, 
            std::numeric_limits<typename range_type::iterator>::is_specialized
        > iterator;        

        /** adds a given range to the current union
        * e.g.: 
        * [(0,5)] += (4,10) -> [(0,10)]
        * [(0,5)] += (9,10) -> [(0,5),(9,10)]
        */
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

        /** removes a given range to the current union
        * e.g.:
        * [(0,10)] -= (4,10) -> [(0,4)]
        * [(0,10)] -= (7,8) -> [(0,7),(8,10)]
        */
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
        /**
        * remove all ranges from the given range_union
        */
        const range_union& operator-=(const range_union& rhs)
        {
            for (const_ranges_iterator it = rhs.ranges_begin(); it != rhs.ranges_end(); ++it)
                *this -= *it;
            return *this;
        }

        /**
        * add all ranges from the given range_union
        */
        const range_union& operator+=(const range_union& rhs)
        {
            for (const_ranges_iterator it = rhs.ranges_begin(); it != rhs.ranges_end(); ++it)
                *this += *it;
            return *this;
        }

        /**
        * get the beginning of the iterator. Iterates over all elements speicified by the ranges
        */
        iterator begin() const { return iterator(*this, ranges_begin()); }
        iterator end() const { return iterator(*this, ranges_end()); }
        iterator cbegin() const { return const_iterator(*this, ranges_begin()); }
        iterator cend() const { return const_iterator(*this, ranges_end()); }

        /**
        * get the beginning of the range_iterator. Iterates over all ranges inside the union
        */
        const_ranges_iterator ranges_begin() const { return m_ranges.begin(); }
        const_ranges_iterator ranges_end() const { return m_ranges.end(); }
        range_type ranges_front() const { return *m_ranges.begin(); }
        range_type ranges_back() const { return *m_ranges.rbegin(); }

        size_t size() { return m_ranges.size(); }
        bool empty() { return m_ranges.empty(); }

        /** 
        * return true, if the given range is disjunct with all the ranges saved in the union. otherwise false
        */
        bool disjunct(const range_type& rhs) 
        { 
            std::pair<ranges_iterator, ranges_iterator> ov = std::equal_range(m_ranges.begin(), m_ranges.end(), rhs, less_or_disjunct()); 
            if (ov.first == ov.second)
                return true;
            return ov.first->begin() >= rhs.end();
            
        }

    };
}

#endif //__TYTI_RANGE_UNION_HPP__