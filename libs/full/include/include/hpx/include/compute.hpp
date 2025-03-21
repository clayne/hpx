//  Copyright (c) 2016-2022 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/compute/host.hpp>
#include <hpx/compute_local/serialization/vector.hpp>
#include <hpx/compute_local/vector.hpp>

#if defined(HPX_HAVE_CUDA) || defined(HPX_HAVE_HIP)
#include <hpx/modules/async_cuda.hpp>
#endif
#if defined(HPX_WITH_SYCL)
#include <hpx/modules/async_sycl.hpp>
#endif
