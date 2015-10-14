#!/usr/bin/env python
""" cdecl.py - parse c declarations

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""

import sys
import string
import types
import copy

#from cparse import BasicType, Qualifier, StorageClass, Typedef, Ellipses, GCCBuiltin
#from cparse import *

import cparse as host

class LexError(Exception):
  pass

class Lexer(object):
  def __init__(self,s="",verbose=0,**kw):
    self.verbose = verbose
    self.lookup = {} # a map for keywords and typedefs
    for t in \
      "float double void char int".split():
      self.lookup[t] = host.BasicType( t )
    for t in \
      "register signed unsigned short long const volatile inline".split(): # inline here ???
      self.lookup[t] = host.Qualifier( t )
    for t in "extern static auto".split():
      self.lookup[t] = host.StorageClass( t )
    self.lookup['typedef'] = host.Typedef()
    #self.lookup['__inline__'] = host.GCCBuiltin('__inline__')
    #self.lookup['__extension__'] = host.Qualifier('__extension__')
    self.lookup['...'] = host.Ellipses()
    if s:
      self.lex(s)
    for key in kw.keys():
      self.__dict__[key] = kw[key]

  def lex(self,s):
    self.stack = None
    self.lines = s.splitlines()
    self.set_state("","",0,0)
    self.so_file = ""
    self._newline()
    self.get_token() # start

  def mktypedef(self,tok,node):
    if self.verbose:
      print "%s.mktypedef(%s,%s)"%(self,tok,node)
    self.lookup[ tok ] = node

  def rmtypedef(self,tok):
    " used in round trip testing "
#    print "# rmtypedef(%s)"%tok
    assert isinstance( self.lookup[ tok ], host.Node ) # existance
    del self.lookup[ tok ]

  def _get_kind(self,tok):
    #print '_get_kind(%s)'%tok,self.lookup
    try:
      return self.lookup[tok]
      #return self.lookup[tok].clone()
    except KeyError:
      if tok.startswith("__builtin"):
        node = host.GCCBuiltin(tok)
        self.lookup[tok] = node
        return node
      #elif tok in ( "__extension__", ):
        #node = GCCBuiltin(tok)
        #self.lookup[tok] = node
        #return node
      return None

  def _newline(self):
    while self.lno < len(self.lines):
      line = self.lines[self.lno]
      if not line or line[0] != "#":
        break
      l = line.split('"')
      assert len(l)>=2
      self.so_file = l[1]
      #self.so_lno = int( l[0].split()[1] )
      #sys.stderr.write("# %s %s: %s\n"%(so_lno,so_file,l))
      self.lno+=1

  def get_brace_token( self ):
    self.push_state()
    ident_chars0 = string.letters+"_"
    ident_chars1 = string.letters+string.digits+"_"
    tok, kind = "", ""
    while self.lno < len(self.lines):
      s = self.lines[self.lno]
      i=self.col
      while i < len(s):
        if s[i] not in '{}':
          i=i+1
          continue
        else:
          tok = s[i]
          kind = tok
          self.col = i+1
          break
        # keep moving
        #sys.stderr.write( "lexer ignoring '%s'\n"%s[i] )
        i=i+1
      if i==len(s):
        # nothing found
        assert tok == ""
        self.col=0
        self.lno+=1
        self._newline()
      else:
        assert tok
        break
    self.set_state(tok,kind,self.lno,self.col)

  def get_token(self):
    self.push_state()
    ident_chars0 = string.letters+"_"
    ident_chars1 = string.letters+string.digits+"_"
    tok, kind = "", ""
    while self.lno < len(self.lines):
      s = self.lines[self.lno]
      i=self.col
      while i < len(s):
        if s[i].isspace():
          i=i+1
          continue
        #if s[i] in ident_chars0:
        if s[i].isalpha() or s[i]=='_':
          # identifier
          j=i+1
          while j<len(s):
            if s[j] in ident_chars1:
              j=j+1
            else:
              break
          tok = s[i:j]
          self.col = j
          kind = self._get_kind(tok)
          break
        if s[i].isdigit() or \
            (i+1<len(s) and s[i] in '+-.' and s[i+1].isdigit()):
          # number literal
          is_float = s[i]=='.'
          is_hex = s[i:i+2]=='0x'
          if is_hex:
            i=i+2
            assert s[i].isdigit() or s[i] in "abcdefABCDEF", self.err_string()
          j=i+1
          while j<len(s):
            #print "lex ",repr(s[i]),is_float
            if s[j].isdigit() or (is_hex and s[j] in "abcdefABCDEF"):
              j=j+1
            elif s[j]=='.' and not is_float:
              assert not is_hex
              j=j+1
              is_float=1
            else:
              break 
          tok = s[i:j]
          self.col = j
          if is_float:
            kind = float(tok)
          elif is_hex:
            kind = int(tok,16)
          else:
            kind = int(tok)
          break
        if s[i:i+3]=='...':
          # ellipses
          #sys.stderr.write( "ELLIPSES "+str(self.get_state()) )
          tok = s[i:i+3]
          kind = self._get_kind(tok)
          self.col = i+3
          break
        if s[i] in '*/{}()[]:;,=+-~.<>|&':
          tok = s[i]
          kind = tok
          self.col = i+1
          break
        if s[i] == "'":
          j = i+2
          while j<len(s) and s[j]!="'":
            j+=1
          if j==len(s):
            raise LexError( self.err_string() + "unterminated char constant" )
          tok = s[i:j+1]
          self.col = j+1
          kind = s[i:j+1]
          break
        # keep moving
        #sys.stderr.write( "lexer ignoring '%s'\n"%s[i] )
        sys.stderr.write( "lexer ignoring '%s' lno=%d\n"%(s[i],self.lno+1) )
        i=i+1
        # end while i < len(s)
      if i==len(s):
        # nothing found, go to next line
        assert tok == ""
        self.col=0
        self.lno+=1
        self._newline()
      else:
        # we got one
        assert tok
        break
      # end while self.lno < len(self.lines):
    self.set_state(tok,kind,self.lno,self.col)

  def err_string(self):
    "Return helpful error string :)"
    return self.lines[self.lno]+"\n"+" "*self.col+"^\n"

  def push_state(self):
    self.stack = self.get_state() # a short stack :)
    #self.stack.push( self.get_state() )

  def unget_token(self):
    assert self.stack is not None
    self.set_state(*self.stack)
    self.stack = None

  def set_state(self,tok,kind,lno,col):
    if self.verbose:
      print "tok,kind,lno,col = ",(tok,kind,lno,col)
    self.tok = tok
    self.kind = kind
    self.lno = lno # line
    self.col = col # column

  def get_state(self):
    return self.tok,self.kind,self.lno,self.col

  def get_file(self):
    return self.so_file

###################################################################
#
###################################################################
#


