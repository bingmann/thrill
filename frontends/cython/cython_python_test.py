#!/usr/bin/env python
##########################################################################
# frontends/cython/cython_python_test.py
#
# Part of Project Thrill.
#
# Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

import thrill

print("hello from python")

def my_pycallback(i):
    print("Hello from pycallback %d" % i)

a = thrill.PyMyClass(9)
a.run()
a.set_callback(my_pycallback)
a = None

##########################################################################
