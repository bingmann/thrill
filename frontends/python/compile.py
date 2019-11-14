#!/usr/bin/env python3
##########################################################################
# frontends/python/compile.py
#
# Part of Project Thrill - http://project-thrill.org
#
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

setup(
    name = 'My Program Name',
    cmdclass = {'build_ext': build_ext},
    ext_modules = [
        Extension("test",  ["test.py"])
    ]
)

##########################################################################
