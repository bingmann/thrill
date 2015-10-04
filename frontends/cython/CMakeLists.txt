################################################################################
# frontends/cython/CMakeLists.txt
#
# Part of Project Thrill.
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

include(UseCython)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/thrill.pyx
  ${CMAKE_CURRENT_SOURCE_DIR}/cython_test.pyx
  PROPERTIES CYTHON_IS_CXX TRUE)

# compile python extension thrill using cython.
cython_add_module(cython_thrill thrill.pyx)

# fixup target shared object name.
set_target_properties(cython_thrill PROPERTIES OUTPUT_NAME "thrill")

# compile cython test program
cython_add_standalone_executable(cython_test MAIN_MODULE cython_test.pyx)
target_link_libraries(cython_test thrill ${ALL_LIBRARIES})

# run cython test
thrill_test_only(cython_test)

# silence warnings in cython generated code
set_join(NO_WARN
  "-Wno-redundant-decls -Wno-cast-qual")
set_source_files_properties(
  cython_test_static.cxx
  cython_thrill.cxx
  PROPERTIES COMPILE_FLAGS "${NO_WARN}")

# run python test
add_test(
  NAME cython_python_test
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cython_python_test.py)

set_tests_properties(cython_python_test PROPERTIES
  ENVIRONMENT PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR})

################################################################################
