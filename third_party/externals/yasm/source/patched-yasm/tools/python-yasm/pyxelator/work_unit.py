#!/usr/bin/env python

""" 

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""


import sys
import os

import cparse
import ir

def callcmd(cmd):
    try:
        from subprocess import call
        try:
            retcode = call(cmd, shell=True)
            assert retcode == 0, "command failed: %s"%cmd
        except OSError, e:
            assert False, "command failed: %s"%e
    except ImportError:
        status = os.system( cmd )
        assert status == 0, "command failed: %s"%cmd

class WorkUnit(object):
    def __init__(self, files, modname, filename,
                 std=False, strip=False, mark_cb=None, 
                 extradefs="", use_header=None, CC="gcc", CPP="gcc -E",
                 CPPFLAGS=""):
        self.files = tuple(files)
        self.modname = modname
        self.filename = filename
        self.CPPFLAGS = CPPFLAGS
        self.CPP = CPP
        if CC == 'g++':
            self.CPPFLAGS += " -D__cplusplus"
        self.std = std
        self.strip = strip
        self.mark_cb = mark_cb
        self.node = None
        self.extradefs = extradefs
        self.CC = CC
        self.use_header = use_header

    def mkheader( self ):
        if self.use_header:
            return self.use_header
        tmpname = str(abs(hash( (self.files,self.CPPFLAGS) )))
        name = '.tmp/%s' % tmpname
        ifile = open( name+'.h', "w" )
        ifile.write( """
#define __attribute__(...) 
#define __const const
#define __restrict 
#define __extension__
#define __asm__(...)
#define __asm(...)
#define __inline__  
#define __inline
""" )
        for filename in self.files:
            if self.std:
                line = '#include <%s>\n'%filename
            else:
                line = '#include "%s"\n'%filename
            ifile.write( line )
            print line,
        ifile.close()
        cmd = '%s %s %s > %s'%(self.CPP,name+'.h',self.CPPFLAGS,name+'.E')
        sys.stderr.write( "# %s\n" % cmd )
        callcmd( cmd )
        assert open(name+'.E').read().count('\n') > 10, "failed to run preprocessor"
        cmd = '%s -dM %s %s > %s'%(self.CPP,name+'.h',self.CPPFLAGS,name+'.dM')
        sys.stderr.write( "# %s\n" % cmd )
        callcmd( cmd )
        assert open(name+'.dM').read().count('\n') > 10, "failed to run preprocessor with -dM"
        return name

    def parse(self, verbose=False):
        sys.stderr.write( "# parse %s\n" % str(self.files) )
        name = self.mkheader()
        # read macros
        f = open(name+'.dM')
        macros = {}
        for line in f.readlines():
            if line:
                macro = line.split()[1]
                if macro.count('('):
                    macro = macro[:macro.index('(')]
                macros[macro] = None
        #keys = macros.keys()
        #keys.sort()
        #for key in keys:
            #print key
        self.macros = macros
        # parse preprocessed code
        f = open(name+'.E')
        s = f.read() + self.extradefs
        self.node = cparse.TransUnit(verbose = verbose)
        sys.stderr.write( "# parsing %s lines\n" % s.count('\n') )
        self.node.parse( s )
        if self.strip:
            self.node.strip(self.files)

    def transform(self, verbose=False, test_parse=False, test_types=False):
        sys.stderr.write( "# processing...\n" )
        self.node = ir.TransUnit( self.node )
        self.node.transform(verbose, test_parse, test_types)
        #self.node[0].psource()
        if self.mark_cb is not None:
            self.node.mark(self.mark_cb,verbose=False)

    def output( self, func_cb = None ):
        sys.stderr.write( "# pyxstr...\n" )
        decls = self.node.pyx_decls(self.files, self.modname, macros = self.macros, func_cb = func_cb, names={}, cprefix="" )

        name = self.filename
        assert name.endswith(".pyx")

        pxi = name[:-3]+'pxi'
        file = open( pxi, "w" )
        file.write(decls)
        sys.stderr.write( "# wrote %s, %d lines\n" % (pxi,decls.count('\n')) )

    def pprint(self):
        for decl in self.node:
            #decl.psource()
            #cstr = decl.cstr()
            #cstr = cstr.replace( '\n', '\n# ' )
            print
            #print '#', cstr
            print decl.deepstr()

def file_exists(path):
    try:
        os.stat(path)
        return True
    except OSError:
        return False

if sys.platform.count('darwin'):
    shared_ext = '.dylib'
else:
    shared_ext = '.so'

def get_syms(libs, libdirs):
    # XX write interface to objdump -t XX
    libnames = []
    for lib in libs:
        for ext in shared_ext,'.a':
            libname = 'lib'+lib+ext
            for libdir in libdirs:
                path = libdir+'/'+libname
                if file_exists(path):
                    libnames.append(path)
                    break
            #else:
                #print "cannot find %s lib as %s in %s" % ( lib, libname, libdir )
    print 'libnames:', libnames
    syms = {}
    accept = [ ' %s '%c for c in 'TVWBCDGRS' ]
    #f = open('syms.out','w')
    for libname in libnames:
        try:
            from subprocess import Popen, PIPE
            p = Popen(['nm', libname], bufsize=1, stdout=PIPE)
            fout = p.stdout
        except ImportError:
            fin, fout = os.popen2( 'nm %s' % libname )
        for line in fout.readlines():
            for acc in accept:
                if line.count(acc):
                    left, right = line.split(acc)
                    sym = right.strip()
                    if sys.platform.count('darwin'):
                        if sym[0] == '_':
                            sym = sym[1:] # remove underscore prefix
                        if sym.endswith('.eh'):
                            sym = sym[:-len('.eh')]
                    syms[sym] = None
                    #f.write( '%s: %s %s\n' % (sym,line[:-1],libname) )
                    break
    return syms



