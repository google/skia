#!/usr/bin/env python

""" 

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

"""

import sys

from lexer import Lexer
from parse_core import Symbols, Parser
import node as node_module


class Node(node_module.Node):

  def is_typedef(self):
    for x in self:
      if isinstance(x,Node):
        if x.is_typedef():
          return 1
    return 0

  #def explain(self):
    #l = []
    #for x in self:
      #if isinstance(x,Node):
        #l.append(x.explain())
      #else:
        #l.append(str(x))
    #return string.join(l," ") 
      ##(self.__class__.__name__,string.join(l) )

  def psource(self):
    if hasattr(self,'lines'):
      print "# "+string.join(self.lines,"\n# ")+"\n"


###################################################################
#
###################################################################
#


class BasicType(Node):
  " int char short etc. "
  def __init__(self,name):
    Node.__init__(self,name)

class Qualifier(Node):
  """
  """
  def __init__(self,name):
    Node.__init__(self,name)
    self.name=name

class StorageClass(Node):
  """
  """
  def __init__(self,name):
    Node.__init__(self,name)
    self.name=name

class Typedef(StorageClass):
  """
  """
  def __init__(self,s='typedef'):
    Node.__init__(self,s)
  #def explain(self):
    #return "type"

class Ellipses(Node):
  """
  """
  def __init__(self,s='...'):
    Node.__init__(self,s)

class GCCBuiltin(BasicType):
  """
  """
  pass


class Identifier(Node):
  """
  """
  def __init__(self,name="",*items):
    if name or 1:
      Node.__init__(self,name,*items)
    else:
      Node.__init__(self)
    self.name=name

class Function(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)

  def parse(self,lexer,symbols):
    symbols = Symbols(symbols)
    args = ''
    #lexer.get_token()
    if lexer.tok != ')':
      if not lexer.tok:
        self.parse_error(lexer)
      #lexer.unget_token() # unget start of decl
      while lexer.tok != ')':
        node = ParameterDeclaration()
        node.parse(lexer,symbols)
        self.append( node )
        if lexer.tok != ')' and lexer.tok != ',':
          self.parse_error(lexer)
        if lexer.tok == ',':
          lexer.get_token()
    lexer.get_token()


class Pointer(Node):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)

class Array(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)

  def parse(self,lexer,symbols):
    lexer.get_token() # a number or ']'
    # XX
    # HACK HACK: constant c expressions can appear in here:
    # eg. [ 15 * sizeof (int) - 2 * sizeof (void *) ]
    # XX
    toks = []
    while lexer.tok != ']':
      #self.append( lexer.kind )
      toks.append( lexer.tok )
      lexer.get_token()
    child = " ".join(toks)
    if child == "":
      child = None
    self.append( child )
    lexer.get_token() # read past the ']'

class Tag(Node):
  """
  """
  pass


class Compound(Node,Parser):
  "Struct or Union"

  def __init__(self,*items,**kw):
    Node.__init__(self,*items,**kw)

  def parse(self,lexer,symbols):
    symbols = Symbols(symbols)
    tag = "" # anonymous
    if lexer.tok != '{':
      tag = lexer.tok
      if not ( tag[0]=='_' or tag[0].isalpha() ):
        self.parse_error(lexer ,"expected tag, got '%s'"%tag )
      lexer.get_token()
    if tag:
      self.append(Tag(tag))
    else:
      self.append(Tag())
    self.tag = tag
    if lexer.tok == '{':
      fieldlist = []
      lexer.get_token()
      if lexer.tok != '}':
        if not lexer.tok: self.parse_error(lexer)
        while lexer.tok != '}':
          node = StructDeclaration()
          node.parse(lexer,symbols)
          fieldlist.append( node )
      self += fieldlist
      lexer.get_token()
    if self.verbose: 
      print "%s.__init__() #<--"%(self)

class Struct(Compound):
  """
  """
  pass

class Union(Compound):
  """
  """
  pass

