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
import thrill

print("hello from python")

cdef thrill.MyClass* c = new thrill.MyClass(5)
c.run()

cdef size_t my_callback(size_t i, void* c):
    print("Hello from callback %d" % i)

c.set_callback(my_callback, NULL)
c.run()

del c

def my_pycallback(i):
    print("Hello from pycallback %d" % i)

a = thrill.PyMyClass(7)
a.run()
a.set_callback(my_pycallback)
a = None

##########################################################################
