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
    ctypedef size_t (*callbackp)(size_t value);
    cdef cppclass MyClass:
        MyClass (size_t num);
        void run();
        void set_callback_plain(callbackp cb);
        void set_callback(callback cb, void* cookie);
        void set_callback_templ[Functor](Functor f);

    cdef cppclass PyCallback1:
        PyCallback1(callback cb, void* cookie);

cdef extern from "<thrill/api/context.hpp>" namespace "thrill::api":
    cdef cppclass HostContext:
        size_t workers_per_host() const;

    vector[HostContext*] ConstructHostLoopbackPlain(
        size_t host_count, size_t workers_per_host);

    cdef cppclass Context:
        Context(HostContext& host_context, size_t local_worker_id);
        size_t num_hosts() const;
        size_t workers_per_host() const;
        size_t my_rank() const;
        size_t num_workers() const;
        size_t host_rank() const;
        size_t local_worker_id() const;

cdef inline size_t PyCallback(size_t value, void* cookie):
    return (<object>cookie)(value)

cdef extern from "<thrill/api/dia.hpp>" namespace "thrill":
    cdef cppclass DIA[T]:
        bool IsValid() const;

cdef extern from "<thrill/api/generate.hpp>" namespace "thrill::api":
    ctypedef object (*Generator)(size_t index);
    cdef object Generate(Context& ctx, const Generator& gem, size_t size);

##########################################################################