class Enum(Node,Parser):
  """
  """
  def __init__(self,*items,**kw):
    Node.__init__(self,*items,**kw)

  def parse(self,lexer,symbols):
    tag = "" # anonymous
    if lexer.tok != '{':
      tag = lexer.tok
      if not ( tag[0]=='_' or tag[0].isalpha() ):
        self.parse_error(lexer ,"expected tag, got '%s'"%tag )
      lexer.get_token()
    if tag:
      self.append(Tag(tag))
    else:
      self.append(Tag())
    self.tag = tag
    if lexer.tok == '{':
      lexer.get_token()
      if lexer.tok != '}': # XX dopey control flow
        if not lexer.tok: # XX dopey control flow
          self.parse_error(lexer) # XX dopey control flow
        while lexer.tok != '}': # XX dopey control flow
          if lexer.kind is not None:
            self.expected_error(lexer ,"identifier" )
          ident = Identifier(lexer.tok)
          if symbols[ident[0]] is not None:
            self.parse_error(lexer,"%s already defined."%ident[0])
          symbols[ident[0]]=ident
          self.append( ident )
          lexer.get_token()
          if lexer.tok == '=':
            lexer.get_token()
            # ConstantExpr
            # XX hack hack XX
            while lexer.tok!=',' and lexer.tok!='}':
              lexer.get_token()
#            if type( lexer.kind ) is not int:
#              #self.parse_error(lexer ,"expected integer" )
#              # XX hack hack XX
#              while lexer.tok!=',' and lexer.tok!='}':
#                lexer.get_token()
#            else:
#              # put initializer into the Identifier
#              ident.append( lexer.kind )
#              lexer.get_token()
          if lexer.tok != '}':
            if lexer.tok != ',':
              self.expected_error(lexer,"}",",")
            lexer.get_token() # ','
      lexer.get_token()
    if self.verbose:
      print "%s.__init__() #<--"%(self)



class Declarator(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)
    self.ident = None

  def parse(self,lexer,symbols):
    #Parser.parse_enter(self,lexer)
    stack = []
    # read up to identifier, pushing tokens onto stack
    self.ident = self.parse_identifier(lexer,symbols,stack)
    self.name = ''
    if self.ident is not None:
      self.append( self.ident )
      self.name = self.ident.name
    # now read outwards from identifier
    self.parse_declarator(lexer,symbols,stack)
    #Parser.parse_leave(self,lexer)

  def parse_identifier(self,lexer,symbols,stack):
    if self.verbose:
      print "%s.parse_identifier()"%self
    ident = None
    if lexer.tok != ';':
      while lexer.tok and lexer.kind is not None:
        stack.append( (lexer.tok, lexer.kind) )
        lexer.get_token()
      if lexer.tok:
        ident = Identifier( lexer.tok )
        #stack.append( (ident.name, ident) )
        lexer.get_token()
    if self.verbose:
      print "%s.parse_identifier()=%s"%(self,repr(ident))
    return ident

  def parse_declarator(self,lexer,symbols,stack,level=0):
    if self.verbose:
      print "  "*level+"%s.parse_declarator(%s) # --->"%\
        (self,stack)
    if lexer.tok == '[':
      while lexer.tok == '[':
        node = Array()
        node.parse(lexer,symbols)
        self.append(node)
      if lexer.tok == '(':
        self.parse_error(lexer ,"array of functions" )
    elif lexer.tok == '(':
      lexer.get_token()
      node = Function()
      node.parse(lexer,symbols)
      self.append( node )
      if lexer.tok == '(':
        self.parse_error(lexer ,"function returns a function" )
      if lexer.tok == '[':
        self.parse_error(lexer ,"function returns an array" )
    while stack:
      tok, kind = stack[-1] # peek
      if tok == '(':
        stack.pop()
        self.consume(lexer,')')
        self.parse_declarator(lexer,symbols,stack,level+1)
      elif tok == '*':
        stack.pop()
        self.append( Pointer() )
      else:
        tok, kind = stack.pop()
        self.append( kind )
    if self.verbose:
      print "  "*level+"%s.parse_declarator(%s) # <---"%\
        (self,stack)


class AbstractDeclarator(Declarator):
  """ used in ParameterDeclaration; may lack an identifier """

  def parse_identifier(self,lexer,symbols,stack):
    if self.verbose:
      print "%s.parse_identifier()"%self
    ident = None
    ident = Identifier()
    while 1:
      if lexer.tok == ';':
        self.parse_error(lexer)
      if lexer.tok == ')':
        break
      if lexer.tok == ',':
        break
      if lexer.tok == '[':
        break
      if lexer.kind is None:
        #print "%s.new identifier"%self
        ident = Identifier( lexer.tok )
        lexer.get_token()
        #stack.append( (ident.name, ident) )
        break
      stack.append( (lexer.tok, lexer.kind) )
      lexer.get_token()
    if self.verbose:
      print "%s.parse_identifier()=%s"%(self,repr(ident))
    return ident

