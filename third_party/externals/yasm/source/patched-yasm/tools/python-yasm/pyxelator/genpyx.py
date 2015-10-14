#!/usr/bin/env python
""" genpyx.py - parse c declarations

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

This is a module of mixin classes for ir.py .

Towards the end of ir.py our global class definitions
are remapped to point to the class definitions in ir.py .
So, for example, when we refer to Node we get ir.Node .

"""

import sys
from datetime import datetime

# XX use this Context class instead of all those kw dicts !! XX
class Context(object):
    " just a record (struct) "
    def __init__( self, **kw ):
        for key, value in kw.items():
            setattr( self, key, value )
    def __getattr__( self, name ):
        return None # ?
    def __getitem__( self, name ):
        return getattr(self, name)

class OStream(object):
    def __init__( self, filename=None ):
        self.filename = filename
        self.tokens = []
        self._indent = 0
    def put( self, token="" ):
        assert type(token) is str
        self.tokens.append( token )
    def startln( self, token="" ):
        assert type(token) is str
        self.tokens.append( '    '*self._indent + token )
    def putln( self, ln="" ):
        assert type(ln) is str
        self.tokens.append( '    '*self._indent + ln + '\n')
    def endln( self, token="" ):
        assert type(token) is str
        self.tokens.append( token + '\n')
    def indent( self ):
        self._indent += 1
    def dedent( self ):
        self._indent -= 1
        assert self._indent >= 0, self._indent
    def join( self ):
        return ''.join( self.tokens )
    def close( self ):
        s = ''.join( self.tokens )
        f = open( self.filename, 'w' )
        f.write(s)

#
###############################################################################
#

class Node(object):
    """
        tree structure
    """
    _unique_id = 0
    def get_unique_id(cls):
        Node._unique_id += 1
        return Node._unique_id
    get_unique_id = classmethod(get_unique_id)

# XX toks: use a tree of tokens: a list that can be push'ed and pop'ed XX
    def pyxstr(self,toks=None,indent=0,**kw):
        """
            Build a list of tokens; return the joined tokens string
        """
        if toks is None:
            toks = []
        for x in self:
            if isinstance(x,Node):
                x.pyxstr(toks, indent, **kw)
            else:
                toks.insert(0,str(x)+' ')
        s = ''.join(toks)
        return s

#
#################################################

class Named(object):
    "has a .name property"
    pass

class BasicType(object):
    "float double void char int"
    pass

class Qualifier(object):
    "register signed unsigned short long const volatile inline"
    def pyxstr(self,toks=None,indent=0,**kw):
        if toks is None:
            toks = []
        x = self[0]
        if x not in ( 'const','volatile','inline','register'): # ignore these
            toks.insert(0,str(x)+' ')
        s = ''.join(toks)
        return s

class StorageClass(object):
    "extern static auto"
    def pyxstr(self,toks=None,indent=0,**kw):
        return ""

class Ellipses(object):
    "..."
    pass

class GCCBuiltin(BasicType):
    "things with __builtin prefix"
    pass

class Identifier(object):
    """
    """
    def pyxstr(self,toks=None,indent=0,**kw):
        if toks is None:
            toks=[]
        if self.name:
            toks.append( self.name )
        return " ".join(toks)

class TypeAlias(object):
    """
     typedefed things, eg. size_t 
    """
    def pyxstr(self,toks=None,indent=0,cprefix="",**kw):
        if toks is None:
            toks = []
        for x in self:
            if isinstance(x,Node):
                x.pyxstr(toks, indent, cprefix=cprefix, **kw)
            else:
                s = str(x)+' '
                if cprefix:
                    s = cprefix+s
                toks.insert(0,s)
        s = ''.join(toks)
        return s

class Function(object):
    """
    """
    def pyxstr(self,toks,indent=0,**kw):
        #print '%s.pyxstr(%s)'%(self,toks)
        _toks=[]
        assert len(self)
        i=0
        while isinstance(self[i],Declarator):
            if not self[i].is_void():
                _toks.append( self[i].pyxstr(indent=indent, **kw) )
            i=i+1
        toks.append( '(%s)'% ', '.join(_toks) )
        while i<len(self):
            self[i].pyxstr(toks, indent=indent, **kw)
            i=i+1
        return " ".join(toks)

