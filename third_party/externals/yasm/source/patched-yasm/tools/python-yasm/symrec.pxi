# Python bindings for Yasm: Pyrex input file for symrec.h
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

cdef class Symbol:
    cdef yasm_symrec *sym

    def __cinit__(self, symrec):
        self.sym = NULL
        if PyCObject_Check(symrec):
            self.sym = <yasm_symrec *>__get_voidp(symrec, Symbol)
        else:
            raise NotImplementedError

    # no deref or destroy necessary

    property name:
        def __get__(self): return yasm_symrec_get_name(self.sym)

    property status:
        def __get__(self):
            cdef yasm_sym_status status
            s = set()
            status = yasm_symrec_get_status(self.sym)
            if <int>status & <int>YASM_SYM_USED: s.add('used')
            if <int>status & <int>YASM_SYM_DEFINED: s.add('defined')
            if <int>status & <int>YASM_SYM_VALUED: s.add('valued')
            return s

    property in_table:
        def __get__(self):
            return bool(<int>yasm_symrec_get_status(self.sym) &
                        <int>YASM_SYM_NOTINTABLE)

    property visibility:
        def __get__(self):
            cdef yasm_sym_vis vis
            s = set()
            vis = yasm_symrec_get_visibility(self.sym)
            if <int>vis & <int>YASM_SYM_GLOBAL: s.add('global')
            if <int>vis & <int>YASM_SYM_COMMON: s.add('common')
            if <int>vis & <int>YASM_SYM_EXTERN: s.add('extern')
            if <int>vis & <int>YASM_SYM_DLOCAL: s.add('dlocal')
            return s

    property equ:
        def __get__(self):
            cdef yasm_expr *e
            e = yasm_symrec_get_equ(self.sym)
            if not e:
                raise AttributeError("not an EQU")
            return __make_expression(yasm_expr_copy(e))

    property label:
        def __get__(self):
            cdef yasm_symrec_get_label_bytecodep bc
            if yasm_symrec_get_label(self.sym, &bc):
                return None #Bytecode(bc)
            else:
                raise AttributeError("not a label or not defined")

    property is_special:
        def __get__(self): return bool(yasm_symrec_is_special(self.sym))

    property is_curpos:
        def __get__(self): return bool(yasm_symrec_is_curpos(self.sym))

    def get_data(self): pass # TODO
        #return <object>(yasm_symrec_get_data(self.sym, PyYasmAssocData))

    def set_data(self, data): pass # TODO
        #yasm_symrec_set_data(self.sym, PyYasmAssocData, data)

#
# Use associated data mechanism to keep Symbol reference paired with symrec.
#

cdef void __python_symrec_cb_destroy(void *data):
    Py_DECREF(<object>data)
cdef void __python_symrec_cb_print(void *data, FILE *f, int indent_level):
    pass
__python_symrec_cb = __assoc_data_callback(
        PyCObject_FromVoidPtr(&__python_symrec_cb_destroy, NULL),
        PyCObject_FromVoidPtr(&__python_symrec_cb_print, NULL))

cdef object __make_symbol(yasm_symrec *symrec):
    cdef void *data
    __error_check()
    data = yasm_symrec_get_data(symrec,
                                (<__assoc_data_callback>__python_symrec_cb).cb)
    if data != NULL:
        return <object>data
    symbol = Symbol(__pass_voidp(symrec, Symbol))
    yasm_symrec_add_data(symrec,
                         (<__assoc_data_callback>__python_symrec_cb).cb,
                         <void *>symbol)
    Py_INCREF(symbol)       # We're keeping a reference on the C side!
    return symbol

cdef class Bytecode
cdef class SymbolTable

cdef class SymbolTableKeyIterator:
    cdef yasm_symtab_iter *iter

    def __cinit__(self, symtab):
        if not isinstance(symtab, SymbolTable):
            raise TypeError
        self.iter = yasm_symtab_first((<SymbolTable>symtab).symtab)

    def __iter__(self):
        return self

    def __next__(self):
        if self.iter == NULL:
            raise StopIteration
        rv = yasm_symrec_get_name(yasm_symtab_iter_value(self.iter))
        self.iter = yasm_symtab_next(self.iter)
        return rv

cdef class SymbolTableValueIterator:
    cdef yasm_symtab_iter *iter

    def __cinit__(self, symtab):
        if not isinstance(symtab, SymbolTable):
            raise TypeError
        self.iter = yasm_symtab_first((<SymbolTable>symtab).symtab)

    def __iter__(self):
        return self

    def __next__(self):
        if self.iter == NULL:
            raise StopIteration
        rv = __make_symbol(yasm_symtab_iter_value(self.iter))
        self.iter = yasm_symtab_next(self.iter)
        return rv