class FieldLength(Node):
  """
  """
  pass

class StructDeclarator(Declarator):
  """
  """
  def parse(self,lexer,symbols):
    if lexer.tok != ':':
      Declarator.parse(self,lexer,symbols)
    if lexer.tok == ':':
      lexer.get_token()
      # ConstantExpr
      length = int(lexer.tok)
      #print "length = ",length
      self.append( FieldLength(length) )
      lexer.get_token()

class DeclarationSpecifiers(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)

  def __eq__(self,other):
    " unordered (set/bag) equality "
    if not isinstance(other,Node):
      return 0
    for i in range(len(self)):
      if not self[i] in other:
        return 0
    for i in range(len(other)):
      if not other[i] in self:
        return 0
    return 1

  def parse(self,lexer,symbols):
    self.parse_spec(lexer,symbols)
    self.reverse()

  def parse_spec(self,lexer,symbols):
    typespec = None
    while lexer.tok:
      if isinstance( lexer.kind, TypeAlias ) or\
        isinstance( lexer.kind, BasicType ):
        if typespec is not None:
          self.parse_error(lexer ,"type already specified as %s"\
            %typespec )
        typespec=lexer.kind
        self.append( lexer.kind )
        lexer.get_token()
      elif isinstance( lexer.kind, Qualifier ):
        self.append( lexer.kind )
        lexer.get_token()
      elif isinstance( lexer.kind, StorageClass ):
        self.append( lexer.kind )
        lexer.get_token()
      elif lexer.tok=='struct':
        lexer.get_token()
        self.parse_struct(lexer,symbols)
        break #?
      elif lexer.tok=='union':
        lexer.get_token()
        self.parse_union(lexer,symbols)
        break #?
      elif lexer.tok=='enum':
        lexer.get_token()
        self.parse_enum(lexer,symbols)
        break #?
      elif lexer.kind is None:
        # identifier
        break
      else:
        break

  def parse_struct(self,lexer,symbols):
    if self.verbose:
      print "%s.parse_struct()"%(self)
    node = Struct()
    node.parse(lexer,symbols)
    _node = None
    if node.tag:
      _node = symbols.get_tag( node.tag )
    if _node is not None:
      if not isinstance( _node, Struct ):
        self.parse_error(lexer,"tag defined as wrong kind")
      if len(node)>1:
        if len(_node)>1:
          self.parse_error(lexer,"tag already defined as %s"%_node)
        #symbols.set_tag( node.tag, node )
      #else:
        # refer to the previously defined struct
        ##node = _node
        #node = _node.clone()
    if 0:
      # refer to the previously defined struct
      if len(node)==1:
        _node = symbols.deep_get_tag( node.tag )
        if _node is not None:
          node=_node
      # But what about any future reference to the struct ?
    if node.tag:
      symbols.set_tag( node.tag, node )
    self.append( node )

  def parse_union(self,lexer,symbols):
    if self.verbose:
      print "%s.parse_union(%s)"%(self,node)
    node = Union()
    node.parse(lexer,symbols)
    _node = None
    if node.tag:
      _node = symbols.get_tag( node.tag )
    if _node is not None:
      if not isinstance( _node, Union ):
        self.parse_error(lexer,"tag %s defined as wrong kind"%repr(node.tag))
      if len(node)>1:
        if len(_node)>1:
          self.parse_error(lexer,"tag already defined as %s"%_node)
        #symbols.set_tag( node.tag, node )
      #else:
        #node = _node
    #if len(node)==1:
      #_node = symbols.deep_get_tag( node.tag )
      #if _node is not None:
        #node=_node
    if node.tag:
      symbols.set_tag( node.tag, node )
    self.append( node )

  def parse_enum(self,lexer,symbols):
    if self.verbose:
      print "%s.parse_enum(%s)"%(self,node)
    node = Enum()
    node.parse(lexer,symbols)
    _node = None
    if node.tag:
      _node = symbols.get_tag( node.tag )
    if _node is not None:
      if not isinstance( _node, Enum ):
        self.parse_error(lexer,"tag defined as wrong kind")
      if len(node)>1:
        if len(_node)>1:
          self.parse_error(lexer,"tag already defined as %s"%_node)
        #symbols.set_tag( node.tag, node )
      #else:
        #node = _node
    #if len(node)==1:
      #_node = symbols.deep_get_tag( node.tag )
      #if _node is not None:
        #node=_node
    if node.tag:
      symbols.set_tag( node.tag, node )
    self.append( node )

  def is_typedef(self):
    return self.find(Typedef) is not None

  def needs_declarator(self):
    for node in self:
      if isinstance( node, Struct ):
        return False 
      if isinstance( node, Enum ):
        return False 
      if isinstance( node, Union ):
        return False 
    return True



