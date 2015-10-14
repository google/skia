# Python bindings for Yasm: Pyrex input file for value.h
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

cdef class Value:
    cdef yasm_value value
    def __cinit__(self, value=None, size=None):
        cdef unsigned int sz
        if size is None:
            sz = 0
        else:
            sz = size;

        yasm_value_initialize(&self.value, NULL, sz)
        if value is None:
            pass
        elif isinstance(value, Expression):
            yasm_value_initialize(&self.value,
                                  yasm_expr_copy((<Expression>value).expr), sz)
        elif isinstance(value, Symbol):
            yasm_value_init_sym(&self.value, (<Symbol>value).sym, sz)
        else:
            raise TypeError("Invalid value type '%s'" % type(value))

    def __dealloc__(self):
        yasm_value_delete(&self.value)

    def finalize(self, precbc=None):
        if precbc is None:
            return yasm_value_finalize(&self.value, NULL)
        elif isinstance(precbc, Bytecode):
            return yasm_value_finalize(&self.value, (<Bytecode>precbc).bc)
        else:
            raise TypeError("Invalid precbc type '%s'" % type(precbc))