cdef class SymbolTableItemIterator:
    cdef yasm_symtab_iter *iter

    def __cinit__(self, symtab):
        if not isinstance(symtab, SymbolTable):
            raise TypeError
        self.iter = yasm_symtab_first((<SymbolTable>symtab).symtab)

    def __iter__(self):
        return self

    def __next__(self):
        cdef yasm_symrec *sym
        if self.iter == NULL:
            raise StopIteration
        sym = yasm_symtab_iter_value(self.iter)
        rv = (yasm_symrec_get_name(sym), __make_symbol(sym))
        self.iter = yasm_symtab_next(self.iter)
        return rv

cdef int __parse_vis(vis) except -1:
    if not vis or vis == 'local': return YASM_SYM_LOCAL
    if vis == 'global': return YASM_SYM_GLOBAL
    if vis == 'common': return YASM_SYM_COMMON
    if vis == 'extern': return YASM_SYM_EXTERN
    if vis == 'dlocal': return YASM_SYM_DLOCAL
    msg = "bad visibility value %r" % vis
    PyErr_SetString(ValueError, msg)
    return -1

cdef class SymbolTable:
    cdef yasm_symtab *symtab

    def __cinit__(self):
        self.symtab = yasm_symtab_create()

    def __dealloc__(self):
        if self.symtab != NULL: yasm_symtab_destroy(self.symtab)

    def use(self, name, line):
        return __make_symbol(yasm_symtab_use(self.symtab, name, line))

    def define_equ(self, name, expr, line):
        if not isinstance(expr, Expression):
            raise TypeError
        return __make_symbol(yasm_symtab_define_equ(self.symtab, name,
                yasm_expr_copy((<Expression>expr).expr), line))

    def define_label(self, name, precbc, in_table, line):
        if not isinstance(precbc, Bytecode):
            raise TypeError
        return __make_symbol(yasm_symtab_define_label(self.symtab, name,
                (<Bytecode>precbc).bc, in_table, line))

    def define_special(self, name, vis):
        return __make_symbol(
            yasm_symtab_define_special(self.symtab, name,
                                       <yasm_sym_vis>__parse_vis(vis)))

    def declare(self, name, vis, line):
        return __make_symbol(
            yasm_symtab_declare(self.symtab, name,
                                <yasm_sym_vis>__parse_vis(vis), line))

    #
    # Methods to make SymbolTable behave like a dictionary of Symbols.
    #

    def __getitem__(self, key):
        cdef yasm_symrec *symrec
        symrec = yasm_symtab_get(self.symtab, key)
        if symrec == NULL:
            raise KeyError
        return __make_symbol(symrec)

    def __contains__(self, key):
        cdef yasm_symrec *symrec
        symrec = yasm_symtab_get(self.symtab, key)
        return symrec != NULL

    def keys(self):
        cdef yasm_symtab_iter *iter
        l = []
        iter = yasm_symtab_first(self.symtab)
        while iter != NULL:
            l.append(yasm_symrec_get_name(yasm_symtab_iter_value(iter)))
            iter = yasm_symtab_next(iter)
        return l

    def values(self):
        cdef yasm_symtab_iter *iter
        l = []
        iter = yasm_symtab_first(self.symtab)
        while iter != NULL:
            l.append(__make_symbol(yasm_symtab_iter_value(iter)))
            iter = yasm_symtab_next(iter)
        return l

    def items(self):
        cdef yasm_symtab_iter *iter
        cdef yasm_symrec *sym
        l = []
        iter = yasm_symtab_first(self.symtab)
        while iter != NULL:
            sym = yasm_symtab_iter_value(iter)
            l.append((yasm_symrec_get_name(sym), __make_symbol(sym)))
            iter = yasm_symtab_next(iter)
        return l

    def has_key(self, key):
        cdef yasm_symrec *symrec
        symrec = yasm_symtab_get(self.symtab, key)
        return symrec != NULL

    def get(self, key, x):
        cdef yasm_symrec *symrec
        symrec = yasm_symtab_get(self.symtab, key)
        if symrec == NULL:
            return x
        return __make_symbol(symrec)

    def iterkeys(self): return SymbolTableKeyIterator(self)
    def itervalues(self): return SymbolTableValueIterator(self)
    def iteritems(self): return SymbolTableItemIterator(self)
    def __iter__(self): return SymbolTableKeyIterator(self)

