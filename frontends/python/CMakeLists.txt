################################################################################
# frontends/python/CMakeLists.txt
#
# Part of Project Thrill - http://project-thrill.org
#
# Copyright (C) 2019 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(CMAKE_DEBUG_POSTFIX "")

add_subdirectory(pybind11)
pybind11_add_module(thrill_python module.cpp)

# rename thrill_python target output to thrill
set_target_properties(thrill_python PROPERTIES OUTPUT_NAME thrill)
target_link_libraries(thrill_python PRIVATE thrill)

################################################################################
