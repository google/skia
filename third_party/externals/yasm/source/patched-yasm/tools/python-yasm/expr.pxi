# Python bindings for Yasm: Pyrex input file for expr.h
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

cdef extern from *:
    # Defined as a macro, so not automatically brought in by pyxelator
    cdef yasm_expr *yasm_expr_simplify(yasm_expr *e, int calc_bc_dist)

import operator
__op = {}
for ops, operation in [
    ((operator.__add__, operator.add, '+'), YASM_EXPR_ADD),
    ((operator.__and__, operator.and_, '&'), YASM_EXPR_AND),
    ((operator.__div__, operator.div, '/'), YASM_EXPR_SIGNDIV),
    ((operator.__floordiv__, operator.floordiv, '//'), YASM_EXPR_SIGNDIV),
    ((operator.__ge__, operator.ge, '>='), YASM_EXPR_GE),
    ((operator.__gt__, operator.gt, '>'), YASM_EXPR_GT),
    ((operator.__inv__, operator.inv, '~'), YASM_EXPR_NOT),
    ((operator.__invert__, operator.invert), YASM_EXPR_NOT),
    ((operator.__le__, operator.le, '<='), YASM_EXPR_LE),
    ((operator.__lt__, operator.lt, '<'), YASM_EXPR_LT),
    ((operator.__mod__, operator.mod, '%'), YASM_EXPR_SIGNMOD),
    ((operator.__mul__, operator.mul, '*'), YASM_EXPR_MUL),
    ((operator.__neg__, operator.neg), YASM_EXPR_NEG),
    ((operator.__not__, operator.not_, 'not'), YASM_EXPR_LNOT),
    ((operator.__or__, operator.or_, '|'), YASM_EXPR_OR),
    ((operator.__sub__, operator.sub, '-'), YASM_EXPR_SUB),
    ((operator.__xor__, operator.xor, '^'), YASM_EXPR_XOR),
    ]:
    for op in ops:
        __op[op] = operation

del operator, op, ops, operation

cdef object __make_expression(yasm_expr *expr):
    return Expression(__pass_voidp(expr, Expression))

cdef class Expression:
    cdef yasm_expr *expr

    def __cinit__(self, op, *args, **kwargs):
        self.expr = NULL

        if isinstance(op, Expression):
            self.expr = yasm_expr_copy((<Expression>op).expr)
            return
        if PyCObject_Check(op):
            self.expr = <yasm_expr *>__get_voidp(op, Expression)
            return

        cdef size_t numargs
        cdef unsigned long line

        op = __op.get(op, op)
        numargs = len(args)
        line = kwargs.get('line', 0)

        if numargs == 0 or numargs > 2:
            raise NotImplementedError
        elif numargs == 2:
            self.expr = yasm_expr_create(op, self.__new_item(args[0]),
                                         self.__new_item(args[1]), line)
        else:
            self.expr = yasm_expr_create(op, self.__new_item(args[0]), NULL,
                                         line)

    cdef yasm_expr__item* __new_item(self, value) except NULL:
        cdef yasm_expr__item *retval
        if isinstance(value, Expression):
            return yasm_expr_expr(yasm_expr_copy((<Expression>value).expr))
        #elif isinstance(value, Symbol):
        #    return yasm_expr_sym((<Symbol>value).sym)
        #elif isinstance(value, Register):
        #    return yasm_expr_reg((<Register>value).reg)
        elif isinstance(value, FloatNum):
            return yasm_expr_float(yasm_floatnum_copy((<FloatNum>value).flt))
        elif isinstance(value, IntNum):
            return yasm_expr_int(yasm_intnum_copy((<IntNum>value).intn))
        else:
            try:
                intnum = IntNum(value)
            except:
                raise ValueError("Invalid item value type '%s'" % type(value))
            else:
                retval = yasm_expr_int((<IntNum>intnum).intn)
                (<IntNum>intnum).intn = NULL
                return retval

    def __dealloc__(self):
        if self.expr != NULL: yasm_expr_destroy(self.expr)

    def simplify(self, calc_bc_dist=False):
        self.expr = yasm_expr_simplify(self.expr, calc_bc_dist)

    def extract_segoff(self):
        cdef yasm_expr *retval
        retval = yasm_expr_extract_segoff(&self.expr)
        if retval == NULL:
            raise ValueError("not a SEG:OFF expression")
        return __make_expression(retval)

    def extract_wrt(self):
        cdef yasm_expr *retval
        retval = yasm_expr_extract_wrt(&self.expr)
        if retval == NULL:
            raise ValueError("not a WRT expression")
        return __make_expression(retval)

    def get_intnum(self, calc_bc_dist=False):
        cdef yasm_intnum *retval
        retval = yasm_expr_get_intnum(&self.expr, calc_bc_dist)
        if retval == NULL:
            raise ValueError("not an intnum expression")
        return __make_intnum(yasm_intnum_copy(retval))

