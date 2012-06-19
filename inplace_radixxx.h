#ifndef INCLUDE_GUARD_INPLACE_RADIXXX_H_
#define INCLUDE_GUARD_INPLACE_RADIXXX_H_

#include <climits>
#include <cstddef>
#include <algorithm>
#include <iterator>

namespace inplace_radixxx {

namespace detail {

struct unsigned_tag {};
struct signed_tag {};
struct bool_tag {};
struct others_tag {};

template <typename T>
struct get_tag {
    typedef others_tag type;
};

#define INPLACE_RADIXXX_SPECIALIZE_GET_TAG(type_, tag_) \
template <> \
struct get_tag<type_> { \
    typedef tag_ type; \
}

INPLACE_RADIXXX_SPECIALIZE_GET_TAG(unsigned char, unsigned_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(unsigned short, unsigned_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(unsigned, unsigned_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(unsigned long, unsigned_tag);
#if '\xff' >= 0 // char is unsigned
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(char unsigned_tag);
#else
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(char, signed_tag);
#endif
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(signed char, signed_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(short, signed_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(int, signed_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(long, signed_tag);
INPLACE_RADIXXX_SPECIALIZE_GET_TAG(bool, bool_tag);
#undef INPLACE_RADIXXX_SPECIALIZE_GET_TAG

template <typename T>
struct make_unsigned {
    typedef T type;
};

#define INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(signed_, unsigned_) \
template <> \
struct make_unsigned<signed_> { \
    typedef unsigned_ type; \
}

INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(char, unsigned char);
INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(signed char, unsigned char);
INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(short, unsigned short);
INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(int, unsigned);
INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED(long, unsigned long);
#undef INPLACE_RADIXXX_SPECIALIZE_MAKE_UNSIGNED

std::size_t const nbits = 8;
std::size_t const nbuckets = 1 << nbits;

template <typename Int>
struct initial_shift {
    static std::size_t const value = sizeof(Int) * CHAR_BIT - nbits;
};

template <typename Int, typename Tag>
struct initial_mask {
    static Int const value = (nbuckets - 1) << initial_shift<Int>::value;
};

template <typename Int>
struct initial_mask<Int, bool_tag> {
    static bool const value = false;
};

template <typename Int>
struct initial_mask<Int, others_tag> {
    static int const value = 0;
};

template <typename T>
struct remove_reference {
    typedef T type;
};

template <typename T>
struct remove_reference<T&> : remove_reference<T> {};

#if __GXX_EXPERIMENTAL_CXX0X__
template <typename T>
struct remove_reference<T&&> : remove_reference<T> {};
#endif

template <typename T>
struct remove_const {
    typedef T type;
};

template <typename T>
struct remove_const<T const> : remove_const<T> {};

template <typename T>
struct remove_volatile {
    typedef T type;
};

template <typename T>
struct remove_volatile<T volatile> : remove_volatile<T> {};

template <typename T>
struct remove_cv : remove_const<typename remove_volatile<T>::type> {};

template <typename T>
struct remove_ref_and_cv : remove_cv<typename remove_reference<T>::type> {};

#if __GNUG__
template <typename T>
struct add_reference {
    typedef T& type;
};

template <typename T>
struct add_reference<T&> : add_reference<T> {};

template <typename T>
typename add_reference<T>::type declval_();

template <typename>
struct result_of;

template <typename Functor, typename Argument>
struct result_of<Functor (Argument)>
    : remove_cv<__typeof__(declval_<Functor>() (declval_<Argument>()))>
{};
#else
template <bool HasResultType, typename Functor, typename Argument>
struct result_of_impl
    : remove_ref_and_cv<typename Functor::template result<Functor (Argument)>::type>
{};

template <typename Functor, typename Argument>
struct result_of_impl<true, Functor, Argument>
    : remove_ref_and_cv<typename Functor::result_type>
{};

typedef char no_type;
struct yes_type { char x[2]; };

template <typename>
no_type has_result_type(...);
template <typename T>
yes_type has_result_type(typename remove_reference<typename T::result_type>::type*);

template <typename>
struct result_of;

template <typename Functor, typename Argument>
struct result_of<Functor (Argument)>
    : result_of_impl<sizeof(has_result_type<Functor>(0)) == sizeof(yes_type),
                     Functor,
                     Argument>
{};

template <typename Return, typename Argument, typename Argument_>
struct result_of<Return (*(Argument_))(Argument)> {
    typedef Return type;
};

template <typename Return, typename Argument, typename Argument_>
struct result_of<Return (&(Argument_))(Argument)>
    : result_of<Return (*(Argument_))(Argument)>
{};
#endif // #if __GNUG__

template <typename T, typename Result, typename Argument>
struct result_of<Result (T::*(Argument))()>
    : remove_ref_and_cv<Result>
{};
template <typename T, typename Result, typename Argument>
struct result_of<Result (T::*(Argument))() const>
    : remove_ref_and_cv<Result>
{};

template <typename T, typename U, typename A>
struct result_of<U T::* (A)>
    : remove_ref_and_cv<U>
{};


template <typename Functor>
struct compare_key {
    explicit compare_key(Functor const& get_key) : get_key_(get_key) {}

    template <typename T>
    bool operator()(T const& x, T const& y) const {
        return get_key_(x) < get_key_(y);
    }

private:
    Functor get_key_;
};

template <typename Iterator, typename T, typename Functor>
void sort_impl(Iterator first, Iterator last, T mask, std::size_t shift,
               Functor const& get_key, unsigned_tag tag)
{
    typedef typename std::iterator_traits<Iterator>::difference_type diff_t;
    if (std::distance(first, last) <= diff_t(4 * nbuckets)) {
        compare_key<Functor> cmp(get_key);
        std::sort(first, last, cmp);
        return;
    }

    typedef typename std::iterator_traits<Iterator>::value_type value_t;
    typedef typename detail::result_of<Functor (value_t)>::type key_t;

    std::size_t count_[nbuckets] = {};
    for (Iterator it = first; it != last; ++it) {
        T pos = (get_key(*it) & mask) >> shift;
        ++count_[pos];
    }
    for (std::size_t i = 1; i < nbuckets; ++i)
        count_[i] += count_[i-1];
    Iterator upper_bounds[nbuckets];
    for (std::size_t i = 0; i < nbuckets; ++i) {
        upper_bounds[i] = first;
        std::advance(upper_bounds[i], count_[i]);
    }
    Iterator its[nbuckets];
    its[0] = first;
    for (std::size_t i = 1; i < nbuckets; ++i)
        its[i] = upper_bounds[i-1];
    for (std::size_t i = 0; i < nbuckets; ++i) {
        while (its[i] != upper_bounds[i]) {
            T const m = (get_key(*its[i]) & mask) >> shift;
            std::iter_swap(its[i], its[m]);
            ++its[m];
        }
    }
    if (mask >>= nbits) {
        shift -= nbits;
        sort_impl(first, upper_bounds[0], mask, shift, get_key, tag);
        for (std::size_t i = 1; i < nbuckets; ++i)
            sort_impl(upper_bounds[i-1], upper_bounds[i], mask, shift, get_key, tag);
    }
}

template <typename Functor>
struct key_is_negative {
    explicit key_is_negative(Functor const& get_key) : get_key_(get_key) {}
    template <typename T>
    bool operator()(T const& x) const {
        return get_key_(x) < 0;
    }

private:
    Functor get_key_;
};

template <typename Iterator, typename T, typename Functor>
void sort_impl(Iterator first, Iterator last, T mask, std::size_t shift,
               Functor const& get_key, signed_tag)
{
    Iterator it = std::partition(first, last, key_is_negative<Functor>(get_key));
    sort_impl(first, it, mask, shift, get_key, unsigned_tag());
    sort_impl(it, last, mask, shift, get_key, unsigned_tag());
}

template <typename Functor>
struct key_not {
    explicit key_not(Functor const& get_key) : get_key_(get_key) {}
    template <typename T>
    bool operator()(T const& x) const {
        return !get_key_(x);
    }

private:
    Functor get_key_;
};

template <typename Iterator, typename T, typename Functor>
inline void sort_impl(Iterator first, Iterator last, T, std::size_t, Functor const& get_key, bool_tag)
{
    std::partition(first, last, key_not<Functor>(get_key));
}

template <typename Iterator, typename T, typename Functor>
inline void sort_impl(Iterator first, Iterator last, T, std::size_t, Functor const& get_key, others_tag)
{
    std::sort(first, last, compare_key<Functor>(get_key));
}

template <typename T, typename R>
class mem_fun_ptr_wrapper {
    typedef R (T::*mem_fun_ptr)();

public:
    typedef R result_type;

    explicit mem_fun_ptr_wrapper(mem_fun_ptr p)
        : p_(p)
    {}

    result_type operator()(T& t) const {
        return (t.*p_)();
    }
    result_type operator()(T* t) const {
        return (t->*p_)();
    }

private:
    mem_fun_ptr p_;
};

template <typename T, typename R>
class mem_fun_const_ptr_wrapper {
    typedef R (T::*mem_fun_ptr)() const;

public:
    typedef R result_type;

    explicit mem_fun_const_ptr_wrapper(mem_fun_ptr p)
        : p_(p)
    {}

    result_type operator()(T const& t) const {
        return (t.*p_)();
    }
    result_type operator()(T const* t) const {
        return (t->*p_)();
    }

private:
    mem_fun_ptr p_;
};

template <typename T, typename U>
class mem_ptr_wrapper {
public:
    typedef U result_type;

    explicit mem_ptr_wrapper(U T::* p)
        : p_(p)
    {}

    result_type operator()(T const& t) const {
        return t.*p_;
    }
    result_type operator()(T const* t) const {
        return t->*p_;
    }

private:
    U T::* p_;
};

template <typename T>
T& mem_fn_(T& t)
{
    return t;
}

template <typename T>
T const& mem_fn_(T const& t)
{
    return t;
}

template <typename T, typename R>
mem_fun_ptr_wrapper<T, R> mem_fn_(R (T::*p)())
{
    return mem_fun_ptr_wrapper<T, R>(p);
}

template <typename T, typename R>
mem_fun_const_ptr_wrapper<T, R> mem_fn_(R (T::*p)() const)
{
    return mem_fun_const_ptr_wrapper<T, R>(p);
}

template <typename T, typename U>
mem_ptr_wrapper<T, U> mem_fn_(U T::* p)
{
    return mem_ptr_wrapper<T, U>(p);
}
} // namespace detail

template <typename Iterator, typename Functor>
inline void sort(Iterator first, Iterator last, Functor get_key)
{
    using detail::get_tag;
    using detail::initial_mask;
    using detail::initial_shift;
    using detail::make_unsigned;

    typedef typename std::iterator_traits<Iterator>::value_type value_t;
    typedef typename detail::result_of<Functor (value_t)>::type key_t;
    typedef typename get_tag<key_t>::type tag;

    detail::sort_impl(first, last,
                      initial_mask<typename make_unsigned<key_t>::type, tag>::value,
                      initial_shift<key_t>::value,
                      detail::mem_fn_(get_key),
                      tag());
}

namespace detail {
struct id {
    template <typename T>
    struct result;

    template <typename Functor, typename T>
    struct result<Functor (T)> {
        typedef T type;
    };

    template <typename T>
    T const& operator()(T const& x) const {
        return x;
    };
};
} // namespace detail

template <typename Iterator>
inline void sort(Iterator first, Iterator last)
{
    typedef typename std::iterator_traits<Iterator>::value_type value_t;
    ::inplace_radixxx::sort(first, last, detail::id());
}

template <typename Iterator>
inline void rsort(Iterator first, Iterator last)
{
    typedef std::reverse_iterator<Iterator> riterator;
    ::inplace_radixxx::sort(riterator(last), riterator(first));
}

template <typename Iterator, typename Functor>
inline void rsort(Iterator first, Iterator last, Functor get_key)
{
    typedef std::reverse_iterator<Iterator> riterator;
    ::inplace_radixxx::sort(riterator(last), riterator(first), get_key);
}
} // namespace inplace_radixxx
#endif // #ifndef INCLUDE_GUARD_INPLACE_RADIXXX_H_