class Pointer(object):
    """
    """
    def pyxstr(self,toks,indent=0,**kw):
        assert len(self)
        node=self[0]
        toks.insert(0,'*')
        if isinstance(node,Function):
            toks.insert(0,'(')
            toks.append(')')
        elif isinstance(node,Array):
            toks.insert(0,'(')
            toks.append(')')
        return Node.pyxstr(self,toks,indent, **kw)

class Array(object):
    """
    """
    def pyxstr(self,toks,indent=0,**kw):
        if self.size is None:
            toks.append('[]')
        else:
            try:
                int(self.size)
                toks.append('[%s]'%self.size)
            except:
                toks.append('[]')
        return Node( *self[:-1] ).pyxstr( toks,indent, **kw )

class Tag(object):
    " the tag of a Struct, Union or Enum "
    pass

class Taged(object):
    "Struct, Union or Enum "
    pass

class Compound(Taged):
    "Struct or Union"
    def pyxstr(self,_toks=None,indent=0,cprefix="",shadow_name=True,**kw):
        if _toks is None:
            _toks=[]
        names = kw.get('names',{})
        kw['names'] = names
        tag_lookup = kw.get('tag_lookup')
        if self.tag:
            tag=self.tag.name
        else:
            tag = ''
        if isinstance(self,Struct):
            descr = 'struct'
        elif isinstance(self,Union):
            descr = 'union'
        _node = names.get(self.tag.name,None)
        if ( _node is not None and _node.has_members() ) or \
                ( _node is not None and not self.has_members() ):
            descr = '' # i am not defining myself here
        #print "Compound.pyxstr", tag
        #print self.deepstr()
        if descr:
            if cprefix and shadow_name:
                tag = '%s%s "%s"'%(cprefix,tag,tag)
            elif cprefix:
                tag = cprefix+tag
            toks = [ descr+' '+tag ] # struct foo
            if self.has_members():
                toks.append(':\n')
                for decl in self[1:]: # XX self.members
                    toks.append( decl.pyxstr(indent=indent+1, cprefix=cprefix, shadow_name=shadow_name, **kw)+"\n" ) # shadow_name = False ?
            #elif not tag_lookup.get( self.tag.name, self ).has_members():
                # define empty struct here, it's the best we're gonna get
                #pass
        else:
            if cprefix: # and shadow_name:
                tag = cprefix+tag
            toks = [ ' '+tag+' ' ] # foo
        while toks:
            _toks.insert( 0, toks.pop() )
        return "".join( _toks )

class Struct(Compound):
    """
    """
    pass

class Union(Compound):
    """
    """
    pass


class Enum(Taged):
    """
    """
    def pyxstr(self,_toks=None,indent=0,cprefix="",shadow_name=True,**kw):
        if _toks is None:
            _toks=[]
        names = kw.get('names',{})
        kw['names'] = names
        if self.tag:
            tag=self.tag.name
        else:
            tag = ''
        _node = names.get(self.tag.name,None)
        if ( _node is not None and _node.has_members() ) or \
                ( _node is not None and not self.has_members() ):
            descr = '' # i am not defining myself here
        else:
            descr = 'enum'
        if descr:
        #if not names.has_key(self.tag.name):
            toks = [ descr+' '+tag ] # enum foo
            toks.append(':\n')
            idents = [ ident for ident in self.members if ident.name not in names ]
            for ident in idents:
                if cprefix and shadow_name:
                    ident = ident.clone()
                    ident.name = '%s%s "%s"' % ( cprefix, ident.name, ident.name )
                #else: assert 0
                toks.append( '    '+'    '*indent + ident.pyxstr(**kw)+"\n" )
                names[ ident.name ] = ident
            if not idents:
                # empty enum def'n !
                #assert 0 # should be handled by parents...
                toks.append( '    '+'    '*indent + "pass\n" )
        else:
            toks = [ ' '+tag+' ' ] # foo
        while toks:
            _toks.insert( 0, toks.pop() )
        return "".join( _toks )

