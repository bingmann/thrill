##########################################################################
# frontends/cython/thrill.pxd
#
# Part of Project Thrill.
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "<thrill/api/context.hpp>" namespace "thrill":
    ctypedef size_t (*callback)(size_t value, void* cookie);
    cdef cppclass MyClass:
        MyClass (size_t num);
        void run();
        void set_callback(callback cb, void* cookie);

cdef inline size_t PyCallback(size_t value, void* cookie):
    return (<object>cookie)(value)

##########################################################################
