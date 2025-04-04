//  Copyright (c) 2014-2024 Hartmut Kaiser
//  Copyright (c) 2017 Ajai V George
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/algorithm.hpp>
#include <hpx/hpx_main.hpp>
#include <hpx/include/parallel_count.hpp>
#include <hpx/include/parallel_for_each.hpp>
#include <hpx/include/partitioned_vector_predef.hpp>
#include <hpx/include/runtime.hpp>
#include <hpx/modules/testing.hpp>

#include <cstddef>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
struct pfo
{
    template <typename T>
    void operator()(T& val) const
    {
        ++val;
    }
};

template <typename T>
struct cmp
{
    explicit cmp(T const& val = T())
      : value_(val)
    {
    }

    template <typename T_>
    bool operator()(T_ const& val) const
    {
        return val == value_;
    }

    T value_;

    template <typename Archive>
    void serialize(Archive& ar, unsigned)
    {
        // clang-format off
        ar & value_;
        // clang-format on
    }
};

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename T>
void verify_values(
    ExPolicy&&, hpx::partitioned_vector<T> const& v, T const& val)
{
    using const_iterator = typename hpx::partitioned_vector<T>::const_iterator;

    std::size_t size = 0;

    const_iterator end = v.end();
    for (const_iterator it = v.begin(); it != end; ++it, ++size)
    {
        HPX_TEST_EQ(*it, val);
    }

    HPX_TEST_EQ(size, v.size());
}

template <typename ExPolicy, typename T>
void verify_values_count(
    ExPolicy&& policy, hpx::partitioned_vector<T> const& v, T const& val)
{
    HPX_TEST_EQ(
        static_cast<std::size_t>(hpx::count(policy, v.begin(), v.end(), val)),
        v.size());
    HPX_TEST_EQ(static_cast<std::size_t>(
                    hpx::count_if(policy, v.begin(), v.end(), cmp<T>(val))),
        v.size());
}

template <typename T>
void test_for_each(hpx::partitioned_vector<T>& v, T val)
{
    verify_values(hpx::execution::seq, v, val);
    verify_values_count(hpx::execution::seq, v, val);

    hpx::for_each(v.begin(), v.end(), pfo());

    verify_values(hpx::execution::seq, v, ++val);
    verify_values_count(hpx::execution::seq, v, val);
}

template <typename ExPolicy, typename T>
void test_for_each(ExPolicy&& policy, hpx::partitioned_vector<T>& v, T val)
{
    verify_values(policy, v, val);
    verify_values_count(policy, v, val);

    hpx::for_each(policy, v.begin(), v.end(), pfo());

    verify_values(policy, v, ++val);
    verify_values_count(policy, v, val);
}

template <typename ExPolicy, typename T>
void verify_values_count_async(
    ExPolicy&& policy, hpx::partitioned_vector<T> const& v, T const& val)
{
    HPX_TEST_EQ(static_cast<std::size_t>(
                    hpx::count(policy, v.begin(), v.end(), val).get()),
        v.size());
    HPX_TEST_EQ(
        static_cast<std::size_t>(
            hpx::count_if(policy, v.begin(), v.end(), cmp<T>(val)).get()),
        v.size());
}

template <typename ExPolicy, typename T>
void test_for_each_async(
    ExPolicy&& policy, hpx::partitioned_vector<T>& v, T val)
{
    verify_values(policy, v, val);
    verify_values_count_async(policy, v, val);

    hpx::for_each(policy, v.begin(), v.end(), pfo()).get();

    verify_values(policy, v, ++val);
    verify_values_count_async(policy, v, val);
}

template <typename T>
struct apply_wrap
{
    template <typename T_>
    void operator()([[maybe_unused]] T_& val) const
    {
    }

    hpx::reference_wrapper<hpx::partitioned_vector<T>> v;

    template <typename Archive>
    void serialize(Archive& ar, unsigned)
    {
        // clang-format off
        ar & v;
        // clang-format on
    }
};

template <typename ExPolicy, typename T>
void test_for_each_apply(ExPolicy&& policy, hpx::partitioned_vector<T>& v)
{
    hpx::for_each(policy, v.begin(), v.end(), apply_wrap<T>{v});
}

///////////////////////////////////////////////////////////////////////////////
template <typename T>
void for_each_tests(std::vector<hpx::id_type>& localities)
{
    {
        hpx::partitioned_vector<T> v;
        hpx::for_each(v.begin(), v.end(), pfo());
        hpx::for_each(hpx::execution::seq, v.begin(), v.end(), pfo());
        hpx::for_each(hpx::execution::par, v.begin(), v.end(), pfo());
        hpx::for_each(hpx::execution::seq(hpx::execution::task), v.begin(),
            v.end(), pfo())
            .get();
        hpx::for_each(hpx::execution::par(hpx::execution::task), v.begin(),
            v.end(), pfo())
            .get();
    }

    constexpr std::size_t length = 12;

    {
        hpx::partitioned_vector<T> v(
            length, T(0), hpx::container_layout(localities));
        test_for_each(v, T(0));
        test_for_each(hpx::execution::seq, v, T(1));
        test_for_each(hpx::execution::par, v, T(2));
        test_for_each_async(hpx::execution::seq(hpx::execution::task), v, T(3));
        test_for_each_async(hpx::execution::par(hpx::execution::task), v, T(4));
    }

    {
        hpx::partitioned_vector<T> v(
            length, T(0), hpx::container_layout(localities));

        v.register_as("foreach_test1");
        test_for_each_apply(hpx::execution::seq, v);
        test_for_each_apply(hpx::execution::par, v);
    }

    {
        std::vector<std::size_t> sizes;
        sizes.reserve(localities.size());

        for (std::size_t i = 0; i != localities.size(); ++i)
        {
            sizes.push_back(length / localities.size());
        }

        hpx::partitioned_vector<T> v(
            length, T(0), hpx::explicit_container_layout(sizes, localities));

        v.register_as("foreach_test2");
        test_for_each_apply(hpx::execution::seq, v);
        test_for_each_apply(hpx::execution::par, v);
    }
}

///////////////////////////////////////////////////////////////////////////////
int main()
{
    std::vector<hpx::id_type> localities = hpx::find_all_localities();
    for_each_tests<int>(localities);
    return hpx::util::report_errors();
}

#endif