class Declarator(object):
    def is_pyxnative( self ):
        # pyrex handles char* too
        # but i don't know if we should make this the default
        # sometimes we want to send a NULL, so ... XX
        self = self.cbasetype() # WARNING: cbasetype may be cached
        if self.is_void():
            return False
        if self.is_primative():
            return True
        if self.enum:
            return True
        #pointer = None
        #if self.pointer:
            #pointer = self.pointer
        #elif self.array:
            #pointer = self.array
        #if pointer and pointer.spec:
            #spec = pointer.spec
            #if BasicType("char") in spec and not Qualifier("unsigned") in spec:
                # char*, const char*
                ##print self.deepstr()
                #return True
        return False

    def _pyxstr( self, toks, indent, cprefix, use_cdef, shadow_name, **kw ):
        " this is the common part of pyxstr that gets called from both Declarator and Typedef "
        names = kw.get('names',{}) # what names have been defined ?
        kw['names']=names
        for node in self.nodes(): # depth-first
            if isinstance(node,Taged):
                #print "Declarator.pyxstr", node.cstr()
                if not node.tag.name:
                    node.tag.name = "_anon_%s" % Node.get_unique_id()
                _node = names.get(node.tag.name,None)
                #tag_lookup = kw.get('tag_lookup')
                #other = tag_lookup.get(node.tag.name, node)
                #if ((_node is None and (not isinstance(other,Compound) or not other.has_members()))
                #    or node.has_members()):
                if _node is None or node.has_members():
                    # either i am not defined at all, or this is my _real_ definition
                    # emit def'n of this node
                    #if isinstance(self,Typedef):
                        #toks.append( '    '*indent + 'ctypedef ' + node.pyxstr(indent=indent, cprefix=cprefix, shadow_name=shadow_name, **kw).strip() )
                    #else:
                    toks.append( '    '*indent + 'cdef ' + node.pyxstr(indent=indent, cprefix=cprefix, shadow_name=shadow_name, **kw).strip() )
                    names[ node.tag.name ] = node
            elif isinstance(node,GCCBuiltin) and node[0] not in names:
                #toks.append( '    '*indent + 'ctypedef long ' + node.pyxstr(indent=indent, **kw).strip() + ' # XX ??'    ) # XX ??
                toks.append( '    '*indent + 'struct __unknown_builtin ' )
                toks.append( '    '*indent + 'ctypedef __unknown_builtin ' + node.pyxstr(indent=indent, **kw).strip() )
                names[ node[0] ] = node
            for idx, child in enumerate(node):
                if type(child)==Array and not child.has_size():
                    # mutate this mystery array into a pointer XX method: Array.to_pointer()
                    node[idx] = Pointer()
                    node[idx].init_from( child ) # warning: shallow init
                    node[idx].pop() # pop the size element

    def pyxstr(self,toks=None,indent=0,cprefix="",use_cdef=True,shadow_name=True,**kw):
        " note: i do not check if my name is already in 'names' "
        self = self.clone() # <----- NOTE
        toks=[]
        names = kw.get('names',{}) # what names have been defined ?
        kw['names']=names

        self._pyxstr( toks, indent, cprefix, use_cdef, shadow_name, **kw )

        if self.name and not names.has_key( self.name ):
            names[ self.name ] = self
        if self.identifier is not None:
            comment = ""
            if self.name in python_kws:
                comment = "#"
            if cprefix and use_cdef and shadow_name:
                # When we are defining this guy, we refer to it using the pyrex shadow syntax.
                self.name = '%s%s "%s" ' % ( cprefix, self.name, self.name )
            cdef = 'cdef '
            if not use_cdef: cdef = '' # sometimes we don't want the cdef (eg. in a cast)
            # this may need shadow_name=False:
            toks.append( '    '*indent + comment + cdef + Node.pyxstr(self,indent=indent, cprefix=cprefix, **kw).strip() ) # + "(cprefix=%s)"%cprefix)
        #else: i am just a struct def (so i already did that) # huh ?? XX bad comment
        return ' \n'.join(toks)

    def pyxsym(self, ostream, names=None, tag_lookup=None, cprefix="", modname=None, cobjects=None):
        assert self.name is not None, self.deepstr()
        ostream.putln( '# ' + self.cstr() )
# This cdef is no good: it does not expose a python object
# and we can't reliably set a global var
        #ostream.putln( 'cdef %s %s' % ( self.pyx_adaptor_decl(cobjects), self.name ) ) # _CObject
        #ostream.putln( '%s = %s()' % (self.name, self.pyx_adaptor_name(cobjects)) )
        #ostream.putln( '%s.p = <void*>&%s' % (self.name, cprefix+self.name) )
        ## expose a python object:
        #ostream.putln( '%s.%s = %s' % (modname,self.name, self.name) )
        ostream.putln( '%s = %s( addr = <long>&%s )' % (self.name, self.pyx_adaptor_name(cobjects), cprefix+self.name) )
        return ostream


