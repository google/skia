# Python bindings for Yasm: Pyrex input file for errwarn.h
#
#  Copyright (C) 2006  Peter Johnson
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

class YasmError(Exception): pass

cdef int __error_check() except 1:
    cdef yasm_error_class errclass
    cdef unsigned long xrefline
    cdef char *errstr, *xrefstr

    # short path for the common case
    if not <int>yasm_error_occurred():
        return 0

    # look up our preferred python error, fall back to YasmError
    # Order matters here. Go from most to least specific within a class
    if yasm_error_matches(YASM_ERROR_ZERO_DIVISION):
        exception = ZeroDivisionError
    # Enable these once there are tests that need them.
    #elif yasm_error_matches(YASM_ERROR_OVERFLOW):
    #   exception = OverflowError
    #elif yasm_error_matches(YASM_ERROR_FLOATING_POINT):
    #   exception = FloatingPointError
    #elif yasm_error_matches(YASM_ERROR_ARITHMETIC):
    #   exception = ArithmeticError
    #elif yasm_error_matches(YASM_ERROR_ASSERTION):
    #   exception = AssertionError
    #elif yasm_error_matches(YASM_ERROR_VALUE):
    #   exception = ValueError # include notabs, notconst, toocomplex
    #elif yasm_error_matches(YASM_ERROR_IO):
    #   exception = IOError
    #elif yasm_error_matches(YASM_ERROR_NOT_IMPLEMENTED):
    #   exception = NotImplementedError
    #elif yasm_error_matches(YASM_ERROR_TYPE):
    #   exception = TypeError
    #elif yasm_error_matches(YASM_ERROR_SYNTAX):
    #   exception = SyntaxError #include parse
    else:
        exception = YasmError

    # retrieve info (clears error)
    yasm_error_fetch(&errclass, &errstr, &xrefline, &xrefstr)

    if xrefline and xrefstr:
        PyErr_Format(exception, "%s: %d: %s", errstr, xrefline, xrefstr)
    else:
        PyErr_SetString(exception, errstr)

    if xrefstr: free(xrefstr)
    free(errstr)
    return 1
