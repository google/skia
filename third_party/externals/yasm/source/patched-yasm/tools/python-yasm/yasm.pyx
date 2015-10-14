# Python bindings for Yasm: Main Pyrex input file
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
"""Interface to the Yasm library.

The Yasm library (aka libyasm) provides the core functionality of the Yasm
assembler.  Classes in this library provide for manipulation of machine
instructions and object file constructs such as symbol tables and sections.

Expression objects encapsulate complex expressions containing registers,
symbols, and operations such as SEG.

Bytecode objects encapsulate data or code objects such as data, reserve,
align, or instructions.

Section objects encapsulate an object file section, including the section
name, any Bytecode objects contained within that section, and other
information.

"""

cdef extern from "Python.h":
    cdef object PyCObject_FromVoidPtr(void *cobj, void (*destr)(void *))
    cdef object PyCObject_FromVoidPtrAndDesc(void *cobj, void *desc,
                                             void (*destr)(void *, void *))
    cdef int PyType_Check(object)
    cdef int PyCObject_Check(object)
    cdef void *PyCObject_AsVoidPtr(object)
    cdef void *PyCObject_GetDesc(object)

    cdef object _PyLong_FromByteArray(unsigned char *bytes, unsigned int n,
                                      int little_endian, int is_signed)
    cdef int _PyLong_AsByteArray(object v, unsigned char *bytes, unsigned int n,
                                 int little_endian, int is_signed) except -1

    cdef void Py_INCREF(object o)
    cdef void Py_DECREF(object o)

    cdef void PyErr_SetString(object type, char *message)
    cdef object PyErr_Format(object type, char *format, ...)

cdef extern from "stdlib.h":
    cdef void *malloc(int n)
    cdef void free(void *p)

include "_yasm.pxi"

cdef object __pass_voidp(void *obj, object forclass):
    return PyCObject_FromVoidPtrAndDesc(obj, <void *>forclass, NULL)

cdef void *__get_voidp(object obj, object forclass) except NULL:
    cdef void* desc

    if not PyCObject_Check(obj):
        msg = "obj %r is not a CObject" % obj
        PyErr_SetString(TypeError, msg)
        return NULL

    desc = PyCObject_GetDesc(obj)

    if desc != <void *>forclass:
        if desc == NULL:
            msg = "CObject type is not set (expecting %s)" % forclass
        elif PyType_Check(<object>desc):
            msg = "CObject is for %s not %s" % (<object>desc, forclass)
        else:
            msg = "CObject is incorrect (expecting %s)" % forclass
        PyErr_SetString(TypeError, msg)
        return NULL

    return PyCObject_AsVoidPtr(obj)

#
# Link to associated data mechanism to keep Python references paired with
# yasm objects.
#
cdef class __assoc_data_callback:
    cdef yasm_assoc_data_callback *cb
    def __cinit__(self, destroy, print_):
        self.cb = <yasm_assoc_data_callback *>malloc(sizeof(yasm_assoc_data_callback))
        self.cb.destroy = <void (*) (void *)>PyCObject_AsVoidPtr(destroy)
        #self.cb.print_ = <void (*) (void *, FILE *, int)>PyCObject_AsVoidPtr(print_)
    def __dealloc__(self):
        free(self.cb)


cdef class Register:
    cdef unsigned long reg
    def __cinit__(self, reg):
        self.reg = reg

include "errwarn.pxi"
include "intnum.pxi"
include "floatnum.pxi"
include "expr.pxi"
include "symrec.pxi"
include "value.pxi"

include "bytecode.pxi"

cdef __initialize():
    BitVector_Boot()
    yasm_intnum_initialize()
    yasm_floatnum_initialize()
    yasm_errwarn_initialize()

def __cleanup():
    yasm_floatnum_cleanup()
    yasm_intnum_cleanup()
    yasm_errwarn_cleanup()
    BitVector_Shutdown()

__initialize()
import atexit
atexit.register(__cleanup)

