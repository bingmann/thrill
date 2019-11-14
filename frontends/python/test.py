#!/usr/bin/env python3
##########################################################################
# frontends/python/test.py
#
# Part of Project Thrill - http://project-thrill.org
#
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
##########################################################################

import thrill

host_context = thrill.HostContext()
ctx = thrill.Context(host_context)

# def flat_lambda(x, emit):
#     emit(x)
#     emit(x+1)

# dia = thrill.Generate(ctx, 100, lambda a : a * a)
# print(dia.Size())
# dia.Print()
# dia = dia.Map(lambda x : x + 1000)
# dia = dia.Filter(lambda x : x < 2000)
# dia.Print()
# dia = dia.FlatMap(flat_lambda)
# dia.Print()

# print(dia.AllGather())

# print(dia.AllReduce(lambda x, y : x + y))

# dia2 = dia.Map(lambda x : (x % 100, x))
# dia2.Print()

# dia3 = dia2.Sort()
# dia3.Print()

# dia4 = dia3.ReduceToIndex(lambda x : x % 2, lambda x, y : x + y, 100)
# dia4.Print()

dia5 = thrill.Generate(ctx, 100)
dia5.Print()

dia6 = thrill.ReadLines(ctx, "Makefile")
dia6.Sort().Print()

dia4 = dia5.ReduceToIndex(lambda x : x % 2, lambda x, y : x + y, 2)
dia4.Print()

##########################################################################