class TypeSpecifiers(DeclarationSpecifiers):
  " used in ParameterDeclaration "

  def parse_spec(self,lexer,symbols):
    typespec = None
    while lexer.tok:
      if isinstance( lexer.kind, TypeAlias ) or\
        isinstance( lexer.kind, BasicType ):
        if typespec is not None:
          self.parse_error(lexer ,"type already specified as %s"\
            %typespec )
        typespec=lexer.kind
        self.append( lexer.kind )
        lexer.get_token()
      elif isinstance( lexer.kind, Qualifier ):
        self.append( lexer.kind )
        lexer.get_token()
      elif isinstance( lexer.kind, StorageClass ):
        self.parse_error(lexer ,"'%s' cannot appear here"%lexer.tok )
      elif lexer.tok=='struct':
        lexer.get_token()
        self.parse_struct(lexer,symbols)
        break #?
      elif lexer.tok=='union':
        lexer.get_token()
        self.parse_union(lexer,symbols)
        break #?
      elif lexer.tok=='enum':
        lexer.get_token()
        self.parse_enum(lexer,symbols)
        break #?
      elif lexer.kind is None:
        # identifier
        break
      else:
        break


class Initializer(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)

  def parse(self,lexer,symbols):
    self.parse_error(lexer,"not implemented")


class TypeAlias(Node):
  " typedefed things "

  def __init__(self,name,decl=None):
    Node.__init__(self,name)#,decl)
    self.name=name
    self.decl=decl


class Declaration(Node,Parser):
  """
  """
  def __init__(self,*items):
    Node.__init__(self,*items)
    #self.acted=False

  def parse(self,lexer,symbols):
    if not lexer.tok: 
      return
    Parser.parse_enter(self,lexer)
    declspec = DeclarationSpecifiers() 
    declspec.parse(lexer,symbols)
    if len(declspec)==0:
      if lexer.tok == ';':
        lexer.get_token()
        # empty declaration...
        return
      self.parse_error(lexer,
        "expected specifiers, got '%s'"%lexer.tok )
    self.append(declspec)
    while 1:
      decl = Declarator()
      decl.parse(lexer,symbols)
      if len(decl)==0:
        if declspec.needs_declarator():
          self.parse_error(lexer,
            "expected declarator, got '%s'"%lexer.tok )
      self.append(decl)
      ident = decl.ident
      if ident is not None:
      #if len(ident):
        # install symbol
        node = symbols[ident[0]]
        if node is not None:
          # we allow functions to be defined (as same) again
          #print node.deepstr(),'\n', self.deepstr() 
          _node = node.clone()
          _node.delete(Identifier)
          _self = self.clone()
          _self.delete(Identifier)
          if _node != _self:
            self.parse_error(lexer,
              "\n%s\n  already defined as \n%s\n"%\
              (self.deepstr(),node.deepstr()))
        else:
          if self.is_typedef():
            #lexer.mktypedef( ident[0], self )
            tp = TypeAlias(ident[0],decl) 
            lexer.mktypedef( ident[0], tp )
          else:
            symbols[ident[0]] = self
        if lexer.tok == '=':
          # parse initializer
          lexer.get_token()
          init = Initializer()
          init.parse(lexer,symbols)
          ident.append( init ) # as in Enum
      #else: struct, union or enum
      if lexer.tok == ';':
        # no more declarators
        break
      if lexer.tok == '{':
        # ! ahhh, function body !!!
