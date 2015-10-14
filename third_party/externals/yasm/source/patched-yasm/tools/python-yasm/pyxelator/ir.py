#!/usr/bin/env python
""" ir.py - parse c declarations

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""

import sys
#import cPickle as pickle
import pickle

#from lexer import Lexer
from parse_core import Symbols #, Parser
import node as node_module
import cparse
import genpyx

class Node(genpyx.Node, node_module.Node):
    """
        tree structure
    """
    def __init__( self, *args, **kw ):
        node_module.Node.__init__( self, *args, **kw )
        self._marked = False
    def get_marked( self ):
        return self._marked
    def set_marked( self, marked ):
#    if marked:
#      print "MARK", self
        self._marked = marked
    marked = property( get_marked, set_marked )

#  def __getstate__( self ):
#    return self.__class__, tuple( [ item.__getstate__() for item in self ] )
#  def __setstate__( self, state ):
#    cls, states = state
#    states = list(states)
#    for idx, state in enumerate(states):
#      items[idx] = items[idx].__setstate__( 
    def __getstate__(self):
        return str(self)
    def __setstate__(self, state):
        Node.__init__(self)
        self[:] = eval(state)

#  _unique_id = 0
#  def get_unique_id(cls):
#    Node._unique_id += 1
#    return Node._unique_id
#  get_unique_id = classmethod(get_unique_id)

    def __hash__( self ):
        return hash( tuple([hash(type(self))]+[hash(item) for item in self]) )

    def clone(self):
        l = []
        for item in self:
            if isinstance(item,Node):
                item = item.clone()
            l.append(item)
        return self.__class__(*l, **self.__dict__)

    def init_from( self, other ): # class method ?
        # Warning: shallow init
        self[:] = other
        self.__dict__.update( other.__dict__ )
        return self

#  def is_struct(self):
#    for x in self:
#      if isinstance(x,Node):
#        if x.is_struct():
#          return 1
#    return 0


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
#      print "# "+string.join(self.lines,"\n# ")+"\n"
            print "# "+"\n# ".join(self.lines)+"\n"

    def cstr(self,l=None):
        """
            Build a list of tokens; return the joined tokens string
        """
        if l is None:
            l = []
        for x in self:
            if isinstance(x,Node):
                x.cstr(l)
            else:
                l.insert(0,str(x)+' ')
        s = ''.join(l)
        return s

    def ctype(self): # anon_clone
        " return clone of self without identifiers "
        #print "%s.ctype()"%self
        l=[]
        for x in self:
            if isinstance(x,Node):
                l.append(x.ctype())
            else:
                l.append(x)
        #print "%s.__class__(*%s)"%(self,l)
        return self.__class__(*l, **self.__dict__) # XX **self.__dict__ ?

    def cbasetype(self):
        " return ctype with all TypeAlias's replaced "
        # WARNING: we cache results (so do not mutate self!!)
        l=[]
        for x in self:
            if isinstance(x,Node):
                l.append(x.cbasetype())
            else:
                l.append(x)
        #print "%s.__class__(*%s)"%(self,l)
        return self.__class__(*l, **self.__dict__) # XX **self.__dict__ ?

    def signature( self, tank=None ):
        if tank is None:
            tank = {}
        for node in self.nodes():
            if not tank.has_key( type(node) ):
                tank[ type(node) ] = {}
                type(node).tank = tank[type(node)]
            shape = tuple( [ type(_node).__name__ for _node in node ] )
            if not tank[type(node)].has_key(shape):
                tank[type(node)][shape] = []
            tank[type(node)][shape].append( node )
        return tank
            
    def psig( self, tank=None ):
        if tank is None:
            tank = {}
        tank = self.signature(tank)
        for key in tank.keys():
            print key.__name__
            for shape in tank[key].keys():
                print "  ", shape

#
#################################################

class Named(genpyx.Named, Node):
    " has a .name property "
    def get_name(self):
        if self:
            assert type(self[0])==str
            return self[0]
        return None
    def set_name(self, name):
        if self:
            self[0] = name
        else:
            self.append(name)
    name = property(get_name,set_name)


class BasicType(genpyx.BasicType, Named):
    "float double void char int"
    pass

class Qualifier(genpyx.Qualifier, Named):
    "register signed unsigned short long const volatile inline"
    pass

class StorageClass(genpyx.StorageClass, Named):
    "extern static auto"
    pass

class Ellipses(genpyx.Ellipses, Named):
    "..."
    pass

class GCCBuiltin(genpyx.GCCBuiltin, BasicType):
    "things with __builtin prefix"
    pass

class Identifier(genpyx.Identifier, Named):
    """
        shape = +( str, +ConstExpr )
    """
    #def explain(self):
        #if len(self)==1:
            #return "%s"%self.name
        #else:
            #return "%s initialized to %s"%(self.name,
                #Node(self[1]).explain()) # will handle Initializer

#  def ctype(self):
#    return self.__class__(*self[1:]) #.clone() ?

#  def get_name(self):
#    if self:
#      return self[0]
#  def set_name(self, name):
#    if self:
#      self[0] = name
#    else:
#      self.append(name)
#  name = property(get_name,set_name)

    def cstr(self,l=None):
        if l is None:
            l=[]
        if len(self)>1:
            assert len(self)==2
            l.append( '%s = %s'%(self[0],self[1]) )
        elif len(self)==1:
            l.append( str(self[0]) )
        return " ".join(l)

class TypeAlias(genpyx.TypeAlias, Named):
    """
     typedefed things, eg. size_t 

    """
    def cbasetype( self ):
        node = self.typedef.cbasetype().get_rest()
        return node

class Function(genpyx.Function, Node):
    """
    """
    #def explain(self):
        #if len(self):
            #return "function (%s), returning"%\
                #", ".join( map(lambda x:x.explain(),self) )
        #else:
            #return "function returning"

    def cstr(self,l):
        #print '%s.cstr(%s)'%(self,l)
        _l=[]
        assert len(self)
        i=0
        while isinstance(self[i],Declarator):
            _l.append( self[i].cstr() )
            i=i+1
        l.append( '(%s)'% ', '.join(_l) )
        while i<len(self):
            self[i].cstr(l)
            i=i+1
        return " ".join(l)

    def return_type(self):
        node = self[-1]
        #assert isinstance(node,DeclarationSpecifiers)
        return Declarator( Identifier(), node )
    ret = property(return_type)

    def get_args(self):
        args = [ arg for arg in self[:-1] if not arg.is_void() ]
        return args
    args = property(get_args)

    def arg_types(self):
        return [ AbstractDeclarator().init_from( arg.ctype() ) for arg in self[:-1]]

    def is_varargs(self):
        for node in self.nodes():
            if isinstance(node,Ellipses) or 'va_list' in node:
#        print self, 'is_varargs'
                return True
#    print self, 'is_varargs'
        return False
#    return fn.deepfind(Ellipses) or fn.deepfind('va_list')

    def ctype(self):
        return Function(*self.arg_types()+[self[-1]]) # XX self[-1].ctype


class Pointer(genpyx.Pointer, Node):
    """
    """
    def get_spec(self):
        if type(self[0])==TypeSpecifiers: # isinstance ??
            return self[0]
    spec = property(get_spec)

    #def explain(self):
        #return "pointer to"

    def cstr(self,l):
        assert len(self)
        node=self[0]
        l.insert(0,'*')
        if isinstance(node,Function):
            l.insert(0,'(')
            l.append(')')
        elif isinstance(node,Array):
            l.insert(0,'(')
            l.append(')')
        return Node.cstr(self,l)

class Array(genpyx.Array, Node):
    """
    """
    #def explain(self):
        #s=''
        #if len(self):
            #if type(self[0])==int:
                #s='0 to %s '%(self[0]-1)
        #return "array %sof"%s
    def has_size(self):
        try:
            int(self.size)
            return True
        except:
            return False

    def get_size(self):
        if type(self[-1])==str:
            try: return int(self[-1])
            except: return self[-1]
        return self[-1] # None
    size = property(get_size)

    def get_spec(self):
        if type(self[0])==TypeSpecifiers: # isinstance ??
            return self[0]
    spec = property(get_spec)

    def to_pointer(self):
        node = Pointer()
        node.init_from( self.clone() )
        node.pop() # pop the size element
        return node

    def cstr(self,l):
        if self.size is None:
            l.append('[]')
        else:
            l.append('[%s]'%self.size)
        return Node( *self[:-1] ).cstr( l )

class Tag(genpyx.Tag, Named):
    " the tag of a Struct, Union or Enum "
    pass

class Taged(genpyx.Taged, Node):
    "Struct, Union or Enum "
    def get_tag(self):
        if len(self):
            tag = self[0]
            assert type(tag)==Tag # isinstance ??
        else:
            tag = None
        return tag
    def set_tag(self,tag):
        if len(self):
            self[0] = tag
        else:
            self.append(tag)
    tag = property( get_tag, set_tag )
    def has_members(self):
        return len(self)>1 # more than just a tag
    def get_members(self):
        return self[1:]
    members = property(get_members) # fields ?

    def ctype(self):
        if not self.tag.name:
            #print "# WARNING : anonymous struct " # OK i think
            return self.clone()
#    self = self.clone()
#    return self[:1] # just the tag
        return self.__class__( self.tag, **self.__dict__ ) # just the Tag
#    return self.__class__( *self, **self.__dict__ )

    def cbasetype(self):
        return self.ctype() # is this enough ???
#    return Node.cbasetype(self) # XX lookup my tag if i am empty ..?


class Compound(genpyx.Compound, Taged):
    "Struct or Union"

    def cstr(self,_l=None):
        assert isinstance( self[0], Tag )
        tag=''
        if len(self[0]):
            tag=' '+self[0][0]
        if isinstance(self,Struct):
            l=[ 'struct%s '%tag ]
        elif isinstance(self,Union):
            l=[ 'union%s '%tag ]
        if len(self)>1:
            l.append(' { ')
            for decl in self[1:]:
                l.append( decl.cstr()+"; " )
            l.append('} ')
        if _l is None:
            _l=[]
        while l:
            _l.insert( 0, l.pop() )
        # XX empty struct with no tag -> "struct" XX
        return "".join( _l )

    def ctype(self):
        tp = Taged.ctype(self)
        for i in range(1,len(tp)):
            tp[i] = StructDeclarator().init_from( tp[i] )
        return tp

class Struct(genpyx.Struct, Compound):
    """
    """
    pass


class Union(genpyx.Union, Compound):
    """
    """
    pass


class Enum(genpyx.Enum, Taged):
    """
    """
    def cstr(self,_l=None):
        assert isinstance( self[0], Tag )
        tag=''
        if len(self[0]):
            tag=' '+self[0][0]
        l=[ 'enum%s '%tag ]
        if len(self)>1:
            l.append(' { ')
            for node in self[1:]:
                l.append( node.cstr()+', ' )
            l.append('} ')
        if _l is None:
            _l=[]
        while l:
            _l.insert( 0, l.pop() )
        return ''.join( _l )

class Declarator(genpyx.Declarator, Node):
    """
    """

    def __eq__(self,other):
        " unordered equality "
        # ordering sometimes gets lost when we do a cbasetype
        if not isinstance(other,Node):
            return False
        a, b = self[:], other[:]
        a.sort()
        b.sort()
        return a == b

    def __hash__( self ):
        hs = [hash(item) for item in self]
        hs.sort()
        return hash( tuple([hash(type(self))]+hs) )

    def transform(self):
        return

    def get_identifier(self):
        if len(self)>1:
            return self[0]
    def set_identifier(self, identifier):
        if len(self)>1:
            self[0] = identifier
        else:
            self.insert(0,identifier)
    identifier = property(get_identifier,set_identifier)

    def get_spec(self):
        spec = self[-1]
        if type(spec)==TypeSpecifiers: # isinstance ??
            return spec
    spec = property(get_spec)

    def get_type_alias(self):
        if self.spec:
            if isinstance(self.spec[0], TypeAlias):
                return self.spec[0]
    type_alias = property(get_type_alias)

    def get_tagged(self):
        if self.spec:
            return self.spec.tagged # i am a tagged
    tagged = property(get_tagged)

    def get_compound(self):
        if self.spec:
            return self.spec.compound # i am a compound
    compound = property(get_compound)

    def get_struct(self):
        if self.spec:
            return self.spec.struct # i am a struct
    struct = property(get_struct)

    def get_union(self):
        if self.spec:
            return self.spec.union # i am a union
    union = property(get_union)

    def get_enum(self):
        if self.spec:
            return self.spec.enum # i am an enum
    enum = property(get_enum)

    def get_function(self):
        if len(self)>1 and type(self[1])==Function: # isinstance ??
            return self[1]
    function = property(get_function)

    def get_pointer(self):
        if len(self)>1 and type(self[1])==Pointer: # isinstance ??
            return self[1]
    pointer = property(get_pointer)

    def get_array(self):
        if len(self)>1 and type(self[1])==Array: # isinstance ??
            return self[1]
    array = property(get_array)

    def get_name(self):
        if self.identifier:
            return self.identifier.name
    def set_name(self, name):
        assert self.identifier is not None
        self.identifier.name = name
    name = property(get_name, set_name)

    def get_rest(self): # XX needs a better name
        if len(self)>1:
            return self[1]
        return self[0]

    def pointer_to( self ):
        " return Declarator pointing to self's type "
        decl = Declarator(Identifier(), Pointer(self.get_rest().clone()))
        return decl

    def deref( self ):
        " return (clone of) Declarator that self is pointing to "
        node = self.ctype() # clone
        pointer = node.pointer or node.array
        assert pointer, "cannot dereference non-pointer"
        node[1:2] = pointer
        return node

    def is_void(self):
        return self.spec and BasicType('void') in self.spec

    def is_pointer_to_fn(self):
        return self.pointer and self.deref().function

    def is_pointer_to_char(self):
#    return self.ctype() == TransUnit("char *a;").transform()[0].ctype()
        node = self.pointer or self.array
        if node:
            spec = node.spec 
            if spec and BasicType('char') in spec and not BasicType('unsigned') in spec:
                return True
        return False

    def is_callback(self):
        " i am a pointer to a function whose last arg is void* "
        if self.is_pointer_to_fn():
            fn = self.deref().function
            if fn.args:
                arg = fn.args[-1]
                if arg.pointer and arg.deref().is_void():
                    return True

    def is_complete( self, tag_lookup ):
        if self.tagged and self.tagged.tag.name in tag_lookup and not tag_lookup[self.tagged.tag.name].has_members():
            return False
        return True

    def is_primative( self ):
        "i am a char,short,int,float,double... "
        spec = self.cbasetype().spec
        return spec and spec.find(BasicType)

    def is_pyxnative( self ):
        # pyrex handles char* too
        # but i don't know if we should make this the default
        # sometimes we want to send a NULL, so ... XXX
        self = self.cbasetype()
        if self.is_void():
            return False
        if self.is_primative():
            return True
        if self.enum:
            return True
#    pointer = None
#    if self.pointer:
#      pointer = self.pointer
#    elif self.array:
#      pointer = self.array
#    if pointer and pointer.spec:
#      spec = pointer.spec
#      if BasicType("char") in spec and not Qualifier("unsigned") in spec:
#        # char*, const char*
##        print self.deepstr()
#        return True
        return False

    def cstr(self,l=None):
        return Node.cstr(self,l).strip()

    def ctype(self):
        decl=Declarator()
        decl.init_from( self.clone() )
        decl.identifier = Identifier()
        for i in range(1,len(decl)):
            decl[i]=decl[i].ctype()
        return decl

    def cbasetype(self):
        # WARNING: we cache results (so do not mutate self!!)
        try:
            # this cache improves performance by 50%
            return self.__cbasetype.clone()
        except AttributeError:
            pass
        decl = self.ctype() # gets rid of Identifier names
        for i, node in enumerate(decl):
            decl[i] = decl[i].cbasetype()
#    return decl.get_rest()

        done = False
        while not done:
            done = True
            nodes = decl.deepfilter( TypeSpecifiers )
            for node in nodes:
                if node.deepfind( TypeSpecifiers ) != node:
                    # this node has another TypeSpecifier;
                    decl.expose_node( node )
                    done = False
                    break # start again...

        # each TypeSpecifier needs to absorb primitive siblings (StorageClass, BasicType etc.)
        nodes = decl.deepfilter( TypeSpecifiers )
        for node in nodes:
            parent = decl.get_parent(node)
            i = 0
            while i < len(parent):
                assert not type(parent[i]) in (TypeAlias, Enum, Struct, Union)
                if type(parent[i]) in (StorageClass, BasicType, Qualifier):
                    node.append( parent.pop(i) )
                else:
                    i = i + 1

        self.__cbasetype = decl.clone()
        return decl

    def invalidate(self):
        # flush cache, etc.
        try:
            del self.__cbasetype
        except AttributeError:
            pass

    def declare_str(self,name):
        " return c string declaring name with same type as self "
        tp = self.ctype()
        tp.name = name
        return tp.cstr()+";"

class Typedef(genpyx.Typedef, Declarator):
    def cstr(self,l=None):
        return 'typedef ' + Declarator.cstr(self,l) #.strip()

class AbstractDeclarator(genpyx.AbstractDeclarator, Declarator):
    """ used in Function; may lack an identifier """

    #def cstr(self,l=None):
        #return Node.cstr(self,l)

#  def ctype(self):
#    # _type_ ignores the name of our identifier
#    return Node.ctype(self)

class FieldLength(genpyx.FieldLength, Node):
    """
    """
    #def explain(self):
        #return ""

    def cstr(self,l):
        l.append(':%s'%self[0]) 

class StructDeclarator(genpyx.StructDeclarator, Declarator): # also used in Union
    """
    """
    #def explain(self):
        #flen = self.find(FieldLength)
        #if flen is not None:
            #i = self.index(flen)
            #self.pop(i)
            #s = Declarator.explain(self)
            #self.insert(i,flen)
            #width = flen[0]
            #if width > 0:
                #return s+" bitfield %s wide"%width
            #else:
                #return s+" alignment bitfield"
        #else:
            #return Declarator.explain(self)
#  def ctype(self):
#    return self   
    def get_field_length(self):
        if len(self)>1 and isinstance( self[1], FieldLength ):
            return self[1]
    field_length = property(get_field_length)


class DeclarationSpecifiers(genpyx.DeclarationSpecifiers, Node):
#class TypeSpecifiers(Node):
    """
    """
    def __eq__(self,other):
        " unordered equality "
        if not isinstance(other,Node):
            return False
        a, b = self[:], other[:]
        a.sort()
        b.sort()
        return a == b

    def __hash__( self ):
        hs = [hash(item) for item in self]
        hs.sort()
        return hash( tuple([hash(type(self))]+hs) )

#  def is_struct(self):
#    return self.find(Struct) is not None


class TypeSpecifiers(genpyx.TypeSpecifiers, DeclarationSpecifiers):
    """
    """
    def get_tagged(self):
        if self and isinstance(self[0],Taged):
            return self[0]
    tagged = property(get_tagged)

    def get_compound(self):
        if self and isinstance(self[0],Compound):
            return self[0]
    compound = property(get_compound)

    def get_struct(self):
        if self and isinstance(self[0],Struct):
            return self[0]
    struct = property(get_struct)

    def get_union(self):
        if self and isinstance(self[0],Union):
            return self[0]
    union = property(get_union)

    def get_enum(self):
        if self and isinstance(self[0],Enum):
            return self[0]
    enum = property(get_enum)

    def cbasetype(self):
        node = Node.cbasetype(self)
#    node.expose( TypeSpecifiers )
#    if node.deepfind(TypeSpecifiers) != node:
        return node

class Initializer(genpyx.Initializer, Node):
    """
    """
    pass



class Declaration(genpyx.Declaration, Node):
    """
    """
    def do_spec(self):
        " distribute DeclarationSpecifiers over each Declarator "
        spec=self[0]
        assert isinstance(spec,DeclarationSpecifiers), spec.deepstr()
        self.pop(0)
        for declarator in self:
            assert isinstance(declarator,Declarator)
            #if isinstance(declarator,DeclarationSpecifiers #huh?
            ##for node in spec:
                ##declarator.append(node.clone())
            declarator.append(spec)

    def transform(self):
        # children go first 
        for node in self.nodes():
            if isinstance(node,Declaration):
                node.do_spec()
            node.file = self.file # overkill ?
        self.expose(Declaration)

    #def explain(self):
        #return string.join([x.explain() for x in self],", ") 
        #return string.join(map(lambda x:x.explain(),self),", ") 


class ParameterDeclaration(genpyx.ParameterDeclaration, Declaration):
    """
    """
    pass


class StructDeclaration(genpyx.StructDeclaration, Declaration):
    """
    """
    pass


class TransUnit(genpyx.TransUnit, Node):
    """
        Top level node.
    """
    def __init__( self, item ): # XX __init__ uses different signature ! XX
        if type(item)==str:
            node = cparse.TransUnit()
            node.parse(item)
        else:
            node = item
            assert isinstance( node, cparse.TransUnit ), str(node)
        Node.__init__(self)
        self[:] = [ self.convert(child) for child in node ]
        self.__dict__.update( node.__dict__ )
        assert "name" not in node.__dict__

        self.syms = {} # map identifier names to their Declarator's
        self.typedefs = {} # map names to Typedef's
        self.tag_lookup = {} # map struct, union, enum tags to Taged's

        # XX should call transform here XX

#    print self.deepstr()
    def __getstate__( self ):
        nodes = tuple( [ repr(node) for node in self ] )
        typedefs = tuple( [ (key,repr(val)) for key,val in self.typedefs.items() ] )
        return nodes, typedefs
    def __setstate__( self, state ):
        Node.__init__(self)
        nodes, typedefs = state
        nodes = [ eval(node) for node in nodes ]
        self[:] = nodes
        typedefs = [ (key,eval(val)) for key,val in typedefs ]
        self.typedefs = dict(typedefs)

    def convert( self, node ):
#    name = node.__class__.__name__
#    cls = globals()[ name ]
        cls = cls_lookup[ type(node) ]
        _node = cls()
        for child in node:
            if isinstance(child, node_module.Node):
                child = self.convert( child )
            else:
                assert child is None or type(child) in (str, int), type(child)
            _node.append( child )
        _node.__dict__.update( node.__dict__ )
        return _node

    def strip(self,files):
        " leave only the declarations from <files> "
        i=0
        while i<len(self):
            if self[i].file in files:
                i=i+1
            else:
                self.pop(i)

    def mark(self,cb,verbose=False):
        " mark our child nodes such that cb(node).. mark dependants too. prune unmarked objects. "
        # mark the nodes:
        for node in self:
            node.marked = cb(self, node)
            if verbose and node.marked:
                print '1:', node.cstr()
        # propagate dependancy:
        i=len(self)
        while i:
            i-=1 # we go backwards
            for node in self[i].nodes(): # bottom-up search
                if verbose and self[i].marked and not node.marked:
                    print '2:', str(node), '<--', self[i].cstr()
                node.marked = self[i].marked or node.marked
                if type(node)==TypeAlias:
                    if verbose and node.marked and not node.typedef.marked:
                        print '3:', node.typedef.cstr(), '<--', node.cstr()
                    node.typedef.marked = node.typedef.marked or node.marked
                if isinstance(node, Taged):
                    if node.tag.name in self.tag_lookup:
                        _node = self.tag_lookup[ node.tag.name ] # look-up the def'n
                        if verbose and node.marked and not _node.marked: 
                            print '4:', _node.cstr(), '<--', self[i].cstr()
#            _node.marked = _node.marked or self[i].marked
                        _node.marked = _node.marked or node.marked
#          else:
#            # this guy has no tag
#            print "lost tag:", self[i].cstr()

                 # XX struct defs acquire marks from members, but XX
                 # XX ordinary definitions do not                 XX
#        if node.marked and not self[i].marked:
#          # one of my descendants is marked
#          if verbose:
#            print '5:', self[i].cstr(), '<--', node.cstr()
#          self[i].marked = True
#    if verbose:
#      for node in self:
#        print '-'*79
#        if node.enum:
#          print str(node.marked) + ': ' + node.cstr()
        # prune:
        f = open(".tmp/pruned.txt","w")
        f.write("// This file autogenerated by '%s' .\n"%__file__)
        f.write("// List of functions pruned from parse tree, for various reasons.\n\n")
        i=0
        while i<len(self):
            if not self[i].marked:
                if verbose: print 'pop:', self[i].cstr()
                f.write( self[i].cstr() + "\n" )
                self.pop(i)
#      elif self[i].compound:
#        # XXXX for now, rip out all struct members XXXX
#        self[i].compound[1:] = [] # XX encapsulation
#        i = i + 1
            else:
                i = i + 1
        for key, value in self.syms.items():
            if not value.marked:
                del self.syms[key]
        for key, value in self.typedefs.items():
            if not value.marked:
                del self.typedefs[key]
        for key, value in self.tag_lookup.items():
            if not value.marked:
                del self.tag_lookup[key]
#    sys.exit(1)

    def assert_no_dups(self):
        check={}
        for node in self.nodes():
            assert not check.has_key(id(node))
            check[id(node)]=1

    def transform(self, verbose=False, test_parse=False, test_types=False ):
        i=0
        while i < len(self):
            if verbose: print "##"*25
            declaration=self[i]

            if verbose: declaration.psource()
            if verbose: print declaration.deepstr(),'\n'
            assert isinstance(declaration,Declaration)
            if verbose: print "# expose declarators from declaration"

            # STAGE 1
            declaration.transform()

            if verbose: print declaration.deepstr(),'\n'
            self[i:i+1] = declaration # expose declarators from declaration

            for j in range(len(declaration)):
                declarator=self[i]

                assert isinstance(declarator,Declarator)
                if verbose: print "# declarator.transform()"

                # STAGE 2
                declarator.transform()

                if verbose: print declarator.deepstr(),'\n'
                if verbose: print "# self.visit_declarator(declarator)"

                # STAGE 3
                self[i] = declarator = self.visit_declarator(declarator)

                # STAGE 4 
                if declarator.name:
                    if isinstance(declarator, Typedef):
                        if verbose: print "# typedef %s" % declarator.name
                        self.typedefs[ declarator.name ] = declarator
                    else:
                        if verbose: print "# sym %s" % declarator.name
                        self.syms[ declarator.name ] = declarator

                for node in declarator.nodes():
                    if isinstance(node,Taged) and node.tag.name:
                        assert type(node.tag.name)==str, node.deepstr()
                        taged = self.tag_lookup.get( node.tag.name, None )
                        if taged is None:
                            if verbose: print "# tag lookup %s = %s" % (declarator.name, node.tag.name)
                            self.tag_lookup[ node.tag.name ] = node
                        elif not taged.has_members():
                            # this is (maybe) the definition of this tag
                            if verbose: print "# definition %s = %s" % (declarator.name, node.tag.name)
                            self.tag_lookup[ node.tag.name ] = node 

                # Annotate the TypeAlias's
                for node in declarator.deepfilter( TypeAlias ):
                    name = node[0]
                    assert type( name ) == str
                    node.typedef = self.typedefs[ name ]

                if verbose: print declarator.deepstr(),'\n'
                #print declarator.ctype().deepstr(),'\n'
                #assert declarator.clone() == declarator

                ###################################################
                # TESTS:
                if test_parse:
                    # test that parse of cstr gives same answer
                    cstr = declarator.cstr()+';\n'
                    if verbose: print '# '+cstr.replace('\n','\n# ')
                    #print
                    if isinstance(declarator,Typedef):
                        name = declarator[0][0]
                        assert type(name)==str
                        self.lexer.rmtypedef( name )
                    declaration = cparse.Declaration()
                    self.lexer.lex( cstr )
                    #print self.lexer.err_string()
                    declaration.parse(  self.lexer, Symbols() ) # use new name-space
                    #declaration.parse(  Lexer( cstr ), Symbols() )
                    declaration = self.convert(declaration)
                    declaration.transform()
                    assert len(declaration)==1
                    decl=declaration[0]
                    decl.transform()
                    decl = self.visit_declarator(decl)
                    if decl!=declarator:
                        if verbose: print "#???????????"
                        if verbose: print decl.deepstr(),'\n\n'
                        #if verbose: print declaration.deepstr(),'\n\n'
                        #assert 0
                    elif verbose: print '# OK\n'

                if test_types:
                    node = declarator.ctype()
                    declare_str= node.declare_str("my_name")
                    if verbose: print "# declarator.ctype() "
                    if verbose: print node.deepstr(),"\n"
                    if verbose: print "#",declare_str.replace('\n','\n# '), '\n'

                i=i+1
        return self

    def visit(self,node):
        #print 'visit(%s)'%node
        for _node in node:
            if isinstance(_node,Declarator):
                _node = self.visit_declarator(_node) # XX replace _node
            elif isinstance(_node,Node):
                _node = self.visit(_node) # XX replace _node
        return node

    def visit_declarator(self,decl):
        assert isinstance(decl,Declarator)

        # STAGE 3.a
        tp = decl.deepfind(Typedef)
        if tp is not None:
            decl.deeprm(tp)
            tp.init_from( decl ) # warning: shallow init
            decl = tp

        # STAGE 3.b
        i=len(decl)
        # accumulate nodes (they become the children of decl)
        children=[]
        while i:
            i=i-1
            node=decl.pop(i)
            if isinstance(node,Declarator):
                node = self.visit_declarator(node) # replace node
            else:
                node = self.visit(node) # replace node
            if isinstance(node,Pointer):
                node+=children
                children=[node]
            elif isinstance(node,Function):
                node+=children
                children=[node]
            elif isinstance(node,Array):
                while children:
                    node.insert(0,children.pop())
                children=[node]
                # array size (if any) at end
            #elif isinstance(node,Identifier):
                #node+=children
                #children=[node]
            else:
                # accumulate
                children.insert(0,node)
        decl[:]=children
        return decl

    cstr = None
    ctype = None
    cbasetype = None


# remap the global class definitions in genpyx to
# point to the definitions in this module
gbl = globals()
for key, val in gbl.items():
    if type(val)==type:
        if issubclass(val,Node):
            setattr( genpyx, key, val )
assert genpyx.Node == Node

cls_lookup = {
#  Node : Node ,
    cparse.BasicType : BasicType ,
    cparse.Qualifier : Qualifier ,
    cparse.StorageClass : StorageClass ,
    cparse.Ellipses : Ellipses ,
    cparse.GCCBuiltin : GCCBuiltin ,
    cparse.Identifier : Identifier ,
    cparse.TypeAlias : TypeAlias ,
    cparse.Function : Function ,
    cparse.Pointer : Pointer ,
    cparse.Array : Array ,
    cparse.Tag : Tag ,
    cparse.Compound : Compound ,
    cparse.Struct : Struct ,
    cparse.Union : Union ,
    cparse.Enum : Enum ,
    cparse.Declarator : Declarator ,
    cparse.Typedef : Typedef ,
    cparse.AbstractDeclarator : AbstractDeclarator ,
    cparse.FieldLength : FieldLength ,
    cparse.StructDeclarator : StructDeclarator ,
    cparse.DeclarationSpecifiers : TypeSpecifiers ,
    cparse.TypeSpecifiers : TypeSpecifiers ,
    cparse.Initializer : Initializer ,
    cparse.Declaration : Declaration ,
    cparse.ParameterDeclaration : ParameterDeclaration ,
    cparse.StructDeclaration : StructDeclaration ,
    cparse.TransUnit : TransUnit ,
}


