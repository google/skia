# Python bindings for Yasm: Pyrex input file for floatnum.h
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

cdef class FloatNum:
    cdef yasm_floatnum *flt
    def __cinit__(self, value):
        self.flt = NULL
        if isinstance(value, FloatNum):
            self.flt = yasm_floatnum_copy((<FloatNum>value).flt)
            return
        if PyCObject_Check(value):  # should check Desc
            self.flt = <yasm_floatnum *>PyCObject_AsVoidPtr(value)
            return

        if isinstance(value, float): string = str(float)
        else: string = value
        self.flt = yasm_floatnum_create(string)

    def __dealloc__(self):
        if self.flt != NULL: yasm_floatnum_destroy(self.flt)

    def __neg__(self):
        result = FloatNum(self)
        yasm_floatnum_calc((<FloatNum>result).flt, YASM_EXPR_NEG, NULL)
        return result
    def __pos__(self): return self