#        sys.stderr.write(
#          "WARNING: function body found at line %s\n"%lexer.lno )
        bcount = 1
        while bcount:
          lexer.get_brace_token()
          if lexer.tok == '}': 
            bcount -= 1
          if lexer.tok == '{': 
            bcount += 1
        lexer.get_token()
        Parser.parse_leave(self,lexer)
        return
      self.consume(lexer,',')
    self.consume(lexer,';')
    Parser.parse_leave(self,lexer)

  def is_typedef(self):
    spec=self[0]
    assert isinstance(spec,DeclarationSpecifiers), self.deepstr()
    return spec.is_typedef()


class ParameterDeclaration(Declaration):
  """
  """
  def parse(self,lexer,symbols):
    typespec = TypeSpecifiers()
    typespec.parse(lexer,symbols)
    self.append(typespec)
    decl = AbstractDeclarator()
    decl.parse(lexer,symbols)
    self.append(decl)
    ident = decl.ident
    if ident is not None and ident[0]:
      node = symbols[ident[0]]
      if node is not None:
        self.parse_error(lexer,
          "%s already defined as %s"%(ident,node))
      else:
        symbols[ident[0]] = self


class StructDeclaration(Declaration):
  """
  """
  def parse(self,lexer,symbols):
    if not lexer.tok: 
      return
    declspec = DeclarationSpecifiers() 
    declspec.parse(lexer,symbols)
    self.append(declspec)
    if len(declspec)==0:
      if lexer.tok == ';':
        lexer.get_token()
        # empty declaration...
        return
      self.parse_error(lexer,
        "expected specifiers, got '%s'"%lexer.tok )
    while 1:
      decl = StructDeclarator()
      decl.parse(lexer,symbols)
      if len(decl)==0:
        self.parse_error(lexer,
          "expected declarator, got '%s'"%lexer.tok )
      self.append(decl)
      ident = decl.ident
      if ident is not None:
        node = symbols[ident[0]]
        if node is not None:
          self.parse_error(lexer ,
            "%s already defined as %s"%(ident,node))
        else:
          if declspec.is_typedef():
            self.parse_error(lexer,"typedef in struct or union")
          else:
            symbols[ident[0]] = self
      if lexer.tok == ';':
        break
      self.consume(lexer,',')
    self.consume(lexer,';')


class TransUnit(Node,Parser):
  """
  """
  def __init__(self,*items,**kw):
    Node.__init__(self,*items,**kw)

  def parse(self,s,verbose=0):
    self.symbols = Symbols()
    self.lexer = Lexer(s,verbose=verbose) #,host=__module__)
    node = None
    while self.lexer.tok:
      node=Declaration()
      node.parse(self.lexer,self.symbols)
      #sys.stderr.write( "# line %s\n"%self.lexer.lno )
      if node:
        self.append(node)
        #node.psource()
        #print node.deepstr(),'\n'
        #node.act()

  def strip(self,files):
    " leave only the declarations from <files> "
    i=0
    while i<len(self):
      if self[i].file in files:
        i=i+1
      else:
        self.pop(i)

  def strip_filter(self,cb):
    " leave only the declarations such that cb(file) "
    i=0
    while i<len(self):
      if cb(self[i].file):
        i=i+1
      else:
        self.pop(i)

  def assert_no_dups(self):
    check={}
    for node in self.nodes():
      assert not check.has_key(id(node))
      check[id(node)]=1



try:
  import NoModule
  import psyco
  from psyco.classes import *
except ImportError:
  class _psyco:
    def jit(self):      pass
    def bind(self, f):  pass
    def proxy(self, f): return f
  psyco = _psyco()
psyco.bind( Lexer.get_token )
psyco.bind( Node )

def run0():
  verbose = 0
  if not sys.argv[1:]:
    s = sys.stdin.read()
  if sys.argv[1:]:
    s = sys.argv[1]
    #if sys.argv[2:]:
      #verbose = int(sys.argv[2])
  if 0:
    import profile
    profile.run('TransUnit(s)','prof.out')
    import pstats
    p=pstats.Stats('prof.out')
    p.strip_dirs().sort_stats(-1).print_stats()
  else:
    node = TransUnit(verbose = 1 )
    node.parse(s)
    node.act(1,1,1)

def run1():
  cstr = "char *(*)() ,"
  node = AbstractDeclarator()
  node.parse( Lexer(cstr,True), Symbols() )
  print node.deepstr()

if __name__=="__main__":
  pass


