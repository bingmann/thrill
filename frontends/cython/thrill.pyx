##########################################################################
# frontends/cython/thrill.pyx
#
# Part of Project Thrill.
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

cimport thrill

cdef class PyMyClass:
    cdef thrill.MyClass *ptr
    def __cinit__(self, size_t i):
        self.ptr = new thrill.MyClass(i)
    def __dealloc__(self):
        del self.ptr
    def run(self):
        self.ptr.run()
    def set_callback(self, cb):
        self.ptr.set_callback(thrill.PyCallback, <void*>cb)

##########################################################################