class Typedef(Declarator):
    def pyxstr(self,toks=None,indent=0,cprefix="",use_cdef=True,shadow_name=True,**kw): # shadow_name=True
        " warning: i do not check if my name is already in 'names' "
        assert shadow_name == True
        self = self.clone() # <----- NOTE
        toks=[]
        names = kw.get('names',{}) # what names have been defined ?
        kw['names']=names

        #if self.tagged and not self.tagged.tag.name:
            ## "typedef struct {...} foo;" => "typedef struct foo {...} foo;"
            ## (to be emitted in the node loop below, and suppressed in the final toks.append)
            #self.tagged.tag = Tag( self.name ) # this is how pyrex does it: tag.name == self.name
        # XX that doesn't work (the resulting c fails to compile) XX

        self._pyxstr( toks, indent, cprefix, use_cdef, shadow_name, **kw )

        #print self.deepstr()
        if self.name and not names.has_key( self.name ):
            names[ self.name ] = self
        if not (self.tagged and self.name == self.tagged.tag.name):
            comment = ""
            if self.name in python_kws:
                comment = "#"
                #if cprefix:
                #  self.name = '%s%s "%s" ' % ( cprefix, self.name, self.name ) # XX pyrex can't do this
            if cprefix: # shadow_name=True
                # My c-name gets this prefix. See also TypeAlias.pyxstr(): it also prepends the cprefix.
                self.name = '%s%s "%s" ' % ( cprefix, self.name, self.name )
            toks.append( '    '*indent + comment + 'ctypedef ' + Node.pyxstr(self,indent=indent, cprefix=cprefix, **kw).strip() )
        return ' \n'.join(toks)


class AbstractDeclarator(Declarator):
    """ used in Function; may lack an identifier """
    def pyxstr(self,toks=None,indent=0,**kw):
        if self.name in python_kws:
            # Would be better to do this in __init__, but our subclass doesn't call our __init__.
            self.name = '_' + self.name
        #return '    '*indent + Node.pyxstr(self,toks,indent, **kw).strip()
        return Node.pyxstr(self,toks,indent, **kw).strip()


class FieldLength(object):
    """
    """
    def pyxstr(self,toks,indent,**kw):
        pass


class StructDeclarator(Declarator): # also used in Union
    """
    """
    def pyxstr(self,toks=None,indent=0,**kw):
        comment = ""
        if self.name in python_kws:
            comment = "#"
        return '    '*indent + comment + Node.pyxstr(self,toks,indent, **kw).strip()

class DeclarationSpecifiers(object):
    """
    """
    pass

class TypeSpecifiers(DeclarationSpecifiers):
    """
    """
    pass

class Initializer(object):
    """
    """
    pass

class Declaration(object):
    """
    """
    pass

class ParameterDeclaration(Declaration):
    """
    """
    pass

class StructDeclaration(Declaration):
    """
    """
    pass

class TransUnit(object):
    """
        Top level node.
    """
    def pyx_decls(self, filenames, modname, macros = {}, names = {}, func_cb=None, cprefix="", **kw):
        # PART 1: emit extern declarations
        ostream = OStream()
        now = datetime.today()
        ostream.putln( now.strftime('# Code generated by pyxelator on %x at %X') + '\n' )
        ostream.putln("# PART 1: extern declarations")
        for filename in filenames:
            ostream.putln( 'cdef extern from "%s":\n    pass\n' % filename )
        ostream.putln( 'cdef extern from *:' )
        file = None # current file
        for node in self:
            ostream.putln('')
            ostream.putln('    # ' + node.cstr() )
            assert node.marked
            comment = False
            if node.name and node.name in names:
                comment = True # redeclaration
            #ostream.putln( node.deepstr( comment=True ) )
            s = node.pyxstr(indent=1, names=names, tag_lookup = self.tag_lookup, cprefix=cprefix, **kw)
            if s.split():
                if comment:
                    s = "#"+s.replace( '\n', '\n#' ) + " # redeclaration "
                if node.file != file:
                    file = node.file
                    #ostream.putln( 'cdef extern from "%s":' % file )
                    ostream.putln( '    # "%s"' % file )
                ostream.putln( s )
        ostream.putln('\n')
        #s = '\n'.join(toks)
        return ostream.join()

# XX warn when we find a python keyword XX
python_kws = """
break continue del def except exec finally pass print raise
return try global assert lambda yield
for while if elif else and in is not or import from """.split()
python_kws = dict( zip( python_kws, (None,)*len(python_kws) ) )


