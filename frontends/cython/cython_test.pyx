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
from thrill cimport *
import thrill

print("hello from cython")

cdef thrill.MyClass* c = new thrill.MyClass(5)
c.run()

cdef size_t my_callback(size_t i):
    print("Hello from callback %d" % i)

c.set_callback_templ(my_callback)
c.run()

del c

def my_pycallback(i):
    print("Hello from pycallback %d" % i)

a = thrill.PyMyClass(7)
a.run()
a.set_callback(my_pycallback)
a = None

### Create HostContext and Context

cdef vector[HostContext*] hctx = ConstructHostLoopbackPlain(1, 1)
print("hctx.size()", hctx.size())
print("num_hosts", hctx[0].workers_per_host())

cdef Context* ctx = new Context(hctx[0][0], 0)
print("ctx.num_hosts()", ctx.num_hosts())

cdef object my_gen(size_t i):
    return i * i

# this does not work, and I found no solution. Either we have to get auto to
# work here -> DIA[T] where T is the return type of the cython function.
#
# Or we have no choice but to use DIA[object] everywhere, and have generic
# callbacks object -> object, but that would make performance suck.

#cdef auto dia1 = Generate(ctx[0], &my_gen, 10)

##########################################################################
