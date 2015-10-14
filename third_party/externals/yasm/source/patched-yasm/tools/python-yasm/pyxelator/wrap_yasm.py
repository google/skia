#!/usr/bin/env python

""" 

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""


import sys
import os

from work_unit import WorkUnit, get_syms
import ir


def mk_tao(CPPFLAGS = "", CPP = "gcc -E", modname = '_yasm', oname = None, YASM_DIR = ".", **options):
    if oname is None:
        oname = modname+'.pyx'
    CPPFLAGS += " -I"+YASM_DIR
    CPPFLAGS += " -DYASM_PYXELATOR"
    CPPFLAGS += " -DYASM_LIB_INTERNAL"
    CPPFLAGS += " -DYASM_BC_INTERNAL"
    CPPFLAGS += " -DYASM_EXPR_INTERNAL"
    files = [ 'libyasm.h', 'libyasm/assocdat.h', 'libyasm/bitvect.h' ]

    syms = get_syms( ['yasm'], [YASM_DIR] )
    def cb(trans_unit, node, *args):
        name, file = node.name, node.file
        return True
        return name in syms
    extradefs = ""
    unit = WorkUnit(files,modname,oname,False,mark_cb=cb,extradefs=extradefs,
        CPPFLAGS=CPPFLAGS, CPP=CPP, **options)


    unit.parse( False )
    unit.transform(verbose=False, test_parse=False, test_types=False)
    unit.output()

def main():
    options = {}
    for i,arg in enumerate(sys.argv[1:]):
        if arg.count('='):
            key,val = arg.split('=', 1)
            options[key]=val
    mk_tao(**options)

if __name__=="__main__":
    main()




