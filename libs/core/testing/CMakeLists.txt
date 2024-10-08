# Copyright (c) 2019-2020 The STE||AR-Group
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Default location is $HPX_ROOT/libs/testing/include
set(testing_headers hpx/modules/testing.hpp hpx/testing/performance.hpp)

# Default location is $HPX_ROOT/libs/testing/include_compatibility
# cmake-format: off
set(testing_compat_headers
    hpx/testing.hpp => hpx/modules/testing.hpp
    hpx/util/lightweight_test.hpp => hpx/modules/testing.hpp
)
# cmake-format: on

# Default location is $HPX_ROOT/libs/testing/src
set(testing_sources testing.cpp performance.cpp)

set(module_deps
    hpx_assertion
    hpx_config
    hpx_format
    hpx_functional
    hpx_preprocessor
    hpx_util
    hpx_command_line_handling_local
)

if(HPX_WITH_NANOBENCH)
  set(module_deps ${module_deps} nanobench)
endif()

include(HPX_AddModule)
add_hpx_module(
  core testing
  GLOBAL_HEADER_GEN OFF
  SOURCES ${testing_sources}
  HEADERS ${testing_headers}
  COMPAT_HEADERS ${testing_compat_headers}
  MODULE_DEPENDENCIES ${module_deps}
  CMAKE_SUBDIRS examples tests
)
