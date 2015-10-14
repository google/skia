#!/usr/bin/env python
""" cdecl.py - parse c declarations

(c) 2002, 2003, 2004, 2005 Simon Burton <simon@arrowtheory.com>
Released under GNU LGPL license.

version 0.xx

"""

import string


class Node(list):
    " A node in a parse tree "

    def __init__(self,*items,**kw):
        list.__init__( self, items )
        self.lock1 = 0 # these two should be properties (simplifies serializing)
        self.lock2 = 0
        self.verbose = 0
        for key in kw.keys():
            self.__dict__[key] = kw[key]

    def __str__(self):
        attrs = []
        for item in self:
            if isinstance(item,Node):
                attrs.append( str(item) )
            else:
                attrs.append( repr(item) )
        attrs = ','.join(attrs)
        return "%s(%s)"%(self.__class__.__name__,attrs)

    def safe_repr( self, tank ):
        tank[ str(self) ] = None
        attrs = []
        for item in self:
            if isinstance(item,Node):
                attrs.append( item.safe_repr(tank) ) # can we use repr here ?
            else:
                attrs.append( repr(item) )
        # this is the dangerous bit:
        for key, val in self.__dict__.items():
            if isinstance(val,Node):
                if str(val) not in tank:
                    attrs.append( '%s=%s'%(key,val.safe_repr(tank)) )
            else:
                attrs.append( '%s=%s'%(key,repr(val)) )
        attrs = ','.join(attrs)
        return "%s(%s)"%(self.__class__.__name__,attrs)

    def __repr__(self):
        #attrs = ','.join( [repr(item) for item in self] + \
        # [ '%s=%s'%(key,repr(val)) for key,val in self.__dict__.items() ] )
        #return "%s%s"%(self.__class__.__name__,tuple(attrs))
        return self.safe_repr({})

    def __eq__(self,other):
        if not isinstance(other,Node):
            return 0
        if len(self)!=len(other):
            return 0
        for i in range(len(self)):
            if not self[i]==other[i]:
                return 0
        return 1

    def __ne__(self,other):
        return not self==other

    def filter(self,cls):
        return [x for x in self if isinstance(x,cls)]
        #return filter( lambda x:isinstance(x,cls), self )

    def deepfilter(self,cls):
        " bottom-up "
        return [x for x in self.nodes() if isinstance(x,cls)]

    def find(self,cls):
        for x in self:
            if isinstance(x,cls):
                return x
        return None

    def deepfind(self,cls):
        " bottom-up isinstance search "
        for x in self:
            if isinstance(x,Node):
                if isinstance(x,cls):
                    return x
                node = x.deepfind(cls)
                if node is not None:
                    return node
        if isinstance(self,cls):
            return self
        return None

    def leaves(self):
        for i in self:
            if isinstance( i, Node ):
                for j in i.leaves():
                    yield j
            else:
                yield i

    def nodes(self):
        " bottom-up iteration "
        for i in self:
            if isinstance( i, Node ):
                for j in i.nodes():
                    yield j
        yield self

    def deeplen(self):
        i=0
        if not self.lock2:
            self.lock2=1
            for item in self:
                i+=1
                if isinstance(item,Node):
                    i+=item.deeplen()
            self.lock2=0
        else:
            i+=1
        return i

    def deepstr(self,level=0,comment=False,nl='\n',indent='    '):
        if self.deeplen() < 4:
            nl = ""; indent = ""
        #else:
            #nl="\n"; indent = "    "
        s = []
        if not self.lock1:
            self.lock1=1
            for item in self:
                if isinstance(item,Node):
                    s.append( indent*(level+1)+item.deepstr(level+1,False,nl,indent) )
                else:
                    s.append( indent*(level+1)+repr(item) )
            self.lock1=0
        else:
            for item in self:
                if isinstance(item,Node):
                    s.append( indent*(level+1)+"<recursion...>" )
                else:
                    s.append( indent*(level+1)+"%s"%repr(item) )
        s = "%s(%s)"%(self.__class__.__name__,nl+string.join(s,","+nl))
        if comment:
            s = '#' + s.replace('\n','\n#')
        return s

    def clone(self):
        items = []
        for item in self:
            if isinstance(item,Node):
                item = item.clone()
            items.append(item)
        # we skip any attributes...
        return self.__class__(*items)

    def fastclone(self):
        # XX is it faster ???
        #print "clone"
        nodes = [self]
        idxs = [0]
        itemss = [ [] ]
        while nodes:
            assert len(nodes)==len(idxs)==len(itemss)
            node = nodes[-1]
            items = itemss[-1]
            assert idxs[-1] == len(items)
            while idxs[-1]==len(node):
                # pop
                _node = node.__class__( *items )
                _node.__dict__.update( node.__dict__ )
                nodes.pop(-1)
                idxs.pop(-1)
                itemss.pop(-1)
                if not nodes:
                    #for node0 in self.nodes():
                        #for node1 in _node.nodes():
                            #assert node0 is not node1
                    #assert _node == self
                    return _node # Done !!
                node = nodes[-1]
                items = itemss[-1]
                items.append(_node) # set
                idxs[-1] += 1
                assert idxs[-1] == len(items)
                #assert idxs[-1] < len(node), str( (node,nodes,idxs,itemss) )

            _node = node[ idxs[-1] ]
            # while idxs[-1]<len(node): 
            if isinstance(_node,Node):
                # push
                nodes.append( _node )
                idxs.append( 0 )
                itemss.append( [] )
            else:
                # next
                items.append(_node)
                idxs[-1] += 1
                assert idxs[-1] == len(items)

    def expose(self,cls):
        ' expose children of any <cls> instance '
        # children first
        for x in self:
            if isinstance(x,Node):
                x.expose(cls)
        # now the tricky bit
        i=0
        while i < len(self):
            if isinstance(self[i],cls):
                node=self.pop(i)
                for x in node:
                    assert not isinstance(x,cls)
                    # pass on some attributes
                    if hasattr(node,'lines') and not hasattr(x,'lines'):
                        x.lines=node.lines
                    if hasattr(node,'file') and not hasattr(x,'file'):
                        x.file=node.file
                    self.insert(i,x) # expose
                    i=i+1
                    assert i<=len(self)
            else:
                i=i+1

    def get_parent( self, item ): # XX 25% CPU time here XX
        assert self != item
        if item in self:
            return self
        for child in self:
            if isinstance(child, Node):
                parent = child.get_parent(item)
                if parent is not None:
                    return parent
        return None

    def expose_node( self, item ):
        assert self != item
        parent = self.get_parent(item)
        idx = parent.index( item )
        parent[idx:idx+1] = item[:]

    def delete(self,cls):
        ' delete any <cls> subtree '
        for x in self:
            if isinstance(x,Node):
                x.delete(cls)
        # now the tricky bit
        i=0
        while i < len(self):
            if isinstance(self[i],cls):
                self.pop(i)
            else:
                i=i+1

    def deeprm(self,item):
        ' remove any items matching <item> '
        for x in self:
            if isinstance(x,Node):
                x.deeprm(item)
        # now the tricky bit
        i=0
        while i < len(self):
            if self[i] == item:
                self.pop(i)
            else:
                i=i+1

    def idem(self,cls):
        " <cls> is made idempotent "
        # children first
        for x in self:
            if isinstance(x,Node):
                x.idem(cls)
        if isinstance(self,cls):
            # now the tricky bit
            i=0
            while i < len(self):
                if isinstance(self[i],cls):
                    node = self.pop(i)
                    for x in node:
                        assert not isinstance(x,cls)
                        self.insert(i,x) # idempotent
                        i=i+1
                        assert i<=len(self)
                else:
                    i=i+1

if __name__=="__main__":
    node = Node( 'a', Node(1,2), Node(Node(Node(),1)) )

    print node
    print node.clone()




