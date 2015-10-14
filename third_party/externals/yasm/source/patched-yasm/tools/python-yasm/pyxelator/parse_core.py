#!/usr/bin/env python
""" cdecl.py - parse c declarations

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""

import sys


class Symbols(object):
  def __init__(self,parent=None,verbose=False):
    self.verbose = verbose
    self.parent=parent # are we a nested namespace?
    self.lookup = {} # identifiers
    self.tags = {} # struct, union, enum tags

  def __str__(self):
    return "Symbols(%s,%s)"%(self.lookup,self.tags)

  def __getitem__(self,key):
    try:
      item = self.lookup[key]
    except KeyError:
      item = None
      #if self.parent is not None:
        #item = self.parent[item]
        ## self[key] = item # cache
    #if self.verbose: print "%s.get('%s')='%s'"%(self,key,item)
    return item

  def __setitem__(self,key,val):
    #if self.verbose: print "%s.set('%s','%s')"%(self,key,val)
    assert val is not None
    self.lookup[key] = val

  def set_tag(self,key,val):
    #if self.verbose: print "%s.set_tag(%s,%s)"%(self,key,val)
    assert len(key)
    self.tags[key] = val

  def deep_get_tag(self,key):
    try:
      item = self.tags[key]
    except KeyError:
      item = None
      if self.parent is not None:
        item = self.parent.deep_get_tag(key)
    #if self.verbose: print "%s.get_tag(%s)=%s"%(self,key,item)
    return item

  def get_tag(self,key):
    try:
      item = self.tags[key]
    except KeyError:
      item = None
    #if self.verbose: print "%s.get_tag(%s)=%s"%(self,key,item)
    return item

###################################################################
#
###################################################################
#


class ParseError(Exception):
  def __init__(self,*e):
    self.e = e

  def __str__(self):
    return "".join(map(str,self.e))


class Parser(object):
  def parse_error(self,lexer,reason="?",*blah):
    sys.stderr.write( "%s.parse_error()\n"%self.deepstr() )
    sys.stderr.write( "at line %s: %s\n"%(lexer.lno+1,reason) )
    sys.stderr.write( lexer.err_string() )
    raise ParseError(reason,*blah)

  def expected_error(self,lexer,*l):
    self.parse_error( lexer, "expected %s, got '%s'"\
      %(" or ".join(map(repr,l)),lexer.tok))

  def consume(self,lexer,tok):
    if lexer.tok != tok:
      self.expected_error(lexer, tok)
    lexer.get_token()

  def parse_enter(self,lexer):
    #return
    self.start_lno=lexer.lno
    self.file=lexer.so_file

  def parse_leave(self,lexer):
    #return
    self.lines = lexer.lines[self.start_lno:max(lexer.lno,self.start_lno+1)]

###################################################################
#
###################################################################
#

