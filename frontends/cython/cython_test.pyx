##########################################################################
# frontends/cython/cython_test.pyx
#
# Part of Project Thrill.
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

cimport thrill

print("hello from python")

cdef thrill.MyClass* c = new thrill.MyClass(5)
c.run()

cdef size_t my_callback(size_t i):
    print("Hello from callback %d" % i)

c.set_callback(my_callback)
c.run()

del c

##########################################################################
