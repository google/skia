# Python bindings for Yasm: Pyrex input file for bytecode.h
#
#  Copyright (C) 2006  Michael Urman, Peter Johnson
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cdef class Bytecode:
    cdef yasm_bytecode *bc

    cdef object __weakref__     # make weak-referenceable

    def __cinit__(self, bc):
        self.bc = NULL
        if PyCObject_Check(bc):
            self.bc = <yasm_bytecode *>__get_voidp(bc, Bytecode)
        else:
            raise NotImplementedError

    def __dealloc__(self):
        # Only free if we're not part of a section; if we're part of a section
        # the section takes care of freeing the bytecodes.
        if self.bc.section == NULL:
            yasm_bc_destroy(self.bc)

    property len:
        def __get__(self): return self.bc.len
        def __set__(self, value): self.bc.len = value
    property mult_int:
        def __get__(self): return self.bc.mult_int
        def __set__(self, value): self.bc.mult_int = value
    property line:
        def __get__(self): return self.bc.line
        def __set__(self, value): self.bc.line = value
    property offset:
        def __get__(self): return self.bc.offset
        def __set__(self, value): self.bc.offset = value
    property bc_index:
        def __get__(self): return self.bc.bc_index
        def __set__(self, value): self.bc.bc_index = value
    property symbols:
        # Someday extend this to do something modifiable, e.g. return a
        # list-like object.
        def __get__(self):
            cdef yasm_symrec *sym
            cdef int i
            if self.bc.symrecs == NULL:
                return []
            s = []
            i = 0
            sym = self.bc.symrecs[i]
            while sym != NULL:
                s.append(__make_symbol(sym))
                i = i+1
                sym = self.bc.symrecs[i]
            return s

#
# Keep Bytecode reference paired with bc using weak references.
# This is broken in Pyrex 0.9.4.1; Pyrex 0.9.5 has a working version.
#

from weakref import WeakValueDictionary as __weakvaldict
__bytecode_map = __weakvaldict()
#__bytecode_map = {}

cdef object __make_bytecode(yasm_bytecode *bc):
    __error_check()
    vptr = PyCObject_FromVoidPtr(bc, NULL)
    data = __bytecode_map.get(vptr, None)
    if data:
        return data
    bcobj = Bytecode(__pass_voidp(bc, Bytecode))
    __bytecode_map[vptr] = bcobj
    return bcobj

# Org bytecode
def __org__new__(cls, start, value=0, line=0):
    cdef yasm_bytecode *bc
    bc = yasm_bc_create_org(start, line, value)
    obj = Bytecode.__new__(cls, __pass_voidp(bc, Bytecode))
    __bytecode_map[PyCObject_FromVoidPtr(bc, NULL)] = obj
    return obj
__org__new__ = staticmethod(__org__new__)
class Org(Bytecode):
    __new__ = __org__new__


#cdef class Section:
